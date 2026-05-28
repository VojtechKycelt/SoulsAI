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

void AEnemyCharacter::SelectGoal()
{
	if (!SelectedTarget)
	{
		// In reusable version of enemy, we could have 'DefaultGoal' var that can be set from BP - like ReturnToHomeLocation
		SelectedGoal = EGoal::Idle;
		return;
	}
	
	UpdateCombatWheel();
	
	SpinCombatWheel();
	
	UE_LOG(LogSoulsAI, Warning, TEXT("SelectGoal(): %s"),
	*UEnum::GetValueAsString(SelectedGoal));
	
}

void AEnemyCharacter::Attack()
{
	// UE_LOG(LogSoulsAI, Warning, TEXT("ATTACK"));
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
				AddLongAttacks(1);
				CombatWheel.Add(EAction::Chase, 2);
			} else if (TargetInAttackRange)
			{
				AddAttacks(1);
			} else if (DistanceToTarget < ChaseRadius)
			{
				CombatWheel.Add(EAction::Chase, 1);
			}
			break;

		case EGoal::Attack:
				if (TargetInAttackRange && CurrentMontageSectionCount > 1)
				{
					CombatWheel.Add(EAction::DualSwordSwingRepeat, 4);
				}
				CombatWheel.Add(EAction::Strafe, 1);

			break;
		case EGoal::Attack_Repeat:
				if (TargetInAttackRange)
				{
					if (CurrentComboIndex < (CurrentMontageSectionCount - 2))
					{
						CombatWheel.Add(EAction::DualSwordSwingRepeat, 4);
					} else
					{
						CombatWheel.Add(EAction::DualSwordSwingFinal, 4);
					}
				}
				CombatWheel.Add(EAction::Strafe, 1);
			break;
		case EGoal::Attack_Final:
				// CombatWheel.Add(EAction::Cooldown, 5);
				// CombatWheel.Add(EAction::Roll, 5);
				CombatWheel.Add(EAction::Strafe, 2);
				if (TargetInAttackRange)
				{
					AddAttacks(1);
				} else
				{
					CombatWheel.Add(EAction::Chase, 2);
					if (DistanceToTarget < LongAttackRadius)
					{
						AddLongAttacks(1);
					}
				}
			break;
				
		default:
			break;
	}
}

void AEnemyCharacter::AddLongAttacks(const uint32 Probability /* = 1 */)
{
	CombatWheel.Add(EAction::DashSlash, Probability);
	CombatWheel.Add(EAction::ChargeTwoHandStab, Probability);
	CombatWheel.Add(EAction::LeapStab, Probability);
	CombatWheel.Add(EAction::RollSlam, Probability);
	CombatWheel.Add(EAction::JumpSlam, Probability);
}

void AEnemyCharacter::AddAttacks(const uint32 Probability /* = 1 */)
{
	CombatWheel.Add(EAction::UpperCut, Probability);
	CombatWheel.Add(EAction::DualSwordSwing, Probability);
}

void AEnemyCharacter::SpinCombatWheel()
{
	uint32 ProbabilitiesTotal = 0;
	UE_LOG(LogSoulsAI, Warning, TEXT("SpinCombatWheel(): "));
	for (const auto& Pair : CombatWheel)
	{
		ProbabilitiesTotal += Pair.Value;
		UE_LOG(LogSoulsAI, Warning, TEXT("Action: %s, Probability: %u"), *UEnum::GetValueAsString(Pair.Key), Pair.Value);
	}
	UE_LOG(LogSoulsAI, Warning, TEXT("ProbabilitiesTotal: %u"), ProbabilitiesTotal);

	if (ProbabilitiesTotal == 0)
	{
		SelectedGoal = EGoal::Idle;
		return;
	}
	
	const int32 Fate = FMath::RandRange(1, ProbabilitiesTotal);
	int32 TempFate = 0;
	EAction SelectedAction = EAction::None;
	for (const auto& Pair : CombatWheel)
	{
		TempFate += Pair.Value;
		if (TempFate >= Fate)
		{
			SelectedAction = Pair.Key;
			break;
		}
	}
	UE_LOG(LogSoulsAI, Warning, TEXT("SelectedAction: %s"), *UEnum::GetValueAsString(SelectedAction));

	PerformAction(SelectedAction);
}

void AEnemyCharacter::PerformAction(const EAction Action)
{
	switch (Action)
	{
		// Long Attacks
		case EAction::DashSlash:
			bShouldRotateToTarget = true;
			CurrentMontage = AM_DashSlash;
			SelectedGoal = EGoal::Attack;
			break;
		case EAction::ChargeTwoHandStab:
			bShouldRotateToTarget = true;
			CurrentMontage = AM_ChargeTwoHandStab;
			SelectedGoal = EGoal::Attack;
			break;
		case EAction::LeapStab:
			bShouldRotateToTarget = true;
			CurrentMontage = AM_LeapStab;
			SelectedGoal = EGoal::Attack;
			break;
		case EAction::RollSlam:
			bShouldRotateToTarget = true;
			CurrentMontage = AM_RollSlam;
			SelectedGoal = EGoal::Attack;
			break;
		case EAction::JumpSlam:
			bShouldRotateToTarget = true;
			CurrentMontage = AM_JumpSlam;
			SelectedGoal = EGoal::Attack;
			break;

		// Attacks
		case EAction::UpperCut:
			bShouldRotateToTarget = true;
			CurrentMontage = AM_UpperCut;
			SelectedGoal = EGoal::Attack;
			break;
		case EAction::DualSwordSwing:
			bShouldRotateToTarget = true;
			CurrentMontage = AM_DualSwordSwing;
			SelectedGoal = EGoal::Attack;
			break;
		
		// Combos
		case EAction::DualSwordSwingRepeat:
			bShouldRotateToTarget = true;
			CurrentMontage = AM_DualSwordSwing;
			SelectedGoal = EGoal::Attack_Repeat;
			AttackCombo();
			break;
		case EAction::DualSwordSwingFinal:
			bShouldRotateToTarget = true;
			CurrentMontage = AM_DualSwordSwing;
			SelectedGoal = EGoal::Attack_Final;
			AttackCombo();
			break;

		// Movement
		case EAction::Chase:
			bShouldRotateToTarget = true;
			SelectedGoal = EGoal::Chase;
			break;
		case EAction::Strafe:
			bShouldRotateToTarget = true;
			SelectedGoal = EGoal::Strafe;
			break;

		// Dodge
		// case EAction::Roll:
		// 	// Roll -> SelectGoal()
		// 	SelectedGoal = EGoal::Roll;
		// 	break;

		// Cooldown
		// case EAction::Cooldown:
		// 	SelectedGoal = EGoal::Cooldown;
		// 	break;

		default:
			SelectedGoal = EGoal::Idle;
			break;
	}
	
}

// void CheckSubgoals()
// {
// 	while (! Subgoals.IsEmpty())
// 	{
// 		switch (const EGoal NewGoal = Subgoals[0])
// 		{
// 			case EGoal::Attack:
// 			case EGoal::Attack_Repeat:
// 			case EGoal::Attack_Final:
// 				{
// 					Subgoals.RemoveAt(0);
// 					// If the player distance is bigger than AttackRadius * 2, do not attack since he is too far.
// 					if (TargetInAttackRange)
// 					{
// 						SelectedGoal = NewGoal;
// 						return;
// 					}
// 					break;
// 				}
// 			default:
// 				{
// 					SelectedGoal = NewGoal;
// 					Subgoals.RemoveAt(0);
// 					return;
// 				}
// 		}
// 	}
// 	SelectedGoal = EGoal::Idle;
// }
