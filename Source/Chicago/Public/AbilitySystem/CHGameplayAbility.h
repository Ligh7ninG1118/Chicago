// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "CHGameplayAbility.generated.h"

UENUM(BlueprintType)
enum class ECHAbilityActivationPolicy : uint8
{
	OnInputTriggered,
	WhileInputActive,
	OnSpawn
};

/**
 * 
 */
UCLASS()
class CHICAGO_API UCHGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UCHGameplayAbility();

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
	FGameplayTag AbilityInputTag;

	ECHAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }

	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	ECHAbilityActivationPolicy ActivationPolicy;
};
