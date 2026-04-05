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

DECLARE_LOG_CATEGORY_EXTERN(LogSouls, Log, All);

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
	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* RollForwardAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* RollLeftAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* RollRightAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* DodgeAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* LightAttackAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* HeavyAttackAnimMontage;

	USoulsPlayerCharacterAnimInstance* AnimInstance;

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

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
