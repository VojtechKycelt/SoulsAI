// Copyright (c) 2026 Vojtech Kycelt, master's thesis project at the Faculty of Mathematics and Physics, Charles University, Prague

#include "Variant_Souls/AI/EnemyCharacter.h"
#include "SoulsAI.h"
#include "GameFramework/CharacterMovementComponent.h"

AEnemyCharacter::AEnemyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// Do not rotate character instantly to match controller (prevents snapping to AI focus)
	bUseControllerRotationYaw = false;

	// Rotate character smoothly toward movement direction
	GetCharacterMovement()->bOrientRotationToMovement = true;
	
	// Set the current hp to maximum amount of hp
	CurrentHP = MaxHP;
}

// Called when the game starts or when spawned
void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		UE_LOG(LogSoulsAI, Error, TEXT("[AEnemyCharacter] AnimInstance is null"));
	}
}

// Called every frame
void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (bIsDead)
	{
		return;
	}
	
	if (! SelectedTarget)
	{
		bIsRotatedToTarget = false;
	} else
	{
		// Calculate direction to target
		FVector DirectionToTarget = SelectedTarget->GetActorLocation() - GetActorLocation();
		DirectionToTarget.Z = 0.0f;  // Ignore height difference - makes it 2D
		DirectionToTarget.Normalize();

		const float TargetYaw = DirectionToTarget.Rotation().Yaw;
		const float CurrentYaw = GetActorRotation().Yaw;
		bIsRotatedToTarget = (FMath::Abs(FMath::FindDeltaAngleDegrees(CurrentYaw, TargetYaw))) < (RotationToTargetThreshold);
	}
	
} 
