// Copyright (c) 2026 Vojtech Kycelt, master's thesis project at the Faculty of Mathematics and Physics, Charles University, Prague

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

ASoulsPlayerCharacter::ASoulsPlayerCharacter()
{
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // true causes rotation jitter
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
	
	// Bind Montage Ended events to appropriate functions
	OnAttackMontageEnded.BindUObject(this, &ASoulsPlayerCharacter::AttackMontageEnded);
	OnGetHitMontageEnded.BindUObject(this, &ASoulsPlayerCharacter::GetHitMontageEnded);
	OnUseItemMontageEnded.BindUObject(this, &ASoulsPlayerCharacter::UseItemMontageEnded);
	OnRollMontageEnded.BindUObject(this, &ASoulsPlayerCharacter::RollMontageEnded);
	
	// Stats
	CurrentHP = MaxHP;
	CurrentStamina = MaxStamina;
	CurrentDamage = LightAttackDamage;
	CurrentHealFlasksCount = MaxHealFlasksCount;
}

void ASoulsPlayerCharacter::CheckCachedInput()
{
	if (GetWorld()->GetTimeSeconds() - InputCachedTime <= InputCachedTimeTolerance)
	{
		InputCachedTime = 0.0f;

		switch (CachedInputType)
		{
			case ECachedInputType::Roll:
				Roll();
				break;
			case ECachedInputType::LightAttack:
				LightAttack();
				break;
			case ECachedInputType::HeavyAttack:
				HeavyAttack();
				break;
			default:
				break;
		}
		
		CachedInputType = ECachedInputType::None;
	}
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
		EnhancedInputComponent->BindAction(CameraLockAction, ETriggerEvent::Started, this, &ASoulsPlayerCharacter::CameraTargetLock);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASoulsPlayerCharacter::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ASoulsPlayerCharacter::MoveCompleted);

		// Looking
		// Mouse
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ASoulsPlayerCharacter::Look);
		// Controller
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASoulsPlayerCharacter::Look);

		//Rolling
		EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Triggered, this, &ASoulsPlayerCharacter::RollPressed);
		
		//Rolling
		EnhancedInputComponent->BindAction(UseItemAction, ETriggerEvent::Triggered, this, &ASoulsPlayerCharacter::UseItemPressed);

		//Attacks
		EnhancedInputComponent->BindAction(LightAttackAction, ETriggerEvent::Triggered, this, &ASoulsPlayerCharacter::LightAttackPressed);
		EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Triggered, this, &ASoulsPlayerCharacter::HeavyAttackPressed);
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
	AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		UE_LOG(LogSoulsAI, Error, TEXT("[ASoulsPlayerCharacter] AnimInstance is null"));
	}

}

