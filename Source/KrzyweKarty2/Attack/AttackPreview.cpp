// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackPreview.h"
#include "AttackSequence.h"
#include "DamageCalculationExecution.h"

#include "KrzyweKarty2/AbilitySystem/Attributes/KKAttributeSet.h"
#include "KrzyweKarty2/Characters/KKCharacter.h"

void UAttackPreview::BeginPreviewAttack()
{
	check(AttackContext.IsValid())

	OriginalCharacterWidget = GetVictim()->GetCharacterWidget();
	// Make copies of attacker and victim
	// Capture important stats
	// Use captured stats in attack calculation
	// Create new character widget and assign it to the victim
	// When preview ends restore original character widget
}

void UAttackPreview::ExecuteDamage()
{
	const int32 Result = UDamageCalculationExecution::CalculateDamage(Damage, GetVictim()->GetDefence());
}


UAttackSequence* UAttackPreview::BeginPreviewAttackSequence(UObject* WorldContextObject, const AKKCharacter* Attacker, const AKKCharacter* Victim, EAttackType AttackType, int32 InAbilityIndex, int32 InDamage)
{
	check(0);
	return nullptr;
}

DEFINE_FUNCTION(UAttackPreview::execBeginPreviewAttackSequence)
{
	P_GET_OBJECT(UObject, WorldContextObject);
	P_GET_OBJECT(AKKCharacter, Attacker);
	P_GET_OBJECT(AKKCharacter, Victim);
	PARAM_PASSED_BY_VAL(AttackType, FEnumProperty, EAttackType);
	PARAM_PASSED_BY_VAL(InAbilityIndex, FIntProperty, int32);
	PARAM_PASSED_BY_VAL(InDamage, FIntProperty, int32);
	P_FINISH;
	
	P_NATIVE_BEGIN;
	
	// Create copies of the attacker and victim to avoid modifying the original objects
	AKKCharacter* AttackerCopy = CreateCharacterCopy(Attacker, WorldContextObject);
	AKKCharacter* VictimCopy = CreateCharacterCopy(Victim, WorldContextObject);

	
	const FAttackContext AttackContext = FAttackContext(AttackerCopy, VictimCopy, AttackType, InAbilityIndex);
	
	UAttackPreview* AttackPreviewSequence = NewObject<UAttackPreview>(WorldContextObject);
	//TODO

	
	*static_cast<UAttackSequence**>(RESULT_PARAM) = AttackPreviewSequence;
	P_NATIVE_END;
}

AKKCharacter* UAttackPreview::CreateCharacterCopy(const AKKCharacter* Character, UObject* WorldContextObject)
{
	AKKCharacter* CharacterCopy = DuplicateObject(Character, WorldContextObject);
	UKKAttributeSet* AttributeSetCopy = DuplicateObject(Character->AttributeSet, WorldContextObject);
	
	CharacterCopy->AbilitySystemComponent->RegisterComponent();
	CharacterCopy->AttributeSet = AttributeSetCopy;
	CharacterCopy->AbilitySystemComponent->AddSpawnedAttribute(AttributeSetCopy);
	CharacterCopy->InitializeAbilitySystemComponent();

	return CharacterCopy;
}


