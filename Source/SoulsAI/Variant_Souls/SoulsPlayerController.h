// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SoulsPlayerController.generated.h"

class UInputMappingContext;

/**
 *
 */
UCLASS()
class SOULSAI_API ASoulsPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category = "Input | Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditDefaultsOnly, Category = "Input | Input Mappings")
	class UInputAction* PauseAction;
	
	UFUNCTION(BlueprintImplementableEvent)
	void PauseGame();
};
