// Copyright (c) 2026 Vojtech Kycelt, master's thesis project at the Faculty of Mathematics and Physics, Charles University, Prague

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "SoulsPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

/**
* Types of camera state.
*/
UENUM(BlueprintType)
enum class ECameraState : uint8
{
	Default     UMETA(DisplayName = "Default"),
	Locked      UMETA(DisplayName = "Locked"),
	Cinematic   UMETA(DisplayName = "Cinematic")
};

/**
* Types of cached input for input buffering.
*/
UENUM(BlueprintType)
enum class ECachedInputType : uint8
{
	None			UMETA(DisplayName = "None"),
	Roll			UMETA(DisplayName = "Roll"),
	LightAttack     UMETA(DisplayName = "LightAttack"),
	HeavyAttack		UMETA(DisplayName = "HeavyAttack")
};

/**
* Souls-like player character.
*/
UCLASS(abstract)
class SOULSAI_API ASoulsPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** Actor which camera looks at while in Locked state. */
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	AActor* LockedTarget = nullptr;

	/** Distance in which to look for targets to lock on. */
	UPROPERTY(EditAnywhere, Category = "Lock On")
	float LockOnRadius = 1500.f;

	/** Angle in which to look for targets to lock on. */
	UPROPERTY(EditAnywhere, Category = "Lock On")
	float LockOnConeHalfAngle = 60.f; // degrees

	/** Switches camera state. */
	UFUNCTION()
	void SwitchCameraState();
	
	/** Searches target to lock on within LockOnRadius and LockOnConeHalfAngle. */
	UFUNCTION()
	void FindLockOnTarget();
	
public:
	/** Current amount of HP. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats")
	float CurrentHP = 100.f;
	
	/** Maximum amount of HP. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats")
	float MaxHP = 100.f;
	
	/** Current amount of stamina. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats")
	float CurrentStamina = 100.f;
	
	/** Maximum amount of stamina. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats")
	float MaxStamina = 100.f;
	
	/** Rate at which stamina is recovered. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats")
	float StaminaRecoverySpeed = 0.1f;
	
	/** Cost of stamina for use of Roll. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats")
	float RollStaminaCost = 25.f;
	
	/** Cost of stamina for use of LightAttack. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats")
	float LightAttackStaminaCost = 30.f;
	
	/** Cost of stamina for use of HeavyAttack. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats")
	float HeavyAttackStaminaCost = 40.f;
	
	/** Damage while performing LightAttack. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats")
	float LightAttackDamage = 10.f;
	
	/** Damage while performing HeavyAttack. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats")
	float HeavyAttackDamage = 20.f;

	/** Current Damage target takes while getting hit. */
	UPROPERTY(BlueprintReadOnly)
	float CurrentDamage = 10.f;
	
	/** Base movement speed when not using item. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats")
	float BaseMovementSpeed = 500.0f;
	
	/** Base movement speed when using item. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats")
	float UsingItemMovementSpeed = 300.0f;
	
	/** Max heal flasks count. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats")
	int32 MaxHealFlasksCount = 5;
	
	/** Current heal flask count. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats")
	int32 CurrentHealFlasksCount = 5;
	
	/** Health restored when using flask. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats")
	float HealthRestoredPerFlash = 30.0f;
	
protected:
	/** Input Actions for binding to correct functions. */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* CameraLockAction;
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookAction;
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MouseLookAction;
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* RollAction;
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LightAttackAction;
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* HeavyAttackAction;
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* UseItemAction;
	
	/** Animation Montages to easily and correctly assign animation to correct action. */
	UPROPERTY(EditAnywhere, Category = "Animation | Dodge")
	UAnimMontage* RollForwardAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation | Dodge")
	UAnimMontage* RollLeftAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation | Dodge")
	UAnimMontage* RollRightAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation | Dodge")
	UAnimMontage* RollBackwardAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation | Dodge")
	UAnimMontage* DodgeAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation | Attack")
	UAnimMontage* LightAttackAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation | Attack")
	UAnimMontage* HeavyAttackAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation | Deffence")
	UAnimMontage* GetHitAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation | Deffence")
	UAnimMontage* DeathAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation | Item")
	UAnimMontage* DrinkAnimMontage;

	/** Names of the AnimMontage sections that correspond to each stage of the combo attack. */
	UPROPERTY(EditAnywhere, Category="Melee Attack|Combo")
	TArray<FName> ComboSectionNames;
	
	/** Animation instance pointer to call montages from code. */
	UPROPERTY()
	UAnimInstance* AnimInstance;
	
	/** Montage ended delegate */
	FOnMontageEnded OnAttackMontageEnded;
	FOnMontageEnded OnGetHitMontageEnded;
	FOnMontageEnded OnUseItemMontageEnded;
	FOnMontageEnded OnRollMontageEnded;

