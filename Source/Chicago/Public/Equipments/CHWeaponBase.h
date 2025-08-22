// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "CHWeaponBase.generated.h"

class UGameplayEffect;
class IWeaponHolder;
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
	IWeaponHolder* WeaponHolder;

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

	float FireRateInterval;
	
	FTimerHandle FireRateTimer;

	float TimeOfLastShot = 0.0f;
	
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon: Sound")
	USoundBase* FireSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon: Animation")
	UAnimMontage* GunFireAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon: Animation")
	UAnimMontage* FirstPersonFireAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon: Animation")
	UAnimMontage* GunReloadAnimation;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon: Animation")
	UAnimMontage* FirstPersonReloadAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon: Animation")
	UAnimMontage* EquipAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon: Animation")
	UAnimMontage* UnequipAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon: Effects")
	TSubclassOf<AActor> AmmoCasingClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon: Effects")
	FName EjectionPortSocketName;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	UFUNCTION(BlueprintCallable)
	void StartFiring();
	
	UFUNCTION(BlueprintCallable)
	void StopFiring();	

protected:
	virtual void Fire();

	virtual void ShootHitScan();

	virtual bool CanFire();
	
	UFUNCTION(BlueprintCallable)
	virtual void Reload();

	virtual void FinishReload();

	virtual bool CanReload();
public:
	UFUNCTION(BlueprintPure, Category="Weapon")
	USkeletalMeshComponent* GetGunMesh() const { return GunMesh; };
};
