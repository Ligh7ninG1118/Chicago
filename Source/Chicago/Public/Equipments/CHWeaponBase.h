// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "CHWeaponBase.generated.h"

class UGameplayEffect;
class IWeaponHolder;
class USkeletalMeshComponent;
class UAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAmmoUpdateDelegate, int32, AmmoInMag, int32, AmmoInReserve);

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FTargetHitDelegate, )

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

	UPROPERTY(BlueprintAssignable)
	FAmmoUpdateDelegate OnAmmoUpdate;
	
protected:
	IWeaponHolder* WeaponHolder;

	UAbilitySystemComponent* OwnerASCRef;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay: Message")
	FGameplayTag HitMessageChannelTag;
	
	UPROPERTY(EditDefaultsOnly, Category="Weapon: Ammo")
	int32 MaxMagazineSize = 30;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Weapon: Ammo")
	int32 CurrentAmmoInMagazine = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon: Ammo")
	FGameplayTag AmmoType;	

	//FTimerHandle ReloadTimerHandle;

	bool bIsReloading = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon: Gameplay")
	bool bIsOpenBolt;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon: Gameplay" )
	float FireRatePerMin;

	float FireRateInterval;
	
	FTimerHandle FireRateTimerHandle;

	float TimeOfLastShot = 0.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon: Gameplay" )
	bool bIsFullAuto;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon: Gameplay" )
	float HitScanMaxRange;
	
	bool bIsFiring = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon: Gameplay" )
	float DefaultDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon: Gameplay" )
	TMap<FGameplayTag, float> BodyPartDamageModifierMap;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon: Gameplay")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon: Recoil")
	float HipFireSpread;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon: Recoil")
	float VerticalRecoilRangeMin;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon: Recoil")
	float VerticalRecoilRangeMax;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon: Recoil")
	float HorizontalRecoilLeftRange;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon: Recoil")
	float HorizontalRecoilRightRange;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon: Recoil")
	float HeatGainedPerShot = 0.01f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon: Recoil")
	float HeatRecoverDelay = 0.1f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon: Recoil")
	float HeatRecoveredRate = 10.0f;

	//TODO: Expose this for the crosshair widget?
	float CurrentHeat;
	
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

	UFUNCTION(BlueprintCallable)
	virtual void Reload();

protected:
	virtual void Fire();

	virtual void ShootHitScan();

	virtual bool CanFire();

	UFUNCTION()
	virtual void FinishReloadByNotify(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);

	virtual void FinishReload();
	
	virtual bool CanReload();
public:
	UFUNCTION(BlueprintPure, Category="Weapon")
	USkeletalMeshComponent* GetGunMesh() const { return GunMesh; };
};
