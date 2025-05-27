// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAction.h"
#include "KrzyweKarty2/KKBlueprintFunctionLibrary.h"
#include "KrzyweKarty2/Characters/KKCharacter.h"



bool UCharacterAction::CanExecuteAction(const AKKCharacter* Character) const
{
	if(Character->CharacterActions >= ActionWeight || (bRequireCharacterOnGameBoard && !Character->IsCharacterOnTheBoard()))
	{
		return false;
	}

	if(QueryStruct.IsValid())
	{
		const bool bNoValidSlots = GetBoardQueryResults(Character).IsEmpty();
		return !bNoValidSlots; // can't execute if there are no valid slots
	}

	return true;
}

TArray<ACharacterSlot*> UCharacterAction::GetBoardQueryResults(const AKKCharacter* Character) const
{
	return UKKBlueprintFunctionLibrary::QueryCharacterSlots(Character, QueryStruct);
}


///////// ABILITY ACTIONS //////////

bool UCharacterAction_Ability::CanExecuteAction(const AKKCharacter* Character) const
{
	if(!Super::CanExecuteAction(Character))
	{
		return false;
	}

	bool bCanAnyAbilityBeExecuted = false;
	for (uint8 i = 0; i < Character->CharacterDataAsset->ActiveAbilities.Num(); i++)
	{
		bCanAnyAbilityBeExecuted |= Character->CanActivateCharacterAbility(i);
	}

	return bCanAnyAbilityBeExecuted;
}


