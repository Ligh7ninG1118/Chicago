// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CHCharacterMovementComponent.generated.h"

class ACHCharacterBase;
/**
 * 
 */
UCLASS()
class CHICAGO_API UCHCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	class FSavedMove_CH : public FSavedMove_Character
	{
		using Super =  FSavedMove_Character;

	public:

		uint8 Saved_bWantsToSprint:1;
		uint8 Saved_bPrevWantsToCrouch:1;
		
		FSavedMove_CH();
		
		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData) override;
		virtual void PrepMoveFor(ACharacter* C) override;
	};

	class FNetworkPredictionData_Client_CH : public FNetworkPredictionData_Client_Character
	{
		using Super = FNetworkPredictionData_Client_Character;
	public:
		FNetworkPredictionData_Client_CH(const UCharacterMovementComponent& ClientMovement);

		virtual FSavedMovePtr AllocateNewMove() override;
	};

public:
	UCHCharacterMovementComponent();

	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	virtual float GetMaxSpeed() const override;

	UFUNCTION(BlueprintCallable)
	void SprintPressed();

	UFUNCTION(BlueprintCallable)
	void SprintReleased();

	UFUNCTION(BlueprintCallable)
	void CrouchPressed();

protected:
	virtual void InitializeComponent() override;
	
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
	
private:
	UPROPERTY(Transient)
	ACHCharacterBase* CHCharacterOwner;
	
	// Control Flags
	bool Safe_bWantsToSprint;
	bool Safe_bPrevWantsToCrouch;
	
	UPROPERTY(EditDefaultsOnly)
	float MaxSprintSpeed = 850.0f;
};
