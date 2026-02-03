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
	ClearOccupiedSlice(AIAgent);

	if (RegisteredCombatants.Contains(AIAgent))
	{
		RegisteredCombatants.Remove(AIAgent);
		AIAgent->OnDestroyed.RemoveDynamic(this, &UCombatBrainSubsystem::OnCombatantDestroyed);
	}
	
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
	if (ActiveAttackTokenUsers.Contains(AIAgent))
	{
		ActiveAttackTokenUsers.Remove(AIAgent);
	}
}

int32 UCombatBrainSubsystem::GetCurrentFlankTokenUsage() const
{
	int32 TotalUsage = 0;
	for (const auto& Pair : ActiveAttackTokenUsers)
		TotalUsage += Pair.Value;
	
	return TotalUsage;
}

FVector UCombatBrainSubsystem::RequestCoverPosition(const FVector& CurrentSelfPosition)
{
	// Sort a slice index array with weight
	TArray<int32> SortedIndices;
	SortedIndices.Reserve(SliceNum);
	for (int32 i = 0;i<SliceNum;i++)
		SortedIndices.Add(i);

	SortedIndices.Sort([this](const int32 A, const int32 B)
	{
		return CurrentSliceWeights[A] < CurrentSliceWeights[B];
	});

	// Randomly choose a slice index from the lowest three
	int32 ChosenCandidate = FMath::RandRange(0, 3);
	int32 NewSliceIndex = SortedIndices[ChosenCandidate];

	return CalculateNextCoverPosition(NewSliceIndex, CurrentSelfPosition);
}

FVector UCombatBrainSubsystem::CalculateNextCoverPosition(int32 SliceIndex, const FVector& CurrentSelfPosition) const
{
	const FVector PlayerPos = GetPlayerPosition();
	const FVector CombatAvgPos = GetCombatantAveragePosition();
	
	FVector Vpa = CombatAvgPos - PlayerPos;

	const float SliceWidthDeg = 180.0f / float(SliceNum);
	const float SliceCenterAngleDeg = -90.0f + (SliceWidthDeg * (float(SliceIndex) + 0.5f));

	const FVector SliceDir2D = FRotator(0.0f, SliceCenterAngleDeg, 0.0f).RotateVector(Vpa);

	float DesiredDist = (CurrentSelfPosition - PlayerPos).Length();
	DesiredDist *= 0.9f;

	FVector FinalPos = PlayerPos + SliceDir2D * DesiredDist;
	FinalPos.Z = CurrentSelfPosition.Z;
	return FinalPos;
}

void UCombatBrainSubsystem::UpdateSliceWeight(int32 SliceIndex, bool bIsAdding)
{
	const int32 Sign = bIsAdding ? 1 : -1;

	for (int32 i=0;i<SliceWeightLUT.Num();i++)
	{
		const float Delta = Sign * SliceWeightLUT[i];

		auto UpdateWeight = [this, Delta](int32 Index)
		{
			if (!CurrentSliceWeights.IsValidIndex(Index))
				return;

			CurrentSliceWeights[Index] = FMath::Max(0.0f, CurrentSliceWeights[Index] + Delta);
		};

		// For center slice, add the weight
		if (i == 0)
		{
			UpdateWeight(SliceIndex);
			continue;
		}

		// Adding weight to neighbor
		UpdateWeight(SliceIndex + i);
		UpdateWeight(SliceIndex - i);
	}
}

