// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "CHPlayerCameraManager.generated.h"

/**
 * 
 */
UCLASS()
class CHICAGO_API ACHPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	float CrouchBlendDuration = 0.2f;

	float CrouchBlendTimer;
	
public:
	/** Constructor */
	ACHPlayerCameraManager();

	virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;
};
