// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttackProperties.h"
#include "AttackSequence.h"

#include "UObject/Object.h"
#include "AttackPreview.generated.h"

class UCharacterWidget;
class UAttackSequence;
/**
 * 
 */
UCLASS()
class KRZYWEKARTY2_API UAttackPreview : public UAttackSequence
{
	GENERATED_BODY()

protected:
	void BeginPreviewAttack();
	
	virtual void ExecuteDamage() override;

public:

	UFUNCTION(BlueprintCallable, CustomThunk, meta = (WorldContext = "WorldContextObject"))
	static UAttackSequence* BeginPreviewAttackSequence(UObject* WorldContextObject, const AKKCharacter* Attacker, const AKKCharacter* Victim, EAttackType AttackType = EAttackType::DefaultAttack, int32 InAbilityIndex = 0, int32 InDamage = -1);

protected:
	UPROPERTY()
	UCharacterWidget* OriginalCharacterWidget;

private:
	DECLARE_FUNCTION(execBeginPreviewAttackSequence);
	
	static AKKCharacter* CreateCharacterCopy(const AKKCharacter* Character, UObject* WorldContextObject);



	
};
