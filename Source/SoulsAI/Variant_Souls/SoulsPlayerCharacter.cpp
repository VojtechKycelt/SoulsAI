// Fill out your copyright notice in the Description page of Project Settings.


#include "Variant_Souls/SoulsPlayerCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AI/EnemyCharacter.h"
#include "SoulsAI.h"

// Sets default values
ASoulsPlayerCharacter::ASoulsPlayerCharacter()
{
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
	
	OnAttackMontageEnded.BindUObject(this, &ASoulsPlayerCharacter::AttackMontageEnded);
	OnGetHitMontageEnded.BindUObject(this, &ASoulsPlayerCharacter::GetHitMontageEnded);
	
	//Stats
	CurrentHP = MaxHP;
}

// Called to bind functionality to input
void ASoulsPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		// Jumping
		// EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		// EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		// EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ASoulsPlayerCharacter::SoulsJump);
		
		// Camera
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ASoulsPlayerCharacter::CameraTargetLock);

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
		UE_LOG(LogSoulsAI, Error, TEXT("[ASoulsPlayerCharacter] AnimInstance is null"));
	}

}

// Called every frame
void ASoulsPlayerCharacter::Tick(float DeltaTime)
{
	switch (CameraState)
	{
	case ECameraState::Default:
		FollowCamera->SetWorldRotation(
			FMath::RInterpTo(
				FollowCamera->GetComponentRotation(), 
				CameraBoom->PreviousDesiredRot, 
				DeltaTime, 
				5.f));
		break;
	case ECameraState::Locked:
		if (!LockedTarget || (FVector::Dist(GetActorLocation(), LockedTarget->GetActorLocation()) > LockOnRadius))
		{
			TryLockOn();
			break;
		} else if (const AEnemyCharacter* EnemyTarget = Cast<AEnemyCharacter>(LockedTarget))
		{
			if (EnemyTarget->bIsDead)
			{
				TryLockOn();
				break;
			}
		}
		
		// Character faces enemy
		if (! AnimInstance->bIsRecovering)
		{
			FVector ToEnemy = LockedTarget->GetActorLocation() - GetActorLocation();
			ToEnemy.Z = 0.f;
			ToEnemy.Normalize();
			SetActorRotation(
				FMath::RInterpTo(
					GetActorRotation(), 
					ToEnemy.Rotation(),
					DeltaTime, 
					GetCharacterMovement()->RotationRate.Yaw / 100.f));
		}
		
		// Camera looks at enemy
		const FRotator FollowCamTargetRotation = FRotator((LockedTarget->GetActorLocation() - FollowCamera->GetComponentLocation()).Rotation());
		FollowCamera->SetWorldRotation(FMath::RInterpTo(FollowCamera->GetComponentRotation(), FollowCamTargetRotation, DeltaTime, 10.f));
		
		// Camera positioned behind players back, slightly up (- pitch)
		FRotator CamBoomTargetRotation = FRotator((LockedTarget->GetActorLocation() - GetActorLocation()).Rotation());
		CamBoomTargetRotation.Pitch -= 30;
		Controller->SetControlRotation(FMath::RInterpTo(Controller->GetControlRotation(), CamBoomTargetRotation, DeltaTime, 10.f));
		
		break;
	default:
		break;
	}
	
	if (GetCharacterMovement()->Velocity.IsNearlyZero())
	{
		MovementRight = 0.0f;
		MovementForward = 0.0f;
	}


}

bool ASoulsPlayerCharacter::CanPerformAction()
{
	if (!AnimInstance) return false;
	if (AnimInstance->bIsRolling || GetCharacterMovement()->IsFalling() || AnimInstance->bIsRecovering) return false;
	return true;
}

