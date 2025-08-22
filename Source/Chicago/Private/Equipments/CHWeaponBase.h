// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "CHWeaponBase.generated.h"

class UGameplayEffect;
class ACHCharacterBase;
class USkeletalMeshComponent;

UCLASS(Abstract)
class CHICAGO_API ACHWeaponBase : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* GunMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* MagazineMesh;
	
public:	
	// Sets default values for this actor's properties
	ACHWeaponBase();

protected:
	ACHCharacterBase* WeaponOwner;

	UPROPERTY(EditDefaultsOnly, Category="Weapon: Ammo")
	int32 MaxMagazineSize = 30;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Weapon: Ammo")
	int32 CurrentAmmoInMagazine = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon: Ammo")
	FGameplayTag AmmoType;	

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon: Gameplay")
	bool bIsOpenBolt;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon: Gameplay" )
	float FireRatePerMin;
	
	FTimerHandle FireRateTimer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon: Gameplay" )
	bool bIsFullAuto;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon: Gameplay" )
	float WeaponMaxRange;
	
	bool bIsFiring = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon: Gameplay" )
	float WeaponDefaultDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon: Gameplay" )
	TMap<FGameplayTag, float> BodyPartDamageModifierMap;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon: Gameplay")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon: Effects")
	USoundBase* FireSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon: Effects")
	UAnimMontage* FireAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon: Effects")
	UAnimMontage* ReloadAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon: Effects")
	UAnimMontage* EquipAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon: Effects")
	UAnimMontage* UnequipAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon: Effects")
	TSubclassOf<AActor> AmmoCasingClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon: Effects")
	FName EjectionPortSocketName;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	void StartFiring();

	void StopFiring();

protected:
	UFUNCTION(BlueprintCallable)
	virtual void Fire();

	virtual void ShootHitScan();

	virtual bool CanFire();

	virtual void Reload();

	virtual bool CanReload();
};
