// Copyright Lost in Game Studio, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VirtualControlSetup.h"
#include "GameFramework/SaveGame.h"
#include "VirtualControlSave.generated.h"

USTRUCT(BlueprintType)
struct FTouchInterfaceConfiguration
{
	GENERATED_BODY()

	UPROPERTY(Category="GlobalSettings", BlueprintReadOnly)
	float ActiveOpacity;
	
	UPROPERTY(Category="GlobalSettings", BlueprintReadOnly)
	float InactiveOpacity;

	UPROPERTY(Category="GlobalSettings", BlueprintReadOnly)
	float TimeUntilDeactivated;

	UPROPERTY(Category="GlobalSettings", BlueprintReadOnly)
	float TimeUntilReset;

	UPROPERTY(Category="GlobalSettings", BlueprintReadOnly)
	float ActivationDelay;

	UPROPERTY(Category="GlobalSettings", BlueprintReadOnly)
	float StartupDelay;

	UPROPERTY(Category="GlobalSettings", BlueprintReadOnly)
	bool bCalculatePositionAuto;

	UPROPERTY(Category="VirtualControl", BlueprintReadOnly)
	TArray<FVirtualControl> VirtualControls;

	FTouchInterfaceConfiguration() :
	ActiveOpacity(1.0f),
	InactiveOpacity(0.2f),
	TimeUntilDeactivated(2.0f),
	TimeUntilReset(3.0f),
	ActivationDelay(0.0f),
	StartupDelay(0.0f),
	bCalculatePositionAuto(false)
	{
		VirtualControls = {};
	}
};

/**
 * 
 */
UCLASS()
class TOUCHINTERFACE_API UVirtualControlSave : public USaveGame
{
	GENERATED_BODY()

public:
	UVirtualControlSave();

	UPROPERTY(Category="SlotData", BlueprintReadOnly)
	FString SaveSlotName;

	UPROPERTY(Category="SlotData", BlueprintReadOnly)
	int32 UserIndex;

	UPROPERTY(Category="SlotData", BlueprintReadOnly)
	TMap<int32, FTouchInterfaceConfiguration> Configurations;
};