void ASoulsPlayerCharacter::FindLockOnTarget()
{
    TArray<AActor*> OverlappedActors;
    TArray<AActor*> ActorsToIgnore = { this };

    // Get all nearby enemies with sphere overlap
    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        GetActorLocation(),
        LockOnRadius,
        TArray<TEnumAsByte<EObjectTypeQuery>>(),
        AEnemyCharacter::StaticClass(),   // only grab EnemyBase and children
        ActorsToIgnore,
        OverlappedActors
    );

    if (OverlappedActors.IsEmpty())
    {
        LockedTarget = nullptr;
        return;
    }
	
	// Iterate through all overlapped actors and get the one closest to the cone center
    AActor* BestTarget = nullptr;
    float BestDot = -1.f; // dot product: 1 = directly ahead, -1 = directly behind
    for (AActor* Actor : OverlappedActors)
    {
        // Direction from player to this enemy
        const FVector ToEnemy = (Actor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    	
        // Dot product tells us how aligned they are with camera forward
        const float Dot = FVector::DotProduct(FollowCamera->GetForwardVector(), ToEnemy);

        // Convert cone half angle to dot product threshold
        const float ConeThreshold = FMath::Cos(FMath::DegreesToRadians(LockOnConeHalfAngle));
        if (ConeThreshold > Dot)
        {
            continue; // outside the cone, skip
        }

        // Line trace to check line of sight
        FHitResult HitResult;
        const bool bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            GetActorLocation(),
            Actor->GetActorLocation(),
            ECC_Visibility
        );

        // If trace hits something that isn't the enemy, it's occluded
        if (bHit && HitResult.GetActor() != Actor)
        {
            continue;
        }
    	
    	// If enemy is dead, ignore it.
    	if (const AEnemyCharacter* EnemyTarget = Cast<AEnemyCharacter>(Actor))
        {
        	if (EnemyTarget->bIsDead)
        	{
        		continue;
        	}
        }
    	
        // BestDot = most centered in camera view = BestTarget
        if (Dot > BestDot)
        {
            BestDot = Dot;
            BestTarget = Actor;
        }
    }

    LockedTarget = BestTarget;
}

void ASoulsPlayerCharacter::TryLockOn()
{
    if (LockedTarget)
    {
        CameraState = ECameraState::Default;
    	GetCharacterMovement()->bOrientRotationToMovement = true;
        LockedTarget = nullptr;
        return;
    }

    FindLockOnTarget();

    if (LockedTarget)
    {
        CameraState = ECameraState::Locked;
    	GetCharacterMovement()->bOrientRotationToMovement = false;
    }
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
	if (! CanPerformAction() || AnimInstance->IsAnyMontagePlaying())
	{
		return;
	}
	// if (GetCharacterMovement()->Velocity.IsNearlyZero()) {
	if ((MovementRight == 0.f) && (MovementForward == 0.f)) {
		AnimInstance->Montage_Play(DodgeAnimMontage);
	}
	else {
		if (CameraState == ECameraState::Locked)
		{
			if (FMath::Abs(MovementRight) > FMath::Abs(MovementForward))
			{
				// predominantly moving sideways
				if (MovementRight > 0.f)
				{
					AnimInstance->Montage_Play(RollRightAnimMontage);
				}
				else
				{
					AnimInstance->Montage_Play(RollLeftAnimMontage);
				}
			}
			else
			{
				// predominantly moving forward/backward
				if (MovementForward >= 0.f)
				{
					AnimInstance->Montage_Play(RollForwardAnimMontage);
				}
				else
				{
					//TODO RollBackwardAnimMontage instead
					AnimInstance->Montage_Play(RollBackwardAnimMontage);
				}
			}
		} else
		{
			AnimInstance->Montage_Play(RollForwardAnimMontage);
		}
	}
	
}

