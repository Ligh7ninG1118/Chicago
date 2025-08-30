// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/CHAbilitySystemComponent.h"

#include "AbilitySystem/CHGameplayAbility.h"

void UCHAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		for (const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
		{
			if (Spec.Ability && Cast<UCHGameplayAbility>(Spec.Ability)->AbilityInputTag == InputTag)
			{
				InputPressedSpecHandles.AddUnique(Spec.Handle);
				InputHeldSpecHandles.AddUnique(Spec.Handle);
			}
		}
	}
}

void UCHAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		for (const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
		{
			//(Spec.DynamicAbilityTags.HasTagExact(InputTag))
			if (Spec.Ability && Cast<UCHGameplayAbility>(Spec.Ability)->AbilityInputTag == InputTag)
			{
				InputReleasedSpecHandles.AddUnique(Spec.Handle);
			}
		}
	}
}

void UCHAbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	auto AllTags = GameplayTagCountContainer.GetExplicitGameplayTags();

	FString Output = "";
	for (auto Tag : AllTags)
	{
		Output += Tag.ToString() + " ";
	}
	GEngine->AddOnScreenDebugMessage(50, 2.0f, FColor::Yellow, *Output);

	//TODO: Check input blocked tag

	static TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;
	AbilitiesToActivate.Reset();

	for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandles)
	{
		if (const FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability && !AbilitySpec->IsActive())
			{
				const UCHGameplayAbility* AbilityCDO = Cast<UCHGameplayAbility>(AbilitySpec->Ability);
				if (AbilityCDO && AbilityCDO->GetActivationPolicy() == ECHAbilityActivationPolicy::WhileInputActive)
					AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
			}
		}
	}

	for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = true;

				if (AbilitySpec->IsActive())
				{
					AbilitySpecInputPressed(*AbilitySpec);
				}
				else
				{
					const UCHGameplayAbility* AbilityCDO = Cast<UCHGameplayAbility>(AbilitySpec->Ability);
					
					if (AbilityCDO && AbilityCDO->GetActivationPolicy() == ECHAbilityActivationPolicy::OnInputTriggered)
						AbilitiesToActivate.AddUnique(AbilitySpec->Handle);	
				}
			}
		}
	}

	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitiesToActivate)
	{
		TryActivateAbility(AbilitySpecHandle);
	}

	for (const FGameplayAbilitySpecHandle& SpecHandle : InputReleasedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = false;

				if (AbilitySpec->IsActive())
				{
					AbilitySpecInputReleased(*AbilitySpec);
				}
			}
		}
	}

	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}

void UCHAbilitySystemComponent::OnTagUpdated(const FGameplayTag& Tag, bool TagExists)
{
	OnTagUpdate.Broadcast(Tag, TagExists);
	if (TagExists)
		GEngine->AddOnScreenDebugMessage(82, 2.0f, FColor::Yellow, FString::Printf(TEXT("%s added"), *Tag.ToString()));
	else
		GEngine->AddOnScreenDebugMessage(82, 2.0f, FColor::Yellow, FString::Printf(TEXT("%s removed"), *Tag.ToString()));
}
