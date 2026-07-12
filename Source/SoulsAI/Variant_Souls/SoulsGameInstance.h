// Copyright (c) 2026 Vojtech Kycelt, master's thesis project at the Faculty of Mathematics and Physics, Charles University, Prague

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SoulsGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class SOULSAI_API USoulsGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDebugUIEnabled = false;
	
	/** Enemy */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	float EnemyTimeDilation = 1.0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	float EnemyDelayAfterAttack = 3.0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bStateTreeDisabled = false;
	
	/** Player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bOverridePlayerStats = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bPlayerInvincible = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	float PlayerMaxHP = 300.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	float PlayerMaxStamina = 300.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	float PlayerLightAttackDamage = 50.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	float PlayerHeavyAttackDamage = 100.0f;
};
