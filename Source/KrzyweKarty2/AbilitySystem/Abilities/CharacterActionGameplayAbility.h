// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"

#include "KrzyweKarty2/GameBoard/CharacterSlot.h"

#include "CharacterActionGameplayAbility.generated.h"

class UCharacterAction;
class UCharacterSlotStatus;
class ACharacterSlot;
class AKKGameBoard;
class AKKGameState;
class AKKCharacter;
/**
 * 
 */
UCLASS()
class KRZYWEKARTY2_API UCharacterActionGameplayAbility : public UGameplayAbility // basic character abilities - movement, attack, etc.
{
	GENERATED_BODY()

public:

	UCharacterActionGameplayAbility();
	
	UPROPERTY(EditDefaultsOnly, Instanced)
	UCharacterAction* CharacterAction;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void CommitExecute(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;


	//Inspired by Xist XCL_ClientServerAbility (https://www.youtube.com/watch?v=VvkIuUnk05M)
	
	UFUNCTION(BlueprintImplementableEvent, DisplayName="Activate Local Player Ability")
	void K2_ActivateLocalPlayerAbility();

	UFUNCTION(BlueprintImplementableEvent, DisplayName="Activate Ability With TargetData")
	void K2_ActivateAbilityWithTargetData(const FGameplayAbilityTargetDataHandle& TargetDataHandle, FGameplayTag ApplicationTag);

	UFUNCTION(BlueprintImplementableEvent, DisplayName="Activate Server Ability")
	void K2_ActivateServerAbility();
	
	UFUNCTION(BlueprintCallable)
	virtual void NotifyTargetDataReady(const FGameplayAbilityTargetDataHandle& TargetDataHandle, FGameplayTag ApplicationTag);

	virtual void ActivateServerAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo);

	UFUNCTION(Client, Reliable, BlueprintCallable)
	virtual void ApplyStatusToCharacterSlot(ACharacterSlot* CharacterSlot, const UCharacterSlotStatus* SlotStatus);

	UFUNCTION(Client, Reliable, BlueprintCallable)
	virtual void ApplyStatusToCharacterSlots(const TArray<ACharacterSlot*>& CharacterSlots, const UCharacterSlotStatus* SlotStatus);

	UFUNCTION(BlueprintCallable)
	virtual void ExecuteCharacterAction(const UCharacterSlotStatus* SlotStatus);
	
protected:
	
	UPROPERTY(BlueprintReadOnly)
	AKKCharacter* SourceCharacter;

	UPROPERTY(BlueprintReadOnly)
	AKKGameState* GameState;

	UPROPERTY(BlueprintReadOnly)
	AKKGameBoard* GameBoard;

	FDelegateHandle OnNotifyTargetDataReady;

	UPROPERTY(BlueprintReadWrite)
	TArray<ACharacterSlot*> ActionSlots;
};