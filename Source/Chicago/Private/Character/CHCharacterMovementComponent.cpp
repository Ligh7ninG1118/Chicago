// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/CHCharacterMovementComponent.h"

#include "Character/CHCharacterBase.h"

UCHCharacterMovementComponent::FSavedMove_CH::FSavedMove_CH()
{
	Saved_bWantsToSprint = 0;
	Saved_bPrevWantsToCrouch = 0;
}

bool UCHCharacterMovementComponent::FSavedMove_CH::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	FSavedMove_CH* NewCHMove = static_cast<FSavedMove_CH*>(NewMove.Get());
	if (NewCHMove->Saved_bWantsToSprint != Saved_bWantsToSprint)
		return false;

	return FSavedMove_Character::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void UCHCharacterMovementComponent::FSavedMove_CH::Clear()
{
	FSavedMove_Character::Clear();

	Saved_bWantsToSprint = 0;
}

uint8 UCHCharacterMovementComponent::FSavedMove_CH::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();
	if (Saved_bWantsToSprint)
	{
		Result |= FLAG_Custom_0;
	}

	return Result;
}

void UCHCharacterMovementComponent::FSavedMove_CH::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel,
	class FNetworkPredictionData_Client_Character& ClientData)
{
	FSavedMove_Character::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	auto PYMove = Cast<UCHCharacterMovementComponent>(C->GetMovementComponent());

	Saved_bWantsToSprint = PYMove->Safe_bWantsToSprint;
	Saved_bPrevWantsToCrouch = PYMove->Safe_bPrevWantsToCrouch;
}

void UCHCharacterMovementComponent::FSavedMove_CH::PrepMoveFor(ACharacter* C)
{
	FSavedMove_Character::PrepMoveFor(C);

	auto CHMove = Cast<UCHCharacterMovementComponent>(C->GetMovementComponent());

	CHMove->Safe_bWantsToSprint = Saved_bWantsToSprint;
	CHMove->Safe_bPrevWantsToCrouch = Saved_bPrevWantsToCrouch;
}

UCHCharacterMovementComponent::FNetworkPredictionData_Client_CH::FNetworkPredictionData_Client_CH(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr UCHCharacterMovementComponent::FNetworkPredictionData_Client_CH::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_CH);
}

UCHCharacterMovementComponent::UCHCharacterMovementComponent()
{
	NavAgentProps.bCanCrouch = true;
}

FNetworkPredictionData_Client* UCHCharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr)

	if (ClientPredictionData == nullptr)
	{
		UCHCharacterMovementComponent* MutableThis = const_cast<UCHCharacterMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_CH(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.0f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.0f;
	}

	return ClientPredictionData;
}

float UCHCharacterMovementComponent::GetMaxSpeed() const
{
	if (MovementMode == MOVE_Walking && Safe_bWantsToSprint && !IsCrouching())
		return MaxSprintSpeed;
		
	return Super::GetMaxSpeed();
}

void UCHCharacterMovementComponent::SprintPressed()
{
	Safe_bWantsToSprint = true;

	if (bWantsToCrouch)
		bWantsToCrouch = false;
}

void UCHCharacterMovementComponent::SprintReleased()
{
	Safe_bWantsToSprint = false;
}

void UCHCharacterMovementComponent::CrouchPressed()
{
	bWantsToCrouch = !bWantsToCrouch;
}

void UCHCharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();

	CHCharacterOwner = Cast<ACHCharacterBase>(GetOwner());
}

void UCHCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	Safe_bWantsToSprint = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}

void UCHCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

	Safe_bPrevWantsToCrouch = bWantsToCrouch;
}
