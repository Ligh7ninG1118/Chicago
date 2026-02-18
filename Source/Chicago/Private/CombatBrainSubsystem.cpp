// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatBrainSubsystem.h"

#include "Kismet/GameplayStatics.h"

void UCombatBrainSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	RegisteredCombatants.Empty();
	ActiveAttackTokenUsers.Empty();
	ActiveFlankTokenUsers.Empty();
	CombatantOccupiedSlice.Empty();
	
	CurrentSliceWeights.SetNumZeroed(SliceNum);

	// If not assigned in editor, default to this
	if (SliceWeightLUT.Num() <= 0)
	{
		SliceWeightLUT.Reset(4);
		SliceWeightLUT.Add(10);
		SliceWeightLUT.Add(7);
		SliceWeightLUT.Add(4);
		SliceWeightLUT.Add(1);
	}
}

void UCombatBrainSubsystem::Deinitialize()
{
	RegisteredCombatants.Empty();
	ActiveAttackTokenUsers.Empty();
	ActiveFlankTokenUsers.Empty();
	CombatantOccupiedSlice.Empty();
	
	Super::Deinitialize();
}

void UCombatBrainSubsystem::RegisterCombatant(AActor* AIAgent)
{
	if (!IsValid(AIAgent))
		return;

	if (!RegisteredCombatants.Contains(AIAgent))
	{
		RegisteredCombatants.Add(AIAgent);
		AIAgent->OnDestroyed.AddDynamic(this, &UCombatBrainSubsystem::OnCombatantDestroyed);
	}
}

void UCombatBrainSubsystem::UnregisterCombatant(AActor* AIAgent)
{
	if (!IsValid(AIAgent))
		return;

	ReturnAttackToken(AIAgent);
	ReturnFlankToken(AIAgent);
	ClearCombatantOccupiedSlice(AIAgent);

	if (RegisteredCombatants.Contains(AIAgent))
	{
		RegisteredCombatants.Remove(AIAgent);
		AIAgent->OnDestroyed.RemoveDynamic(this, &UCombatBrainSubsystem::OnCombatantDestroyed);
	}
}

void UCombatBrainSubsystem::OnCombatantDestroyed(AActor* DestroyedAgent)
{
	UnregisterCombatant(DestroyedAgent);
}

bool UCombatBrainSubsystem::TryRequestAttackToken(AActor* AIAgent, int32 TokenCost)
{
	if (!IsValid(AIAgent) || TokenCost <= 0)
		return false;

	// Reject request if already assigned attack token
	if (ActiveAttackTokenUsers.Contains(AIAgent))
		return false;

	// Not enough tokens
	if ((TokenCost + GetCurrentAttackTokenUsage()) > MaxAttackToken)
		return false;
	
	ActiveAttackTokenUsers.Add(AIAgent, TokenCost);
	return true;
}

void UCombatBrainSubsystem::ReturnAttackToken(AActor* AIAgent)
{
	if (ActiveAttackTokenUsers.Contains(AIAgent))
	{
		ActiveAttackTokenUsers.Remove(AIAgent);
	}
}

int32 UCombatBrainSubsystem::GetCurrentAttackTokenUsage() const
{
	int32 TotalUsage = 0;
	for (const auto& Pair : ActiveAttackTokenUsers)
		TotalUsage += Pair.Value;
	
	return TotalUsage;
}

bool UCombatBrainSubsystem::TryRequestFlankToken(AActor* AIAgent, int32 TokenCost)
{
	if (!IsValid(AIAgent) || TokenCost <= 0)
		return false;

	// Reject request if already assigned token
	if (ActiveFlankTokenUsers.Contains(AIAgent))
		return false;

	// Not enough tokens
	if ((TokenCost + GetCurrentFlankTokenUsage()) > MaxFlankToken)
		return false;
	
	ActiveFlankTokenUsers.Add(AIAgent, TokenCost);
	return true;
}

