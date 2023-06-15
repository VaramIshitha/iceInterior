// Copyright Lost in Game Studio. All Rights Reserved.

#include "VirtualControlSetup.h"

//DEFINE_LOG_CATEGORY_STATIC(LogVirtualControlSetup, All, All);

UVirtualControlSetup::UVirtualControlSetup()
{
	// defaults value
	ActiveOpacity = 1.0f;
	InactiveOpacity = 0.1f;
	TimeUntilDeactivated = 0.5f;
	TimeUntilReset = 2.0f;
	ActivationDelay = 0.f;
	StartupDelay = 0.f;
	bCalculatePortraitPositionAtRuntime = false;

#if WITH_EDITOR
	CurrentSelectedControlIndex = 0;
	CurrentSelectedControlType = EControlType::Button;
	BackgroundSettings = FDesignerBackgroundSettings();
#endif
}

#if WITH_EDITOR
EControlType UVirtualControlSetup::GetSelectedControlType() const
{
	return VirtualControls[CurrentSelectedControlIndex].Type;
}

int32 UVirtualControlSetup::GetLastIndex() const
{
	return FMath::Clamp(VirtualControls.Num()-1, 0, VirtualControls.Num());
}

int32 UVirtualControlSetup::GetUniqueId(const EControlType Type)
{
	FString ControlTypeName;
	
	switch (Type)
	{
	case EControlType::Button:
		ControlTypeName = "New Button";
		break;
	case EControlType::Joystick:
		ControlTypeName = "New Joystick";
		break;
	case EControlType::TouchRegion:
		ControlTypeName = "New Touch Region";
		break;
	default:
		ControlTypeName = "New Control";
		break;
	}
	
	for (uint8 Itr = 0; Itr < 100; ++Itr)
	{
		//const FString GeneratedString = FString::Printf(TEXT("New %hs_%d"), Type ? "Joystick" : "Button", Itr);
		const FString GeneratedString = ControlTypeName + FString::Printf(TEXT("_%d"), Itr);
		
		const FName GeneratedName = FName(GeneratedString);

		if (!DoesThisNameExistInControl(GeneratedName))
		{
			return Itr;
		}
	}

	return -1;
}

FName UVirtualControlSetup::GetUniqueName(const EControlType ControlType)
{
	FString ControlTypeName;

	switch (ControlType)
	{
	case EControlType::Button:
		ControlTypeName = "New Button";
		break;
	case EControlType::Joystick:
		ControlTypeName = "New Joystick";
		break;
	case EControlType::TouchRegion:
		ControlTypeName = "New Touch Region";
		break;
	default:
		ControlTypeName = "New Control";
		break;
	}
	
	const FString GeneratedString = FString::Printf(TEXT("%s_%d"), *ControlTypeName, GetUniqueId(ControlType));
	return FName(GeneratedString);
}

bool UVirtualControlSetup::DoesThisNameExistInControl(const FName OtherName)
{
	for (FVirtualControl Item : VirtualControls)
	{
		if (Item.ControlName.IsEqual(OtherName))
		{
			return true;
		}
	}

	return false;
}

int32 UVirtualControlSetup::SelectLastControl()
{
	//Todo: If array empty, set index to -1
	CurrentSelectedControlIndex = GetLastIndex();
	return CurrentSelectedControlIndex;
}

FVirtualControl UVirtualControlSetup::GetSelectedControl() const
{
	if (CurrentSelectedControlIndex >= 0)
	{
		return VirtualControls[CurrentSelectedControlIndex];
	}

	return FVirtualControl();
}

FVirtualControl& UVirtualControlSetup::GetVirtualControlRef(const FName Name)
{
	const int32 Index = GetControlIndexByName(Name);
	check(Index != -1)
	
	return VirtualControls[Index];
}

int32 UVirtualControlSetup::GetControlIndexByName(const FName Name)
{
	for (int32 Itr = 0; Itr < VirtualControls.Num(); Itr++)
	{
		if (VirtualControls[Itr].ControlName == Name)
		{
			return Itr;
		}
	}

	return -1;
}
#endif

