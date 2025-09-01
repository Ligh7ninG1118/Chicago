// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CHPlayerCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "AbilitySystem/CHAbilitySystemComponent.h"
#include "Camera/CameraModifier.h"
#include "Character/CHCharacterMovementComponent.h"
#include "Components/TimelineComponent.h"
#include "Equipments/CHWeaponBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Input/CHEnhancedInputComponent.h"
#include "Player/CHPlayerController.h"

#pragma region Character Overrides

ACHPlayerCharacter::ACHPlayerCharacter(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(34.0f, 96.0f);
	
	// Create the first person mesh that will be viewed only by this character's owner

	/*FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));*/

	//TODO: Temp solution for rotating the gun to view
	
	// Create the Camera Component	
	
	//FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));
	FirstPersonMesh->SetupAttachment(FirstPersonCameraComponent);
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	
	
	//TODO: Do I need all these?
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	/*FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;*/

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;

	LeanTimeline = CreateDefaultSubobject<UTimelineComponent>("LeanTimeline");
}

void ACHPlayerCharacter::BeginPlay()
{
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.Instigator = this;
	
	AActor* Weapon = GetWorld()->SpawnActor(InitialWeaponClass, nullptr, nullptr, SpawnParameters);
	CurrentWeapon = Cast<ACHWeaponBase>(Weapon);

	if (auto* PlayerController = Cast<ACHPlayerController>(GetController()))
	{
		ADSCameraModifier = PlayerController->PlayerCameraManager->AddNewCameraModifier(ADSCameraModifierClass);
		ADSCameraModifier->DisableModifier(true);
	}

	FOnTimelineFloat InterpFloat;
	InterpFloat.BindUFunction(this, FName("OnLeanTimelineUpdate"));
	LeanTimeline->AddInterpFloat(LeanCurve, InterpFloat);

	FOnTimelineEvent FinishedEvent;
	FinishedEvent.BindUFunction(this, FName("OnLeanTimelineFinished"));
	LeanTimeline->SetTimelineFinishedFunc(FinishedEvent);

	LeanTimeline->SetPropertySetObject(this);
	LeanTimeline->SetDirectionPropertyName(FName("LeanTimelineDirection"));
	
	// Calls BeginPlay on BP class last
	Super::BeginPlay();
}

void ACHPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bShouldProcessRecoil)
		ProcessRecoilClimb(DeltaSeconds);

	if (TotalRecoilClimb.Length() >= KINDA_SMALL_NUMBER && GetWorld()->TimeSeconds - TimeOfLastShot >= RecenterDelayTime)
	{
		ProcessRecentering(DeltaSeconds);
	}

	CanContextualLeanCheck();
}

void ACHPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UCHEnhancedInputComponent* EnhancedInputComponent = Cast<UCHEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACHPlayerCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACHPlayerCharacter::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACHPlayerCharacter::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACHPlayerCharacter::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ACHPlayerCharacter::LookInput);

		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ACHPlayerCharacter::DoCrouch);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ACHPlayerCharacter::DoSprintStart);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ACHPlayerCharacter::DoSprintStop);
		
		EnhancedInputComponent->BindAction(PrimaryFireAction, ETriggerEvent::Started, this, &ACHPlayerCharacter::DoPrimaryFireStart);
		EnhancedInputComponent->BindAction(PrimaryFireAction, ETriggerEvent::Completed, this, &ACHPlayerCharacter::DoPrimaryFireEnd);

		EnhancedInputComponent->BindAction(ADSAction, ETriggerEvent::Started, this, &ACHPlayerCharacter::DoAimingDownSightStart);
		EnhancedInputComponent->BindAction(ADSAction, ETriggerEvent::Completed, this, &ACHPlayerCharacter::DoAimingDownSightStop);

		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &ACHPlayerCharacter::DoReload);
		
		if (AbilitiesInputConfig != nullptr)
		{
			TArray<uint32> BindHandles;
			EnhancedInputComponent->BindAbilityAction(AbilitiesInputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased, BindHandles);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

#pragma endregion Character Overrides


#pragma region Input

void ACHPlayerCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	MoveInputVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MoveInputVector.X, MoveInputVector.Y);

}

