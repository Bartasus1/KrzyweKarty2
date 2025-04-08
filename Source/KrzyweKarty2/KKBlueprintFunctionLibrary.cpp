// Fill out your copyright notice in the Description page of Project Settings.


#include "KKBlueprintFunctionLibrary.h"
#include "KrzyweKartySettings.h"

#include "Characters/KKCharacter.h"

#include "Core/KKGameState.h"

#include "GameBoard/GameBoardQueries/GameBoardQuery.h"

// FText UKKBlueprintFunctionLibrary::GetFractionNameByTag(const FGameplayTag& FractionTag)
// {
// 	const UKrzyweKartySettings* DeveloperSettings = UKrzyweKartySettings::Get();
//
// 	return DeveloperSettings->FractionTagToNameMap.FindRef(FractionTag);
// }

AKKGameBoard* UKKBlueprintFunctionLibrary::GetGameBoard(const UObject* WorldContextObject)
{
	const AKKGameState* GameState = WorldContextObject->GetWorld()->GetGameState<AKKGameState>();
	return GameState->GetGameBoard();
}

TArray<ACharacterSlot*> UKKBlueprintFunctionLibrary::QueryCharacterSlots(const AKKCharacter* Character, const TInstancedStruct<FGameBoardQuery>& GameBoardQuery)
{
	const FGameBoardQuery* GameBoardQueryPtr = GameBoardQuery.GetPtr<FGameBoardQuery>();
	check(GameBoardQueryPtr);
	return QueryCharacterSlots(Character, GameBoardQueryPtr);
}

TArray<ACharacterSlot*> UKKBlueprintFunctionLibrary::QueryCharacterSlots(const AKKCharacter* Character, const FGameBoardQuery* GameBoardQuery)
{
	const AKKGameBoard* GameBoard = GetGameBoard(Character);
	return GameBoardQuery->ExecuteQuery(GameBoard, Character);
}
