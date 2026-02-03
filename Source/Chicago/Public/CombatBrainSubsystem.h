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
	
	// Combatant Management
public:
	UFUNCTION(BlueprintCallable, Category = "Combat Brain")
	void RegisterCombatant(AActor* AIAgent);

	UFUNCTION(BlueprintCallable, Category = "Combat Brain")
	void UnregisterCombatant(AActor* AIAgent);

protected:
	UPROPERTY()
	TArray<AActor*> RegisteredCombatants;
	
private:
	UFUNCTION()
	void OnCombatantDestroyed(AActor* DestroyedAgent);

	// Attack Token
public:
	UFUNCTION(BlueprintCallable, Category = "Combat Brain")
	bool TryRequestAttackToken(AActor* AIAgent, int32 TokenCost);

	UFUNCTION(BlueprintCallable, Category = "Combat Brain")
	void ReturnAttackToken(AActor* AIAgent);

	UFUNCTION(BlueprintPure, Category = "Combat Brain")
	int32 GetCurrentAttackTokenUsage() const;
	
protected:
	UPROPERTY()
	TMap<AActor*, int32> ActiveAttackTokenUsers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Brain")
	int32 MaxAttackToken = 10;


	// Flank Token
public:
	UFUNCTION(BlueprintCallable, Category = "Combat Brain")
	bool TryRequestFlankToken(AActor* AIAgent, int32 TokenCost);

	UFUNCTION(BlueprintCallable, Category = "Combat Brain")
	void ReturnFlankToken(AActor* AIAgent);

	UFUNCTION(BlueprintPure, Category = "Combat Brain")
	int32 GetCurrentFlankTokenUsage() const;

protected:
	UPROPERTY()
	TMap<AActor*, int32> ActiveFlankTokenUsers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Brain")
	int32 MaxFlankToken = 1;
	
	// Cover Selection
public:
	UFUNCTION(BlueprintCallable, Category = "Combat Brain")
	FVector RequestCoverPosition(const FVector& CurrentSelfPosition);

protected:
	UFUNCTION(BlueprintCallable, Category = "Combat Brain")
	FVector CalculateNextCoverPosition(int32 SliceIndex, const FVector& CurrentSelfPosition) const;

	UFUNCTION(BlueprintCallable, Category = "Combat Brain")
	void UpdateSliceWeight(int32 SliceIndex, bool bIsAdding);

	UFUNCTION(BlueprintCallable, Category = "Combat Brain")
	int32 GetSliceIndexFromWorldPosition(const FVector& WorldPosition);

	UFUNCTION(BlueprintCallable, Category = "Combat Brain")
	void OccupiedSlice(AActor* Combatant, int32 NewIndex);

	UFUNCTION(BlueprintCallable, Category = "Combat Brain")
	void ClearOccupiedSlice(AActor* Combatant);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Brain")
	int32 SliceNum = 12;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Brain")
	TArray<int32> CurrentSliceWeights;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Brain")
	TArray<int32> SliceWeightLUT;

	UPROPERTY()
	TMap<AActor*, int32> CombatantOccupiedSlice;
	
	// Utility
public:
	UFUNCTION(BlueprintPure, Category = "Combat Brain")
	FVector GetPlayerPosition() const;

	UFUNCTION(BlueprintCallable, Category = "Combat Brain")
	FVector GetCombatantAveragePosition() const;

protected:
	void DebugDrawInfos();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Brain")
	bool bShouldDrawDebugInfo = false;
};