void ACHPlayerCharacter::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	LookInputVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookInputVector.X, LookInputVector.Y);

	if (TotalRecoilClimb.Length() >= KINDA_SMALL_NUMBER)
	{
		// If it's already recentering, clear TotalRecoilClimb altogether
		if (GetWorld()->TimeSeconds - TimeOfLastShot >= RecenterDelayTime)
		{
			TotalRecoilClimb = FVector2f::Zero();
		}
		
		TotalRecoilClimb.Y += LookInputVector.Y;
		TotalRecoilClimb.X += LookInputVector.X;
	}
}

void ACHPlayerCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// pass the rotation inputs
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ACHPlayerCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void ACHPlayerCharacter::DoJumpStart()
{
	// pass Jump to the character
	Jump();
}

void ACHPlayerCharacter::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();
}

void ACHPlayerCharacter::DoCrouch()
{
	//CHCMC->CrouchPressed();
}

void ACHPlayerCharacter::DoSprintStart()
{
	GetAbilitySystemComponent()->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("GAS.Character.Movement.Sprinting")));
	CHCMC->SprintPressed();
}

void ACHPlayerCharacter::DoSprintStop()
{
	GetAbilitySystemComponent()->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("GAS.Character.Movement.Sprinting")));
	CHCMC->SprintReleased();
}

void ACHPlayerCharacter::DoAimingDownSightStart()
{
	GetAbilitySystemComponent()->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("GAS.Character.Action.Aiming")));
	ADSCameraModifier->EnableModifier();

	if (CanLeanState == ECanLeanState::CanLeanLeft || CanLeanState == ECanLeanState::CanLeanRight)
	{
		bHasLeaned = true;
		bHasFinishLeaned = false;
		ContextualLean(true);
	}
}

void ACHPlayerCharacter::DoAimingDownSightStop()
{
	GetAbilitySystemComponent()->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("GAS.Character.Action.Aiming")));
	ADSCameraModifier->DisableModifier();

	if (bHasLeaned)
		ContextualLean(false);
}

void ACHPlayerCharacter::DoPrimaryFireStart()
{
	CurrentWeapon->StartFiring();
}

void ACHPlayerCharacter::DoPrimaryFireEnd()
{
	CurrentWeapon->StopFiring();
}

void ACHPlayerCharacter::DoReload()
{
	CurrentWeapon->Reload();
}

void ACHPlayerCharacter::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (AbilitySystemComponent != nullptr)
	{
		if (UCHAbilitySystemComponent* ASC = Cast<UCHAbilitySystemComponent>(AbilitySystemComponent))
		{
			ASC->AbilityInputTagPressed(InputTag);
		}
	}
}

void ACHPlayerCharacter::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (AbilitySystemComponent != nullptr)
	{
		if (UCHAbilitySystemComponent* ASC = Cast<UCHAbilitySystemComponent>(AbilitySystemComponent))
		{
			ASC->AbilityInputTagReleased(InputTag);
		}
	}
}
#pragma endregion Input

#pragma region Weapon

void ACHPlayerCharacter::ProcessRecoilClimb(float DeltaTime)
{
	if (RecoilTarget.Length() <= KINDA_SMALL_NUMBER)
	{
		RecoilTarget = FVector2f::Zero();
		bShouldProcessRecoil = false;
		return;
	}

	// Smooth recoil climb
	FVector SmoothTarget = FMath::VInterpConstantTo(FVector::Zero(), FVector(RecoilTarget.X, RecoilTarget.Y, 0.0f), DeltaTime, RecoilSmoothClimbSpeed);

	AddControllerYawInput(SmoothTarget.X);
	AddControllerPitchInput(SmoothTarget.Y);

	RecoilTarget.X -= SmoothTarget.X;
	RecoilTarget.Y -= SmoothTarget.Y;
}

