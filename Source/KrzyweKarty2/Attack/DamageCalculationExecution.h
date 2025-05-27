// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "DamageCalculationExecution.generated.h"

/**
 * 
 */

struct FAttackAttributeCapture 
{
	float Strength = 0.f;
	float Defence = 0.f;	
};

UCLASS()
class KRZYWEKARTY2_API UDamageCalculationExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UDamageCalculationExecution();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

	static int32 CalculateDamage(int32 InDamage, int32 VictimDefence);

private:
	static FAttackAttributeCapture CaptureAttributes(const FGameplayEffectCustomExecutionParameters& ExecutionParams);
};