// Called every frame
void ASoulsPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	switch (CameraState)
	{
	case ECameraState::Default:
		FollowCamera->SetWorldRotation(
			FMath::RInterpTo(
				FollowCamera->GetComponentRotation(), 
				CameraBoom->PreviousDesiredRot, 
				DeltaTime, 
				5.f));
		
		// Character faces the movement direction - velocity
		// Custom orient-rotation-to-movement because the built-in one causes jitter
		if (!bIsRecovering && !bIsRolling && !bIsAttacking)
		{
			FVector CurrentDir = GetActorRotation().Vector();
			CurrentDir.Z = 0.f;
			CurrentDir.Normalize();
			
			FVector TargetDir = GetVelocity();
			TargetDir.Z = 0.f;
			TargetDir.Normalize();
			
			if (! TargetDir.IsNearlyZero())
			{
				SetActorRotation(
					FMath::RInterpConstantTo(
						CurrentDir.Rotation(), 
						TargetDir.Rotation(),
						DeltaTime, 
						GetCharacterMovement()->RotationRate.Yaw));
			}
		}
		
		break;
	case ECameraState::Locked:
		if (!LockedTarget || (FVector::Dist(GetActorLocation(), LockedTarget->GetActorLocation()) > LockOnRadius))
		{
			SwitchCameraState();
			break;
		} else if (const AEnemyCharacter* EnemyTarget = Cast<AEnemyCharacter>(LockedTarget))
		{
			if (EnemyTarget->bIsDead)
			{
				SwitchCameraState();
				break;
			}
		}
		
		// Character faces enemy
		if (! bIsRecovering && !bIsRolling)
		{
			FVector ToEnemy = LockedTarget->GetActorLocation() - GetActorLocation();
			ToEnemy.Z = 0.f;
			ToEnemy.Normalize();
			SetActorRotation(
				FMath::RInterpConstantTo(
					GetActorRotation(), 
					ToEnemy.Rotation(),
					DeltaTime, 
					GetCharacterMovement()->RotationRate.Yaw));
		}
		{
			// Camera looks at enemy
			const FRotator FollowCamTargetRotation = FRotator((LockedTarget->GetActorLocation() - FollowCamera->GetComponentLocation()).Rotation());
			FollowCamera->SetWorldRotation(FMath::RInterpTo(FollowCamera->GetComponentRotation(), FollowCamTargetRotation, DeltaTime, 10.f));
			
			// Camera positioned behind players back, slightly up (- pitch)
			FRotator CamBoomTargetRotation = FRotator((LockedTarget->GetActorLocation() - GetActorLocation()).Rotation());
			CamBoomTargetRotation.Pitch -= 30;
			Controller->SetControlRotation(FMath::RInterpTo(Controller->GetControlRotation(), CamBoomTargetRotation, DeltaTime, 10.f));
			
			break;
		}
	default:
		break;
	}
	
	if (GetCharacterMovement()->Velocity.IsNearlyZero())
	{
		MovementRight = 0.0f;
		MovementForward = 0.0f;
	}
	if (! bIsRolling && !bIsAttacking)
	{
		CurrentStamina += StaminaRecoverySpeed * DeltaTime;
		CurrentStamina = FMath::Clamp(CurrentStamina, 0.0f, MaxStamina);
	}
}

bool ASoulsPlayerCharacter::CanPerformAction()
{
	if (!AnimInstance) return false;
	if (bIsRolling || GetCharacterMovement()->IsFalling() || bIsRecovering || bIsHealing) return false;
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

void ASoulsPlayerCharacter::SwitchCameraState()
{
    if (LockedTarget)
    {
        CameraState = ECameraState::Default;
        LockedTarget = nullptr;
        return;
    }

    FindLockOnTarget();

    if (LockedTarget)
    {
        CameraState = ECameraState::Locked;
    }
}

void ASoulsPlayerCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void ASoulsPlayerCharacter::MoveCompleted(const FInputActionValue& Value)
{
	MovementRight = 0.0f;
	MovementForward = 0.0f;
}

void ASoulsPlayerCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, -LookAxisVector.Y);
}

void ASoulsPlayerCharacter::RollPressed(const FInputActionValue& Value)
{
	MovementRightCached = MovementRight;
	MovementForwardCached = MovementForward;
	
	if (AnimInstance->IsAnyMontagePlaying())
	{
		InputCachedTime = GetWorld()->GetTimeSeconds();
		CachedInputType = ECachedInputType::Roll;
		return;
	}
	
	Roll();
}

