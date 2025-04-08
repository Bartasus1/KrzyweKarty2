// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterActionGameplayAbility.h"

#include "Kismet/KismetSystemLibrary.h"

#include "KrzyweKarty2/KKBlueprintFunctionLibrary.h"
#include "KrzyweKarty2/Characters/KKCharacter.h"
#include "KrzyweKarty2/Characters/CharacterActions/CharacterAction.h"
#include "KrzyweKarty2/Core/KKGameState.h"
#include "KrzyweKarty2/Core/KKPlayerState.h"
#include "KrzyweKarty2/GameBoard/CharacterSlot.h"

UCharacterActionGameplayAbility::UCharacterActionGameplayAbility(): CharacterAction(nullptr), SourceCharacter(nullptr), GameState(nullptr), GameBoard(nullptr)
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

bool UCharacterActionGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	check(CharacterAction);
	
	if(const AKKCharacter* Character = Cast<AKKCharacter>(ActorInfo->AvatarActor.Get()))
	{
		const AKKGameBoard* WorldGameBoard = UKKBlueprintFunctionLibrary::GetGameBoard(Character);
		
		if(!Character->PlayerState->bIsMyTurn || !CharacterAction->CanExecuteAction(Character, WorldGameBoard))
		{
			return false;
		}
	}
	
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UCharacterActionGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// initialize member variables
	SourceCharacter = Cast<AKKCharacter>(GetAvatarActorFromActorInfo());
	GameState = GetWorld()->GetGameState<AKKGameState>();
	GameBoard = GameState->GetGameBoard();

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if(ActorInfo->IsLocallyControlled())
	{
		K2_ActivateLocalPlayerAbility();
		
	}
	if(ActorInfo->IsNetAuthority())
	{
		ActivateServerAbility(Handle, ActorInfo, ActivationInfo);
		
		K2_ActivateServerAbility();
	}
}

void UCharacterActionGameplayAbility::CommitExecute(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::CommitExecute(Handle, ActorInfo, ActivationInfo);
	
	if(ActorInfo->IsNetAuthority())
	{
		SourceCharacter->CharacterActions = CharacterAction->ActionWeight;
		GameState->MarkCharacterUsedInRound(SourceCharacter);
	}
}

void UCharacterActionGameplayAbility::ExecuteCharacterAction(const UCharacterSlotStatus* SlotStatus)
{
	if(CharacterAction->QueryStruct.IsValid() && GetCurrentActorInfo()->IsNetAuthority())
	{
		ActionSlots = UKKBlueprintFunctionLibrary::QueryCharacterSlots(SourceCharacter, CharacterAction->QueryStruct);
		ApplyStatusToCharacterSlots(ActionSlots, SlotStatus);
	}
}

void UCharacterActionGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if(UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).Remove(OnNotifyTargetDataReady);
		ASC->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());
	}
	
	if(ActorInfo->IsNetAuthority())
	{
		ApplyStatusToCharacterSlots(ActionSlots, nullptr);
		ActionSlots.Empty();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UCharacterActionGameplayAbility::NotifyTargetDataReady(const FGameplayAbilityTargetDataHandle& TargetDataHandle, FGameplayTag ApplicationTag)
{
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	
	FScopedPredictionWindow ScopedPredictionWindow(AbilitySystemComponent);
	
	if(CurrentActorInfo->IsLocallyControlled() && !CurrentActorInfo->IsNetAuthority())
	{
		AbilitySystemComponent->CallServerSetReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey(), TargetDataHandle, ApplicationTag, AbilitySystemComponent->ScopedPredictionKey);
	}

	K2_ActivateAbilityWithTargetData(TargetDataHandle, ApplicationTag);

	AbilitySystemComponent->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());
}

void UCharacterActionGameplayAbility::ActivateServerAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	OnNotifyTargetDataReady = ASC->AbilityTargetDataSetDelegate(Handle, ActivationInfo.GetActivationPredictionKey()).AddUObject(this, &UCharacterActionGameplayAbility::NotifyTargetDataReady);
}

void UCharacterActionGameplayAbility::ApplyStatusToCharacterSlot_Implementation(ACharacterSlot* CharacterSlot, const UCharacterSlotStatus* SlotStatus)
{
	CharacterSlot->SetLocalStatus(SlotStatus);
}

void UCharacterActionGameplayAbility::ApplyStatusToCharacterSlots_Implementation(const TArray<ACharacterSlot*>& CharacterSlots, const UCharacterSlotStatus* SlotStatus)
{
	for (ACharacterSlot* CharacterSlot : CharacterSlots)
	{
		CharacterSlot->SetLocalStatus(SlotStatus);
	}
}
