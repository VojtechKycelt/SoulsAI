// Copyright (c) 2026 Vojtech Kycelt, master's thesis project at the Faculty of Mathematics and Physics, Charles University, Prague
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

class UStateTreeAIComponent;

UCLASS()
class SOULSAI_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()
	
	/** StateTree Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStateTreeAIComponent* StateTreeAI;

public:

	/** Constructor */
	AEnemyAIController();
	
};
