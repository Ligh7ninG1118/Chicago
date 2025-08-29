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
