// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterStructs.h"
#include "GameplayTagContainer.h"

#include "Engine/DataAsset.h"
#include "CharacterDataAsset.generated.h"


class UGameplayAbility;

UCLASS(BlueprintType)
class KRZYWEKARTY2_API UCharacterDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText CharacterName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="Fractions"))
	FGameplayTag CharacterFraction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> CharacterCardTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ShowOnlyInnerProperties))
	FCharacterStats CharacterStats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> CharacterAbilityClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(TitleProperty="{AbilityName} --> {AbilityDescription}"), Category="Abilities")
	TArray<FAbilityDescription> ActiveAbilities;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(TitleProperty="{AbilityName} --> {AbilityDescription}"), Category="Abilities")
	TArray<FAbilityDescription> PassiveAbilities;
	
};
