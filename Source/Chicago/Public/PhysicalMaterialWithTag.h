// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "PhysicalMaterialWithTag.generated.h"

/**
 * 
 */
UCLASS()
class CHICAGO_API UPhysicalMaterialWithTag : public UPhysicalMaterial
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayProperties)
	FGameplayTag AttachedTag;
	
};
