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
	case EGoal::Attack:
	case EGoal::Attack_Repeat:
	case EGoal::Attack_Final:
		//	add boolean should rotate to target and rotate only if its true
		// could be handy just to rotate half of attack montage or something
	case EGoal::Strafe:
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
	// TODO Check if SelectedTarget is valid and if we shouldn't change it 
	// based on flags and distance and who attacked us
	if (!SelectedTarget)
	{
		SelectedGoal = EGoal::Idle;
		return;
	}
	// UE_LOG(LogSoulsAI, Warning, TEXT("SelectGoal()"));
	
	UpdateCombatWheel();
	
	//maybe return bool if task should finish?
	SpinCombatWheel();
	UE_LOG(LogSoulsAI, Warning, TEXT("SelectGoal(): %s"),
	*UEnum::GetValueAsString(SelectedGoal));

	// const float DistanceToTarget = FVector::Dist(GetActorLocation(), SelectedTarget->GetActorLocation());
	// const bool TargetInAttackRange = (DistanceToTarget <= AttackRadius * 2);
	// if (SelectedGoal == EGoal::Attack)
	// {
	// 	//UpdateCombatWheel()
	// 	
	// 	//Spin the wheel
	// 	const int32 CurrentMontageSectionCount = CurrentMontage ? CurrentMontage->GetNumSections() : 0;
	// 	const int32 Random = FMath::RandRange(1, 10); // inclusive on both ends
	// 	if (TargetInAttackRange && (Random <= 8) && CurrentComboIndex < (CurrentMontageSectionCount - 1))
	// 	{
	// 		++CurrentComboIndex;
	// 		AnimInstance->Montage_JumpToSection(ComboSectionNames[CurrentComboIndex], CurrentMontage);
	// 		return;
	// 	}
	// 	
	// 	// UE_LOG(LogSoulsAI, Warning, TEXT("[AEnemyCharacter] Random number: %i"), Random);
	// }
	
	// while (! Subgoals.IsEmpty())
	// {
	// 	switch (const EGoal NewGoal = Subgoals[0])
	// 	{
	// 		case EGoal::Attack:
	// 		case EGoal::Attack_Repeat:
	// 		case EGoal::Attack_Final:
	// 			{
	// 				Subgoals.RemoveAt(0);
	// 				// If the player distance is bigger than AttackRadius * 2, do not attack since he is too far.
	// 				if (TargetInAttackRange)
	// 				{
	// 					SelectedGoal = NewGoal;
	// 					return;
	// 				}
	// 				break;
	// 			}
	// 		default:
	// 			{
	// 				SelectedGoal = NewGoal;
	// 				Subgoals.RemoveAt(0);
	// 				return;
	// 			}
	// 	}
	// }
	// SelectedGoal = EGoal::Idle;
}

void AEnemyCharacter::Attack()
{
	// UE_LOG(LogSoulsAI, Warning, TEXT("ATTACK"));
	// SelectedGoal = EGoal::Attack;
	AnimInstance->Montage_Play(CurrentMontage);
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

	switch (SelectedGoal) // selection based on previous goal (currently SelectedGoal)
	{
		case EGoal::Idle:
		case EGoal::Chase:
			if ((DistanceToTarget < LongAttackRadius) && (DistanceToTarget > AttackRadius))
			{
				//Add probability of Long Attacks
				CombatWheel.Add(EAction::DashSlash, 5);
				CombatWheel.Add(EAction::ChargeTwoHandStab, 5);
				CombatWheel.Add(EAction::LeapStab, 5);
				CombatWheel.Add(EAction::RollSlam, 5);
				CombatWheel.Add(EAction::JumpSlam, 5);
			} else if (DistanceToTarget < (AttackRadius * 2))
			{
				CombatWheel.Add(EAction::UpperCut, 50);
				CombatWheel.Add(EAction::DualSwordSwing, 50);
			} else if (DistanceToTarget < ChaseRadius)
			{
				CombatWheel.Add(EAction::Chase, 25);
			}
			break;

		case EGoal::Attack:
				if (TargetInAttackRange && CurrentMontageSectionCount > 1)
				{
					CombatWheel.Add(EAction::DualSwordSwingRepeat, 75);
				}
				CombatWheel.Add(EAction::Strafe, 25);

			break;
		case EGoal::Attack_Repeat:
				if (TargetInAttackRange)
				{
					if (CurrentComboIndex < (CurrentMontageSectionCount - 2))
					{
						CombatWheel.Add(EAction::DualSwordSwingRepeat, 75);
					} else
					{
						CombatWheel.Add(EAction::DualSwordSwingFinal, 75);
					}
				}
				CombatWheel.Add(EAction::Strafe, 25);
			break;
		case EGoal::Attack_Final:
				CombatWheel.Add(EAction::Strafe, 50);
				CombatWheel.Add(EAction::Chase, 50);
				// CombatWheel.Add(EAction::Cooldown, 50);
				// CombatWheel.Add(EAction::Roll, 50);
			break;
		case EGoal::Strafe:
				if (DistanceToTarget <= ChaseRadius)
				{
					CombatWheel.Add(EAction::Chase, 100);
				}
			break;
		default:
			break;
	}
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
			// Attack(AM_DashSlash);
			CurrentMontage = AM_DashSlash;
			SelectedGoal = EGoal::Attack;
			break;
		case EAction::ChargeTwoHandStab:
			// Attack(AM_ChargeTwoHandStab);
			CurrentMontage = AM_ChargeTwoHandStab;
			SelectedGoal = EGoal::Attack;
			break;
		case EAction::LeapStab:
			CurrentMontage = AM_LeapStab;
			SelectedGoal = EGoal::Attack;
			break;
		case EAction::RollSlam:
			// Attack(AM_RollSlam);
			CurrentMontage = AM_RollSlam;
			SelectedGoal = EGoal::Attack;
			break;
		case EAction::JumpSlam:
			// Attack(AM_JumpSlam);
			CurrentMontage = AM_JumpSlam;
			SelectedGoal = EGoal::Attack;
			break;

		// Attacks
		case EAction::UpperCut:
			// Attack(AM_UpperCut);
			CurrentMontage = AM_UpperCut;
			SelectedGoal = EGoal::Attack;
			break;
		case EAction::DualSwordSwing:
			// Attack(AM_DualSwordSwing);
			CurrentMontage = AM_DualSwordSwing;
			SelectedGoal = EGoal::Attack;
			break;
		
		// Combos
		case EAction::DualSwordSwingRepeat:
			// AttackCombo(AM_DualSwordSwing);
			CurrentMontage = AM_DualSwordSwing;
			SelectedGoal = EGoal::Attack_Repeat;
			AttackCombo();
			break;
		case EAction::DualSwordSwingFinal:
			// AttackCombo(AM_DualSwordSwing);
			CurrentMontage = AM_DualSwordSwing;
			SelectedGoal = EGoal::Attack_Final;
			AttackCombo();
			break;

		// Movement
		case EAction::Chase:
			SelectedGoal = EGoal::Chase;
			break;
		case EAction::Strafe:
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
	
	//Maybe stop all currently running tasks here somehow?
}
