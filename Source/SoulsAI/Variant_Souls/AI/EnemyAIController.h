// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

class UStateTreeAIComponent;

/**
 * 
 */
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
