// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AI/CHAICharacter.h"

#include "Equipments/CHWeaponBase.h"

void ACHAICharacter::StartFiring()
{
	//CurrentWeapon->StartFiring();
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Start firing"));
}

void ACHAICharacter::StopFiring()
{
	//CurrentWeapon->StopFiring();
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Stop firing"));
}
