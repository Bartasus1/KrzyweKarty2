// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "KrzyweKarty2/AbilitySystem/Abilities/CharacterActionGameplayAbility.h"

#include "UObject/Object.h"
#include "CharacterAssetsCreationTool.generated.h"

class UGameplayAbilityBlueprint;
class UCharacterDataAsset;
/**
 * 
 */
USTRUCT()
struct  KRZYWEKARTY2EDITOR_API FCharacterAssetsCreationTool
{
	GENERATED_BODY()
	
	FCharacterAssetsCreationTool()
	{
		CreateCharacterAssetToolEntry();
	}

protected:
	void CreateCharacterAssetToolEntry();
	
	void CreateCharacterAssets(TArray<FString> SelectedFolderPaths);

private:
	UCharacterDataAsset* CreateCharacterDataAsset(FString AssetPath);
	UGameplayAbilityBlueprint* CreateCharacterAbility(FString AssetPath);
	UBlueprint* CreateCharacterBlueprint(FString AssetPath);

	template<typename T>
	UBlueprint* CreateBlueprint(FString AssetPath);

	template<typename T>
	T* CreateAsset(FString AssetPath, UClass* AssetClass, UFactory* AssetFactory);
	
};

