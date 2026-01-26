// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameplayTagContainer.h"
#include "CHAIController.generated.h"

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

USTRUCT(BlueprintType)
struct FCoverData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector AnchorPosition = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	AActor* CoverActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(Bitmask, BitmaskEnum="ECoverPeekFlag"))
	uint8 PeekOptions = 0;

	void Reset()
	{
		CoverActor = nullptr;
		AnchorPosition = FVector::ZeroVector;
		PeekOptions = 0;
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStateTreeAIComponent* StateTreeAI;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UAIPerceptionComponent* AIPerception;

protected:
	UPROPERTY(EditAnywhere, Category="AI: Gameplay")
	FGameplayTag TeamTag;

	TObjectPtr<AActor> TargetEnemy;

	virtual void OnPossess(APawn* InPawn) override;
	
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
	
protected:
	void ProbingCoverCorner(const FHitResult& HitResult, FVector RoughPosition);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Cover")
	FCoverData CurrentCoverData;
};
