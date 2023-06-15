// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#include "Landscaping.h"
#include "EditorStyle/Public/EditorStyleSet.h"

static const FName LandscapingTabName("Landscaping");

#define LOCTEXT_NAMESPACE "FLandscapingModule"

ULandscapingSettings* FLandscapingModule::Settings;

void FLandscapingModule::StartupModule()
{
    // This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
    RegisterSettings();

    FLandscapingStyle::Initialize();
    FLandscapingStyle::ReloadTextures();

    FLandscapingCommands::Register();

    PluginCommands = MakeShareable(new FUICommandList);

    PluginCommands->MapAction(
        FLandscapingCommands::Get().OpenPluginWindow,
        FExecuteAction::CreateRaw(this, &FLandscapingModule::PluginButtonClicked),
        FCanExecuteAction());

    UToolMenus::RegisterStartupCallback(
        FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FLandscapingModule::RegisterMenus));

    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(LandscapingTabName,
                                                      FOnSpawnTab::CreateRaw(
                                                          this, &FLandscapingModule::OnSpawnPluginTab))
                            .SetDisplayName(LOCTEXT("FLandscapingTabTitle", "Landscaping"))
                            .SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FLandscapingModule::ShutdownModule()
{
    // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
    // we call this function before unloading the module.

    UToolMenus::UnRegisterStartupCallback(this);

    UToolMenus::UnregisterOwner(this);

    FLandscapingStyle::Shutdown();

    FLandscapingCommands::Unregister();

    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(LandscapingTabName);
    if (UObjectInitialized())
    {
        UnregisterSettings();
    }
}

TSharedRef<SDockTab> FLandscapingModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
    FLandscapingStyle::Initialize();
    FLandscapingStyle::ReloadTextures();
    Settings = GetMutableDefault<ULandscapingSettings>();
    DefaultDS = NewObject<UDefaultDataSource>();
    GisFileManager = GEditor->GetEditorSubsystem<UGISFileManager>();
    GisFileManager->Init();
    // Make sure we have at least default cache dir set
    if(Settings->CacheDirectory.IsEmpty())
    {
        Settings->CacheDirectory = "C:/Temp/Landscaping";
    }
    GisFileManager->SetCacheDirectory(Settings->CacheDirectory);
    FRasterImporterUI* RasterImporterUI = new FRasterImporterUI(GisFileManager);
    FVectorImporterUI* VectorImporterUI = new FVectorImporterUI(GisFileManager);
    FFoliageAutomationUI* FoliageAutomationUI = new FFoliageAutomationUI(GisFileManager);
    FUserInterfaceParts* UserInterfaceParts = new FUserInterfaceParts();
    TSharedRef<SDockTab> Tab = SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)
        [
            SNew(SBox)
            .Padding(FMargin(15, 10, 15, 0))
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                  .HAlign(HAlign_Center)
                  .AutoHeight()
                [
                    SNew(STextBlock)
                    .Text(FText::FromString("Landscaping"))
                    #if ENGINE_MINOR_VERSION < 1
                    .TextStyle(FEditorStyle::Get(), "LargeText")
                    #else
                    .TextStyle(FAppStyle::Get(), "LargeText")
                    #endif
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(SSeparator)
                    #if ENGINE_MINOR_VERSION < 1
                    .SeparatorImage(FEditorStyle::GetBrush("Menu.Separator"))
                    #else
                    .SeparatorImage(FAppStyle::GetBrush("Menu.Separator"))
                    #endif
                    .Orientation(Orient_Horizontal)
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot()
                    .Padding(2, 2, 0, 0)
                    .HAlign(HAlign_Center)
                    .FillWidth(2)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Tip: To reset temporary values, close the tab and open it again."))
                        .WrapTextAt(600.0f)
                    ]
                ]
                + SVerticalBox::Slot()
                [
                    SNew(SScrollBox)
                    + SScrollBox::Slot()
                    .Padding(0.6f)
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        [
                            RasterImporterUI->ImportRasterFilesUI()
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        [
                            VectorImporterUI->ImportVectorFilesUI()
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        [
                            FoliageAutomationUI->FoliageAutomationUI()
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        [
                            UserInterfaceParts->SHeader1("Utilities", "")
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        [
                            SNew(SButton)
                            .Text(FText::FromString("Inspect DTM"))
                            .HAlign(HAlign_Center)
                            .OnClicked_UObject(GisFileManager, &UGISFileManager::InspectDTM)
                            .ToolTipText(FText::FromString("Writes metadata of choosen DTM file(s) into the Output Log"))
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        [
                            SNew(SButton)
                            .Text(FText::FromString("Spawn Geo Referencing System"))
                            .HAlign(HAlign_Center)
                            .OnClicked_UObject(GisFileManager, &UGISFileManager::SpawnGeoReferencingSystem)
                            .ToolTipText(FText::FromString("Adds the UE Geo Referencing System with current origin and CRS in this level"))
                        ]
                    ]
                ]
            ]
        ];
    Tab.Get().SetOnTabClosed(SDockTab::FOnTabClosedCallback::CreateRaw(this, &FLandscapingModule::OnTabClosed));
    return Tab;
}

ILandscapingDataSource* FLandscapingModule::GetDataSource()
{
    return DefaultDS;
}

void FLandscapingModule::ResetDataSource()
{
    DefaultDS = NewObject<UDefaultDataSource>();
}

void FLandscapingModule::OnTabClosed(TSharedRef<SDockTab> DockTab)
{
    GisFileManager->Cleanup(true);
}

void FLandscapingModule::PluginButtonClicked()
{
#if ENGINE_MAJOR_VERSION < 5
    FGlobalTabmanager::Get()->InvokeTab(LandscapingTabName);
#else
    FGlobalTabmanager::Get()->TryInvokeTab(LandscapingTabName);
#endif
}

void FLandscapingModule::RegisterMenus()
{
    // Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
    FToolMenuOwnerScoped OwnerScoped(this);

    {
        UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
        {
            FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
            Section.AddMenuEntryWithCommandList(FLandscapingCommands::Get().OpenPluginWindow, PluginCommands);
        }
    }

    {
        UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
        {
            FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
            {
                FToolMenuEntry& Entry = Section.AddEntry(
                    FToolMenuEntry::InitToolBarButton(FLandscapingCommands::Get().OpenPluginWindow));
                Entry.SetCommandList(PluginCommands);
            }
        }
    }
}

bool FLandscapingModule::HandleSettingsSaved()
{
#if WITH_EDITORONLY_DATA
	Settings = GetMutableDefault<ULandscapingSettings>();
	Settings->SaveConfig(); 
	return true;
#endif
	return false;
}

void FLandscapingModule::RegisterSettings()
{
#if WITH_EDITORONLY_DATA
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings")) 
	{
		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings(
            "Project", 
            "Plugins", 
            "Landscaping",
			LOCTEXT("RuntimeGeneralName", "Landscaping"),
			LOCTEXT("RuntimeGeneralDescription", "General settings for Landscaping Plugin"),
			GetMutableDefault<ULandscapingSettings>());
		
		if (SettingsSection.IsValid()) 
        {
			SettingsSection->OnModified().BindRaw(this, &FLandscapingModule::HandleSettingsSaved); 
		}
	}
#endif
}

void FLandscapingModule::UnregisterSettings()
{
#if WITH_EDITORONLY_DATA
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings")) 
    {
		SettingsModule->UnregisterSettings("Project", "Plugins", "Landscaping");
	}
#endif
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FLandscapingModule, Landscaping)
