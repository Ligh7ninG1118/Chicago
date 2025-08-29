// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/CHStateTreeUtility.h"
#include "Character/AI/CHAICharacter.h"
#include "StateTreeExecutionContext.h"
#include "Character/AI/CHAIController.h"
#include "Camera/CameraComponent.h"
#include "StateTreeAsyncExecutionContext.h"
#include "Perception/AIPerceptionTypes.h"


bool FStateTreeHasLineOfSightToTargetCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	// ensure the target is valid
	if (!IsValid(InstanceData.Target))
	{
		return !InstanceData.bMustHaveLineOfSight;
	}
	
	// check if the character is facing towards the target
	const FVector TargetDir = (InstanceData.Target->GetActorLocation() - InstanceData.Character->GetActorLocation()).GetSafeNormal();

	const float FacingDot = FVector::DotProduct(TargetDir, InstanceData.Character->GetActorForwardVector());
	const float MaxDot = FMath::Cos(FMath::DegreesToRadians(InstanceData.LineOfSightConeAngle));

	// is the facing outside of our cone half angle?
	if (FacingDot <= MaxDot)
	{
		return !InstanceData.bMustHaveLineOfSight;
	}

	// get the target's bounding box
	FVector CenterOfMass, Extent;
	InstanceData.Target->GetActorBounds(true, CenterOfMass, Extent, false);

	// divide the vertical extent by the number of line of sight checks we'll do
	const float ExtentZOffset = Extent.Z * 2.0f / InstanceData.NumberOfVerticalLineOfSightChecks;

	// get the character's camera location as the source for the line checks
	const FVector Start = InstanceData.Character->GetFirstPersonCameraComponent()->GetComponentLocation();

	// ignore the character and target. We want to ensure there's an unobstructed trace not counting them
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(InstanceData.Character);
	QueryParams.AddIgnoredActor(InstanceData.Target);

	FHitResult OutHit;

	// run a number of vertically offset line traces to the target location
	for (int32 i = 0; i < InstanceData.NumberOfVerticalLineOfSightChecks - 1; ++i)
	{
		// calculate the endpoint for the trace
		const FVector End = CenterOfMass + FVector(0.0f, 0.0f, Extent.Z - ExtentZOffset * i);

		InstanceData.Character->GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, QueryParams);

		// is the trace unobstructed?
		if (!OutHit.bBlockingHit)
		{
			// we only need one unobstructed trace, so terminate early
			return InstanceData.bMustHaveLineOfSight;
		}
	}

	// no line of sight found
	return !InstanceData.bMustHaveLineOfSight;
}

FText FStateTreeHasLineOfSightToTargetCondition::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView,
	const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	return FText::FromString("<b>Has Line of Sight</b>");
}

EStateTreeRunStatus FStateTreeFaceToActorTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	if (Transition.ChangeType == EStateTreeStateChangeType::Changed)
	{
		FInstanceDataType& InsData = Context.GetInstanceData(*this);

		InsData.Controller->SetFocus(InsData.ActorToFaceTowards);
	}

	return EStateTreeRunStatus::Running;
}

void FStateTreeFaceToActorTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	if (Transition.ChangeType == EStateTreeStateChangeType::Changed)
	{
		FInstanceDataType& InsData = Context.GetInstanceData(*this);

		InsData.Controller->ClearFocus(EAIFocusPriority::Gameplay);
	}
}

FText FStateTreeFaceToActorTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup,
	EStateTreeNodeFormatting Formatting) const
{
	return FText::FromString("<b>Face Towards Actor</b>");
}

EStateTreeRunStatus FStateTreeDetectEnemiesTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	if (Transition.ChangeType == EStateTreeStateChangeType::Changed)
	{
		FInstanceDataType& InsData = Context.GetInstanceData(*this);

		InsData.Controller->OnAIPerceptionUpdated.BindLambda(
			[WeakContext = Context.MakeWeakExecutionContext()](AActor* DetectedActor, const FAIStimulus& Stimulus)
			{
				FInstanceDataType* LambdaInsData = WeakContext.MakeStrongExecutionContext().GetInstanceDataPtr<FInstanceDataType>();

				if (!LambdaInsData)
					return;

				bool bDirectLOS = false;

				const FVector ActorLocation = LambdaInsData->Character->GetActorLocation();

				const FVector StimulusDir = (Stimulus.StimulusLocation - ActorLocation).GetSafeNormal();

				const float DirDot = FVector::DotProduct(StimulusDir, LambdaInsData->Character->GetActorForwardVector());
				const float MaxDot = FMath::Cos(FMath::DegreesToRadians(LambdaInsData->DirectLineOfSightConeHalfDeg));

				if (DirDot >= MaxDot)
				{
					FCollisionQueryParams QueryParams;
					QueryParams.AddIgnoredActor(LambdaInsData->Character);
					QueryParams.AddIgnoredActor(DetectedActor);

					FHitResult HitResult;

					bDirectLOS = !LambdaInsData->Character->GetWorld()->LineTraceSingleByChannel(HitResult, ActorLocation, DetectedActor->GetActorLocation(),
						ECC_Visibility, QueryParams);
				}

				if (bDirectLOS)
				{
					LambdaInsData->Controller->SetCurrentTarget(DetectedActor);

					LambdaInsData->TargetActor = DetectedActor;
					
					LambdaInsData->bHasTarget = true;
				}
			}
			);

		//TODO: Forgotten
	}

	return EStateTreeRunStatus::Running;
}

void FStateTreeDetectEnemiesTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	if (Transition.ChangeType == EStateTreeStateChangeType::Changed)
	{
		FInstanceDataType& InsData = Context.GetInstanceData(*this);

		InsData.Controller->OnAIPerceptionUpdated.Unbind();
		InsData.Controller->OnAIPerceptionForgotten.Unbind();
	}
}

FText FStateTreeDetectEnemiesTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup,
	EStateTreeNodeFormatting Formatting) const
{
	return FText::FromString("<b>Detect Enemies</b>");
}
