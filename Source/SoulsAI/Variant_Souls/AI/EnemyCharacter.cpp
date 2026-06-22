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
		constexpr float RotationThreshold = 30.0f; //degrees
		bIsRotatedToTarget = (FMath::Abs(FMath::FindDeltaAngleDegrees(CurrentYaw, TargetYaw))) < (RotationThreshold);
	}
	
}

void AEnemyCharacter::ClearCombatWheel()
{
	CombatWheel.Reset();
}

void AEnemyCharacter::AddActionToCombatWheel(const EAction Action, const int32 Probability /* = 1 */ )
{
	CombatWheel.Add(Action, Probability);
}

void AEnemyCharacter::AddActionArrayToCombatWheel(const TArray<FWeightedAction>& Actions)
{
	for (const FWeightedAction& Entry : Actions)
	{
		CombatWheel.Add(Entry.Action, Entry.Probability);
	}
}

EAction AEnemyCharacter::SelectActionFromCombatWheel()
{
	UE_LOG(LogSoulsAI, Warning, TEXT("SelectActionFromCombatWheel(): "));
	
	// Count total probabilities
	uint32 ProbabilitiesTotal = 0;
	for (const auto& Pair : CombatWheel)
	{
		ProbabilitiesTotal += Pair.Value;
		UE_LOG(LogSoulsAI, Warning, TEXT("- Action: %s, Probability: %u"), *UEnum::GetValueAsString(Pair.Key), Pair.Value);
	}
	UE_LOG(LogSoulsAI, Warning, TEXT("ProbabilitiesTotal = %u"), ProbabilitiesTotal);

	
	// Stop if there is no action in combat wheel
	if (ProbabilitiesTotal == 0)
	{
		// SelectedGoal = EGoal::Idle;
		return EAction::None;
	}
	
	
	// Generate random number and choose action
	const int32 Fate = FMath::RandRange(1, ProbabilitiesTotal);
	int32 TempFate = 0;
	for (const auto& Pair : CombatWheel)
	{
		TempFate += Pair.Value;
		if (TempFate >= Fate)
		{
			UE_LOG(LogSoulsAI, Warning, TEXT("SelectedAction: %s"), *UEnum::GetValueAsString(Pair.Key));
			return Pair.Key;
		}
	}
	UE_LOG(LogSoulsAI, Error, TEXT("No action selected"));

	return EAction::None;
}

void AEnemyCharacter::Attack(UAnimMontage *Montage)
{
	// UE_LOG(LogSoulsAI, Warning, TEXT("ATTACK"));
	CurrentMontage = Montage;
	AnimInstance->Montage_Play(CurrentMontage); //can specify InPlayRate float to slower AM for easier difficulty
	AnimInstance->Montage_SetEndDelegate(OnAttackMontageEnded, CurrentMontage);
	bAttackMontageEnded = false;
}

void AEnemyCharacter::AttackCombo()
{
	// UE_LOG(LogSoulsAI, Warning, TEXT("AttackCombo called"));
	const int32 CurrentMontageSectionCount = CurrentMontage ? CurrentMontage->GetNumSections() : 0;
	if (CurrentComboIndex < (CurrentMontageSectionCount - 1))
	{
		++CurrentComboIndex;
		AnimInstance->Montage_JumpToSection(ComboSectionNames[CurrentComboIndex], CurrentMontage);
	}
}

void AEnemyCharacter::AttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	UE_LOG(LogSoulsAI, Warning, TEXT("AttackMontageEnded"));
	bAttackMontageEnded = true;
	CurrentComboIndex = 0;
	CurrentMontage = nullptr;
}

void AEnemyCharacter::UpdateCombatWheel()
{
	
	// 1. Clear the combat wheel
	CombatWheel.Reset();
	
	// 2. Fill the combat wheel
	const float DistanceToTarget = FVector::Dist(GetActorLocation(), SelectedTarget->GetActorLocation());
	const bool TargetInAttackRange = (DistanceToTarget <= AttackRadius * 2);
	const int32 CurrentMontageSectionCount = CurrentMontage ? CurrentMontage->GetNumSections() : 0;

	//Note: if it wasn't for the probabilities update this could be directly in state tree
	// just transitions and select randomly
	switch (SelectedGoal) // selection based on previous goal (currently SelectedGoal)
	{
		case EGoal::Idle:
		case EGoal::Chase:
		case EGoal::Strafe:
			if ((DistanceToTarget < LongAttackRadius) && (DistanceToTarget > AttackRadius))
			{
				//Add probability of Long Attacks
				// AddLongAttacks(1);
				CombatWheel.Add(EAction::Chase, 2);
			} else if (TargetInAttackRange)
			{
				// AddAttacks(1);
			} else if (DistanceToTarget < ChaseRadius)
			{
				CombatWheel.Add(EAction::Chase, 1);
			}
			break;

		case EGoal::Combat:
				if (TargetInAttackRange && CurrentMontageSectionCount > 1)
				{
					CombatWheel.Add(EAction::DualSwordSwingRepeat, 4);
				}
				CombatWheel.Add(EAction::Strafe, 1);

			break;
		// case EGoal::Attack_Repeat:
		// 		if (TargetInAttackRange)
		// 		{
		// 			if (CurrentComboIndex < (CurrentMontageSectionCount - 2))
		// 			{
		// 				CombatWheel.Add(EAction::DualSwordSwingRepeat, 4);
		// 			} else
		// 			{
		// 				CombatWheel.Add(EAction::DualSwordSwingFinal, 4);
		// 			}
		// 		}
		// 		CombatWheel.Add(EAction::Strafe, 1);
		// 	break;
		// case EGoal::Attack_Final:
		// 		// CombatWheel.Add(EAction::Cooldown, 5);
		// 		// CombatWheel.Add(EAction::Roll, 5);
		// 		CombatWheel.Add(EAction::Strafe, 2);
		// 		if (TargetInAttackRange)
		// 		{
		// 			// AddAttacks(1);
		// 		} else
		// 		{
		// 			CombatWheel.Add(EAction::Chase, 2);
		// 			if (DistanceToTarget < LongAttackRadius)
		// 			{
		// 				// AddLongAttacks(1);
		// 			}
		// 		}
		// 	break;
				
		default:
			break;
	}
}
