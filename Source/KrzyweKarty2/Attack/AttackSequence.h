// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttackProperties.h"
#include "GameplayEffectTypes.h"
#include "UObject/Object.h"
#include "AttackSequence.generated.h"


class UAttackComponent;
class FControlFlow;
class AKKCharacter;
class UGameplayEffect;

USTRUCT()
struct FAttackComponents
{
	GENERATED_BODY();
	
	FAttackComponents()
	{
		AttackComponents.Reserve(3); //should rarely be more than 3 components
	}
	
	UPROPERTY()
	TArray<UAttackComponent*> AttackComponents;
};

UCLASS(BlueprintType)
class KRZYWEKARTY2_API UAttackSequence : public UObject
{
	GENERATED_BODY()

public:
	UAttackSequence();

	UFUNCTION(BlueprintCallable)
	void StopExecution();

	UFUNCTION(BlueprintCallable)
	void ModifyDamage(int32 InDamage, EGameplayModOp::Type ModificationType = EGameplayModOp::Override);

protected:
	void BeginAttack();

	FAttackContext AttackContext;
	int32 Damage = -1; // option to specify damage at the beginning of an attack (other than attacker's strength - useful in abilities)

public:
	UFUNCTION(BlueprintCallable, CustomThunk, meta = (WorldContext = "WorldContextObject"))
	static UAttackSequence* BeginDefaultAttackSequence(UObject* WorldContextObject, const AKKCharacter* Attacker, const AKKCharacter* Victim);

	UFUNCTION(BlueprintCallable, CustomThunk, meta = (WorldContext = "WorldContextObject"))
	static UAttackSequence* BeginAbilityAttackSequence(UObject* WorldContextObject, const AKKCharacter* Attacker, const AKKCharacter* Victim, int32 InAbilityIndex = 0, int32 InDamage = -1);

private:
	DECLARE_FUNCTION(execBeginDefaultAttackSequence);
	DECLARE_FUNCTION(execBeginAbilityAttackSequence);

protected:
	void AssembleAttackPipeline(const AKKCharacter* Character);
	void ExecuteAttackStage(EAttackStage Stage);
	
	virtual void InitDamage();
	virtual void ExecuteDamage();
	virtual void CheckVictimDeath();

private:
	UPROPERTY(Transient)
	TMap<EAttackStage, FAttackComponents> AttackPipeline;
	
	TSharedPtr<FControlFlow> AttackFlow;

public:
	UFUNCTION(BlueprintPure)
	FORCEINLINE AKKCharacter* GetAttacker() const { return AttackContext.Attacker; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE AKKCharacter* GetVictim() const { return AttackContext.Victim; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE EAttackType GetAttackType() const { return AttackContext.AttackType; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool IsAbilityAttack() const { return AttackContext.AttackType != EAttackType::DefaultAttack; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE uint8 GetAbilityIndex() const { return AttackContext.AbilityIndex.Get(0); }
};
