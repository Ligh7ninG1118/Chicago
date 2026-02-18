// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameplayTagContainer.h"
#include "CHAIController.generated.h"

class ACHAICharacter;
struct FAIStimulus;
class UStateTreeAIComponent;
class UAIPerceptionComponent;


DECLARE_DELEGATE_TwoParams(FAIPerceptionUpdatedDelegate, AActor*, const FAIStimulus&);
DECLARE_DELEGATE_OneParam(FAIPerceptionForgottenDelegate, AActor*);

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMasks))
enum class ECoverPeekFlag : uint8
{
	NONE	= 0,
	LEFT	= 1 << 0,	// 1
	RIGHT	= 1 << 1,	// 2
	UP		= 1 << 2	// 4
};

ENUM_CLASS_FLAGS(ECoverPeekFlag)

USTRUCT(BlueprintType)
struct FCoverData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector AnchorPosition = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector PeekOutPosition = FVector::ZeroVector;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	AActor* CoverActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(Bitmask, BitmaskEnum="ECoverPeekFlag"))
	ECoverPeekFlag PeekOptions = ECoverPeekFlag::NONE;

	void Reset()
	{
		CoverActor = nullptr;
		AnchorPosition = FVector::ZeroVector;
		PeekOutPosition = FVector::ZeroVector;
		PeekOptions = ECoverPeekFlag::NONE;
	}

	bool IsSet() const
	{
		return CoverActor != nullptr;
	}
};


/**
 * 
 */
UCLASS()
class CHICAGO_API ACHAIController : public AAIController
{
	GENERATED_BODY()

	ACHAICharacter* AICharacter;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStateTreeAIComponent* StateTreeAI;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UAIPerceptionComponent* AIPerception;

protected:
	UPROPERTY(EditAnywhere, Category="AI: Gameplay")
	FGameplayTag TeamTag;

	TObjectPtr<AActor> TargetEnemy;

	virtual void OnPossess(APawn* InPawn) override;

	virtual void OnUnPossess() override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void Tick(float DeltaSeconds) override;
	
public:
	ACHAIController();

	void SetCurrentTarget(AActor* Target);

	void ClearCurrentTarget();

	AActor* GetCurrentTarget() const { return TargetEnemy; };
	
	FAIPerceptionUpdatedDelegate OnAIPerceptionUpdated;
	
	FAIPerceptionForgottenDelegate OnAIPerceptionForgotten;
		
protected:
	UFUNCTION()
	void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION()
	void OnPerceptionForgotten(AActor* Actor);

	
	// Cover
public:
	UFUNCTION()
	FVector CalculateCoverAnchor(FVector RoughPosition);
	
	UFUNCTION(BlueprintCallable, Category = "AI|Cover")
	void ClearCoverData();
	
	void LeanOut();

	void LeanBack();
	
protected:
	void ProbingCoverCorner(const FHitResult& HitResult, FVector RoughPosition);

	bool HasProbingHitCover(FVector StartPosition, FVector EndPosition, const AActor* CoverActor);

	// How much distance should AI stand away from cover
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Cover")
	float CoverAnchorAwayOffset = 15.0f;

	// How much distance should AI stand aside from cover's edge
	//TODO: Ideally this should dynamically derive from capsule's radius
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Cover")
	float CoverAnchorSideOffset = 45.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Cover")
	float WallProbingDistance = 200.0f;

	// Iteration depth for calculating cover's edge using binary search
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Cover")
	uint8 WallProbingIterationDepth = 5;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Cover")
	FCoverData CurrentCoverData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Cover")
	bool bShouldShowDebugInfo = false;

	// Weapon
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat", meta=(ClampMin="0.05", UIMin="0.05"))
	float DefaultBurstDuration = 0.5f;

	UFUNCTION(BlueprintCallable, Category="Combat")
	bool FireAtCurrentTarget();

	UFUNCTION(BlueprintCallable, Category="Combat")
	void StopFiring();
	
protected:
	FTimerHandle TimerHandle_StopFire;
	
};
