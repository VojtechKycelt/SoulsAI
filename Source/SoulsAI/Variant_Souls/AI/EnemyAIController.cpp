// Copyright (c) 2026 Vojtech Kycelt, master's thesis project at the Faculty of Mathematics and Physics, Charles University, Prague

#include "Variant_Souls/AI/EnemyAIController.h"
#include "Components/StateTreeAIComponent.h"

AEnemyAIController::AEnemyAIController()
{
	// create the StateTree AI Component
	StateTreeAI = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeAI"));
	check(StateTreeAI);

	// ensure we start the StateTree when we possess the pawn
	bStartAILogicOnPossess = true;

	// ensure we're attached to the possessed character.
	// this is necessary for EnvQueries to work correctly
	bAttachToPawn = true;
}