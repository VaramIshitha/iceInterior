// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#include "LandscapingMapbox.h"

#define LOCTEXT_NAMESPACE "FLandscapingMapboxModule"


ULandscapingMapboxSettings* FLandscapingMapboxModule::Settings;

void FLandscapingMapboxModule::StartupModule() 
{
    RegisterSettings();
}

void FLandscapingMapboxModule::ShutdownModule() 
{
    if (UObjectInitialized())
	{
		UnregisterSettings();
	}
}

void FLandscapingMapboxModule::ResetDataSource()
{
	UE_LOG(LogTemp, Log, TEXT("LandscapingMapbox: LandscapingMapbox - reset Data Source"));
	MapboxDS.Empty();
}

ILandscapingDataSource* FLandscapingMapboxModule::GetDataSource()
{
	UE_LOG(LogTemp, Log, TEXT("LandscapingMapbox: LandscapingMapbox - get Data Source"));
	MapboxDS.Add(NewObject<UMapboxDataSource>());
	return MapboxDS.Last();
}

bool FLandscapingMapboxModule::HandleSettingsSaved()
{
#if WITH_EDITORONLY_DATA
	Settings = GetMutableDefault<ULandscapingMapboxSettings>();
	Settings->SaveConfig(); 
	return true;
#endif
	return false;
}

void FLandscapingMapboxModule::RegisterSettings()
{
#if WITH_EDITORONLY_DATA
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings")) 
	{
		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings(
            "Project", 
            "Plugins", 
            "Landscaping Mapbox",
			LOCTEXT("RuntimeGeneralName", "Landscaping Mapbox"),
			LOCTEXT("RuntimeGeneralDescription", "General settings for Landscaping Mapbox Plugin"),
			GetMutableDefault<ULandscapingMapboxSettings>());
		
		if (SettingsSection.IsValid()) 
        { 
			SettingsSection->OnModified().BindRaw(this, &FLandscapingMapboxModule::HandleSettingsSaved); 
		}
	}
#endif
}

void FLandscapingMapboxModule::UnregisterSettings()
{
#if WITH_EDITORONLY_DATA
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings")) 
    {
		SettingsModule->UnregisterSettings("Project", "Plugins", "Landscaping Mapbox");
	}
#endif
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLandscapingMapboxModule, LandscapingMapbox)