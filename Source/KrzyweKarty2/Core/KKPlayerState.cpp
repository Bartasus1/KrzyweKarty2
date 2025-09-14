// Fill out your copyright notice in the Description page of Project Settings.


#include "KKPlayerState.h"
#include "KKGameMode.h"
#include "KKGameState.h"

#include "Components/RoundManager.h"

#include "KrzyweKarty2/Characters/KKCharacter.h"

#include "Net/UnrealNetwork.h"

AKKPlayerState::AKKPlayerState()
{
}

void AKKPlayerState::BeginPlay()
{
	Super::BeginPlay();
}

void AKKPlayerState::SetPlayerFraction(const FFractionCharacters& InFractionCharacters)
{
	PlayerFractionCharacters = InFractionCharacters;
	AliveCharactersCount = PlayerFractionCharacters.Characters.Num();

	for (AKKCharacter* Character : PlayerFractionCharacters.Characters)
	{
		Character->OnCharacterDeath.AddDynamic(this, &AKKPlayerState::NotifyCharacterDeath);
	}
}

void AKKPlayerState::NotifyCharacterDeath()
{
	AliveCharactersCount--;

	if(AliveCharactersCount <= 0)
	{
		GetWorld()->GetAuthGameMode<AKKGameMode>()->EndGame(this);
	}
}

const TArray<AKKCharacter*>& AKKPlayerState::GetPlayableCharacters() const
{
	return  PlayerFractionCharacters.Characters;
}

void AKKPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AKKPlayerState, bIsMyTurn);

	DOREPLIFETIME_CONDITION(AKKPlayerState, PlayerFractionCharacters, COND_InitialOnly);
}

void AKKPlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();

	if(GetWorld())
	{
		if(AKKGameState* GameState = GetWorld()->GetGameState<AKKGameState>())
		{
			GameState->GetRoundManager()->OnRoundChanged.Broadcast();
		}
	}
	// update round widget
}