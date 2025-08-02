// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/CHGameplayAbility.h"

#include "AbilitySystemComponent.h"

UCHGameplayAbility::UCHGameplayAbility()
{
	ActivationPolicy = ECHAbilityActivationPolicy::OnInputTriggered;
}

void UCHGameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	if (ActivationPolicy == ECHAbilityActivationPolicy::OnSpawn)
		ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle, false);
}