// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SoulsPlayerCharacterAnimInstance.h"
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
	UPROPERTY(EditAnywhere, Category = "Stats")
	float CurrentHP = 100.f;
	
	UPROPERTY(EditAnywhere, Category = "Stats")
	float MaxHP = 100.f;

protected:
	/** Input Actions for binding to correct functions. */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;
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
	USoulsPlayerCharacterAnimInstance* AnimInstance;
	
	/** Attack montage ended delegate */
	FOnMontageEnded OnAttackMontageEnded;
	FOnMontageEnded OnGetHitMontageEnded;

public:
	/** Whether mid-combo attack input press plays another section of combo montage. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bComboInputWindowOpen = false;
	
	/** Current index of combo animation montage. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentComboIndex = 0;
	
	bool bShouldContinueCombo = false;
	
protected:
	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Called when the game starts or when spawned. */
	virtual void BeginPlay() override;

	/** Called every frame */
	virtual void Tick(float DeltaTime) override;

	bool CanPerformAction();

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called for rolling input */
	void Roll(const FInputActionValue& Value);

	/** Called for attacking inputs */
	void LightAttack(const FInputActionValue& Value);
	void HeavyAttack(const FInputActionValue& Value);

	/** Wrapper function for Jump to decline jump if there is conditions preventing it (like rolling) */
	void SoulsJump(const FInputActionValue& Value);
	
	/** Called for changing camera state */
	void CameraTargetLock(const FInputActionValue& Value);
	
	void AttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
	void GetHitMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
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
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void GetHit(const float Damage);
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void HandleDeath();
	
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
