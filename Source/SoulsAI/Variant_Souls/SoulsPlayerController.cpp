// Fill out your copyright notice in the Description page of Project Settings.


#include "Variant_Souls/SoulsPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"

void ASoulsPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

		}
	}

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (PauseAction)
		{
			PauseAction->bTriggerWhenPaused = true; // Required!
			EnhancedInput->BindAction(PauseAction, ETriggerEvent::Triggered, this, &ASoulsPlayerController::Pause);
		}
	}
}

void ASoulsPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void ASoulsPlayerController::Pause()
{
	//Super::Pause();
	UE_LOG(LogTemp, Warning, TEXT("[ASoulsPlayerController::Pause] Not implemented"));

	// if (ASoulsHUD* HUD = Cast<ASoulsHUD>(GetHUD()))
	// {
	// 	HUD->PauseMenu();
	// }
	//
}
