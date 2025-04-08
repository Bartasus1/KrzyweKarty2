// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterActiveGameplayAbility.h"

#include "KrzyweKarty2/Characters/KKCharacter.h"
#include "KrzyweKarty2/Characters/CharacterActions/CharacterAction.h"
#include "KrzyweKarty2/Core/KKPlayerState.h"

UCharacterActiveGameplayAbility::UCharacterActiveGameplayAbility()
{
	CharacterAction = CreateDefaultSubobject<UCharacterAction_Ability>("Character Action");

	ActivationRequiredTags.AddTag(FGameplayTag::RequestGameplayTag("CharacterState.IsOnBoard"));
}

bool UCharacterActiveGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if(const AKKCharacter* Character = Cast<AKKCharacter>(ActorInfo->AvatarActor.Get()))
	{
		const uint8 AbilityIndex = GetAbilityLevel(Handle, ActorInfo);
		if(!Character->CanActivateCharacterAbility(AbilityIndex))
		{
			return false;
		}
	}
	
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UCharacterActiveGameplayAbility::CommitExecute(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	const uint8 AbilityIndex = GetAbilityLevel(Handle, ActorInfo);
	SourceCharacter->GetAbilityCost(AbilityIndex).CommitAbility(SourceCharacter->GetAbilitySystemComponent());

	Super::CommitExecute(Handle, ActorInfo, ActivationInfo);
}

void UCharacterActiveGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	SourceCharacter->PlayerState->bIsUsingAbility = false;
}
