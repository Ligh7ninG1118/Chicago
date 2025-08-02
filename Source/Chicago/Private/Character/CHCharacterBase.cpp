// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/CHCharacterBase.h"
#include "AbilitySystem/CHAbilitySystemComponent.h"
#include "AbilitySystem/CHAttributeSetBase.h"

ACHCharacterBase::ACHCharacterBase(const class FObjectInitializer& ObjectInitializer)
{
}

UAbilitySystemComponent* ACHCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent.Get();
}

bool ACHCharacterBase::IsAlive() const
{
	if (AttributeSetBase.IsValid())
		return AttributeSetBase->GetHealth() > 0.0f;

	return false;
}

// Called when the game starts or when spawned
void ACHCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent.IsValid())
	{
		InitializeAttributes();

		HealthChangeDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			AttributeSetBase->GetHealthAttribute()).AddUObject(this, &ACHCharacterBase::HealthChanged);
		
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

	GEngine->AddOnScreenDebugMessage(37, 2.0f, FColor::Green, FString::Printf(TEXT("Health: %.2f"), Health));

	if (!IsAlive())
	{
		
	}
}




