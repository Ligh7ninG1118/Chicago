// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "CHCharacterBase.generated.h"

class UAbilitySystemComponent;
class UCHAbilitySystemComponent;
class UCHAttributeSetBase;
struct FOnAttributeChangeData;

UCLASS()
class CHICAGO_API ACHCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACHCharacterBase(const FObjectInitializer& ObjectInitializer);

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable)
	virtual bool IsAlive() const;

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
	
	virtual void InitializeAttributes();

	FDelegateHandle HealthChangeDelegateHandle;

	virtual void HealthChanged(const FOnAttributeChangeData& Data);

};
