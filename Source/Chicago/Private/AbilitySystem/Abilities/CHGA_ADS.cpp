// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/CHGA_ADS.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Camera/CameraModifier.h"

UCHGA_ADS::UCHGA_ADS()
{
}

void UCHGA_ADS::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// If this is first we activating the ability, add the modifier to CameraManager (will enable automatically)
	if (CameraModifier)
		CameraModifier->EnableModifier();
	else
		CameraModifier = ActorInfo->PlayerController->PlayerCameraManager->AddNewCameraModifier(CameraModifierClass);

	if (GameplayEffectClass)
	{
		UGameplayEffect* GE = GameplayEffectClass->GetDefaultObject<UGameplayEffect>();
		GEHandle = ApplyGameplayEffectToOwner(Handle, ActorInfo, ActivationInfo, GE, 1.0f);
	}
}

void UCHGA_ADS::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UCHGA_ADS::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                           const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
	CameraModifier->DisableModifier();

	if (GEHandle.IsValid())
	{
		GetAbilitySystemComponentFromActorInfo_Ensured()->RemoveActiveGameplayEffect(GEHandle);
	}
}
