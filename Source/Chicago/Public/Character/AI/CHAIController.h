// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameplayTagContainer.h"
#include "CHAIController.generated.h"

struct FAIStimulus;
class UStateTreeAIComponent;
class UAIPerceptionComponent;


DECLARE_DELEGATE_TwoParams(FAIPerceptionUpdatedDelegate, AActor*, const FAIStimulus&);
DECLARE_DELEGATE_OneParam(FAIPerceptionForgottenDelegate, AActor*);

/**
 * 
 */
UCLASS()
class CHICAGO_API ACHAIController : public AAIController
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStateTreeAIComponent* StateTreeAI;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UAIPerceptionComponent* AIPerception;

protected:
	UPROPERTY(EditAnywhere, Category="AI: Gameplay")
	FGameplayTag TeamTag;

	TObjectPtr<AActor> TargetEnemy;

	virtual void OnPossess(APawn* InPawn) override;
	
public:
	ACHAIController();

	void SetCurrentTarget(AActor* Target);

	void ClearCurrentTarget();

	AActor* GetCurrentTarget() const { return TargetEnemy; };
	
	FAIPerceptionUpdatedDelegate OnAIPerceptionUpdated;
	
	FAIPerceptionForgottenDelegate OnAIPerceptionForgotten;
	
protected:
	UFUNCTION()
	void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION()
	void OnPerceptionForgotten(AActor* Actor);
	
};
