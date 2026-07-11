// Copyright (c) 2026 Vojtech Kycelt, master's thesis project at the Faculty of Mathematics and Physics, Charles University, Prague

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SoulsGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class SOULSAI_API USoulsGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDebugUIEnabled = false;
};
