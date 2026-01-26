// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AI/CHAIController.h"

#include "Components/StateTreeAIComponent.h"
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
		TargetPosition,		// Tracing from rough position all the way to target, guaranteed (?) to hit any cover
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
	FVector CloseToCoverPosition = HitResult.ImpactPoint + HitResult.ImpactNormal * 75.0f;

	CurrentCoverData.AnchorPosition = CloseToCoverPosition;

	DrawDebugLine(GetWorld(), HitResult.ImpactPoint, CloseToCoverPosition, FColor::Purple, true, 10.0f);
	
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("Cover Anchor Adjusted"));
	
	DrawDebugSphere(GetWorld(),
			CloseToCoverPosition,
			50.0f,
			8,
			FColor::Green,
			true,
			10.0f);
	
	DrawDebugSphere(GetWorld(),
		RoughPosition,
		50.0f,
		8,
		FColor::Cyan,
		true,
		10.0f);
}
