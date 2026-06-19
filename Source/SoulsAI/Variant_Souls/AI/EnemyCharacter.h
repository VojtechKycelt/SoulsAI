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

USTRUCT(BlueprintType)
struct FWeightedAction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAction Action = EAction::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
	int32 Probability = 1;
	
	//TODO add montage variable 
	//TODO and max and min range variables -> utility AI -> if player is further, probability is less
	// for instance rollslam can be performed only if the player is in interval 500-700
};

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
	
	/** Clears combat goal */
	UFUNCTION(BlueprintCallable)
	virtual void ClearCombatWheel();
	
	/** Adds an action with a selection weight to the combat wheel */
	UFUNCTION(BlueprintCallable)
	virtual void AddActionToCombatWheel(const EAction Action, const int32 Probability = 1);
	
	/** Adds array of actions with a selection weight to the combat wheel */
	UFUNCTION(BlueprintCallable)
	void AddActionArrayToCombatWheel(const TArray<FWeightedAction>& Actions);
	
	// Returns random action based on probabilities of CombatWheel.
	UFUNCTION(BlueprintCallable)
	EAction SelectActionFromCombatWheel();
	
	/** Attack */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void Attack();
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void AttackCombo();
	
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

	// Radius in which player is attacked.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AI")
	float AttackRadius = 300.0f;
	
	// Radius in which player is attacked.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AI")
	float LongAttackRadius = 1200.0f;

	/** Animation Montages to easily and correctly assign animation to correct action. */
	/** Long Attack Anim Montages */
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimationMontage | Attack | Long")
	UAnimMontage* AM_DashSlash; // Long Sweep
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimationMontage | Attack | Long")
	UAnimMontage* AM_ChargeTwoHandStab;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimationMontage | Attack | Long")
	UAnimMontage* AM_LeapStab;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimationMontage | Attack | Long")
	UAnimMontage* AM_RollSlam;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimationMontage | Attack | Long")
	UAnimMontage* AM_JumpSlam;
	
	/** Close Attack Anim Montages */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimationMontage | Attack | Close")
	UAnimMontage* AM_UpperCut;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimationMontage | Attack | Close")
	UAnimMontage* AM_DualSwordSwing;
	
	/** Dodge Anim Montages */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimationMontage | Deffence")
	UAnimMontage* AM_RollForward;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimationMontage | Deffence")
	UAnimMontage* AM_RollLeft;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimationMontage | Deffence")
	UAnimMontage* AM_RollRight;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimationMontage | Deffence")
	UAnimMontage* AM_RollBackward;
	
	/** Interaction on getting hit Montages*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimationMontage | Deffence")
	UAnimMontage* AM_GetHit;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimationMontage | Deffence")
	UAnimMontage* AM_Death;
	
	UPROPERTY()
	UEnemyAnimInstance* AnimInstance;
	
	/** Attack montage ended delegate */
	FOnMontageEnded OnAttackMontageEnded;
	
	void AttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	bool bAttackMontageEnded = false;
	
	/** Can be used for any task to be completed prematurely when we set this bool to true. */
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	bool bShouldEndTask = false;
	
	/** Can be used for any task to be completed prematurely when we set this bool to true. */
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	bool bShouldRotateToTarget = true;
	
	/** Can be used for any task to be completed prematurely when we set this bool to true. */
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	bool bIsRotatedToTarget = false;
	
	/** Can be used for any task to be completed prematurely when we set this bool to true. */
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	float RotationSpeed = 100.0f;
	
	/** Current montage that is being played. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AI")
	UAnimMontage* CurrentMontage = nullptr;

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
	
	// Array of all goals this character can select. Should be set in blueprint editor.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	TArray<EGoal> RelevantGoals = TArray<EGoal>(); //maybe drop the constructor? or unify all of them
	
	// Map of probabilities of each goal
	UPROPERTY()
	TMap<EAction, uint32> CombatWheel;
	
	// Update probabilities for each relevant goal.
	void UpdateCombatWheel();
	
	// Section names for Montages with more sections to be able to jump to another section without changing montage.
	// Could be a 2D Array/Map that maps section names for each montage.
	// For now sections names are named only Attack1, Attack2,...
	UPROPERTY(EditAnywhere, Category="Attack|Combo")
	TArray<FName> ComboSectionNames;
	
	/** Current index of combo animation montage. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	int32 CurrentComboIndex = 0;
};
