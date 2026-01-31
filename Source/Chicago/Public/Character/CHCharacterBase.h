// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Hittable.h"
#include "Equipments/WeaponHolder.h"
#include "GameFramework/Character.h"
#include "CHCharacterBase.generated.h"

class UCameraComponent;
class UCHInventoryManager;
class UCHCharacterMovementComponent;
class UAbilitySystemComponent;
class UCHAbilitySystemComponent;
class UCHAttributeSetBase;
struct FOnAttributeChangeData;

UCLASS()
class CHICAGO_API ACHCharacterBase : public ACharacter, public IAbilitySystemInterface, public IHittable, public IWeaponHolder
{
	GENERATED_BODY()
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCHInventoryManager* InventoryManager;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCHCharacterMovementComponent* CHCMC;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;
	
public:
	// Sets default values for this character's properties
	ACHCharacterBase(const FObjectInitializer& ObjectInitializer);

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	void AddAbility(TSubclassOf<class UGameplayAbility>& Ability);

	UCHInventoryManager* GetInventoryManager() const {return InventoryManager;}
	
	/** Returns first person camera component **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
	
	UFUNCTION(BlueprintCallable)
	virtual bool IsAlive() const;
	
	virtual void Die();
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	TWeakObjectPtr<UCHAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	UCHAbilitySystemComponent* HardRefASC; 
	
	TWeakObjectPtr<UCHAttributeSetBase> AttributeSetBase;

	UPROPERTY()
	UCHAttributeSetBase* HardRefAttributeSet;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Abilities")
	TArray<TSubclassOf<class UGameplayAbility>> CharacterAbilities;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Abilities")
	TSubclassOf<class UGameplayEffect> DefaultAttributes;

	void AddCharacterAbilities();
	
	virtual void InitializeAttributes();

	FDelegateHandle HealthChangeDelegateHandle;

	virtual void HealthChanged(const FOnAttributeChangeData& Data);

	bool ShouldShowHitEffect_Implementation() const;
	
	void HandleHit_Implementation(const FHitResult& HitResult, const AActor* Instigator, float Damage, float HitForce);

// Weapon

public:
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category ="Weapon")
	TSubclassOf<ACHWeaponBase> InitialWeaponClass;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	ACHWeaponBase* CurrentWeapon;



#pragma region IWeaponHolder Interface
	
public:
	virtual void AttachWeaponMeshes(ACHWeaponBase* Weapon) override;

	virtual void PlayFiringMontage(UAnimMontage* Montage) override;

	virtual float PlayReloadMontage(UAnimMontage* Montage) override;

	virtual void HandleWeaponRecoil(FVector2f Recoil) override;

	virtual float GetMovementAccuracyPenalty() const override;

	virtual UCameraComponent* GetFiringComponent() const override;

	virtual UAnimInstance* GetAnimInstance() const override;
	
#pragma endregion IWeaponHolder Interface
};
