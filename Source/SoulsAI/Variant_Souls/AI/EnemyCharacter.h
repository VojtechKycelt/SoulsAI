// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Containers/Queue.h"
#include "CoreMinimal.h"
#include "Enums/Enums.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "EnemyCharacter.generated.h"

class UWidgetComponent;
class UEnemyAnimInstance;

UCLASS()
class SOULSAI_API AEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Constructor
	AEnemyCharacter();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	/** HEALTH STATS */
	
	// Max amount of health points.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats | Health", meta = (ClampMin = 0))
	float MaxHP = 100.0f;
	
	// Current amount of health points.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats | Health", meta = (ClampMin = 0))
	float CurrentHP = 0.0f;
	
	/** DEFENSE STATS */
	// Used for receiving damage calculations in combat.
	
	// Base amount of Armor.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats | Defense", meta = (ClampMin = 0))
	float BaseArmor = 10.0f;

	// Base amount of Fire resistance.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats | Defense", meta = (ClampMin = 0))
	float BaseFireResistance = 20.0f;
	
	// Base amount of Lightning resistance.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats | Defense", meta = (ClampMin = 0))
	float BaseLightningResistance = 0.0f;
	
	// Base amount of Magic resistance.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats | Defense", meta = (ClampMin = 0))
	float BaseMagicResistance = 10.0f;
	
	/** ATTACK STATS */
	
	// Base amount of Attack Damage.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats | Attack", meta = (ClampMin = 0))
	float BaseAD = 10.0f;
	
	/** AI PROPERTIES */

	// Radius in which player is chased.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AI")
	float ChaseRadius = 1000.0f;
	
	// Radius in which sight of chased player is lost and should stop chasing.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AI")
	float ChaseSightRadius = 1500.0f;

	// Radius in which player can be close range attacked.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AI")
	float AttackRadius = 300.0f;
	
	// Radius in which player can be long range attacked.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AI")
	float LongAttackRadius = 1200.0f;
	
	// Rotation threshold in degrees within the character is considered rotated to selected target.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AI")
	float RotationToTargetThreshold = 30.0f;
	
	UPROPERTY()
	UEnemyAnimInstance* AnimInstance;
	
	/** If character should rotate to target. */
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	bool bShouldRotateToTarget = true;
	
	/** If character is rotated to target. */
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	bool bIsRotatedToTarget = false;
	
	/** Rotation Speed. */
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	float RotationSpeed = 100.0f;
	
	/** Current montage that is being played. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AI")
	UAnimMontage* CurrentMontage = nullptr;
	
	// Counts number of attack actions since last reposition - used in State Tree to increase probability of reposition.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AI")
	int32 AttackActionCounter = 0;
	
protected:
	// Selected target to look at / attack.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	AActor* SelectedTarget = nullptr;
	
	// Currently selected goal that State Tree uses to determine the state and tasks to execute.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	EGoal SelectedGoal = EGoal::Idle;
	
	// Queue of subgoals which are waiting to be executed after SelectedGoal is done.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	TArray<EGoal> Subgoals = TArray<EGoal>();
	
	// Section names for Montages with more sections to be able to jump to another section without changing montage.
	// Could be a 2D Array/Map that maps section names for each montage.
	// For now sections names are named only Attack1, Attack2,...
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TArray<FName> ComboSectionNames;
	
	/** Current index of combo animation montage. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	int32 CurrentComboIndex = 0;
};
