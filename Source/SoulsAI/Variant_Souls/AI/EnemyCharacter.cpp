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

	// GEngine->AddOnScreenDebugMessage(
	// -1,
	// 1.f,
	// FColor::Green,
	// UEnum::GetDisplayValueAsText(SelectedGoal).ToString()
	// );
	
	// Update UI
	DrawDebugString(
		GetWorld(),
		GetActorLocation() + FVector(0, 0, 100.f), // above head
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
		GetActorLocation() + FVector(0, 0, 150.f + (i * 100.f)), // above head
		UEnum::GetDisplayValueAsText(Subgoals[i]).ToString(),
		nullptr,
		FColor::White,
		0.f,     // duration (0 = one frame, so call every tick)
		true     // draw shadow
	);
	}
}

void AEnemyCharacter::SelectGoal()
{
	const APawn* Player = GetWorld()->GetFirstPlayerController()->GetPawn();
	if (!Player)
	{
		UE_LOG(LogSoulsAI, Error, TEXT("[AEnemyCharacter] Player is null"));
		return;
	}
	const float Distance = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
	
	while (! Subgoals.IsEmpty())
	{
		switch (const EGoalCommon NewGoal = Subgoals[0])
		{
			case EGoalCommon::Attack:
			case EGoalCommon::Combo_Attack:
			case EGoalCommon::Combo_Repeat:
			case EGoalCommon::Combo_Final:
				{
					Subgoals.RemoveAt(0);
					if (Distance <= AttackRadius)
					{
						SelectedGoal = NewGoal;
						return;
					} 
					break;
				}
			default:
				{
					SelectedGoal = NewGoal;
					Subgoals.RemoveAt(0);
					return;
				}
		}
	}
	SelectedGoal = EGoalCommon::Idle;
}
