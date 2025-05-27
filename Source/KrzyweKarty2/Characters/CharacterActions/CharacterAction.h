// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InstancedStruct.h"
#include "KrzyweKarty2/GameBoard/GameBoardQueries/GameBoardQuery.h"
#include "CharacterAction.generated.h"

class UCharacterSlotStatus;
class AKKGameBoard;
class AKKCharacter;
/**
 * 
 */
UCLASS(Abstract, EditInlineNew, DefaultToInstanced)
class KRZYWEKARTY2_API UCharacterAction : public UObject
{
	GENERATED_BODY()

public:

	UCharacterAction()
	{
	}

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	uint8 ActionWeight = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bRequireCharacterOnGameBoard = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TInstancedStruct<FGameBoardQuery> QueryStruct;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UCharacterSlotStatus> ActionStatus = nullptr;

	virtual bool CanExecuteAction(const AKKCharacter* Character) const;
	virtual TArray<ACharacterSlot*> GetBoardQueryResults(const AKKCharacter* Character) const;
};

UCLASS()
class UCharacterAction_Summon : public UCharacterAction
{
	GENERATED_BODY()

public:

	UCharacterAction_Summon()
	{
		ActionWeight = 1;
		bRequireCharacterOnGameBoard = false;
		QueryStruct.InitializeAs<FGameBoardQuery_Summon>();
	}
};

UCLASS()
class UCharacterAction_Movement : public UCharacterAction
{
	GENERATED_BODY()

public:

	UCharacterAction_Movement()
	{
		ActionWeight = 2;
		bRequireCharacterOnGameBoard = true;
		QueryStruct.InitializeAs<FGameBoardQuery_Movement>();
	}
};

UCLASS()
class UCharacterAction_Attack : public UCharacterAction
{
	GENERATED_BODY()

public:

	UCharacterAction_Attack()
	{
		ActionWeight = 3;
		bRequireCharacterOnGameBoard = true;
		QueryStruct.InitializeAs<FGameBoardQuery_Attack>();
	}
};

UCLASS()
class UCharacterAction_Ability : public UCharacterAction
{
	GENERATED_BODY()

public:

	UCharacterAction_Ability()
	{
		ActionWeight = 3;
		bRequireCharacterOnGameBoard = true;
		// don't initialize QueryStruct to make it invalid
	}

	// can ANY ability be executed
	virtual bool CanExecuteAction(const AKKCharacter* Character) const override;
};
