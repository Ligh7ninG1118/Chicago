// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/CHCharacterBase.h"
#include "Components/TimelineComponent.h"
#include "Equipments/WeaponHolder.h"
#include "CHPlayerCharacter.generated.h"

class UTimelineComponent;
class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;
struct FGameplayTag;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRecoilUpdateDelegate, FVector2f, Recoil);


UENUM(BlueprintType)
enum class ECanLeanState : uint8
{
	NoLean = 0,
	CanLeanLeft,
	CanLeanRight,
	CanLeanOver,
};

/**
 * 
 */
UCLASS()
class CHICAGO_API ACHPlayerCharacter : public ACHCharacterBase, public IWeaponHolder
{
	GENERATED_BODY()
#pragma region Components
	
public:
	/** Returns the first person mesh **/
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }
protected:
	/** Pawn mesh: first person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;
	
#pragma endregion Components

#pragma region Character Overrides
	
public:
	ACHPlayerCharacter(const class FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;
	
protected:
	/** Set up input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	
#pragma endregion Character Overrides

#pragma region Input
protected:
	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category ="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category ="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category ="Input")
	class UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category ="Input")
	class UInputAction* MouseLookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category ="Input")
	class UInputAction* CrouchAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category ="Input")
	class UInputAction* SprintAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category ="Input")
	class UInputAction* ADSAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category ="Input")
	class UInputAction* PrimaryFireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category ="Input")
	class UInputAction* ReloadAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	class UInputConfig* AbilitiesInputConfig;

private:
	/** Called from Input Actions for movement input */
	void MoveInput(const FInputActionValue& Value);

	/** Called from Input Actions for looking input */
	void LookInput(const FInputActionValue& Value);

	/** Handles aim inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoAim(float Yaw, float Pitch);

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles jump start inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump end inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoCrouch();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoSprintStart();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoSprintStop();
	
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoAimingDownSightStart();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoAimingDownSightStop();
	
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoPrimaryFireStart();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoPrimaryFireEnd();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoReload();	

	//Handle Gameplay Ability Input Bindings
	void AbilityInputTagPressed(FGameplayTag InputTag);
	
	void AbilityInputTagReleased(FGameplayTag InputTag);
	
#pragma endregion Input

#pragma region Weapon
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category ="Weapon")
	TSubclassOf<ACHWeaponBase> InitialWeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category ="Weapon")
	FName FirstPersonWeaponSocket = FName("ik_hand_gun");
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float RecoilSmoothClimbSpeed = 7.0f;

	virtual void ProcessRecoil(float DeltaTime);

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	ACHWeaponBase* CurrentWeapon;

	UPROPERTY(BlueprintReadOnly)
	bool bIsAiming = false;
	
	FVector2f RecoilTarget = FVector2f::Zero();

#pragma endregion Weapon

#pragma region Camera
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	TSubclassOf<UCameraModifier> ADSCameraModifierClass;
    
	UCameraModifier* ADSCameraModifier;
	
#pragma endregion Camera
	
#pragma region Contextual Lean

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Contextual Lean")
	UTimelineComponent* LeanTimeline;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Contextual Lean")
	UCurveFloat* LeanCurve;

	UFUNCTION()
	virtual void OnLeanTimelineUpdate(float Value);

	UFUNCTION()
	virtual void OnLeanTimelineFinished();

	UPROPERTY()
	TEnumAsByte<ETimelineDirection::Type> LeanTimelineDirection;
	
	UFUNCTION(BlueprintCallable, Category = "Contextual Lean")
	virtual void CanContextualLeanCheck();

	UFUNCTION(BlueprintCallable, Category = "Contextual Lean")
	virtual void ContextualLean(bool IsStartLeaning);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Contextual Lean")
	float SideStepDistance = 50.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Contextual Lean")
	float WallCheckDistance = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Contextual Lean")
	float SideWallCheckDeg = 45.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Contextual Lean")
	float SideWallCheckDistance = 300.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Contextual Lean")
	ECanLeanState CanLeanState;

	FVector MeshStartingRelativeOffset;
	FVector MeshTargetOffset;
	
	UPROPERTY(BlueprintReadOnly, Category = "Contextual Lean")
	bool bIsCloseToWall = false;
	
	bool bIsLeaning = false;
	bool bHasLeaned = false;
	bool bHasFinishLeaned = false;
	
#pragma endregion Contextual Leaning


#pragma region IWeaponHolder Interface
	
public:
	virtual void AttachWeaponMeshes(ACHWeaponBase* Weapon) override;

	virtual void PlayFiringMontage(UAnimMontage* Montage) override;

	virtual float PlayReloadMontage(UAnimMontage* Montage) override;

	virtual void HandleWeaponRecoil(FVector2f Recoil) override;

	virtual float GetMovementAccuracyPenalty() const override;

	virtual UCameraComponent* GetFiringComponent() const override;

	virtual UAnimInstance* GetAnimInstance() const override;
	
#pragma endregion IWeaponHolder Interface

};
