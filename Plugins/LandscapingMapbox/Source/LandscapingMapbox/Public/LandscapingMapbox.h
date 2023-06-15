// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "ILandscapingModuleInterface.h"
#include "MapboxDataSource.h"
#include "LandscapingMapboxSettings.h"
#include "Interfaces/IPluginManager.h"
#include "Developer/Settings/Public/ISettingsContainer.h"
#include "Developer/Settings/Public/ISettingsSection.h" 
#include "Developer/Settings/Public/ISettingsModule.h"

class FLandscapingMapboxModule : public ILandscapingModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** ILandscapingModuleInterface implementation */
	virtual ILandscapingDataSource* GetDataSource() override;
	virtual void ResetDataSource() override;

	bool HandleSettingsSaved();
	void RegisterSettings();
	void UnregisterSettings();

	static ULandscapingMapboxSettings* GetSettings() { return Settings; }

	TArray<UMapboxDataSource*> MapboxDS = TArray<UMapboxDataSource*>();
private:
	static ULandscapingMapboxSettings* Settings;

};