public:
	/** Whether combo input window is open for player to press. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bComboInputWindowOpen = false;
	
	/** Current index of combo animation montage. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentComboIndex = 0;
	
	/** Whether mid-combo attack input press was registered and should play another section of combo montage. */
	bool bShouldContinueCombo = false;
	
	/** Type of cached input. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	ECachedInputType CachedInputType = ECachedInputType::None;
	
	/** Cached movement input right. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	float MovementRightCached = 0.0f;
	
	/** Cached movement input forward. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	float MovementForwardCached = 0.0f;
	
	/** Time at which an action input was last pressed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	float InputCachedTime = 0.0f;
	
	/** Max amount of time that may elapse for a buffered input to be considered valid. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (ClampMin = 0, ClampMax = 2, Units = "s"))
	float InputCachedTimeTolerance = 0.25f;
	
	/** Called on montage ended delegates to check if any buffered input can be executed. */
	void CheckCachedInput();
	
	/** If any attack animation is in progress. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation state")
	bool bIsAttacking = false;
	
	/** If character is currently recovering after taking damage. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
	bool bIsRecovering = false;
	
	/** If character is currently healing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
	bool bIsHealing = false;
	
	/** If character is currently rolling and can not be damaged. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
	bool bIsRolling = false;
	
	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Called when the game starts or when spawned. */
	virtual void BeginPlay() override;

	/** Called every frame */
	virtual void Tick(float DeltaTime) override;

protected:
	bool CanPerformAction();

	/** Called for movement input */
	void Move(const FInputActionValue& Value);
	void MoveCompleted(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called when RollAction input is pressed. */
	void RollPressed(const FInputActionValue& Value);
	void Roll();
	
	/** Called when UseItem input is pressed. */
	void UseItemPressed(const FInputActionValue& Value);
	void UseItem();
	
	/** Event called when interrupted while using item. */
	UFUNCTION(BlueprintImplementableEvent)
	void UseItemInterrupted();
	
	/** Called when LightAttackAction input is pressed. */
	void LightAttackPressed(const FInputActionValue& Value);
	void LightAttack();
	
	/** Called when HeavyAttackAction input is pressed. */
	void HeavyAttackPressed(const FInputActionValue& Value);
	void HeavyAttack();

	/** Wrapper function for Jump to decline jump if there is conditions preventing it (like rolling). */
	void SoulsJump(const FInputActionValue& Value);
	
	/** Called for changing camera state. */
	void CameraTargetLock(const FInputActionValue& Value);
	
	/** Events called when montage ended. */
	void AttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void GetHitMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void UseItemMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void RollMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
	/** Current camera state. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	ECameraState CameraState = ECameraState::Default;
	
	/** Current movement right input. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
	float MovementRight = 0.0f;
	
	/** Current movement forward input. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
	float MovementForward = 0.0f;

public:
	/** Sets default values for this character's properties */
	ASoulsPlayerCharacter();

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpStart();

	/** Handles jump end. */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpEnd();
	
	/** Called when notify in multi-section combo montage is called to check if we should continue combo. */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void CheckCombo();
	
	/** Called when notify in roll montage is called to check if we should perfrom roll attack. */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void CheckRollAttack();
	
	/** Called when character is hit by enemy. */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void GetHit(const float Damage);
	
	/** Handles death when current hp is 0. */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void HandleDeath();
	
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