void ACHPlayerCharacter::ProcessRecentering(float DeltaTime)
{
	if (TotalRecoilClimb.Length() <= 0.3f)
	{
		TotalRecoilClimb = FVector2f::Zero();
		return;
	}

	if (TotalRecoilClimb.Y >= 0.0f)
		TotalRecoilClimb.Y = 0.0f;
	
	FVector RecenterTarget = FMath::VInterpTo(FVector(TotalRecoilClimb.X, TotalRecoilClimb.Y, 0.0f), FVector::Zero(), DeltaTime, RecenterRecoverSpeed);
	
	AddControllerYawInput(-RecenterTarget.X);
	AddControllerPitchInput(-RecenterTarget.Y);

	TotalRecoilClimb.X -= RecenterTarget.X;
	TotalRecoilClimb.Y -= RecenterTarget.Y;
}


#pragma endregion Weapon

#pragma region Contextual Lean

void ACHPlayerCharacter::OnLeanTimelineUpdate(float Value)
{
	FVector LerpOffset = FMath::Lerp(MeshStartingRelativeOffset, MeshTargetOffset, Value);
	GetMesh()->SetRelativeLocation(LerpOffset);
}

void ACHPlayerCharacter::OnLeanTimelineFinished()
{
	if (LeanTimelineDirection == ETimelineDirection::Forward)
	{
		bHasFinishLeaned = true;
	}
}

void ACHPlayerCharacter::CanContextualLeanCheck()
{
	CanLeanState = ECanLeanState::NoLean;

	FHitResult OutHit;
	const FVector CamPos = FirstPersonCameraComponent->GetComponentLocation();
	const FVector CamFwd = FirstPersonCameraComponent->GetForwardVector();
	const FVector CamRight = FirstPersonCameraComponent->GetRightVector();
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	
	// Check if we are close to a wall
	FVector WallCheckEndPos = CamPos + CamFwd * WallCheckDistance;
	
	bIsCloseToWall = GetWorld()->LineTraceSingleByChannel(OutHit, CamPos, WallCheckEndPos, ECC_WorldDynamic, QueryParams);
	//DrawDebugLine(GetWorld(), CamPos, WallCheckEndPos, FColor::Green, false, 0.2f);

	// If not close to wall, early out
	if (!bIsCloseToWall)
		return;

	// If leaning and close to wall, cancel lean
	if (bIsCloseToWall && bHasFinishLeaned)
	{
		LeanTimeline->Reverse();
	}

	// Check if our frontal left side is blocked by a wall
	FVector LeftSideWallCheck = CamFwd.RotateAngleAxis(-SideWallCheckDeg, FVector::UpVector);
	FVector LeftWallCheckEndPos = CamPos + LeftSideWallCheck * SideWallCheckDistance;
	
	//DrawDebugLine(GetWorld(), CamPos, LeftWallCheckEndPos, FColor::Blue, false, 0.2f);
	if (GetWorld()->LineTraceSingleByChannel(OutHit, CamPos, LeftWallCheckEndPos, ECC_WorldDynamic, QueryParams))
	{
		// Left side is blocked, now checking if sidestep to our right side can result in a clear line of sight
		FVector PosAfterLeanRight = CamPos + CamRight * SideStepDistance;
		FVector RightLOSCheckEndPos = PosAfterLeanRight + CamFwd * SideWallCheckDistance;
		
		//DrawDebugLine(GetWorld(), PosAfterLeanRight, RightLOSCheckEndPos, FColor::Cyan, false, 0.2f);
		if (!GetWorld()->LineTraceSingleByChannel(OutHit, PosAfterLeanRight, RightLOSCheckEndPos, ECC_WorldDynamic, QueryParams))
		{
			CanLeanState = ECanLeanState::CanLeanRight;
			return;
		}
	}

	// Check if our frontal right side is blocked by a wall
	FVector RightSideWallCheck = CamFwd.RotateAngleAxis(SideWallCheckDeg, FVector::UpVector);
	FVector RightWallCheckEndPos = CamPos + RightSideWallCheck * SideWallCheckDistance;

	//DrawDebugLine(GetWorld(), CamPos, RightWallCheckEndPos, FColor::Red, false, 0.2f);
	if (GetWorld()->LineTraceSingleByChannel(OutHit, CamPos, RightWallCheckEndPos, ECC_WorldDynamic, QueryParams))
	{
		// Right side is blocked, now checking if sidestep to our left side can result in a clear line of sight
		FVector PosAfterLeanLeft = CamPos + CamRight * SideStepDistance * (-1.0f);
		FVector LeftLOSCheckEndPos = PosAfterLeanLeft + CamFwd * SideWallCheckDistance;

		//DrawDebugLine(GetWorld(), PosAfterLeanLeft, LeftLOSCheckEndPos, FColor::Magenta, false, 0.2f);
		if (!GetWorld()->LineTraceSingleByChannel(OutHit, PosAfterLeanLeft, LeftLOSCheckEndPos, ECC_WorldDynamic, QueryParams))
		{
			CanLeanState = ECanLeanState::CanLeanLeft;
			return;
		}
	}
}

