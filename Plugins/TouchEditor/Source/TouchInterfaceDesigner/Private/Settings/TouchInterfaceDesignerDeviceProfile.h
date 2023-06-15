// Copyright Lost in Game Studio, Inc. All Rights Reserved.

#pragma once

#include "Styling/ISlateStyle.h"

struct FDesignerDeviceProfile
{
	FName Category;
	FString DeviceProfileName;
	FString StyleName;
	FVector2D ScreenDefinition;
	FVector2D LogicalResolution;
	FString ScreenFormat;
	uint8 bIsGeneric:1;

	//Todo: SafeZone (front camera, notch, other captor and curved screen)
	//Todo: Make function to draw array of red box. Center and size
	//Todo: Show Display Rate (240hz, 120, 60...), Touch Sample Rate ?
	
	//Default constructor
	FDesignerDeviceProfile()
	: bIsGeneric(false)
	{
		
	}

	FDesignerDeviceProfile(const FName InCategory, const FString InDeviceProfileName, const FString InStyleName, const FVector2D InLogicalResolution, const FVector2D InScreenDefinition, const FString InScreenFormat, const bool InIsGeneric)
	: Category(InCategory)
	, DeviceProfileName(InDeviceProfileName)
	, StyleName(InStyleName)
	, ScreenDefinition(InScreenDefinition)
	, LogicalResolution(InLogicalResolution)
	, ScreenFormat(InScreenFormat)
	, bIsGeneric(InIsGeneric)
	{
		
	}
};

class FTouchInterfaceDesignerDeviceProfile
{
	
public:
	FTouchInterfaceDesignerDeviceProfile();

	static void Initialize();
	static void Deinitialize();

private:
	/**
	 * @param Category Used for menu builder sub menu
	 * @param DeviceName Name of device like "Samsung Galaxy Note 20"
	 * @param DeviceProfileName Name of profile like "Android_Adreno6xx"
	 * @param StyleName Name of style like "Samsung.Note.20"
	 * @param ScreenDefinition Native resolution of device
	 * @param ScreenFormat Screen format like 16/9
	 */
	static void SetAndroidProfile(const FName Category, const FString DeviceName, const FString DeviceProfileName, const FString StyleName, const FVector2D ScreenDefinition, const FString ScreenFormat);
	static void SetAppleProfile(const FName Category, const FString DeviceName, const FString DeviceProfileName, const FString StyleName, const FVector2D ScreenDefinition, const FVector2D LogicalResolution, const FString ScreenFormat);
	static void SetGenericProfile(const FName Category, const FString DeviceName, const FString DeviceProfileName, const FString StyleName, const FVector2D ScreenDefinition, const FString ScreenFormat);

public:
	static bool GetProfile(const FString DeviceName, FDesignerDeviceProfile& Profile);

	static TArray<FString> GetAllProfileNameByCategory(const FName Category);

	static bool GetAllCategories(TArray<FName>& Categories);
	
	static bool GetAllProfileByCategory(const FName Category, TArray<FDesignerDeviceProfile>& Profiles);

	static FVector2D GetScreenDefinition(const FString DeviceName);

	static FString GetDeviceProfileName(const FString DeviceName);

	static FString GetDeviceStyleName(const FString DeviceName);

	static const FSlateBrush* GetSlateBrush(const FString DeviceName);

private:
	static TMap<FString, FDesignerDeviceProfile> DesignerDeviceProfiles;
};

class FDesignerDeviceStyle
{
public:
	static void RegisterStyle();
	static void UnregisterStyle();

	/** Reload textures used by slate renderer */
	static void ReloadTextures();

	/** Get instance of Virtual Control Designer SlateStyle */
	static const ISlateStyle& Get()
	{
		return *StyleSet;
	}

	/** Get StyleSet name of Virtual Control Designer SlateStyle */
	static const FName& GetStyleSetName()
	{
		return StyleSet->GetStyleSetName();
	}

private:
	static TSharedRef<ISlateStyle> Create();
	static TSharedPtr<ISlateStyle> StyleSet;
};