void UCombatBrainSubsystem::ReturnFlankToken(AActor* AIAgent)
{
	if (ActiveFlankTokenUsers.Contains(AIAgent))
	{
		ActiveFlankTokenUsers.Remove(AIAgent);
	}
}

int32 UCombatBrainSubsystem::GetCurrentFlankTokenUsage() const
{
	int32 TotalUsage = 0;
	for (const auto& Pair : ActiveFlankTokenUsers)
		TotalUsage += Pair.Value;
	
	return TotalUsage;
}

FVector UCombatBrainSubsystem::RequestCoverPosition(AActor* Combatant)
{
	if(!IsValid(Combatant))
		return FVector::ZeroVector;

	const FVector& CurrentPlayerPos = GetPlayerPosition();
	const FVector& CurrentAIPosition = Combatant->GetActorLocation();

	// If player's current position is some distance away from the cached position,
	// we need to reevaluate all slices' weight for a more accurate representation
	// Otherwise, we accept the small inaccuracy and reuse the previous result
	if (FVector::DistSquared(CurrentPlayerPos, CachedPlayerPosition) > MaxDistToReevaluateWeight * MaxDistToReevaluateWeight)
	{
		UpdateAllSlicesWeight();
	}

	int32 CurrentSliceIndex = GetSliceIndexFromWorldPosition(CurrentAIPosition);

	// Limit search range within [-SliceNum/4, +SliceNum/4] around self
	// Preventing combatant trying to find cover across the battlefield
	const int32 SearchRange = SliceNum / 4;
	TArray<int32> SortedIndices;
	SortedIndices.Reserve(2 * SearchRange + 1);
	
	for (int32 i = -SearchRange;i<=SearchRange;i++)
		SortedIndices.Add(WrapSliceIndex(CurrentSliceIndex + i));

	// Sort the slice index array with weight
	SortedIndices.Sort([this](const int32 A, const int32 B)
	{
		return CurrentSliceWeights[A] < CurrentSliceWeights[B];
	});

	// Randomly choose a slice index from the lowest three
	const int32 PickCount = FMath::Min(3, SortedIndices.Num());
	const int32 ChosenCandidate = FMath::RandRange(0, PickCount-1);
	const int32 NewSliceIndex = SortedIndices[ChosenCandidate];

	ChangeCombatantOccupiedSlice(Combatant, NewSliceIndex);

	return CalculateNextCoverPosition(NewSliceIndex, CurrentAIPosition);
}

FVector UCombatBrainSubsystem::CalculateNextCoverPosition(int32 SliceIndex, const FVector& CurrentSelfPosition) const
{
	const FVector PlayerPos = GetPlayerPosition();
	
	const float SliceWidthDeg = 360.0f / float(SliceNum);
	// 0 deg = +X axis
	const float SliceCenterAngleDeg = SliceWidthDeg * (float(SliceIndex) + 0.5f);
	const float Rad = FMath::DegreesToRadians(SliceCenterAngleDeg);
	const FVector SliceDir2D(FMath::Cos(Rad), FMath::Sin(Rad), 0.0f);

	// Next position should be a bit closer to the player than combatant's current one
	float DesiredDist = (CurrentSelfPosition - PlayerPos).Length();
	DesiredDist *= 0.9f;

	FVector FinalPos = PlayerPos + SliceDir2D * DesiredDist;
	FinalPos.Z = CurrentSelfPosition.Z;
	return FinalPos;
}

