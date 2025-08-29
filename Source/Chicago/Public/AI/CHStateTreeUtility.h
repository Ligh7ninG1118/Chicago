// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeConditionBase.h"


#include "CHStateTreeUtility.generated.h"

/**
 * 
 */

class ACHAICharacter;
class ACHAIController;

USTRUCT()
struct FStateTreeHasLineOfSightToTargetConditionInstanceData
{
	GENERATED_BODY()
	
	/** Targeting character */
	UPROPERTY(EditAnywhere, Category = "Context")
	ACHAICharacter* Character;

	/** Target to check line of sight for */
	UPROPERTY(EditAnywhere, Category = "Condition")
	AActor* Target;

	/** Max allowed line of sight cone angle, in degrees */
	UPROPERTY(EditAnywhere, Category = "Condition")
	float LineOfSightConeAngle = 35.0f;

	/** Number of vertical line of sight checks to run to try and get around low obstacles */
	UPROPERTY(EditAnywhere, Category = "Condition")
	int32 NumberOfVerticalLineOfSightChecks = 5;

	/** If true, the condition passes if the character has line of sight */
	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bMustHaveLineOfSight = true;
};
STATETREE_POD_INSTANCEDATA(FStateTreeHasLineOfSightToTargetConditionInstanceData);

/**
 *  StateTree condition to check if the character is grounded
 */
USTRUCT(DisplayName = "Has Line of Sight to Target", Category="Chicago")
struct FStateTreeHasLineOfSightToTargetCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	/** Set the instance data type */
	using FInstanceDataType = FStateTreeHasLineOfSightToTargetConditionInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	/** Default constructor */
	FStateTreeHasLineOfSightToTargetCondition() = default;
	
	/** Tests the StateTree condition */
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

#if WITH_EDITOR
	/** Provides the description string */
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif

};


USTRUCT()
struct FStateTreeFaceToActorInstanceData
{
	GENERATED_BODY()

	/** AI Controller that will determine the focused actor */
	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<ACHAIController> Controller;

	/** Actor that will be faced towards */
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<AActor> ActorToFaceTowards;
};

/**
 *  StateTree task to face an AI-Controlled Pawn towards an Actor
 */
USTRUCT(meta=(DisplayName="Face Towards Actor", Category="Chicago"))
struct FStateTreeFaceToActorTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	/* Ensure we're using the correct instance data struct */
	using FInstanceDataType = FStateTreeFaceToActorInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	/** Runs when the owning state is entered */
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

	/** Runs when the owning state is ended */
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif // WITH_EDITOR
};


USTRUCT()
struct FStateTreeDetectEnemiesInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<ACHAIController> Controller;

	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<ACHAICharacter> Character;	

	UPROPERTY(EditAnywhere, Category = Output)
	TObjectPtr<AActor> TargetActor;

	UPROPERTY(EditAnywhere, Category = Output)
	bool bHasTarget = false;

	UPROPERTY(EditAnywhere, Category = Parameter)
	FGameplayTag SenseTag;

	UPROPERTY(EditAnywhere, Category = Parameter)
	float DirectLineOfSightConeHalfDeg = 85.0f;

	UPROPERTY(EditAnywhere)
	float LastStimulusStrength = 0.0f;
};

USTRUCT(meta=(DisplayName = "Detect Enemies", Category="Chicago"))
struct FStateTreeDetectEnemiesTask : public FStateTreeTaskCommonBase  
{
	GENERATED_BODY()

	using FInstanceDataType = FStateTreeDetectEnemiesInstanceData;
	virtual const UStruct* GetInstanceDataType() const override {return FInstanceDataType::StaticStruct();}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif
};