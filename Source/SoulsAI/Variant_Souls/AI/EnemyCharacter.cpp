// Fill out your copyright notice in the Description page of Project Settings.


#include "Variant_Souls/AI/EnemyCharacter.h"
#include "EnemyAnimInstance.h"
#include "SoulsAI.h"
#include "BaseGizmos/GizmoElementShared.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AEnemyCharacter::AEnemyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// Do not rotate character instantly to match controller (prevents snapping to AI focus)
	bUseControllerRotationYaw = false;

	// Rotate character smoothly toward movement direction
	GetCharacterMovement()->bOrientRotationToMovement = true;
	
	CurrentHP = MaxHP;
}

// Called when the game starts or when spawned
void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	AnimInstance = Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance());
	if (!AnimInstance)
	{
		UE_LOG(LogSoulsAI, Error, TEXT("[AEnemyCharacter] AnimInstance is null"));
	}
}

// Called every frame
void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Update UI
	if (SelectedTarget)
	{
		DrawDebugString(
			GetWorld(),
			GetActorLocation() + FVector(0.f, 0, 100.f), // above head
			FString::Printf(TEXT("Distance: %.1f"),
				FVector::Dist(GetActorLocation(), SelectedTarget->GetActorLocation())),
			nullptr,
			FColor::Cyan,
			0.f,     // duration (0 = one frame, so call every tick)
			true     // draw shadow
		);
	}
	DrawDebugString(
		GetWorld(),
		GetActorLocation() + FVector(0, 0, 200.f), // above head
		UEnum::GetDisplayValueAsText(SelectedGoal).ToString(),
		nullptr,
		FColor::Green,
		0.f,     // duration (0 = one frame, so call every tick)
		true     // draw shadow
	);
	
	for (size_t i = 0; i < Subgoals.Num(); ++i)
	{
		DrawDebugString(
		GetWorld(),
		GetActorLocation() + FVector(0, 0, 250.f + (i * 100.f)), // above head
		UEnum::GetDisplayValueAsText(Subgoals[i]).ToString(),
		nullptr,
		FColor::White,
		0.f,     // duration (0 = one frame, so call every tick)
		true     // draw shadow
	);
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
