// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackSequence.h"
#include "AbilitySystemGlobals.h"
#include "AttackComponent.h"
#include "ControlFlow.h"
#include "ControlFlowManager.h"
#include "KrzyweKarty2/KrzyweKartySettings.h"
#include "KrzyweKarty2/Characters/KKCharacter.h"

UAttackSequence::UAttackSequence(): AttackContext()
{
	for (EAttackStage AttackStage : TEnumRange<EAttackStage>())
	{
		AttackPipeline.Add(AttackStage, FAttackComponents());
	}
}

void UAttackSequence::BeginAttack()
{
	check(AttackContext.IsValid());
	
	AssembleAttackPipeline(GetAttacker());
	AssembleAttackPipeline(GetVictim());

	FControlFlow& Flow = FControlFlowStatics::Create(this, "AttackFlow")
		.QueueStep("PreAttack",		this, 	&UAttackSequence::ExecuteAttackStage, EAttackStage::PreAttack)
		.QueueStep("InitDamage",		this,	&UAttackSequence::InitDamage)
		.QueueStep("Before",			this,	&UAttackSequence::ExecuteAttackStage, EAttackStage::Before)
		.QueueStep("ExecuteDamage",	this, 	&UAttackSequence::ExecuteDamage)
		.QueueStep("After",			this, 	&UAttackSequence::ExecuteAttackStage, EAttackStage::After)
		.QueueStep("CheckDeath",		this, 	&UAttackSequence::CheckVictimDeath);

	AttackFlow = Flow.AsShared();
	AttackFlow->ExecuteFlow();
}

void UAttackSequence::StopExecution()
{
	AttackFlow->CancelFlow();
}

void UAttackSequence::ModifyDamage(int32 InDamage, EGameplayModOp::Type ModificationType)
{
	switch (ModificationType)
	{
	case EGameplayModOp::Additive:
		Damage += InDamage;
		break;
	case EGameplayModOp::Override:
		Damage = InDamage;
		break;
	case EGameplayModOp::Multiplicitive:
		Damage *= InDamage;
		break;
	case EGameplayModOp::Division:
		if(InDamage != 0)
		{
			Damage /= InDamage;
		}
		break;
	default:
		break;
	}
}

void UAttackSequence::AssembleAttackPipeline(const AKKCharacter* Character)
{
	Character->ForEachComponent<UAttackComponent>(false, [this, &Character](UAttackComponent* AttackComponent)
	{
		if(AttackContext.DoesCharacterMatchRole(Character, AttackComponent->AttackRole) &&
			AttackComponent->MatchesAttackType(AttackContext.AttackType, AttackContext.AbilityIndex))
		{
			AttackPipeline[AttackComponent->AttackStage].AttackComponents.Add(AttackComponent);
		}
	});
}

void UAttackSequence::ExecuteAttackStage(EAttackStage Stage)
{
	for (const UAttackComponent* AttackComponent : AttackPipeline[Stage].AttackComponents)
	{
		AttackComponent->AttackStageExecution.Broadcast(this);
	}
}

void UAttackSequence::InitDamage()
{
	if(Damage == -1)
	{
		ModifyDamage(GetAttacker()->GetStrength(), EGameplayModOp::Override);
	}
}

void UAttackSequence::ExecuteDamage()
{
	const float Level = GetAbilityIndex();
	const UGameplayEffect* AttackGameplayEffect = UKrzyweKartySettings::GetAttackGameplayEffect();
	UAbilitySystemComponent* AbilitySystemComponent = GetAttacker()->GetAbilitySystemComponent();

	FGameplayEffectSpec Spec = FGameplayEffectSpec(AttackGameplayEffect, AbilitySystemComponent->MakeEffectContext(), Level);
	Spec.SetSetByCallerMagnitude("Damage", Damage);

	AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(Spec, GetVictim()->GetAbilitySystemComponent());
}

void UAttackSequence::CheckVictimDeath()
{
	if(AKKCharacter* Character = GetVictim(); Character->GetHealth() <= 0)
	{
		Character->OnCharacterDeath.Broadcast();
		Character->Destroy();
	}
}


UAttackSequence* UAttackSequence::BeginDefaultAttackSequence(UObject* WorldContextObject, const AKKCharacter* Attacker, const AKKCharacter* Victim)
{
	check(0);
	return nullptr;
}

UAttackSequence* UAttackSequence::BeginAbilityAttackSequence(UObject* WorldContextObject, const AKKCharacter* Attacker, const AKKCharacter* Victim, int32 InAbilityIndex, int32 InDamage)
{
	check(0);
	return nullptr;
}

DEFINE_FUNCTION(UAttackSequence::execBeginDefaultAttackSequence)
{
	P_GET_OBJECT(UObject, WorldContextObject);
	P_GET_OBJECT(AKKCharacter, Attacker);
	P_GET_OBJECT(AKKCharacter, Victim);
	P_FINISH;
	
	P_NATIVE_BEGIN;
	const FAttackContext AttackContext = FAttackContext(Attacker, Victim, EAttackType::DefaultAttack);
	
	UAttackSequence* AttackSequence = NewObject<UAttackSequence>(WorldContextObject);
	AttackSequence->AttackContext = AttackContext;
	AttackSequence->BeginAttack();

	*static_cast<UAttackSequence**>(RESULT_PARAM) = AttackSequence;
	P_NATIVE_END;
}

DEFINE_FUNCTION(UAttackSequence::execBeginAbilityAttackSequence)
{
	P_GET_OBJECT(UObject, WorldContextObject);
	P_GET_OBJECT(AKKCharacter, Attacker);
	P_GET_OBJECT(AKKCharacter, Victim);
	PARAM_PASSED_BY_VAL(InAbilityIndex, FIntProperty, int32);
	PARAM_PASSED_BY_VAL(InDamage, FIntProperty, int32);
	P_FINISH;
	
	P_NATIVE_BEGIN;
	const FAttackContext AttackContext = FAttackContext(Attacker, Victim, EAttackType::Ability, InAbilityIndex);
	
	UAttackSequence* AttackSequence = NewObject<UAttackSequence>(WorldContextObject);
	AttackSequence->AttackContext = AttackContext;
	AttackSequence->Damage = InDamage;
	AttackSequence->BeginAttack();

	*static_cast<UAttackSequence**>(RESULT_PARAM) = AttackSequence;
	P_NATIVE_END;
}
