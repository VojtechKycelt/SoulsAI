// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SoulsPlayerCharacterAnimInstance.generated.h"

/**
 *
 */
UCLASS()
class SOULSAI_API USoulsPlayerCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character State")
	bool isRolling;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character State")
	bool isAttacking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character State")
	bool isAnimating;
};
