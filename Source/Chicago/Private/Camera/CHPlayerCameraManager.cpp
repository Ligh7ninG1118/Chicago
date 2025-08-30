// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/CHPlayerCameraManager.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/CHPlayerCharacter.h"

ACHPlayerCameraManager::ACHPlayerCameraManager()
{
	// set the min/max pitch
	ViewPitchMin = -70.0f;
	ViewPitchMax = 80.0f;
}

void ACHPlayerCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
	Super::UpdateViewTarget(OutVT, DeltaTime);

	if (auto* Char = Cast<ACHPlayerCharacter>(GetOwningPlayerController()->GetPawn()))
	{
		UCharacterMovementComponent* CMC = Char->GetCharacterMovement();

		float ActualCrouchedHalfHeight = FMath::Max(CMC->GetCrouchedHalfHeight(), Char->GetCapsuleComponent()->GetScaledCapsuleRadius());
		float DefaultHalfHeight = Char->GetClass()->GetDefaultObject<ACHPlayerCharacter>()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

		FVector TargetCrouchOffset = FVector(0.0f, 0.0f, ActualCrouchedHalfHeight - DefaultHalfHeight);

		FVector Offset = FMath::Lerp(FVector::ZeroVector, TargetCrouchOffset, FMath::Clamp(CrouchBlendTimer / CrouchBlendDuration, 0.0f, 1.0f));

		if (CMC->IsCrouching())
		{
			CrouchBlendTimer = FMath::Clamp(CrouchBlendTimer + DeltaTime, 0.0f, CrouchBlendDuration);
			Offset -= TargetCrouchOffset;
		}
		else
		{
			CrouchBlendTimer = FMath::Clamp(CrouchBlendTimer - DeltaTime, 0.0f, CrouchBlendDuration);
		}

		if (CMC->IsMovingOnGround())
		{
			OutVT.POV.Location += Offset;
		}
	}
}
