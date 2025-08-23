// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponHolder.generated.h"


class UCameraComponent;
class ACHWeaponBase;

/**
 * 
 */
UINTERFACE()
class UWeaponHolder : public UInterface
{
	GENERATED_BODY()
};


class CHICAGO_API IWeaponHolder
{
	GENERATED_BODY()
public:
	virtual void AttachWeaponMeshes(ACHWeaponBase* Weapon) = 0;

	virtual void PlayFiringMontage(UAnimMontage* Montage) = 0;
	
	//TODO: Combine this with above function?
	virtual float PlayReloadMontage(UAnimMontage* Montage) = 0;

	virtual void HandleWeaponRecoil(FVector2f Recoil) = 0;
	
	//TODO: How do we handle this for AI characters?
	virtual UCameraComponent* GetFiringComponent() const = 0;

	virtual UAnimInstance* GetAnimInstance() const = 0;
};