int32 UCombatBrainSubsystem::GetSliceIndexFromWorldPosition(const FVector& WorldPosition)
{
	const FVector PlayerPosition = GetPlayerPosition();

	// Vector from player to combatant average
	FVector Vpa = GetCombatantAveragePosition() - PlayerPosition;
	Vpa.Z = 0.0f;
	Vpa = Vpa.GetSafeNormal();

	// Vector from player to current combatant
	FVector Vpc = WorldPosition - PlayerPosition;
	Vpc.Z = 0.0f;
	Vpc = Vpc.GetSafeNormal();

	const float Dot = FVector::DotProduct(Vpa, Vpc);
	const float CrossZ = Vpa.X * Vpc.Y - Vpa.Y * Vpa.X;

	const float AngleDeg = FMath::RadiansToDegrees(FMath::Atan2(CrossZ, Dot));
	const float ClampedAngle = FMath::Clamp(AngleDeg, -90.0f, 90.0f);
	const float Normalized = (ClampedAngle + 90.0f) / 180.0f;
	const int32 SliceIndex = FMath::Clamp(FMath::FloorToInt(Normalized * float(SliceNum)),0, SliceNum-1);

	return SliceIndex;
}

void UCombatBrainSubsystem::OccupiedSlice(AActor* Combatant, int32 NewIndex)
{
	if (const int32* CurrentIndex = CombatantOccupiedSlice.Find(Combatant))
	{
		if (*CurrentIndex == NewIndex)
			return;

		UpdateSliceWeight(*CurrentIndex, false);
	}

	UpdateSliceWeight(NewIndex, true);
	CombatantOccupiedSlice.Add(Combatant, NewIndex);
}

void UCombatBrainSubsystem::ClearOccupiedSlice(AActor* Combatant)
{
	if (IsValid(Combatant))
		return;

	int32* SliceIndex = CombatantOccupiedSlice.Find(Combatant);
	UpdateSliceWeight(*SliceIndex, false);

	CombatantOccupiedSlice.Remove(Combatant);
}

FVector UCombatBrainSubsystem::GetPlayerPosition() const
{
	return UGameplayStatics::GetPlayerPawn(GetWorld(), 0)->GetActorLocation();
}

FVector UCombatBrainSubsystem::GetCombatantAveragePosition() const
{
	//TODO: Commander mode? where combatant centers around a commander AI
	
	FVector Sum = FVector::ZeroVector;
	int32 Count = 0;

	for (AActor* Combatant : RegisteredCombatants)
	{
		if (!IsValid(Combatant))
			continue;;

		Sum += Combatant->GetActorLocation();
		++Count;
	}

	return Count > 0 ? (Sum/float(Count)) : FVector::ZeroVector;
}

void UCombatBrainSubsystem::DebugDrawInfos()
{
	const FVector PlayerPos = GetPlayerPosition();
	const FVector CombatAvgPos = GetCombatantAveragePosition();
	
	const FVector CenterDir2D = CombatAvgPos - PlayerPos;
	const float SliceWidthDeg = 180.0f / float(SliceNum);

	for (int32 i = 0; i <= SliceNum; ++i)
	{
		const float BoundaryAngleDeg = -90.0f + SliceWidthDeg * float(i);
		const FVector Dir = FRotator(0.0f, BoundaryAngleDeg, 0.0f).RotateVector(CenterDir2D).GetSafeNormal();
		DrawDebugLine(GetWorld(), PlayerPos, PlayerPos + Dir * 5000.0f, FColor::Cyan, false, 10.0f, 0, 4);
	}

	
	for (int32 SliceIdx = 0; SliceIdx < SliceNum; ++SliceIdx)
	{
		const float CenterAngleDeg = -90.0f + SliceWidthDeg * (float(SliceIdx) + 0.5f);
		const FVector Dir = FRotator(0.0f, CenterAngleDeg, 0.0f).RotateVector(CenterDir2D).GetSafeNormal();

		const float W = CurrentSliceWeights.IsValidIndex(SliceIdx) ? CurrentSliceWeights[SliceIdx] : 0.0f;
		DrawDebugString(GetWorld(), PlayerPos + Dir * 5000.0f, FString::Printf(TEXT("[%d] %.1f"), SliceIdx, W),
			nullptr, FColor::White, 10.0f, false);
	}
	
}

void UCombatBrainSubsystem::OnCombatantDestroyed(AActor* DestroyedAgent)
{
	UnregisterCombatant(DestroyedAgent);
}
