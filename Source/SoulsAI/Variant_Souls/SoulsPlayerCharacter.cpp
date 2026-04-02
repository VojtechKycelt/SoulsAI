// Fill out your copyright notice in the Description page of Project Settings.


#include "Variant_Souls/SoulsPlayerCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "SoulsAI.h"

// Sets default values
ASoulsPlayerCharacter::ASoulsPlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

// Called to bind functionality to input
void ASoulsPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		// Jumping
		// EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		// EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ASoulsPlayerCharacter::SoulsJump);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASoulsPlayerCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ASoulsPlayerCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASoulsPlayerCharacter::Look);

		//Rolling
		EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Triggered, this, &ASoulsPlayerCharacter::Roll);

		//Attacks
		EnhancedInputComponent->BindAction(LightAttackAction, ETriggerEvent::Triggered, this, &ASoulsPlayerCharacter::LightAttack);
		EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Triggered, this, &ASoulsPlayerCharacter::HeavyAttack);
	}
	else
	{
		UE_LOG(LogSoulsAI, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}

}

// Called when the game starts or when spawned
void ASoulsPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	AnimInstance = Cast<USoulsPlayerCharacterAnimInstance>(GetMesh()->GetAnimInstance());
	if (!AnimInstance)
	{
		UE_LOG(LogSoulsAI, Error, TEXT("AnimInstance is null"));
	}

}

// Called every frame
void ASoulsPlayerCharacter::Tick(float DeltaTime)
{
	//Super::Tick(DeltaTime);

}

bool ASoulsPlayerCharacter::CanPerformAction()
{
	if (!AnimInstance) return false;
	if (AnimInstance->isRolling || AnimInstance->isAnimating || GetCharacterMovement()->IsFalling()) return false;
	return true;
}

void ASoulsPlayerCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}
void ASoulsPlayerCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, -LookAxisVector.Y);
}
void ASoulsPlayerCharacter::Roll(const FInputActionValue& Value)
{
	if (CanPerformAction())
	{
		AnimInstance->Montage_Play(RollAnimMontage);
		//PlayAnimMontage(RollAnimMontage);
	}
}

void ASoulsPlayerCharacter::LightAttack(const FInputActionValue& Value)
{
	if (CanPerformAction())
	{
		AnimInstance->Montage_Play(LightAttackAnimMontage);
		//PlayAnimMontage(LightAttackAnimMontage);
	}
}

void ASoulsPlayerCharacter::HeavyAttack(const FInputActionValue& Value)
{
	if (CanPerformAction())
	{
		AnimInstance->Montage_Play(HeavyAttackAnimMontage);
		//PlayAnimMontage(HeavyAttackAnimMontage);
	}
}

void ASoulsPlayerCharacter::SoulsJump(const FInputActionValue& Value)
{
	if (CanPerformAction())
	{
		Jump();
	}
}

void ASoulsPlayerCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void ASoulsPlayerCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ASoulsPlayerCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void ASoulsPlayerCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}



