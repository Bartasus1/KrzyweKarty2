// Fill out your copyright notice in the Description page of Project Settings.


#include "KKAttributeSet.h"
#include "KrzyweKarty2/Characters/CharacterStructs.h"
#include "Net/UnrealNetwork.h"

UKKAttributeSet::UKKAttributeSet(): MaxCharacterStats(nullptr)
{
}

UKKAttributeSet* UKKAttributeSet::InitFromCharacterStatistics(const FCharacterStats& CharacterStats)
{
	MaxCharacterStats = &CharacterStats;

	InitHealth(MaxCharacterStats->Health);
	InitMana(MaxCharacterStats->Mana);
	InitDefence(MaxCharacterStats->Defence);
	InitStrength(MaxCharacterStats->Strength);

	AttributeToMaxStatMap.Add(GetHealthAttribute(), MaxCharacterStats->Health);
	AttributeToMaxStatMap.Add(GetManaAttribute(), MaxCharacterStats->Mana);
	AttributeToMaxStatMap.Add(GetDefenceAttribute(), MaxCharacterStats->Defence);
	AttributeToMaxStatMap.Add(GetStrengthAttribute(), MaxCharacterStats->Strength);

	return this;
}

int32 UKKAttributeSet::GetMaxValueForAttribute(const FGameplayAttribute& GameplayAttribute) const
{
	if (const int32* FoundValue = AttributeToMaxStatMap.Find(GameplayAttribute))
	{
		return *FoundValue;
	}

	return 1; // hack for when dividing by max value (don't divide by 0)
}

void UKKAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if(!MaxCharacterStats)
	{
		return;
	}

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp<float>(NewValue, 0.f, MaxCharacterStats->Health);
	}
	if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp<float>(NewValue, 0.f, MaxCharacterStats->Mana);
	}
	if (Attribute == GetDefenceAttribute())
	{
		NewValue = FMath::Clamp<float>(NewValue, 0.f, MaxCharacterStats->Defence);
	}
	if (Attribute == GetStrengthAttribute())
	{
		NewValue = FMath::Clamp<float>(NewValue, 0.f, MaxCharacterStats->Strength);
	}
}

void UKKAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UKKAttributeSet, Health, OldHealth);
}

void UKKAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UKKAttributeSet, Mana, OldMana);
}

void UKKAttributeSet::OnRep_Defence(const FGameplayAttributeData& OldDefence)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UKKAttributeSet, Defence, OldDefence);
}

void UKKAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldStrength)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UKKAttributeSet, Strength, OldStrength);
}

void UKKAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UKKAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UKKAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UKKAttributeSet, Defence, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UKKAttributeSet, Strength, COND_None, REPNOTIFY_Always);
}
