// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/CHCharacterBase.h"
#include "Equipments/WeaponHolder.h"
#include "CHPlayerCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;
struct FGameplayTag;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRecoilUpdateDelegate, FVector2f, Recoil);

/**
 * 
 */
UCLASS()
class CHICAGO_API ACHPlayerCharacter : public ACHCharacterBase, public IWeaponHolder
{
	GENERATED_BODY()
	
	/** Pawn mesh: first person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

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

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputConfig* AbilitiesInputConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category ="Weapons")
	FName FirstPersonWeaponSocket = FName("ik_hand_gun");
	
public:
	ACHPlayerCharacter(const class FObjectInitializer& ObjectInitializer);

	virtual void Tick(float DeltaSeconds) override;
	
	UPROPERTY(BlueprintAssignable)
	FRecoilUpdateDelegate OnRecoilUpdate;

	UFUNCTION(BlueprintImplementableEvent)
	void HandleRecoil(FVector2f Recoil);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
	float RecoilSmoothClimbSpeed = 7.0f;
	
	FVector2f RecoilTarget = FVector2f::Zero();

	virtual void ProcessRecoil(float DeltaTime);
	
protected:

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

	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);

protected:

	/** Set up input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	

public:

	/** Returns the first person mesh **/
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

public:
	virtual void AttachWeaponMeshes(ACHWeaponBase* Weapon) override;

	virtual void PlayFiringMontage(UAnimMontage* Montage) override;

	virtual float PlayReloadMontage(UAnimMontage* Montage) override;

	virtual void HandleWeaponRecoil(FVector2f Recoil) override;

	virtual float GetMovementAccuracyPenalty() const override;

	virtual UCameraComponent* GetFiringComponent() const override;

	virtual UAnimInstance* GetAnimInstance() const override;
};
