// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AI/CHAIController.h"

#include "Components/CapsuleComponent.h"
#include "Components/StateTreeAIComponent.h"
#include "GameFramework/Character.h"
#include "Perception/AIPerceptionComponent.h"


ACHAIController::ACHAIController()
{
	StateTreeAI = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeAI"));

	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

	AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &ACHAIController::OnPerceptionUpdated);
	AIPerception->OnTargetPerceptionForgotten.AddDynamic(this, &ACHAIController::OnPerceptionForgotten);
}


void ACHAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}


void ACHAIController::SetCurrentTarget(AActor* Target)
{
	TargetEnemy = Target;
}

void ACHAIController::ClearCurrentTarget()
{
	TargetEnemy = nullptr;
}

void ACHAIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	OnAIPerceptionUpdated.ExecuteIfBound(Actor, Stimulus);
}

void ACHAIController::OnPerceptionForgotten(AActor* Actor)
{
	OnAIPerceptionForgotten.ExecuteIfBound(Actor);
}

FVector ACHAIController::CalculateCoverAnchor(FVector RoughPosition)
{
	CurrentCoverData.Reset();

	//TODO: Adjust this, 100 too big (always hit ground)
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(50.0f);

	//TODO: Add fallback, use AI's forward direction if there is no target
	FVector TargetPosition = GetCurrentTarget()->GetActorLocation();
	RoughPosition.Z = TargetPosition.Z;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetPawn());

	FHitResult Hit;
	//TODO: Consider high/low cover and objects on ground (need to trace from a height not too low nor too high)
	bool bHit = GetWorld()->SweepSingleByChannel(
		Hit,
		RoughPosition,
		TargetPosition, // Tracing from rough position all the way to target, guaranteed (?) to hit any cover
		FQuat::Identity,
		ECC_Visibility,
		SphereShape,
		QueryParams);

	DrawDebugLine(GetWorld(), RoughPosition, TargetPosition, FColor::Orange, true, 10.0f);

	if (bHit)
	{
		DrawDebugSphere(GetWorld(),
		                Hit.ImpactPoint,
		                50.0f,
		                8,
		                FColor::Red,
		                true,
		                10.0f);

		CurrentCoverData.CoverActor = Hit.GetActor();
		ProbingCoverCorner(Hit, RoughPosition);
	}
	else
	{
		// Fallback, use rough position
		CurrentCoverData.AnchorPosition = RoughPosition;
	}

	return CurrentCoverData.AnchorPosition;
}

void ACHAIController::ClearCoverData()
{
	CurrentCoverData.Reset();
}

void ACHAIController::ProbingCoverCorner(const FHitResult& HitResult, FVector RoughPosition)
{
	// Position if we are closely standing against the cover;


	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, TEXT("Probing"));

	//TODO: Wall probing distance depends on cover collider (won't have to go further than half the size)

	auto ComputeEdgeDist = [&](const FVector& TangentDir)
	{
		auto MakeProbingStart = [&](float Dist)
		{
			// From impact point, shift some distance away from the wall, and come out of the wall for a bit
			return HitResult.ImpactPoint + TangentDir * Dist + HitResult.ImpactNormal * 10.0f;
		};

		auto MakeProbingEnd = [&](const FVector& Start)
		{
			// Toward the wall
			return Start - HitResult.ImpactNormal * 100.0f;
		};

		FVector InitialProbingStart = MakeProbingStart(WallProbingDistance);
		FVector InitialProbingEnd = MakeProbingEnd(InitialProbingStart);

		float innerDist = 0.0f;
		float outerDist = 0.0f;

		if (!HasProbingHitCover(InitialProbingStart, InitialProbingEnd, HitResult.GetActor()))
			outerDist = WallProbingDistance;

		for (int i = 0; i < WallProbingIterationDepth; i++)
		{
			if (FMath::Abs(innerDist - outerDist) <= KINDA_SMALL_NUMBER)
				break;

			const float MidPoint = (innerDist + outerDist) * 0.5f;

			FVector ProbingStart = MakeProbingStart(MidPoint);
			FVector ProbingEnd = MakeProbingEnd(ProbingStart);

			if (HasProbingHitCover(ProbingStart, ProbingEnd, HitResult.GetActor()))
				innerDist = MidPoint;
			else
				outerDist = MidPoint;
		}

		return (innerDist + outerDist) * 0.5f;
	};

	const FVector WallTangent = HitResult.ImpactNormal.Cross(FVector::DownVector);
	float LeftEdgeDist = ComputeEdgeDist(WallTangent);
	float RightEdgeDist = ComputeEdgeDist(-WallTangent);

	float FinalEdgeDist = 0.0f;

	GEngine->AddOnScreenDebugMessage(-1, 555.0f, FColor::Cyan,
	                                 FString::Printf(TEXT("Left %f Right %f"), LeftEdgeDist, RightEdgeDist));


	// If left edge and right edge is close enough, then we can peek out from either direction
	if (LeftEdgeDist + RightEdgeDist <= CoverAnchorSideOffset)
	{
		FinalEdgeDist = (LeftEdgeDist - RightEdgeDist) * 0.5f;
		CurrentCoverData.PeekOptions |= ECoverPeekFlag::LEFT;
		CurrentCoverData.PeekOptions |= ECoverPeekFlag::RIGHT;

		GEngine->AddOnScreenDebugMessage(-1, 555.0f, FColor::Cyan, TEXT("Both"));
	}

	// Choose the side with lesser distance
	if (LeftEdgeDist < RightEdgeDist)
	{
		FinalEdgeDist = LeftEdgeDist - CoverAnchorSideOffset;
		CurrentCoverData.PeekOptions |= ECoverPeekFlag::LEFT;

		GEngine->AddOnScreenDebugMessage(-1, 555.0f, FColor::Cyan, TEXT("Left"));
	}
	else
	{
		// Left is positive, right negative
		FinalEdgeDist = -(RightEdgeDist - CoverAnchorSideOffset);
		CurrentCoverData.PeekOptions |= ECoverPeekFlag::RIGHT;

		GEngine->AddOnScreenDebugMessage(-1, 555.0f, FColor::Cyan, TEXT("Right"));
	}

	const float CapsuleRadius = GetCharacter()->GetCapsuleComponent()->GetScaledCapsuleRadius();
	FVector CloseToCoverPosition = HitResult.ImpactPoint + HitResult.ImpactNormal * (CapsuleRadius + CoverAnchorAwayOffset);
	FVector FinalPosition = CloseToCoverPosition + WallTangent * FinalEdgeDist;

	DrawDebugSphere(GetWorld(), FinalPosition, 25.0f, 8, FColor::White, true);
	
	CurrentCoverData.AnchorPosition = FinalPosition;
}

bool ACHAIController::HasProbingHitCover(FVector StartPosition, FVector EndPosition, const AActor* CoverActor)
{
	FHitResult ProbingHitResult;
	//TODO: Maybe we should use multiple trace here in case there are other actors
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		ProbingHitResult,
		StartPosition,
		EndPosition,
		ECC_Visibility);

	//TODO: Should we do a LOS check here?

	DrawDebugLine(GetWorld(), StartPosition, EndPosition, FColor::Green, true);


	// Hit nothing, or hit something but it isn't the cover itself
	// Problem with mesh (no actor)
	if (!bHit || (bHit && ProbingHitResult.GetActor() != CoverActor))
		return false;

	return true;
}
