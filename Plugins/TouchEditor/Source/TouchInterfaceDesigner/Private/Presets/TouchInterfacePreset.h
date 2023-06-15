// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

#include "Classes/VirtualControlSetup.h"
#include "TouchInterfacePreset.generated.h"

USTRUCT()
struct FInterfaceSettings
{
	GENERATED_BODY()

	/** Opacity (0.0 - 1.0) of all controls while any control is active */
	UPROPERTY(EditAnywhere, Category="TouchDesignerInterface")
	float ActiveOpacity;

	/** Opacity (0.0 - 1.0) of all controls while no controls are active */
	UPROPERTY(EditAnywhere, Category="TouchDesignerInterface")
	float InactiveOpacity;

	/** How long after user interaction will all controls fade out to Inactive Opacity */
	UPROPERTY(EditAnywhere, Category="TouchDesignerInterface")
	float TimeUntilDeactivated;

	/** How long after going inactive will controls reset/recenter themselves (0.0 will disable this feature) */
	UPROPERTY(EditAnywhere, Category="TouchDesignerInterface")
	float TimeUntilReset;

	/** How long after joystick enabled for touch (0.0 will disable this feature) */
	UPROPERTY(EditAnywhere, Category="TouchDesignerInterface")
	float ActivationDelay;

	/** Delay at startup before virtual joystick is drawn (0.0 will disable this feature) */
	UPROPERTY(EditAnywhere, Category="TouchDesignerInterface")
	float StartupDelay;

	/*FInterfaceSettings() :
	ActiveOpacity(1.0f),
	InactiveOpacity(0.1f),
	TimeUntilDeactivated(0.5f),
	TimeUntilReset(2.0f),
	ActivationDelay(0.0f),
	StartupDelay(0.0f)
	{
		
	}*/

	explicit FInterfaceSettings(const float InActiveOpacity = 1.0f, const float InInactiveOpacity = 0.1f, const float InTimeUntilDeactivated = 0.5f, const float InTimeUntilReset = 2.0f, const float InActivationDelay = 0.0f, const float InStartupDelay = 0.0f) :
	ActiveOpacity(InActiveOpacity),
	InactiveOpacity(InInactiveOpacity),
	TimeUntilDeactivated(InTimeUntilDeactivated),
	TimeUntilReset(InTimeUntilReset),
	ActivationDelay(InActivationDelay),
	StartupDelay(InStartupDelay)
	{
		
	}
};

UCLASS()
class TOUCHINTERFACEDESIGNER_API UTouchInterfacePreset : public UObject
{
	GENERATED_BODY()

public:
	void SavePreset(FInterfaceSettings InGeneralSettingsData, TArray<FVirtualControl> InVirtualControlData) { GeneralSettings = InGeneralSettingsData; VirtualControls = InVirtualControlData; }

	FInterfaceSettings GetGeneralSettings() const { return GeneralSettings; }

	TArray<FVirtualControl> GetControlsSetting() const { return VirtualControls; }

	TArray<FName> GetTags() const { return Tags; }
	void SetTags(TArray<FName> InTags) { Tags = InTags; }

	void AddTag(const FName NewTag);
	void RemoveTag(const FName TagToRemove);

private:
	/** Contain all data of virtual control saved as preset */
	UPROPERTY(EditAnywhere, Category="Preset")
	TArray<FVirtualControl> VirtualControls;

	/** General settings saved as preset */
	UPROPERTY(EditAnywhere, Category="Preset")
	FInterfaceSettings GeneralSettings;
	
	TArray<FName> Tags;
	
	//Todo: For Preset improvements, add some variable for tag, category, etc.

	//Todo: Recommended settings
};