void UCombatBrainSubsystem::ModifySliceWeight(int32 SliceIndex, bool bIsAdding)
{
	const int32 Sign = bIsAdding ? 1 : -1;

	// Loop thru the Weight LUT, modifying weight changes to the slice and its neighbors
	for (int32 i=0;i<SliceWeightLUT.Num();i++)
	{
		const int32 Delta = Sign * SliceWeightLUT[i];

		auto UpdateWeight = [this, Delta](int32 Index)
		{
			if (!CurrentSliceWeights.IsValidIndex(Index))
				return;

			// Actual weight for any slice shouldn't be below zero
			CurrentSliceWeights[Index] = FMath::Max(0, CurrentSliceWeights[Index] + Delta);
		};

		// The center slice
		if (i == 0)
		{
			UpdateWeight(SliceIndex);
			continue;
		}

		// Neighbors
		UpdateWeight(WrapSliceIndex(SliceIndex + i));
		UpdateWeight(WrapSliceIndex(SliceIndex - i));
	}
}

void UCombatBrainSubsystem::UpdateAllSlicesWeight()
{
	CurrentSliceWeights.SetNumZeroed(SliceNum);
	
	for (const AActor* Combatant : RegisteredCombatants)
	{
		if (!IsValid(Combatant))
			continue;

		const FVector CombatantPos = Combatant->GetActorLocation();
		ModifySliceWeight(GetSliceIndexFromWorldPosition(CombatantPos), true);
	}

	// Current result is in relation to current player position, update it
	CachedPlayerPosition = GetPlayerPosition();
}

int32 UCombatBrainSubsystem::GetSliceIndexFromWorldPosition(const FVector& WorldPosition)
{
	// Vector from player to current combatant
	FVector Vpc = WorldPosition - GetPlayerPosition();
	Vpc.Z = 0.0f;
	Vpc = Vpc.GetSafeNormal();

	const float AngleDeg = FMath::Fmod(FMath::RadiansToDegrees(FMath::Atan2(Vpc.Y, Vpc.X)) + 360.0f, 360.0f);
	const float SliceWidthDeg = 360.0f / float(SliceNum);
	
	const int32 SliceIndex = FMath::Clamp(FMath::FloorToInt(AngleDeg / SliceWidthDeg),0, SliceNum-1);

	return SliceIndex;
}

void UCombatBrainSubsystem::ChangeCombatantOccupiedSlice(AActor* Combatant, int32 NewIndex)
{
	if (const int32* CurrentIndex = CombatantOccupiedSlice.Find(Combatant))
	{
		if (*CurrentIndex == NewIndex)
			return;

		ModifySliceWeight(*CurrentIndex, false);
	}

	ModifySliceWeight(NewIndex, true);
	CombatantOccupiedSlice.Add(Combatant, NewIndex);
}

void UCombatBrainSubsystem::ClearCombatantOccupiedSlice(AActor* Combatant)
{
	if (!IsValid(Combatant))
		return;

	if(int32* SliceIndex = CombatantOccupiedSlice.Find(Combatant))
	{
		ModifySliceWeight(*SliceIndex, false);
		CombatantOccupiedSlice.Remove(Combatant);
	}
}

FVector UCombatBrainSubsystem::GetPlayerPosition() const
{
	return UGameplayStatics::GetPlayerPawn(GetWorld(), 0)->GetActorLocation();
}

void UCombatBrainSubsystem::DebugDrawSlices()
{
	if (!bShouldDrawDebugInfo)
		return;

	const FVector PlayerPos = GetPlayerPosition();

	const float SliceWidthDeg = 360.0f / float(SliceNum);
	const float DebugRadius   = 5000.0f;
	const float LifeTime      = 10.0f;
	const float Thickness     = 2.0f;

	// Draw slice boundaries (0° = +X)
	for (int32 i = 0; i <= SliceNum; ++i)
	{
		const float BoundaryAngleDeg = SliceWidthDeg * float(i);
		const float Rad = FMath::DegreesToRadians(BoundaryAngleDeg);
		const FVector Dir(FMath::Cos(Rad), FMath::Sin(Rad), 0.0f);

		DrawDebugLine(
			GetWorld(),
			PlayerPos,
			PlayerPos + Dir * DebugRadius,
			FColor::Cyan,
			false,
			LifeTime,
			0,
			Thickness
		);
	}

	// Draw per-slice weight labels at slice centers
	for (int32 SliceIdx = 0; SliceIdx < SliceNum; ++SliceIdx)
	{
		const float CenterAngleDeg = SliceWidthDeg * (float(SliceIdx) + 0.5f);
		const float Rad = FMath::DegreesToRadians(CenterAngleDeg);
		const FVector Dir(FMath::Cos(Rad), FMath::Sin(Rad), 0.0f);

		const int32 W = CurrentSliceWeights.IsValidIndex(SliceIdx) ? CurrentSliceWeights[SliceIdx] : 0;

		DrawDebugString(
			GetWorld(),
			PlayerPos + Dir * DebugRadius,
			FString::Printf(TEXT("[%d] %d"), SliceIdx, W),
			nullptr,
			FColor::White,
			LifeTime,
			false
		);
	}
}

