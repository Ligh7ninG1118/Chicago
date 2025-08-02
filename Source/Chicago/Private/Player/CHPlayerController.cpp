// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CHPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "AbilitySystem/CHAbilitySystemComponent.h"
#include "Character/CHCharacterBase.h"

ACHPlayerController::ACHPlayerController()
{
	//TODO: Set player CM here?
}

void ACHPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
		{
			Subsystem->AddMappingContext(CurrentContext, 0);
		}
	}
}

void ACHPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (ACHCharacterBase* PYCharacter = Cast<ACHCharacterBase>(GetCharacter()))
	{
		if (UCHAbilitySystemComponent* ASC = Cast<UCHAbilitySystemComponent>(PYCharacter->GetAbilitySystemComponent()))
		{
			ASC->ProcessAbilityInput(DeltaTime, bGamePaused);
		}
	}
	
	Super::PostProcessInput(DeltaTime, bGamePaused);
}
