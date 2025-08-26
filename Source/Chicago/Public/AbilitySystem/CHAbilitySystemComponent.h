// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "CHAbilitySystemComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTagUpdateDelegate, const FGameplayTag&, Tag, bool, TagExists);

/**
 * 
 */
UCLASS()
class CHICAGO_API UCHAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	/*UPROPERTY()
	FGameplayTagContainer GeneralGameplayTagContainer;*/

	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	
	void AbilityInputTagReleased(const FGameplayTag& InputTag);

	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);
	
	UPROPERTY(BlueprintAssignable)
	FOnTagUpdateDelegate OnTagUpdate;
	
protected:

	virtual void OnTagUpdated(const FGameplayTag& Tag, bool TagExists) override;
	
	// Handles to abilities that had their input pressed this frame.
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;

	// Handles to abilities that had their input released this frame.
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;

	// Handles to abilities that have their input held.
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;	
};