void UCombatBrainSubsystem::DebugPrintCombatantStates()
{
	if (!bShouldDrawDebugInfo)
		return;

	if (!GEngine)
		return;

	const float Duration = 5.0f;

	int32 Key = 91000;

	auto AddLine = [&](const FString& Msg, const FColor& Color = FColor::Green)
	{
		GEngine->AddOnScreenDebugMessage(Key++, Duration, Color, Msg);
	};

	AddLine(FString::Printf(TEXT("CombatBrain | Registered Combatants: %d"), RegisteredCombatants.Num()), FColor::Yellow);

	// Print names (one per line for readability)
	for (AActor* Combatant : RegisteredCombatants)
	{
		AddLine(FString::Printf(TEXT("  - %s"), *GetNameSafe(Combatant)), FColor::White);
	}

	if (SliceNum <= 0)
	{
		AddLine(TEXT("CombatBrain | SliceNum <= 0 (invalid config)"), FColor::Red);
		return;
	}

	// Build inverted view: slice index -> list of occupant names
	TArray<TArray<FString>> OccupantsPerSlice;
	OccupantsPerSlice.SetNum(SliceNum);

	for (const TPair<AActor*, int32>& Pair : CombatantOccupiedSlice)
	{
		AActor* Combatant = Pair.Key;
		if (!IsValid(Combatant))
			continue;

		const int32 RawSlice = Pair.Value;
		const int32 SliceIndex = WrapSliceIndex(RawSlice);

		if (OccupantsPerSlice.IsValidIndex(SliceIndex))
		{
			OccupantsPerSlice[SliceIndex].Add(GetNameSafe(Combatant));
		}
	}

	AddLine(TEXT("CombatBrain | Slices: [Index] Weight | Occupants"), FColor::Yellow);

	for (int32 SliceIdx = 0; SliceIdx < SliceNum; ++SliceIdx)
	{
		const int32 W = CurrentSliceWeights.IsValidIndex(SliceIdx) ? CurrentSliceWeights[SliceIdx] : 0;

		const FString OccStr = (OccupantsPerSlice[SliceIdx].Num() > 0)
			? FString::Join(OccupantsPerSlice[SliceIdx], TEXT(", "))
			: TEXT("None");

		AddLine(FString::Printf(TEXT("  [%02d] W=%d | %s"), SliceIdx, W, *OccStr), FColor::Cyan);
	}
}

int32 UCombatBrainSubsystem::WrapSliceIndex(int32 Index) const
{
	int32 Wrapped = Index % SliceNum;
	if (Wrapped < 0)
		Wrapped += SliceNum;
	return Wrapped;
}

int32 UCombatBrainSubsystem::GetSignedShortestSliceDelta(int32 From, int32 To) const
{
	From = WrapSliceIndex(From);
	To = WrapSliceIndex(To);

	int32 Delta = To - From;

	// Map into [-N/2, N/2] range.
	const int32 Half = SliceNum / 2;
	if (Delta > Half)
		Delta -= SliceNum;
	else if (Delta < -Half)
		Delta += SliceNum;

	return Delta;
}

