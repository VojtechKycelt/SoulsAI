// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Containers/Queue.h"
#include "CoreMinimal.h"
#include "Enums/GoalCommon.h"
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
	
	// Select new goal
	UFUNCTION(BlueprintCallable)
	virtual void SelectGoal();
	
	/** HEALTH STATS */
	
	// Max amount of health points.
	UPROPERTY(EditAnywhere, Category = "Stats | Health", meta = (ClampMin = 0))
	float MaxHP = 100.0f;
	
	// Current amount of health points.
	UPROPERTY(EditAnywhere, Category = "Stats | Health", meta = (ClampMin = 0))
	float CurrentHP = 0.0f;
	
	/** DEFENSE STATS */
	// Used for receiving damage calculations in combat.
	
	// Base amount of Armor.
	UPROPERTY(EditAnywhere, Category = "Stats | Defense", meta = (ClampMin = 0))
	float BaseArmor = 10.0f;

	// Base amount of Fire resistance.
	UPROPERTY(EditAnywhere, Category = "Stats | Defense", meta = (ClampMin = 0))
	float BaseFireResistance = 20.0f;
	
	// Base amount of Lightning resistance.
	UPROPERTY(EditAnywhere, Category = "Stats | Defense", meta = (ClampMin = 0))
	float BaseLightningResistance = 0.0f;
	
	// Base amount of Magic resistance.
	UPROPERTY(EditAnywhere, Category = "Stats | Defense", meta = (ClampMin = 0))
	float BaseMagicResistance = 10.0f;
	
	/** ATTACK STATS */
	
	// Base amount of Attack Damage.
	UPROPERTY(EditAnywhere, Category = "Stats | Attack", meta = (ClampMin = 0))
	float BaseAD = 30.0f;
	
	/** AI PROPERTIES */

	// Radius in which player is chased.
	UPROPERTY(EditAnywhere, Category = "AI")
	float ChaseRadius = 1000.0f;
	
	// Radius in which sight of chased player is lost and should stop chasing.
	UPROPERTY(EditAnywhere, Category = "AI")
	float ChaseSightRadius = 1500.0f;

	// Radius in which player is attacked.
	UPROPERTY(EditAnywhere, Category = "AI")
	float AttackRadius = 300.0f;

	/** Animation Montages to easily and correctly assign animation to correct action. */
	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* DashSlashAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* StabAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* JumpSlamAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* UpperCutAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* StaggerAnimMontage;

	UEnemyAnimInstance* AnimInstance;

protected:
	// Currently selected goal that State Tree uses to determine the state and tasks to execute.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	EGoalCommon SelectedGoal = EGoalCommon::Idle;
	
	// Queue of subgoals which are waiting to be executed after SelectedGoal is done.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	TArray<EGoalCommon> Subgoals = TArray<EGoalCommon>();
	
};