void ACHPlayerCharacter::ContextualLean(bool IsStartLeaning)
{
	if (IsStartLeaning)
	{
		MeshStartingRelativeOffset = GetMesh()->GetRelativeLocation();

		MeshTargetOffset = MeshStartingRelativeOffset;
		MeshTargetOffset.Y += SideStepDistance * (CanLeanState == ECanLeanState::CanLeanLeft ? -1.0f : 1.0f);
		
		LeanTimeline->Play();
	}
	else
	{
		LeanTimeline->Reverse();
	}
}

#pragma endregion Contextual Leaning

#pragma region IWeaponHolder Interface

void ACHPlayerCharacter::AttachWeaponMeshes(ACHWeaponBase* Weapon)
{
	const FAttachmentTransformRules AttachmentRule(EAttachmentRule::SnapToTarget, false);
	Weapon->AttachToActor(this, AttachmentRule);

	Weapon->GetGunMesh()->AttachToComponent(GetFirstPersonMesh(), AttachmentRule, FirstPersonWeaponSocket);
}

void ACHPlayerCharacter::PlayFiringMontage(UAnimMontage* Montage)
{
	UAnimInstance* AnimInstance = FirstPersonMesh->GetAnimInstance();
	if (AnimInstance != nullptr)
	{
		AnimInstance->Montage_Play(Montage);
	}
}

//TODO: Combine this with above function?
float ACHPlayerCharacter::PlayReloadMontage(UAnimMontage* Montage)
{
	UAnimInstance* AnimInstance = FirstPersonMesh->GetAnimInstance();
	if (AnimInstance != nullptr)
	{
		return AnimInstance->Montage_Play(Montage);
	}
	return 0.0f;
}

void ACHPlayerCharacter::HandleWeaponRecoil(FVector2f Recoil)
{
	bShouldProcessRecoil = true;
	TimeOfLastShot = GetWorld()->GetTimeSeconds();
	
	TotalRecoilClimb += Recoil;
	
	// Limit the maximum recoil climb on vertical axis
	if (TotalRecoilClimb.Y >= -MaxRecoilVerticalClimbDeg)
		RecoilTarget.Y += Recoil.Y;
	RecoilTarget.X += Recoil.X;
}

float ACHPlayerCharacter::GetMovementAccuracyPenalty() const
{
	return GetVelocity().Length();
}

UCameraComponent* ACHPlayerCharacter::GetFiringComponent() const
{
	return FirstPersonCameraComponent;
}

UAnimInstance* ACHPlayerCharacter::GetAnimInstance() const
{
	UAnimInstance* AnimInstance = FirstPersonMesh->GetAnimInstance();
	if (AnimInstance != nullptr)
	{
		return AnimInstance;
	}

	return nullptr;
}

#pragma endregion IWeaponHolder Interface

