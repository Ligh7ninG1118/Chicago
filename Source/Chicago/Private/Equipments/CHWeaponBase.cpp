// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipments/CHWeaponBase.h"
#include "Equipments/WeaponHolder.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Camera/CameraComponent.h"
#include "Character/CHCharacterBase.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Physics/PhysicalMaterialWithTag.h"


// Sets default values
ACHWeaponBase::ACHWeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	
	GunMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Gun Mesh"));
	GunMesh->SetupAttachment(RootComponent);

	MagazineMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Magazine Mesh"));
	MagazineMesh->SetupAttachment(GunMesh, "Magazine");
	MagazineMesh->SetCollisionProfileName("NoCollision");
}

// Called when the game starts or when spawned
void ACHWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	WeaponHolder = Cast<IWeaponHolder>(GetOwner());
	WeaponHolder->AttachWeaponMeshes(this);
	
	CurrentAmmoInMagazine = MaxMagazineSize;
	
	FireRateInterval = 60.0f / FireRatePerMin;
}

void ACHWeaponBase::StartFiring()
{
	bIsFiring = true;

	const float TimeSinceLastShot = GetWorld()->GetTimeSeconds() - TimeOfLastShot;

	if (TimeSinceLastShot > FireRateInterval)
	{
		Fire();
	}
	else
	{
		if (bIsFullAuto)
		{
			GetWorld()->GetTimerManager().SetTimer(FireRateTimer, this, &ACHWeaponBase::Fire, TimeSinceLastShot, false);
		}
	}
	
}

void ACHWeaponBase::StopFiring()
{
	bIsFiring = false;
	GetWorld()->GetTimerManager().ClearTimer(FireRateTimer);
}

void ACHWeaponBase::Fire()
{
	if (!CanFire())
		return;
	
	ShootHitScan();

	TimeOfLastShot = GetWorld()->GetTimeSeconds();
	
	CurrentAmmoInMagazine--;

	if (bIsFullAuto)
	{
		GetWorld()->GetTimerManager().SetTimer(FireRateTimer, this, &ACHWeaponBase::Fire, FireRateInterval, false);
	}
	else
	{
		//TODO: Semi-Auto
		//GetWorld()->GetTimerManager().SetTimer(RefireTimer, this, &AShooterWeapon::FireCooldownExpired, RefireRate, false);
	}

	FVector2f RecoilVector(FMath::RandRange(-0.2f, 0.2f), -FMath::RandRange(0.3f, 0.5f));
	
	WeaponHolder->HandleWeaponRecoil(RecoilVector);
	
	if (FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	if (GunFireAnimation != nullptr)
	{
		UAnimInstance* AnimInstance = GunMesh->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(GunFireAnimation);
		}
	}

	if (FirstPersonFireAnimation != nullptr)
	{
		WeaponHolder->PlayFiringMontage(FirstPersonFireAnimation);
	}

	if (AmmoCasingClass != nullptr)
	{
		FTransform CasingSpawnTransform = GunMesh->GetSocketTransform(EjectionPortSocketName);
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot;
		
		auto* Casing = GetWorld()->SpawnActor<AActor>(AmmoCasingClass, CasingSpawnTransform, SpawnParams);
		//TODO: Use specific class for this, getting the casing mesh, and ignore actor (player character) when moving 
	}
	
}

void ACHWeaponBase::ShootHitScan()
{
	UCameraComponent* PlayerCamera = WeaponHolder->GetFiringComponent();

	FVector MuzzlePos = PlayerCamera->GetComponentLocation();
	FVector ShootDir = PlayerCamera->GetForwardVector();
	FVector EndPos = MuzzlePos + ShootDir * WeaponMaxRange;
	FCollisionObjectQueryParams ObjectQueryParams;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.bReturnPhysicalMaterial = true;
	
	FHitResult HitResult;
	//TODO: Replace with multiple when finished with bullet pen
	bool bHasHit = GetWorld()->LineTraceSingleByProfile(HitResult, MuzzlePos, EndPos, FName("Projectile"), QueryParams);
	if (bHasHit)
	{
		if (auto* HitActor = Cast<ACHCharacterBase>(HitResult.GetActor()))
		{
			UAbilitySystemComponent* ASC = HitActor->GetAbilitySystemComponent();
			float finalDamage = -WeaponDefaultDamage;
			
			if (auto* PhysMatWithTag = Cast<UPhysicalMaterialWithTag>(HitResult.PhysMaterial))
			{
				if (BodyPartDamageModifierMap.Contains(PhysMatWithTag->AttachedTag))
				{
					finalDamage *= BodyPartDamageModifierMap[PhysMatWithTag->AttachedTag];
				}
			}

			FGameplayEffectSpecHandle DamageEffectSpecHandle =
				UAbilitySystemBlueprintLibrary::MakeSpecHandleByClass(DamageEffectClass, nullptr, nullptr, 1.0f);

			DamageEffectSpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("GAS.EffectData.Damage")), finalDamage);

			ASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
		}
	}
	
}

bool ACHWeaponBase::CanFire()
{
	if (!bIsFiring)
		return false;
	
	if (WeaponHolder == nullptr)
		return false;

	if (CurrentAmmoInMagazine <= 0)
		return false;

	return true;
}

void ACHWeaponBase::Reload()
{
	if (!CanReload())
		return;

	float ReloadTime = WeaponHolder->PlayReloadMontage(FirstPersonReloadAnimation);

	FTimerHandle ReloadHandle;
	GetWorld()->GetTimerManager().SetTimer(ReloadHandle, this, &ACHWeaponBase::FinishReload, ReloadTime, false);

	if (GunReloadAnimation != nullptr)
	{
		UAnimInstance* AnimInstance = GunMesh->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(GunReloadAnimation);
		}
	}
	
	//TODO: Broadcast event to update UI hud
}

void ACHWeaponBase::FinishReload()
{
	int32 CurrentReserveAmmo = 500;
	
	int32 RequestedAmmo = MaxMagazineSize - CurrentAmmoInMagazine;
	if (CurrentAmmoInMagazine > 0 && !bIsOpenBolt)
	{
		RequestedAmmo += 1;
	}

	int32 AmmoDelta = FMath::Min(RequestedAmmo, CurrentReserveAmmo);
	CurrentAmmoInMagazine += AmmoDelta;
	CurrentReserveAmmo -= AmmoDelta;
}

bool ACHWeaponBase::CanReload()
{
	//TODO: Check reserve ammo
	//TODO: Check current magazine 
	
	return true;
}