void ASoulsPlayerCharacter::LightAttack(const FInputActionValue& Value)
{
	if (! CanPerformAction()) return;
	if (! AnimInstance->IsAnyMontagePlaying()) 
	{
		AnimInstance->Montage_Play(LightAttackAnimMontage);
		AnimInstance->Montage_SetEndDelegate(OnAttackMontageEnded, LightAttackAnimMontage);
	} else if (bComboInputWindowOpen)
	{
		UE_LOG(LogSoulsAI, Warning, TEXT("[ASoulsPlayerCharacter] bComboInputWindowOpen - should continue combo"));
		bShouldContinueCombo = true;
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

void ASoulsPlayerCharacter::AttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (bInterrupted)
	{
		UE_LOG(LogSoulsAI, Warning, TEXT("[ASoulsPlayerCharacter] AttackMontageEnded by interrupting"));
	}
	CurrentComboIndex = 0;
	bShouldContinueCombo = false;
	bComboInputWindowOpen = false;
}

void ASoulsPlayerCharacter::SoulsJump(const FInputActionValue& Value)
{
	if (CanPerformAction())
	{
		Jump();
	}
}

void ASoulsPlayerCharacter::CameraTargetLock(const FInputActionValue& Value)
{
	TryLockOn();
}

void ASoulsPlayerCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		MovementRight = Right;
		MovementForward = Forward;
		if (CameraState == ECameraState::Locked) {
			// Use camera forward projected onto horizontal plane
			FVector CameraForward = FollowCamera->GetForwardVector();
			CameraForward.Z = 0.f;
			CameraForward.Normalize();
			AddMovementInput(CameraForward, Forward);
			
			FVector CameraRight = FollowCamera->GetRightVector();
			CameraRight.Z = 0.f;
			CameraRight.Normalize();
			AddMovementInput(CameraRight, Right);
		} else {
			// find out which way is forward, get forward vector, get right vector
			const FRotator YawRotation(0, GetController()->GetControlRotation().Yaw, 0);
			const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

			// add movement
			AddMovementInput(ForwardDirection, Forward);
			AddMovementInput(RightDirection, Right);
		}
	}
}

void ASoulsPlayerCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr && CameraState != ECameraState::Locked)
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

void ASoulsPlayerCharacter::CheckCombo()
{
	if (bShouldContinueCombo)
	{
		if (CurrentComboIndex == 0)
		{
			AnimInstance->Montage_JumpToSection("Attack2", LightAttackAnimMontage);
			bShouldContinueCombo = false;
			bComboInputWindowOpen = false;
			CurrentComboIndex = 1;
		} else if (CurrentComboIndex == 1)
		{
			AnimInstance->Montage_JumpToSection("Attack3", LightAttackAnimMontage);
			bShouldContinueCombo = false;
			bComboInputWindowOpen = false;
			CurrentComboIndex = 2;
		}
	}
}

void ASoulsPlayerCharacter::GetHit(const float Damage)
{
	if (!AnimInstance || AnimInstance->bIsRolling) return; //|| AnimInstance->bIsRecovering
	
	CurrentHP -= Damage;
	
	UE_LOG(LogSoulsAI, Warning, TEXT("CurrHP: %f"), CurrentHP);
	
	if (CurrentHP <= 0.0f)
	{
		HandleDeath();
		return;
	}
	
	AnimInstance->Montage_Play(GetHitAnimMontage);
	AnimInstance->Montage_SetEndDelegate(OnGetHitMontageEnded, GetHitAnimMontage);

	// Disable movement while we are recovering
	GetCharacterMovement()->DisableMovement();
	AnimInstance->bIsRecovering = true;
}

void ASoulsPlayerCharacter::GetHitMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (bInterrupted)
	{
		return;
	}
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	AnimInstance->bIsRecovering = false;
}

void ASoulsPlayerCharacter::HandleDeath()
{
	UE_LOG(LogSoulsAI, Warning, TEXT("[ASoulsPlayerCharacter]: Death!"));
	AnimInstance->Montage_Play(DeathAnimMontage);

	// Disable movement while we are dead
	GetCharacterMovement()->DisableMovement();
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	
	// Enable full ragdoll physics
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetPhysicsBlendWeight(1.f);
}



