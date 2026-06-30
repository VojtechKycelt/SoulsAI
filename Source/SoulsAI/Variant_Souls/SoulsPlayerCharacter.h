// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "SoulsPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;


UENUM(BlueprintType)
enum class ECameraState : uint8
{
	Default     UMETA(DisplayName = "Default"),
	Locked      UMETA(DisplayName = "Locked"),
	Cinematic   UMETA(DisplayName = "Cinematic")
};

UENUM(BlueprintType)
enum class ECachedInputType : uint8
{
	None			UMETA(DisplayName = "None"),
	Roll			UMETA(DisplayName = "Roll"),
	LightAttack     UMETA(DisplayName = "LightAttack"),
	HeavyAttack		UMETA(DisplayName = "HeavyAttack")
};

/**
* Player-controllable third person character.
* Implements a controllable orbiting camera.
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
	
	UPROPERTY()
	AActor* LockedTarget = nullptr;

	UPROPERTY(EditAnywhere, Category = "Lock On")
	float LockOnRadius = 1500.f;

	UPROPERTY(EditAnywhere, Category = "Lock On")
	float LockOnConeHalfAngle = 60.f; // degrees

	UFUNCTION()
	void TryLockOn();

	UFUNCTION()
	void FindLockOnTarget();
	
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats")
	float CurrentHP = 100.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats")
	float MaxHP = 100.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats")
	float CurrentStamina = 100.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats")
	float MaxStamina = 100.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats")
	float StaminaRecoverySpeed = 0.1f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats")
	float RollStaminaCost = 25.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats")
	float LightAttackStaminaCost = 30.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats")
	float HeavyAttackStaminaCost = 40.f;
	
	// Damage while performing LightAttack.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats")
	float LightAttackDamage = 10.f;
	
	// Damage while performing HeavyAttack.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Stats")
	float HeavyAttackDamage = 20.f;

	// Current Damage target takes while getting hit.
	UPROPERTY(BlueprintReadOnly)
	float CurrentDamage = 10.f;
	
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

	/** Names of the AnimMontage sections that correspond to each stage of the combo attack */
	UPROPERTY(EditAnywhere, Category="Melee Attack|Combo")
	TArray<FName> ComboSectionNames;
	
	UPROPERTY()
	UAnimInstance* AnimInstance;
	
	/** Attack montage ended delegate */
	FOnMontageEnded OnAttackMontageEnded;
	FOnMontageEnded OnGetHitMontageEnded;
	FOnMontageEnded OnRollMontageEnded;

public:
	/** Whether mid-combo attack input press plays another section of combo montage. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bComboInputWindowOpen = false;
	
	/** Current index of combo animation montage. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentComboIndex = 0;
	
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
	
	// If any attack animation is in progress.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation state")
	bool bIsAttacking = false;
	
	// If character is currently recovering after taking damage.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
	bool bIsRecovering = false;
	
	// If character is currently rolling and can not be damaged.
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
	
	/** Called when LightAttackAction input is pressed. */
	void LightAttackPressed(const FInputActionValue& Value);
	void LightAttack();
	
	/** Called when HeavyAttackAction input is pressed. */
	void HeavyAttackPressed(const FInputActionValue& Value);
	void HeavyAttack();

	/** Wrapper function for Jump to decline jump if there is conditions preventing it (like rolling) */
	void SoulsJump(const FInputActionValue& Value);
	
	/** Called for changing camera state */
	void CameraTargetLock(const FInputActionValue& Value);
	
	void AttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
	void GetHitMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
	void RollMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	ECameraState CameraState = ECameraState::Default;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
	float MovementRight = 0.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
	float MovementForward = 0.0f;

public:
	// Sets default values for this character's properties
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

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpEnd();
	
	UFUNCTION(BlueprintCallable, Category = "Input")
	void CheckCombo();
	
	UFUNCTION(BlueprintCallable, Category = "Input")
	void CheckRollAttack();
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void GetHit(const float Damage);
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void HandleDeath();
	
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
