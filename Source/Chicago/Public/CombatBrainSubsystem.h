// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CombatBrainSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class CHICAGO_API UCombatBrainSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Combat Brain")
	void RegisterCombatant(AActor* AIAgent);

	UFUNCTION(BlueprintCallable, Category = "Combat Brain")
	void UnregisterCombatant(AActor* AIAgent);

	UFUNCTION(BlueprintCallable, Category = "Combat Brain")
	bool TryRequestAttackToken(AActor* AIAgent, int32 TokenCost);

	UFUNCTION(BlueprintCallable, Category = "Combat Brain")
	void ReturnAttackToken(AActor* AIAgent);

	UFUNCTION(BlueprintPure, Category = "Combat Brain")
	int32 GetCurrentTokenUsage() const;
	
protected:
	UPROPERTY()
	TArray<AActor*> RegisteredCombatants;

	UPROPERTY()
	TMap<AActor*, int32> ActiveTokenUsers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Brain")
	int32 MaxAttackToken = 10;

private:
	UFUNCTION()
	void OnCombatantDestroyed(AActor* DestroyedAgent);
	
};