void ASoulsPlayerCharacter::Roll()
{
	if (CurrentStamina < RollStaminaCost || ! CanPerformAction())
	{
		return;
	}
	CurrentStamina -= RollStaminaCost;
	
	if ((MovementRightCached == 0.f) && (MovementForwardCached == 0.f)) {
		AnimInstance->Montage_Play(DodgeAnimMontage);
		AnimInstance->Montage_SetEndDelegate(OnRollMontageEnded, DodgeAnimMontage);
	}
	else {
		if (CameraState == ECameraState::Locked)
		{
			if (FMath::Abs(MovementRightCached) > FMath::Abs(MovementForwardCached))
			{
				// predominantly moving sideways
				if (MovementRightCached > 0.f)
				{
					AnimInstance->Montage_Play(RollRightAnimMontage);
					AnimInstance->Montage_SetEndDelegate(OnRollMontageEnded, RollRightAnimMontage);
				}
				else
				{
					AnimInstance->Montage_Play(RollLeftAnimMontage);
					AnimInstance->Montage_SetEndDelegate(OnRollMontageEnded, RollLeftAnimMontage);
				}
			}
			else
			{
				// predominantly moving forward/backward
				if (MovementForwardCached >= 0.f)
				{
					AnimInstance->Montage_Play(RollForwardAnimMontage);
					AnimInstance->Montage_SetEndDelegate(OnRollMontageEnded, RollForwardAnimMontage);
				}
				else
				{
					AnimInstance->Montage_Play(RollBackwardAnimMontage);
					AnimInstance->Montage_SetEndDelegate(OnRollMontageEnded, RollBackwardAnimMontage);
				}
			}
		} else
		{
			AnimInstance->Montage_Play(RollForwardAnimMontage);
			AnimInstance->Montage_SetEndDelegate(OnRollMontageEnded, RollForwardAnimMontage);
		}
	}
}

void ASoulsPlayerCharacter::UseItemPressed(const FInputActionValue& Value)
{
	if (AnimInstance->IsAnyMontagePlaying())
	{
		return;
	}
	UseItem();
}

void ASoulsPlayerCharacter::UseItem()
{
	if (CanPerformAction() && CurrentHealFlasksCount > 0)
	{
		bIsHealing = true;
		GetCharacterMovement()->MaxWalkSpeed = UsingItemMovementSpeed;
		AnimInstance->Montage_Play(DrinkAnimMontage);
		AnimInstance->Montage_SetEndDelegate(OnUseItemMontageEnded, DrinkAnimMontage);
	}
}


void ASoulsPlayerCharacter::RollMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (bInterrupted)
	{
		return;
	}
	CheckCachedInput();
}

void ASoulsPlayerCharacter::LightAttackPressed(const FInputActionValue& Value)
{
	InputCachedTime = GetWorld()->GetTimeSeconds();
	CachedInputType = ECachedInputType::LightAttack;
	LightAttack();
}

void ASoulsPlayerCharacter::LightAttack()
{
	if (! CanPerformAction() || CurrentStamina < LightAttackStaminaCost)
	{
		return;
	}
	CurrentDamage = LightAttackDamage;
	
	 if (CurrentComboIndex == 0 && !bIsAttacking) 
	{
		bIsAttacking = true;
		CurrentStamina -= LightAttackStaminaCost;
		AnimInstance->Montage_Play(LightAttackAnimMontage);
		AnimInstance->Montage_SetEndDelegate(OnAttackMontageEnded, LightAttackAnimMontage);
	} else if (bComboInputWindowOpen)
	{
		bShouldContinueCombo = true;
	}
}

void ASoulsPlayerCharacter::HeavyAttackPressed(const FInputActionValue& Value)
{
	InputCachedTime = GetWorld()->GetTimeSeconds();
	CachedInputType = ECachedInputType::HeavyAttack;
	if (AnimInstance->IsAnyMontagePlaying())
	{
		return;
	}
	HeavyAttack();
}

void ASoulsPlayerCharacter::HeavyAttack()
{
	if (CurrentStamina < HeavyAttackStaminaCost || !CanPerformAction())
	{
		return;
	}
	
	CurrentDamage = HeavyAttackDamage;
	bIsAttacking = true;
	CurrentStamina -= HeavyAttackStaminaCost;
	AnimInstance->Montage_Play(HeavyAttackAnimMontage);
	AnimInstance->Montage_SetEndDelegate(OnAttackMontageEnded, HeavyAttackAnimMontage);
}

