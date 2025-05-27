// Fill out your copyright notice in the Description page of Project Settings.


#include "KKCharacter.h"
#include "AbilitySystemComponent.h"
#include "CharacterDataAsset.h"

#include "Blueprint/UserWidget.h"

#include "CharacterActions/CharacterAction.h"

#include "Components/BaseCharacterComponent.h"

#include "Engine/TextureRenderTarget2D.h"

#include "KrzyweKarty2/KrzyweKartySettings.h"
#include "KrzyweKarty2/AbilitySystem/Abilities/CharacterActionGameplayAbility.h"
#include "KrzyweKarty2/Core/KKPlayerState.h"
#include "KrzyweKarty2/Core/KKGameState.h"
#include "KrzyweKarty2/GameBoard/KKGameBoard.h"
#include "KrzyweKarty2/UI/CharacterWidget.h"
#include "Net/UnrealNetwork.h"
#include "Slate/WidgetRenderer.h"


AKKCharacter::AKKCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicatingMovement(true);

	CardMesh = CreateDefaultSubobject<UStaticMeshComponent>("CardMesh");
	SetRootComponent(CardMesh);

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	AttributeSet = CreateDefaultSubobject<UKKAttributeSet>("CharacterAttributeSet");
}

UAbilitySystemComponent* AKKCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AKKCharacter::InitializeAbilitySystemComponent()
{
	if(PlayerState)
	{
		GetAbilitySystemComponent()->InitAbilityActorInfo(PlayerState, this);
	}
}

void AKKCharacter::TryActivateCharacterAbility_Implementation(uint8 AbilityIndex)
{
	if(PlayerState->bIsUsingAbility || !CharacterAbilityHandles.IsValidIndex(AbilityIndex))
	{
		return;
	}

	PlayerState->bIsUsingAbility = AbilitySystemComponent->TryActivateAbility(CharacterAbilityHandles[AbilityIndex]);
}

bool AKKCharacter::CanActivateCharacterAbility_Implementation(uint8 AbilityIndex) const
{
	if(CharacterAbilityHandles.IsValidIndex(AbilityIndex))
	{
		return GetAbilityCost(AbilityIndex).CheckIfCanAfford(AbilitySystemComponent);
	}

	return false;
}

FAbilityCost AKKCharacter::GetAbilityCost(uint8 AbilityIndex) const
{
	FAbilityCost AbilityCost;
	
	if(CharacterDataAsset->ActiveAbilities.IsValidIndex(AbilityIndex))
	{
		 AbilityCost = CharacterDataAsset->ActiveAbilities[AbilityIndex].AbilityCost;
	}

	return AbilityCost;
}

void AKKCharacter::BeginPlay()
{
	Super::BeginPlay();

	check(CharacterDataAsset); // all characters should have valid DataAsset at BeginPlay

	GetAbilitySystemComponent()->AddSpawnedAttribute(AttributeSet->InitFromCharacterStatistics(CharacterDataAsset->CharacterStats));

	UClass* WidgetClass = UKrzyweKartySettings::Get()->CardWidgetClass.LoadSynchronous();
	CharacterWidget = CreateWidget<UCharacterWidget>(GetWorld(), WidgetClass);
	if(CharacterWidget != nullptr)
	{
		CharacterWidget->SetCharacter(this);
		UpdateCharacterWidgetRender();
	}
	
	if(AKKGameState* GameState = GetWorld()->GetGameState<AKKGameState>())
	{
		GameState->RegisterCharacter(this);
	}

	AbilitySystemComponent->AbilityFailedCallbacks.AddUObject(this, &AKKCharacter::PrintAbilityFailure);
}

void AKKCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateCharacterWidgetRender(DeltaSeconds);
	RotateToLocalPlayer(); // this shouldn't be on a tick, but a weird bug makes character rotate 180 on client, when added to the game board 
}

void AKKCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if(CharacterDataAsset == nullptr)
	{
		return;
	}

	CardMesh->SetStaticMesh(UKrzyweKartySettings::Get()->CardMesh.LoadSynchronous());
	CardMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
	
	if(UMaterialInstanceDynamic* DynamicCardMaterial = CardMesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		DynamicCardMaterial->SetTextureParameterValue(FName("CharacterTexture"), CharacterDataAsset->CharacterCardTexture.LoadSynchronous());
	}
}

void AKKCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AKKCharacter, CharacterID, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(AKKCharacter, Direction, COND_InitialOnly);

	DOREPLIFETIME(AKKCharacter, CharacterActions);
	DOREPLIFETIME(AKKCharacter, CharacterSlotID);
	DOREPLIFETIME(AKKCharacter, PlayerState);
}

TArray<int32> AKKCharacter::GetSlotsForCharacterSpawn_Implementation() const
{
	TArray<int32> CharacterSlots;
	const int32 RowSize = GameBoard::SizeHorizontal;
	const int32 TotalBoardSize = GetGameBoard()->GetTotalMapSize() - 1;
	
	for(int32 i = 1; i <= RowSize; i++)
	{
		CharacterSlots.Add((Direction == 1) ? i : TotalBoardSize - i); // 1,2,3,4 or 20,19,18,17
	}
	
	return CharacterSlots;
}

TArray<FRelativeDirection> AKKCharacter::GetDirectionsForMovement_Implementation() const
{
	TArray<FRelativeDirection> MovementDirections;
	const int32 MovementRange = 1;
	
	for(int32 i = -MovementRange; i <= MovementRange; i++)
	{
		if(i == 0)
			continue; // skip {0, 0} - self
		
		MovementDirections.Add({Direction * i, 0}); //cross pattern
		MovementDirections.Add({0, Direction * i});
	}
	
	return MovementDirections;
}

