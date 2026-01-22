// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatBrainSubsystem.h"

void UCombatBrainSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	RegisteredCombatants.Empty();
	ActiveTokenUsers.Empty();
}

void UCombatBrainSubsystem::Deinitialize()
{
	RegisteredCombatants.Empty();
	ActiveTokenUsers.Empty();
	Super::Deinitialize();
}

void UCombatBrainSubsystem::RegisterCombatant(AActor* AIAgent)
{
	if (!IsValid(AIAgent))
		return;

	if (!RegisteredCombatants.Contains(AIAgent))
	{
		RegisteredCombatants.Add(AIAgent);
		AIAgent->OnDestroyed.AddDynamic(this, &UCombatBrainSubsystem::OnCombatantDestroyed);
	}
}

void UCombatBrainSubsystem::UnregisterCombatant(AActor* AIAgent)
{
	if (!IsValid(AIAgent))
		return;

	if (ActiveTokenUsers.Contains(AIAgent))
		ReturnAttackToken(AIAgent);

	if (RegisteredCombatants.Contains(AIAgent))
	{
		RegisteredCombatants.Remove(AIAgent);
		AIAgent->OnDestroyed.RemoveDynamic(this, &UCombatBrainSubsystem::OnCombatantDestroyed);
	}
	
}

bool UCombatBrainSubsystem::TryRequestAttackToken(AActor* AIAgent, int32 TokenCost)
{
	if (!IsValid(AIAgent) || TokenCost <= 0)
		return false;

	// Reject request if already assigned attack token
	if (ActiveTokenUsers.Contains(AIAgent))
		return false;

	// Not enough tokens
	if ((TokenCost + GetCurrentTokenUsage()) > MaxAttackToken)
		return false;
	
	ActiveTokenUsers.Add(AIAgent, TokenCost);
	return true;
}

void UCombatBrainSubsystem::ReturnAttackToken(AActor* AIAgent)
{
	if (ActiveTokenUsers.Contains(AIAgent))
	{
		ActiveTokenUsers.Remove(AIAgent);
	}
}

int32 UCombatBrainSubsystem::GetCurrentTokenUsage() const
{
	int32 TotalUsage = 0;
	for (const auto& Pair : ActiveTokenUsers)
		TotalUsage += Pair.Value;
	
	return TotalUsage;
}

void UCombatBrainSubsystem::OnCombatantDestroyed(AActor* DestroyedAgent)
{
	UnregisterCombatant(DestroyedAgent);
}
