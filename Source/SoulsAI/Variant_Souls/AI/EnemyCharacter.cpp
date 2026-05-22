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
	
	OnAttackMontageEnded.BindUObject(this, &AEnemyCharacter::AttackMontageEnded);
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
	
	switch (SelectedGoal)
	{
	case EGoalCommon::Attack:
	case EGoalCommon::Strafe:
		{
			if (! SelectedTarget)
			{
				break;
			}
			// Character faces enemy
			FVector ToEnemy = SelectedTarget->GetActorLocation() - GetActorLocation();
			ToEnemy.Z = 0.f;
			ToEnemy.Normalize();
			SetActorRotation(
				FMath::RInterpTo(
					GetActorRotation(), 
					ToEnemy.Rotation(),
					DeltaTime, 
					GetCharacterMovement()->RotationRate.Yaw / 100.f));
			break;
		}
	default:
		{
			break;
		}
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
	const bool PlayerInAttackRange = (Distance <= AttackRadius * 2);
	
	if (SelectedGoal == EGoalCommon::Attack)
	{
		//UpdateProbabilitiesWheel()
		
		//Spin the wheel
		const int32 CurrentMontageSectionCount = CurrentMontage ? CurrentMontage->GetNumSections() : 0;
		const int32 Random = FMath::RandRange(1, 10); // inclusive on both ends
		if (PlayerInAttackRange && (Random <= 8) && CurrentComboIndex < (CurrentMontageSectionCount - 1))
		{
			++CurrentComboIndex;
			AnimInstance->Montage_JumpToSection(ComboSectionNames[CurrentComboIndex], CurrentMontage);
			return;
		}
		
		// UE_LOG(LogSoulsAI, Warning, TEXT("[AEnemyCharacter] Random number: %i"), Random);
	}
	
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
					// If the player distance is bigger than AttackRadius * 2, do not attack since he is too far.
					if (PlayerInAttackRange)
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

void AEnemyCharacter::Attack()
{
	// UE_LOG(LogSoulsAI, Warning, TEXT("ATTACK"));
	CurrentMontage = DashSlashAnimMontage;
	AnimInstance->Montage_Play(CurrentMontage);
	AnimInstance->Montage_SetEndDelegate(OnAttackMontageEnded, CurrentMontage);
	bAttackMontageEnded = false;
}

void AEnemyCharacter::AttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// UE_LOG(LogSoulsAI, Warning, TEXT("AttackMontageEnded"));
	bAttackMontageEnded = true;
	CurrentComboIndex = 0;
	CurrentMontage = nullptr;
}