TArray<FRelativeDirection> AKKCharacter::GetDirectionsForDefaultAttack_Implementation() const
{
	TArray<FRelativeDirection> AttackDirections;
	const int32 AttackRange = CharacterDataAsset->CharacterStats.MaxAttackRange;
	
	for(int32 i = -AttackRange; i <= AttackRange; i++)
	{
		if(i == 0)
			continue; // skip {0, 0} - self
		
		AttackDirections.Add({Direction * i, 0}); //cross pattern
		AttackDirections.Add({0, Direction * i});
	}
	
	return AttackDirections;
}

void AKKCharacter::CancelAllAbilities()
{
	AbilitySystemComponent->CancelAllAbilities();
}

bool AKKCharacter::CanExecuteAction(const TSubclassOf<UCharacterAction> ActionClass) const
{
	const UCharacterAction* CharacterAction = ActionClass->GetDefaultObject<UCharacterAction>();
	return CharacterAction->CanExecuteAction(this);
}

bool AKKCharacter::CanCharacterBeUsed() const
{
	const bool bCanSummon = CanExecuteAction(UCharacterAction_Summon::StaticClass());
	const bool bCanMove = CanExecuteAction(UCharacterAction_Movement::StaticClass());
	const bool bCanAttack = CanExecuteAction(UCharacterAction_Attack::StaticClass());
	const bool bCanUseAnyAbility = CanExecuteAction(UCharacterAction_Ability::StaticClass());
	
	// UE_LOG(LogTemp, Warning, TEXT("%s SUMMON QUERY: %s"), *GetName(), *FString(bCanSummon ? "Can Summon" : "CAN'T Summon"))
	// UE_LOG(LogTemp, Warning, TEXT("%s MOVEMENT QUERY: %s"), *GetName(), *FString(bCanMove ? "Can Move" : "CAN'T Move"))
	// UE_LOG(LogTemp, Warning, TEXT("%s ATTACK QUERY: %s"), *GetName(), *FString(bCanAttack ? "Can Attack" : "CAN'T Attack"))
	// UE_LOG(LogTemp, Warning, TEXT("%s FINAL RESULT: %s"), *GetName(), *FString((bCanSummon || bCanMove || bCanAttack) ? "Can Use Character" : "CAN'T Use Character"))
	// UE_LOG(LogTemp, Warning, TEXT("-------------------------------------------"))

	return bCanSummon || bCanMove || bCanAttack || bCanUseAnyAbility;
}

AKKGameBoard* AKKCharacter::GetGameBoard() const
{
	return GetWorld()->GetGameState<AKKGameState>()->GetGameBoard();
}

void AKKCharacter::RotateToLocalPlayer()
{
	if(const APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController())
	{
		if(PlayerController->IsLocalPlayerController() && GetComponentByClass<UBaseCharacterComponent>() == nullptr)
		{
			SetActorRotation(PlayerController->GetControlRotation());
		}
	}
}

// ABILITY SYSTEM COMPONENT INITIALIZATION //
void AKKCharacter::OnRep_PlayerState() // Client
{
	InitializeAbilitySystemComponent();
	
	if(CharacterWidget) // update widget
	{
		CharacterWidget->SetCharacter(this); 
	}
}

void AKKCharacter::SetPlayerState(AKKPlayerState* NewPlayerState) // Server
{
	PlayerState = NewPlayerState;
	InitializeAbilitySystemComponent();

	if(CharacterDataAsset->CharacterAbilityClass)
	{
		for (uint8 i = 0; i < CharacterDataAsset->ActiveAbilities.Num(); i++) // give character ability with different levels and store a handle to it
		{
			FGameplayAbilitySpecHandle SpecHandle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(CharacterDataAsset->CharacterAbilityClass, i));
			CharacterAbilityHandles.EmplaceAt(i, SpecHandle);
		}
	}
}

// CHARACTER WIDGET UPDATES //
void AKKCharacter::UpdateCharacterWidgetRender(float DeltaTime)
{
	if(!CharacterWidget)
	{
		return;
	}

	FWidgetRenderer* WidgetRenderer = new FWidgetRenderer(false, false);
	WidgetRenderer->DrawWidget(GetWidgetRenderTarget(), CharacterWidget->TakeWidget(), DrawSize, DeltaTime);

	UMaterialInstanceDynamic* DynamicCardMaterial =  CardMesh->CreateAndSetMaterialInstanceDynamic(0);
	DynamicCardMaterial->SetTextureParameterValue(FName("CharacterTexture"), GetWidgetRenderTarget());

	FlushRenderingCommands();
	BeginCleanup(WidgetRenderer);
}

UTextureRenderTarget2D* AKKCharacter::GetWidgetRenderTarget()
{
	if(!TextureRenderTarget2D)
	{
		TextureRenderTarget2D = NewObject<UTextureRenderTarget2D>(this, "RenderTarget");
		TextureRenderTarget2D->RenderTargetFormat = RTF_RGBA8;
		TextureRenderTarget2D->InitAutoFormat(DrawSize.X, DrawSize.Y);
		TextureRenderTarget2D->UpdateResourceImmediate(true);
	}

	return TextureRenderTarget2D;
}

void AKKCharacter::PrintAbilityFailure(const UGameplayAbility* GameplayAbility, const FGameplayTagContainer& GameplayTags)
{
	// UKismetSystemLibrary::PrintString(this, "Ability failed: " + GameplayAbility->GetName());
	// for (const FGameplayTag& Tag : GameplayTags)
	// {
	// 	UKismetSystemLibrary::PrintString(this, Tag.ToString());
	// }
}

