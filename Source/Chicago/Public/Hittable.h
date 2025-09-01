// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Hittable.generated.h"

struct FGameplayEffectSpecHandle;
/**
 * 
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UHittable : public UInterface
{
	GENERATED_BODY()
};


class CHICAGO_API IHittable
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool ShouldShowHitEffect() const;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void HandleHit(const FHitResult& HitResult, const AActor* Instigator, FGameplayEffectSpecHandle& HitEffect, float HitForce);

	//TODO: overload with GameplayEffectSpecHandle
};

