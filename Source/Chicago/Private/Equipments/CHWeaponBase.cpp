// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipments/CHWeaponBase.h"

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

	CurrentAmmoInMagazine = MaxMagazineSize;
}

void ACHWeaponBase::StartFiring()
{
	bIsFiring = true;
}

void ACHWeaponBase::StopFiring()
{
}

void ACHWeaponBase::Fire()
{
	if (!CanFire())
		return;
	
	ShootHitScan();

	CurrentAmmoInMagazine--;

	if (FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	if (FireAnimation != nullptr)
	{
		UAnimInstance* AnimInstance = GunMesh->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(FireAnimation);
		}
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
	UCameraComponent* PlayerCamera = WeaponOwner->GetComponentByClass<UCameraComponent>();
	if (PlayerCamera == nullptr)
	{
		//TODO: Shooting for AI Character. Could have a better way to handle this
	}

	FVector MuzzlePos = PlayerCamera->GetComponentLocation();
	FVector ShootDir = PlayerCamera->GetForwardVector();
	FVector EndPos = MuzzlePos + ShootDir * WeaponMaxRange;
	FCollisionObjectQueryParams ObjectQueryParams;

	FCollisionQueryParams QueryParams;
	
	FHitResult HitResult;
	//TODO: Replace with multiple when finished with bullet pen
	bool bHasHit = GetWorld()->LineTraceSingleByProfile(HitResult, MuzzlePos, EndPos, FName("Projectile"));
	if (bHasHit)
	{
		if (auto* HitActor = Cast<ACHCharacterBase>(HitResult.GetActor()))
		{
			UAbilitySystemComponent* ASC = HitActor->GetAbilitySystemComponent();
			float finalDamage = WeaponDefaultDamage;
			
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
	if (WeaponOwner == nullptr)
		return false;

	if (CurrentAmmoInMagazine <= 0)
		return false;

	return true;
}

void ACHWeaponBase::Reload()
{
	
}

bool ACHWeaponBase::CanReload()
{

	return true;
}

