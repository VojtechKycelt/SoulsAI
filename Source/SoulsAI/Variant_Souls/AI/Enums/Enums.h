// Copyright (c) 2026 Vojtech Kycelt, master's thesis project at the Faculty of Mathematics and Physics, Charles University, Prague

#pragma once

#include "CoreMinimal.h"
#include "Enums.generated.h"

/**
 * Enum for common goals shared amongst most enemies.
 */
UENUM(BlueprintType)
enum class EGoal : uint8
{
	Idle				UMETA(DisplayName = "Idle")
	,Chase              UMETA(DisplayName = "Chase")
	,Combat             UMETA(DisplayName = "Combat")
	,Strafe             UMETA(DisplayName = "Strafe")
	,Dodge              UMETA(DisplayName = "Dodge")
	,Recover            UMETA(DisplayName = "Recover")
};

/**
 * Enum for factions/teams of enemies.
 */
UENUM(BlueprintType)
enum class EFaction : uint8
{
	Hostile				UMETA(DisplayName = "Hostile")
	,Neutral            UMETA(DisplayName = "Neutral")
	,Friendly           UMETA(DisplayName = "Friendly")
};

