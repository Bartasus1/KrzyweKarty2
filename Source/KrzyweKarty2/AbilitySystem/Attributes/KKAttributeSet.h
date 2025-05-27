// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"

#include "KKAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName) \

struct FCharacterStats;

UCLASS()
class KRZYWEKARTY2_API UKKAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:

	UKKAttributeSet();

	UKKAttributeSet* InitFromCharacterStatistics(const FCharacterStats& CharacterStats);

	int32 GetMaxValueForAttribute(const FGameplayAttribute& GameplayAttribute) const;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UKKAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Mana)
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UKKAttributeSet, Mana)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Defence)
	FGameplayAttributeData Defence;
	ATTRIBUTE_ACCESSORS(UKKAttributeSet, Defence)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Strength)
	FGameplayAttributeData Strength;
	ATTRIBUTE_ACCESSORS(UKKAttributeSet, Strength)

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);

	UFUNCTION()
	virtual void OnRep_Mana(const FGameplayAttributeData& OldMana);

	UFUNCTION()
	virtual void OnRep_Defence(const FGameplayAttributeData& OldDefence);

	UFUNCTION()
	virtual void OnRep_Strength(const FGameplayAttributeData& OldStrength);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	const FCharacterStats* MaxCharacterStats;
	
	TMap<FGameplayAttribute, int32> AttributeToMaxStatMap;
};
