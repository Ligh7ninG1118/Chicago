// Copyright Epic Games, Inc. All Rights Reserved.


#include "ChicagoPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "ChicagoCameraManager.h"

AChicagoPlayerController::AChicagoPlayerController()
{
	// set the player camera manager class
	PlayerCameraManagerClass = AChicagoCameraManager::StaticClass();
}

void AChicagoPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Add Input Mapping Context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
		{
			Subsystem->AddMappingContext(CurrentContext, 0);
		}
	}
}
