#pragma once

class AKKCharacter;



UENUM()
enum class EAttackStage : uint8
{
	PreAttack UMETA(DisplayName = "Pre-Attack", ToolTip="Pre eliminary attack stage. Can be used to stop it"),
	Before UMETA(DisplayName = "Before", ToolTip="Before the attack is executed - change applied damage etc."),
	After UMETA(DisplayName = "After", ToolTip="After the attack is executed - apply post attack effects"),
	MAX UMETA(Hidden)
};
ENUM_RANGE_BY_COUNT(EAttackStage, EAttackStage::MAX);

UENUM()
enum class EAttackRole : uint8
{
	Victim UMETA(DisplayName = "Victim", ToolTip="The character is a victim of the attack"),
	Attacker UMETA(DisplayName = "Attacker", ToolTip="The character is an attacker")
};

UENUM(BlueprintType, meta=(Bitflags, UseEnumValuesAsMaskValuesInEditor="true"))
enum class EAttackType : uint8
{
	NONE			= 0 UMETA(Hidden),
	DefaultAttack	= 1 << 0,
	Ability			= 1 << 1,
};

struct FAttackContext
{
	
	AKKCharacter* Attacker;
	
	AKKCharacter* Victim;
	
	EAttackType AttackType = EAttackType::DefaultAttack;
	
	TOptional<uint8> AbilityIndex;

	bool IsValid() const
	{
		return Attacker && Victim;
	}

	bool DoesCharacterMatchRole(const AKKCharacter* Character, const EAttackRole AttackRole) const
	{
		switch (AttackRole)
		{
		case EAttackRole::Attacker:
			return Character == Attacker;
		case EAttackRole::Victim:
			return Character == Victim;
		default:
			return false;
		}
	}
};


constexpr uint8 operator|(EAttackType A, EAttackType B)
{
	return static_cast<uint8>(A) | static_cast<uint8>(B);
}
constexpr bool operator&(const uint8 A, EAttackType B)
{
	return A & static_cast<uint8>(B);
}