void ASoulsPlayerCharacter::CheckCombo()
{
	if (! bShouldContinueCombo)
	{
		return;
	}
	
	const int32 SectionCount = AnimInstance->GetCurrentActiveMontage()->GetNumSections();
	
	if (CurrentComboIndex < SectionCount - 1)
	{
		bIsAttacking = true;
		AnimInstance->Montage_JumpToSection(ComboSectionNames[CurrentComboIndex + 1], LightAttackAnimMontage);
		bShouldContinueCombo = false;
		bComboInputWindowOpen = false;
		CurrentComboIndex += 1;
		CurrentStamina -= LightAttackStaminaCost;
	}
}

void ASoulsPlayerCharacter::CheckRollAttack()
{
	const bool bInputInCachedThreshold = GetWorld()->GetTimeSeconds() - InputCachedTime <= InputCachedTimeTolerance;
	const bool bEnoughStamina = CurrentStamina > LightAttackStaminaCost;
	const bool bCorrectCachedInput = CachedInputType == ECachedInputType::LightAttack;
	
	if (! bInputInCachedThreshold || ! bEnoughStamina || ! bCorrectCachedInput)
	{
		return;
	}
	
	bIsRolling = false;
	bIsAttacking = true;
	CurrentStamina -= LightAttackStaminaCost;
	
	const int32 SectionIndex = LightAttackAnimMontage->GetSectionIndex(TEXT("Attack3"));
	if (SectionIndex != INDEX_NONE)
	{
		float SectionStart = 0.0f;
		float SectionEnd   = 0.0f;
		LightAttackAnimMontage->GetSectionStartAndEndTime(SectionIndex, SectionStart, SectionEnd);

		const float StartTime = SectionStart + 0.2f;
		CurrentDamage = LightAttackDamage;
		AnimInstance->Montage_Play(
			LightAttackAnimMontage,
			1.1f,
			EMontagePlayReturnType::MontageLength,
			StartTime,
			true);
		AnimInstance->Montage_SetEndDelegate(OnAttackMontageEnded, LightAttackAnimMontage);
	} else
	{
		UE_LOG(LogSoulsAI, Error, TEXT("[CheckRollAttack] No section found"));
	}
	
}

void ASoulsPlayerCharacter::AttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsAttacking = false;
	CurrentComboIndex = 0;
	bShouldContinueCombo = false;
	bComboInputWindowOpen = false;
	CheckCachedInput();
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
	SwitchCameraState();
}

void ASoulsPlayerCharacter::DoMove(float Right, float Forward)
{
	if (GetController() == nullptr)
	{
		return;
	}
	
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

void ASoulsPlayerCharacter::GetHit(const float Damage)
{
	if (!AnimInstance || bIsRolling) return;
	
	InputCachedTime = 0.0f;
	CachedInputType = ECachedInputType::None;
	
	CurrentHP -= Damage;
	CurrentHP = FMath::Clamp(CurrentHP, 0.0f, MaxHP);
	
	if (CurrentHP <= 0.0f)
	{
		HandleDeath();
		return;
	}
	
	AnimInstance->Montage_Play(GetHitAnimMontage);
	AnimInstance->Montage_SetEndDelegate(OnGetHitMontageEnded, GetHitAnimMontage);

	// Disable movement while we are recovering
	GetCharacterMovement()->DisableMovement();
	bIsRecovering = true;
}

void ASoulsPlayerCharacter::GetHitMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (bInterrupted)
	{
		return;
	}
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	bIsRecovering = false;
}

void ASoulsPlayerCharacter::UseItemMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (bInterrupted)
	{
		UseItemInterrupted();
	}
	bIsHealing = false;
	GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
}

void ASoulsPlayerCharacter::HandleDeath()
{
	// UE_LOG(LogSoulsAI, Warning, TEXT("[ASoulsPlayerCharacter]: Death!"));
	AnimInstance->Montage_Play(DeathAnimMontage);

	// Disable movement while we are dead
	GetCharacterMovement()->DisableMovement();
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	
	// Enable full ragdoll physics
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetPhysicsBlendWeight(1.f);
}



