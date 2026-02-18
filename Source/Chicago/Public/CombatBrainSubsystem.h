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
	// AI register themself when entering combat
	UFUNCTION(BlueprintCallable, Category = "Combat Brain|Combatants")
	void RegisterCombatant(AActor* AIAgent);

	// AI unregister on death or (unlikely) exiting combat
	UFUNCTION(BlueprintCallable, Category = "Combat Brain|Combatants")
	void UnregisterCombatant(AActor* AIAgent);

protected:
	UPROPERTY()
	TArray<AActor*> RegisteredCombatants;
	
private:
	// Bind to AI's OnDestroyed event
	UFUNCTION()
	void OnCombatantDestroyed(AActor* DestroyedAgent);

	// Attack Token
public:
	// If we have enough attack token to spare
	UFUNCTION(BlueprintCallable, Category = "Combat Brain|Attack Token")
	bool TryRequestAttackToken(AActor* AIAgent, int32 TokenCost);

	// Return attack token after AI's attack is done
	UFUNCTION(BlueprintCallable, Category = "Combat Brain|Attack Token")
	void ReturnAttackToken(AActor* AIAgent);

	// Calculate current token usage
	UFUNCTION(BlueprintPure, Category = "Combat Brain|Attack Token")
	int32 GetCurrentAttackTokenUsage() const;
	
protected:
	UPROPERTY()
	TMap<AActor*, int32> ActiveAttackTokenUsers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Brain|Attack Token", meta = (ClampMin = "0"))
	int32 MaxAttackToken = 10;


	// Flank Token
public:
	// If we have enough flank token to spare
	UFUNCTION(BlueprintCallable, Category = "Combat Brain|Flank Token")
	bool TryRequestFlankToken(AActor* AIAgent, int32 TokenCost);

	// Return flank token after AI done flanking
	UFUNCTION(BlueprintCallable, Category = "Combat Brain|Flank Token")
	void ReturnFlankToken(AActor* AIAgent);

	// Calculate current token usage
	UFUNCTION(BlueprintPure, Category = "Combat Brain|Flank Token")
	int32 GetCurrentFlankTokenUsage() const;

protected:
	UPROPERTY()
	TMap<AActor*, int32> ActiveFlankTokenUsers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Brain|Flank Token", meta = (ClampMin = "0"))
	int32 MaxFlankToken = 1;
	
	// Cover Selection
public:
	UFUNCTION(BlueprintCallable, Category = "Combat Brain|Cover")
	FVector RequestCoverPosition(AActor* Combatant);
	
protected:
	// Calculate the "Cover Position" for EQS from Slice index (direction against the player) and Self Position (distance)
	UFUNCTION(BlueprintCallable, Category = "Combat Brain|Cover")
	FVector CalculateNextCoverPosition(int32 SliceIndex, const FVector& CurrentSelfPosition) const;

	// Modify single slice's weight
	UFUNCTION(BlueprintCallable, Category = "Combat Brain|Cover")
	void ModifySliceWeight(int32 SliceIndex, bool bIsAdding);

	// Upon player's position change, update all slice's weight from combatants
	UFUNCTION(BlueprintCallable, Category = "Combat Brain|Cover")
	void UpdateAllSlicesWeight();

	// Calculate which Slice Index it belongs to from World Position
	UFUNCTION(BlueprintCallable, Category = "Combat Brain|Cover")
	int32 GetSliceIndexFromWorldPosition(const FVector& WorldPosition);

	// Remove Combatant's previous weight and add it to a new slice
	UFUNCTION(BlueprintCallable, Category = "Combat Brain|Cover")
	void ChangeCombatantOccupiedSlice(AActor* Combatant, int32 NewIndex);

	// Remove Combatant's weight and entry in the array (used when dead, exiting fight etc.)
	UFUNCTION(BlueprintCallable, Category = "Combat Brain|Cover")
	void ClearCombatantOccupiedSlice(AActor* Combatant);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Brain|Cover")
	int32 SliceNum = 12;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Brain|Cover")
	TArray<int32> CurrentSliceWeights;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Brain|Cover")
	TArray<int32> SliceWeightLUT;
	
	UPROPERTY()
	TMap<AActor*, int32> CombatantOccupiedSlice;

	// Utility
public:
	UFUNCTION(BlueprintPure, Category = "Combat Brain|Utility")
	FVector GetPlayerPosition() const;

	UFUNCTION(BlueprintCallable, Category = "Combat Brain|Utility")
	void DebugDrawSlices();

	UFUNCTION(BlueprintCallable, Category = "Combat Brain|Utility")
	void DebugPrintCombatantStates();
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Brain|Utility")
	bool bShouldDrawDebugInfo = true;

	// If player's current position vs cached position exceeds this value, reevaluate all slice weight when requested a new flank position
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Brain|Utility")
	float MaxDistToReevaluateWeight = 500.0f;
	
	FVector CachedPlayerPosition = FVector::ZeroVector;

private:
	int32 WrapSliceIndex(int32 Index) const;
	int32 GetSignedShortestSliceDelta(int32 From, int32 To) const;
};
