// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterAssetsTools/CharacterAssetsCreationTool.h"
#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "FileHelpers.h"
#include "GameplayAbilitiesBlueprintFactory.h"
#include "GameplayAbilityBlueprint.h"
#include "IAssetTools.h"

#include "AssetRegistry/AssetRegistryModule.h"

#include "Factories/DataAssetFactory.h"

#include "Kismet2/KismetEditorUtilities.h"

#include "KrzyweKarty2/AbilitySystem/Abilities/CharacterActiveGameplayAbility.h"
#include "KrzyweKarty2/Characters/CharacterDataAsset.h"
#include "KrzyweKarty2/Characters/KKCharacter.h"

void FCharacterAssetsCreationTool::CreateCharacterAssetToolEntry()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	ContentBrowserModule.GetAllPathViewContextMenuExtenders().Add(
		FContentBrowserMenuExtender_SelectedPaths::CreateLambda([this](const TArray<FString>& SelectedPaths) -> TSharedRef<FExtender>
		{
			TSharedRef<FExtender> MenuExtender(new FExtender());
			MenuExtender->AddMenuExtension
			(
				"Explore",
				EExtensionHook::After,
				TSharedPtr<FUICommandList>(),
				FMenuExtensionDelegate::CreateLambda([this, SelectedPaths](FMenuBuilder& MenuBuilder)
				{
					MenuBuilder.AddMenuEntry
					(
						TAttribute(FText::FromString("Create Character Assets")),
						TAttribute(FText::FromString("Create all assets what will be needed for proper character creation")),
						FSlateIcon(),
						FExecuteAction::CreateRaw(this, &FCharacterAssetsCreationTool::CreateCharacterAssets, SelectedPaths)
					);
				})
			);
			
			return MenuExtender;
		})
	);
}

void FCharacterAssetsCreationTool::CreateCharacterAssets(TArray<FString> SelectedFolderPaths)
{
	for (FString& Path : SelectedFolderPaths)
	{
		FString FolderName = FPaths::GetBaseFilename(Path); // Content/Fractions/Fraction/Character
		Path += "/" + FolderName; // Content/Fractions/Fraction/Character/Character...File
        
		UCharacterDataAsset* CharacterDataAsset = CreateCharacterDataAsset(Path);
		UGameplayAbilityBlueprint* CharacterAbility = CreateCharacterAbility(Path);
		UBlueprint* CharacterBlueprint = CreateCharacterBlueprint(Path);

		if(CharacterDataAsset && CharacterAbility && CharacterBlueprint)
		{
			CharacterDataAsset->CharacterAbilityClass = CharacterAbility->GeneratedClass;

			if(AKKCharacter* Character = CharacterBlueprint->GeneratedClass->GetDefaultObject<AKKCharacter>())
			{
				Character->CharacterDataAsset = CharacterDataAsset;

				FCompilerResultsLog CompilerResult;
				FKismetEditorUtilities::CompileBlueprint(CharacterBlueprint, EBlueprintCompileOptions::None, &CompilerResult);
			}
		}

		TArray<UPackage*> Packages = {
			CharacterDataAsset->GetPackage(),
			CharacterAbility->GetPackage(),
			CharacterBlueprint->GetPackage()
		};

		for (UPackage* Package : Packages)
		{
			Package->MarkPackageDirty();
		}

		UEditorLoadingAndSavingUtils::SavePackages(Packages, false);
	}
}

UCharacterDataAsset* FCharacterAssetsCreationTool::CreateCharacterDataAsset(FString AssetPath)
{
	AssetPath += "_DataAsset";
	UDataAssetFactory* DataAssetFactory = NewObject<UDataAssetFactory>();

	UCharacterDataAsset* CharacterDataAsset = CreateAsset<UCharacterDataAsset>(AssetPath, UCharacterDataAsset::StaticClass(), DataAssetFactory);
	return CharacterDataAsset;
}

UGameplayAbilityBlueprint* FCharacterAssetsCreationTool::CreateCharacterAbility(FString AssetPath)
{
	AssetPath += "_Ability";
	
	UGameplayAbilitiesBlueprintFactory* GameplayAbilitiesBlueprintFactory = NewObject<UGameplayAbilitiesBlueprintFactory>();
	GameplayAbilitiesBlueprintFactory->BlueprintType = BPTYPE_Normal;
	GameplayAbilitiesBlueprintFactory->ParentClass = UCharacterActiveGameplayAbility::StaticClass();
	
	return CreateAsset<UGameplayAbilityBlueprint>(AssetPath, UGameplayAbilityBlueprint::StaticClass(), GameplayAbilitiesBlueprintFactory);
}

UBlueprint* FCharacterAssetsCreationTool::CreateCharacterBlueprint(FString AssetPath)
{
	FString CharacterName = FPaths::GetBaseFilename(AssetPath);
	AssetPath.RemoveFromEnd(CharacterName);
	
	AssetPath += "BP_" + CharacterName;

	return CreateBlueprint<AKKCharacter>(AssetPath);
}

template <typename T>
UBlueprint* FCharacterAssetsCreationTool::CreateBlueprint(FString AssetPath)
{
	if(UObject* Object = StaticLoadObject(UObject::StaticClass(), nullptr, *AssetPath))
	{
		return Cast<UBlueprint>(Object); // if it's created, return it
	}

	if(!FKismetEditorUtilities::CanCreateBlueprintOfClass(T::StaticClass()))
	{
		return nullptr;
	}

	if(UPackage* Package = CreatePackage(*AssetPath))
	{
		UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(T::StaticClass(), Package, *FPaths::GetBaseFilename(AssetPath), BPTYPE_Normal);
        
		FAssetRegistryModule::AssetCreated(Blueprint);
		Blueprint->MarkPackageDirty();

		return Blueprint;
	}

	return nullptr;
}

template <typename T>
T* FCharacterAssetsCreationTool::CreateAsset(FString AssetPath, UClass* AssetClass, UFactory* AssetFactory)
{
	if(UObject* Object = StaticLoadObject(UObject::StaticClass(), nullptr, *AssetPath))
	{
		return Cast<T>(Object); // if it's created, return it
	}
	
	IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();

	UObject* Asset = AssetTools.CreateAsset(FPaths::GetBaseFilename(AssetPath), FPaths::GetPath(AssetPath), AssetClass, AssetFactory);
	return Cast<T>(Asset);
}
