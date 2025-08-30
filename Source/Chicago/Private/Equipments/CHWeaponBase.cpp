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
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Messages/FCHActionMessage.h"
#include "Physics/PhysicalMaterialWithTag.h"
#include "Player/CHPlayerCharacter.h"


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
	WeaponHolder->GetAnimInstance()->OnPlayMontageNotifyBegin.AddUniqueDynamic(this, &ACHWeaponBase::FinishReloadByNotify);

	if (auto* PlayerChar = Cast<ACHPlayerCharacter>(GetOwner()))
	{
		OwnerASCRef = PlayerChar->GetAbilitySystemComponent();
	}

	
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
			GetWorld()->GetTimerManager().SetTimer(FireRateTimerHandle, this, &ACHWeaponBase::Fire, TimeSinceLastShot, false);
		}
	}
	
}

void ACHWeaponBase::StopFiring()
{
	bIsFiring = false;
	GetWorld()->GetTimerManager().ClearTimer(FireRateTimerHandle);
}

void ACHWeaponBase::Fire()
{
	if (!CanFire())
		return;
	
	ShootHitScan();

	TimeOfLastShot = GetWorld()->GetTimeSeconds();
	
	CurrentAmmoInMagazine--;
	OnAmmoUpdate.Broadcast(CurrentAmmoInMagazine, 500);

	if (bIsFullAuto)
	{
		GetWorld()->GetTimerManager().SetTimer(FireRateTimerHandle, this, &ACHWeaponBase::Fire, FireRateInterval, false);
	}
	else
	{
		//TODO: Semi-Auto event? Not needed 
	}


	FVector2f RecoilVector(
		FMath::RandRange(-HorizontalRecoilLeftRange, HorizontalRecoilRightRange),
		-FMath::RandRange(VerticalRecoilRangeMin, VerticalRecoilRangeMax));
	
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
	
	if (!OwnerASCRef->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("GAS.Character.Action.Aiming"))))
	{
		ShootDir = FMath::VRandCone(ShootDir, FMath::DegreesToRadians(HipFireSpread));
	}
	
	FVector EndPos = MuzzlePos + ShootDir * HitScanMaxRange;
	FCollisionObjectQueryParams ObjectQueryParams;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.bReturnPhysicalMaterial = true;
	
	DrawDebugLine(GetWorld(), MuzzlePos, EndPos, FColor::Green, false, 5.0f);
	
	FHitResult HitResult;
	//TODO: Replace with multiple when finished with bullet pen
	bool bHasHit = GetWorld()->LineTraceSingleByProfile(HitResult, MuzzlePos, EndPos, FName("Projectile"), QueryParams);
	if (bHasHit)
	{
		DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 10.0f, 8, FColor::Green, false, 5.0f);
		
		if (auto* HitActor = Cast<ACHCharacterBase>(HitResult.GetActor()))
		{
			//IHittable?
			//damage, instigator, shouldShowHitMarker

			FCHActionMessage ActionMsg;
			ActionMsg.Instigator = GetOwner();
			ActionMsg.Target = HitActor;

			UGameplayMessageSubsystem::Get(this).BroadcastMessage(HitMessageChannelTag, ActionMsg);
			
			UAbilitySystemComponent* ASC = HitActor->GetAbilitySystemComponent();
			float finalDamage = -DefaultDamage;
			
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

	if (bIsReloading)
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

	bIsReloading = true;
	
	// Failsafe in case FinishReloadByNotify is not called for any reason
	// The reload still finishes after the whole anim montage finishes
	float ReloadTime = WeaponHolder->PlayReloadMontage(FirstPersonReloadAnimation);
	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, this, &ACHWeaponBase::FinishReload, ReloadTime, false);
	
	if (GunReloadAnimation != nullptr)
	{
		UAnimInstance* AnimInstance = GunMesh->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(GunReloadAnimation);
		}
	}
}

void ACHWeaponBase::FinishReloadByNotify(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload)
{
	if (NotifyName.Compare("MagInserted") == 0)
		FinishReload();
}

void ACHWeaponBase::FinishReload()
{
	// Return if already reloaded by FinishReloadByNotify
	if (!bIsReloading)
		return;

	int32 CurrentReserveAmmo = 500;
	
	int32 RequestedAmmo = MaxMagazineSize - CurrentAmmoInMagazine;
	if (CurrentAmmoInMagazine > 0 && !bIsOpenBolt)
	{
		RequestedAmmo += 1;
	}

	int32 AmmoDelta = FMath::Min(RequestedAmmo, CurrentReserveAmmo);
	CurrentAmmoInMagazine += AmmoDelta;
	CurrentReserveAmmo -= AmmoDelta;
	OnAmmoUpdate.Broadcast(CurrentAmmoInMagazine, 500);

	bIsReloading = false;
}

bool ACHWeaponBase::CanReload()
{
	//TODO: Check reserve ammo

	if (CurrentAmmoInMagazine >= MaxMagazineSize)
		return false;

	if (bIsReloading)
		return false;
	
	return true;
}

