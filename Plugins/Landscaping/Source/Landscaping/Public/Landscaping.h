// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "Defines.h"
#include "CoreMinimal.h"
#include "LandscapingStyle.h"
#include "LandscapingCommands.h"
#include "ILandscapingModuleInterface.h"
#include "ToolMenus.h"
#include "GISFileManager.h"
#include "DefaultDataSource.h"
#include "Landscaping/Private/UI/FFoliageAutomationUI.h"
#include "Landscaping/Private/UI/FVectorImporterUI.h"
#include "Landscaping/Private/UI/FRasterImporterUI.h"
#include "LandscapingSettings.h"
#include "Interfaces/IPluginManager.h"
#include "Developer/Settings/Public/ISettingsContainer.h"
#include "Developer/Settings/Public/ISettingsSection.h" 
#include "Developer/Settings/Public/ISettingsModule.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

class FLandscapingModule : public ILandscapingModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** ILandscapingModuleInterface implementation */
	virtual ILandscapingDataSource* GetDataSource() override;
	virtual void ResetDataSource() override;
	
	/** This function will be bound to Command (by default it will bring up plugin window) */
	void PluginButtonClicked();
	void OnTabClosed(TSharedRef<SDockTab> DockTab);

	bool HandleSettingsSaved();
	void RegisterSettings();
	void UnregisterSettings();

	static ULandscapingSettings* GetSettings() { return Settings; }
	
private:
	
	void RegisterMenus();
	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);
	UGISFileManager* GisFileManager = nullptr;
	TSharedPtr<class FUICommandList> PluginCommands;
	UDefaultDataSource* DefaultDS = nullptr;
	static ULandscapingSettings* Settings;
	
};
