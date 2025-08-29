// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/CHCharacterBase.h"
#include "AbilitySystem/CHAbilitySystemComponent.h"
#include "AbilitySystem/CHAttributeSetBase.h"
#include "AbilitySystem/CHGameplayAbility.h"
#include "Camera/CameraComponent.h"
#include "Equipments/CHInventoryManager.h"

ACHCharacterBase::ACHCharacterBase(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	HardRefASC = CreateDefaultSubobject<UCHAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	HardRefASC->SetIsReplicated(true);
	HardRefASC->SetReplicationMode(EGameplayEffectReplicationMode::Full);
	AbilitySystemComponent = HardRefASC;
	
	HardRefAttributeSet = CreateDefaultSubobject<UCHAttributeSetBase>(TEXT("AttributeSet"));
	AttributeSetBase = HardRefAttributeSet;

	InventoryManager = CreateDefaultSubobject<UCHInventoryManager>(TEXT("InventoryManager"));

	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(GetMesh());
}

UAbilitySystemComponent* ACHCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent.Get();
}

void ACHCharacterBase::AddAbility(TSubclassOf<UGameplayAbility>& Ability)
{
	if (AbilitySystemComponent != nullptr)
	{
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability, 1, INDEX_NONE, this));
	}
}

bool ACHCharacterBase::IsAlive() const
{
	if (AttributeSetBase.IsValid())
		return AttributeSetBase->GetHealth() > 0.0f;

	return false;
}

void ACHCharacterBase::Die()
{
	//Destroy();
}

// Called when the game starts or when spawned
void ACHCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent.IsValid())
	{
		AddCharacterAbilities();
		InitializeAttributes();

		HealthChangeDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			AttributeSetBase->GetHealthAttribute()).AddUObject(this, &ACHCharacterBase::HealthChanged);
		
	}
}

void ACHCharacterBase::AddCharacterAbilities()
{
	if (GetLocalRole() != ROLE_Authority || !AbilitySystemComponent->IsValidLowLevel())
		return;

	for (auto& StartupAbility : CharacterAbilities)
	{
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(StartupAbility, 1, INDEX_NONE, this));
	}
}

void ACHCharacterBase::InitializeAttributes()
{
	if (!AbilitySystemComponent.IsValid())
		return;

	if (!DefaultAttributes)
	{
		UE_LOG(LogTemp, Error, TEXT("%s() missing DefaultAttributes for %s"), *FString(__FUNCTION__), *GetName());
		return;
	}

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	FGameplayEffectSpecHandle NewHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributes, 1.0f, EffectContext);
	if (NewHandle.IsValid())
	{
		FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*NewHandle.Data.Get(), AbilitySystemComponent.Get());
	}
}

void ACHCharacterBase::HealthChanged(const FOnAttributeChangeData& Data)
{
	float Health = Data.NewValue;

	GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("Health: %.2f"), Health));

	if (!IsAlive())
	{
		//TODO: Ensure this only execute once
		Die();
	}
}




