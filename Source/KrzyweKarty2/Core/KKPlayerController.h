// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "GameFramework/PlayerController.h"
#include "KKPlayerController.generated.h"

#define CHARACTER_TRACE_CHANNEL ECollisionChannel::ECC_GameTraceChannel1
#define CHARACTERSLOT_TRACE_CHANNEL ECollisionChannel::ECC_GameTraceChannel2

class UKKInputConfig;
class UAbilitySystemComponent;
class AKKCharacter;
class AFraction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInputActivated, FGameplayTag, InputTag);
/**
 * 
 */
UCLASS()
class KRZYWEKARTY2_API AKKPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AKKPlayerController();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	int32 PlayerID = -1;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AFraction> FractionToSpawn;

	UPROPERTY(EditAnywhere)
	UKKInputConfig* PlayerInputConfig;

	UPROPERTY(BlueprintAssignable)
	FOnInputActivated OnInputActivated;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:

	UFUNCTION(BlueprintCallable)
	AKKCharacter* SelectCharacter();

	void InputActivated(FGameplayTag InputTag);

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	AKKCharacter* SelectedCharacter;

public:

	void ToggleConfirmationState(bool bEnable) const;
};
