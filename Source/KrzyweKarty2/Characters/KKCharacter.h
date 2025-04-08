// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "CharacterDataAsset.h"
#include "GameFramework/Actor.h"
#include "KKCharacter.generated.h"

class UCharacterAction;
class UCharacterSlotStatus;
class ACharacterSlot;
class AKKGameBoard;
class AKKPlayerState;
class UCharacterWidget;
class UCharacterDataAsset;
class UKKAttributeSet;
struct FGameplayEffectCustomExecutionOutput;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAbilityAction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterDeath);

UCLASS(Abstract, Blueprintable, BlueprintType)
class KRZYWEKARTY2_API AKKCharacter : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AKKCharacter();

	// --------------------------------------
	// COMPONENTS 
	// --------------------------------------
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Components")
	UStaticMeshComponent* CardMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Components")
	UAbilitySystemComponent* AbilitySystemComponent;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	// --------------------------------------------------------
	// CHARACTER DATA
	// --------------------------------------------------------
	UPROPERTY(Transient)
	UKKAttributeSet* AttributeSet; //Cached attribute set

	UPROPERTY(BlueprintReadOnly, Category="Character Data");
	TArray<FGameplayAbilitySpecHandle> CharacterAbilityHandles;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character Data")
	UCharacterDataAsset* CharacterDataAsset;
	
	UPROPERTY(BlueprintReadOnly, Replicated, Category="Character Data", VisibleAnywhere)
	int32 Direction = 1;

	UPROPERTY(BlueprintReadOnly, Replicated, Category="Character Data", VisibleAnywhere)
	int32 CharacterID = 1;

	UFUNCTION(BlueprintPure)
	int32 GetCharacterUniqueID() const
	{
		return CharacterID * Direction;
	}

	UPROPERTY(BlueprintReadOnly, Replicated, Category="Character Data", VisibleAnywhere)
	int32 CharacterActions = 0;

	UPROPERTY(BlueprintReadOnly, Replicated, Category="Character Data", VisibleAnywhere)
	int32 CharacterSlotID = -1; // from 0 to 21 -> one for each CharacterSlot

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnCharacterDeath OnCharacterDeath;
	
	// ----------------------------------------------------------
	// ABILITY FUNCTIONS
	// ----------------------------------------------------------

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void TryActivateCharacterAbility(uint8 AbilityIndex);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool CanActivateCharacterAbility(uint8 AbilityIndex) const;
	
	FAbilityCost GetAbilityCost(uint8 AbilityIndex) const;

protected:
	// ~Begin Actor Interface
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// ~End Actor Interface
public:
	// ----------------------------------------------------------
	// CHARACTER ACTION SLOTS
	// ----------------------------------------------------------
	UFUNCTION(BlueprintNativeEvent, BlueprintPure)
	TArray<int32> GetSlotsForCharacterSpawn() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintPure)
	TArray<FRelativeDirection> GetDirectionsForMovement() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintPure)
	TArray<FRelativeDirection> GetDirectionsForDefaultAttack() const;

	UFUNCTION(BlueprintCallable)
	void CancelAllAbilities();

	// --------------------------------------------------
	// CHARACTER ACTIONS
	// --------------------------------------------------

	UFUNCTION(BlueprintCallable)
	virtual bool CanExecuteAction(TSubclassOf<UCharacterAction> ActionClass) const;

	UFUNCTION(BlueprintCallable)
	virtual bool CanCharacterBeUsed() const;
	
protected:

	UFUNCTION(BlueprintCallable)
	AKKGameBoard* GetGameBoard() const;

public:
	// -------------------------------------
	// PLAYER STATE //
	// -------------------------------------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing="OnRep_PlayerState")
	TObjectPtr<AKKPlayerState> PlayerState;

	UFUNCTION()
	void OnRep_PlayerState();

	void SetPlayerState(AKKPlayerState* NewPlayerState);

	UFUNCTION(BlueprintCallable)
	void RotateToLocalPlayer();
	

private:
	// ----------------------------------------------------
	// CHARACTER WIDGET //
	// ----------------------------------------------------
	const FVector2D DrawSize = {512,700};

	UPROPERTY()
	TObjectPtr<UCharacterWidget> CharacterWidget;

	UFUNCTION(BlueprintCallable)
	void UpdateCharacterWidgetRender(float DeltaTime = 0.f);
	
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UTextureRenderTarget2D> TextureRenderTarget2D;

	UTextureRenderTarget2D* GetWidgetRenderTarget();

	UFUNCTION()
	void PrintAbilityFailure(const UGameplayAbility* GameplayAbility, const FGameplayTagContainer& GameplayTags);

public:

	static bool AreInTheSameTeam(const AKKCharacter* A, const AKKCharacter* B)
	{
		return A->PlayerState == B->PlayerState;
	}

	bool IsCharacterOnTheBoard() const
	{
		return CharacterSlotID != -1;
	}

	bool IsRangedCharacter() const
	{
		return CharacterDataAsset->CharacterStats.MaxAttackRange > 1;
	}
	
	UKKAttributeSet* GetAttributeSet() const
	{
		return AttributeSet;
	}
	
	UCharacterDataAsset* GetCharacterDataAsset() const
	{
		return CharacterDataAsset;
	}

	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetHealth() const
	{
		return AttributeSet->GetHealth();
	}

	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetMana() const
	{
		return AttributeSet->GetMana();
	}

	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetDefence() const
	{
		return AttributeSet->GetDefence();
	}

	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetStrength() const
	{
		return AttributeSet->GetStrength();
	}

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void ModifyAttribute(const FGameplayAttribute& Attribute, const float Magnitude, const EGameplayModOp::Type Modification)
	{
		AbilitySystemComponent->ApplyModToAttribute(Attribute, Modification, Magnitude);
	}

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void ModifyHealth(const float Magnitude, const EGameplayModOp::Type Modification)
	{
		AbilitySystemComponent->ApplyModToAttribute(UKKAttributeSet::GetHealthAttribute(), Modification, Magnitude);
	}

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void ModifyMana(const float Magnitude, const EGameplayModOp::Type Modification)
	{
		AbilitySystemComponent->ApplyModToAttribute(UKKAttributeSet::GetManaAttribute(), Modification, Magnitude);
	}

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void ModifyDefence(const float Magnitude, const EGameplayModOp::Type Modification)
	{
		AbilitySystemComponent->ApplyModToAttribute(UKKAttributeSet::GetDefenceAttribute(), Modification, Magnitude);
	}
};
