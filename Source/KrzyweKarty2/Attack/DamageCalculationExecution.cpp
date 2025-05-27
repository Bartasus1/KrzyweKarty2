// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageCalculationExecution.h"

#include "KrzyweKarty2/AbilitySystem/Attributes/KKAttributeSet.h"


struct FDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Strength);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Health);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Defence);

	FDamageStatics()
	{

		DEFINE_ATTRIBUTE_CAPTUREDEF(UKKAttributeSet, Strength, Source, false);
		
		DEFINE_ATTRIBUTE_CAPTUREDEF(UKKAttributeSet, Health, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UKKAttributeSet, Defence, Target, false);
	}
};

static const FDamageStatics& DamageStatics()
{
	static FDamageStatics DStatics;
	return DStatics;
}

UDamageCalculationExecution::UDamageCalculationExecution()
{
	RelevantAttributesToCapture.Add(DamageStatics().StrengthDef);
	RelevantAttributesToCapture.Add(DamageStatics().DefenceDef);
}

void UDamageCalculationExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	Super::Execute_Implementation(ExecutionParams, OutExecutionOutput);

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	const FAttackAttributeCapture AttributeCapture = CaptureAttributes(ExecutionParams);

	const int32 Damage = Spec.GetSetByCallerMagnitude("Damage", false, AttributeCapture.Strength);
	const int32 Result = CalculateDamage(Damage, AttributeCapture.Defence);

	UE_LOG(LogTemp, Warning, TEXT("Dealing damage: %d"), Result);

	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(DamageStatics().HealthProperty, EGameplayModOp::Additive, -Result));
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(DamageStatics().DefenceProperty, EGameplayModOp::Additive, -1));
}

int32 UDamageCalculationExecution::CalculateDamage(const int32 InDamage, const int32 VictimDefence)
{
	return FMath::Max(InDamage - FMath::Max(VictimDefence, 0.f), 0.f);
}


FAttackAttributeCapture UDamageCalculationExecution::CaptureAttributes(const FGameplayEffectCustomExecutionParameters& ExecutionParams)
{
	FAttackAttributeCapture AttributeCapture;

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;
	
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DefenceDef, EvaluationParameters, AttributeCapture.Defence);
	AttributeCapture.Defence = FMath::Max<float>(AttributeCapture.Defence, 0.f);
	
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().StrengthDef, EvaluationParameters, AttributeCapture.Strength);
	AttributeCapture.Strength = FMath::Max<float>(AttributeCapture.Strength, 0.f);

	return AttributeCapture;
} 
