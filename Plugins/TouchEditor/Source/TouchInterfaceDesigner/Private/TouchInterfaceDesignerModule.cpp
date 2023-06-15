// Copyright Lost in Game Studio. All Rights Reserved.

#include "TouchInterfaceDesignerModule.h"

#include "AssetToolsModule.h"

#include "AssetTypeActions/VirtualControlSetupAssetTypeActions.h"
#include "AssetTypeActions/VirtualControlEventAssetType.h"
#include "AssetTypeActions/VirtualControlDesignerEditorMenuExtender.h"

//#include "AssetTypeCategories.h"
#include "IAssetTools.h"
#include "Editor/VirtualControlDesignerEditor.h"
#include "Editor/VirtualControlDesignerEditorCommands.h"

#include "Editor/TouchInterfaceDesignerStyle.h"
#include "DetailsTab/VirtualControlDesignerEditor_DetailsTab.h"
#include "VirtualControlSetup.h"
#include "Framework/Notifications/NotificationManager.h"
#include "GameFramework/InputSettings.h"
#include "Settings/TouchInterfaceDesignerDeviceProfile.h"
#include "Widgets/Notifications/SNotificationList.h"


#define LOCTEXT_NAMESPACE "VirtualControlDesignerEditorModule"

const FName VirtualControlDesignerEditorAppIdentifier = FName(TEXT("VirtualControlDesignerEditorApp"));

TSharedPtr<SNotificationItem> FixSettingsNotification = nullptr;

void FTouchInterfaceDesignerModule::StartupModule()
{
	FTouchInterfaceDesignerStyle::RegisterStyle();
	//FVirtualControlDesignerEditorStyle::ReloadTextures();

	FTouchInterfaceDesignerDeviceProfile::Initialize();

	// Add Touch Designer Commands
	FVirtualControlDesignerCommands::Register();
	
	MenuExtensibilityManager = MakeShareable(new FExtensibilityManager);
	ToolbarExtensibilityManager = MakeShareable(new FExtensibilityManager);

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	AssetTypeCategory = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("TouchInterfaceDesigner")), INVTEXT("Touch Interface Designer"));
	
	RegisterAssetTypeAction<FVirtualControlSetupAssetTypeActions>(AssetTools);
	RegisterAssetTypeAction<FVirtualControlEventAssetType>(AssetTools);

	MenuExtender = MakeShareable(new FVirtualControlDesignerEditorMenuExtender);
	MenuExtender->StartupMenuExtender();
	
	// Retrieve the property editor module and assign properties to DetailsView
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.RegisterCustomClassLayout("VirtualControlSetup", FOnGetDetailCustomizationInstance::CreateStatic(&FTouchDesignerEditor_DetailsTab::MakeInstance));
	PropertyEditorModule.NotifyCustomizationModuleChanged();

	// Custom renderer for Virtual Control Setup asset
	//UThumbnailManager::Get().RegisterCustomRenderer(UWidgetBlueprint::StaticClass(), UWidgetBlueprintThumbnailRenderer::StaticClass());

	//PropertyEditorModule.RegisterCustomClassLayout("TouchInterfaceDesignerBackground", FOnGetDetailCustomizationInstance::CreateStatic(&))
	//PropertyEditorModule.NotifyCustomizationModuleChanged();

	CheckInputSettings();
}

void FTouchInterfaceDesignerModule::ShutdownModule()
{
	MenuExtensibilityManager.Reset();
	ToolbarExtensibilityManager.Reset();

	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		// Unregister our custom created assets from the AssetTools
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for(int32 i = 0; i < RegisteredAssetTypeActions.Num(); i++)
		{
			AssetTools.UnregisterAssetTypeActions(RegisteredAssetTypeActions[i].ToSharedRef());
		}
	}

	// Remove asset type action
	RegisteredAssetTypeActions.Empty();

	//Unregister style
	FTouchInterfaceDesignerStyle::UnregisterStyle();

	// Remove Touch Designer Commands
	FVirtualControlDesignerCommands::Unregister();

	// Unregister custom layout for Touch Designer Interface
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.UnregisterCustomClassLayout("VirtualControlSetup");
	PropertyEditorModule.NotifyCustomizationModuleChanged();

	// Remove Menu Extender
	MenuExtender->ShutdownMenuExtender();
	
	TouchDesignerEditorPtr = nullptr;
}

TSharedRef<FVirtualControlDesignerEditor> FTouchInterfaceDesignerModule::CreateVirtualControlDesignerEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost> &InitToolkitHost, UVirtualControlSetup* TouchDesignerInterface)
{
	TSharedRef<FVirtualControlDesignerEditor> NewVirtualControlDesignerEditor(new FVirtualControlDesignerEditor());
	TouchDesignerEditorPtr = NewVirtualControlDesignerEditor;
	NewVirtualControlDesignerEditor->InitVirtualControlDesignerEditor(Mode, InitToolkitHost, TouchDesignerInterface);
	return NewVirtualControlDesignerEditor;
}

template <typename T>
void FTouchInterfaceDesignerModule::RegisterAssetTypeAction(IAssetTools& AssetTools)
{
	TSharedRef<IAssetTypeActions> Action = MakeShared<T>(AssetTypeCategory);
	AssetTools.RegisterAssetTypeActions(Action);
	RegisteredAssetTypeActions.Add(Action);
}

void FTouchInterfaceDesignerModule::CheckInputSettings()
{
	const UInputSettings* InputSettings = GetDefault<UInputSettings>();

	if (!InputSettings->DefaultTouchInterface.IsNull() || InputSettings->bAlwaysShowTouchInterface || !InputSettings->bUseMouseForTouch)
	{
		FNotificationInfo SuccessNotificationInfo(INVTEXT("Touch Interface Designer requires the modification of some project parameters"));
		SuccessNotificationInfo.bUseSuccessFailIcons=false;
		SuccessNotificationInfo.bUseThrobber=false;
		SuccessNotificationInfo.bUseLargeFont=false;
		SuccessNotificationInfo.bFireAndForget = false;
		//SuccessNotificationInfo.ExpireDuration = 5.0f;
	
		SuccessNotificationInfo.ButtonDetails.Add(FNotificationButtonInfo(INVTEXT("Grant!"), INVTEXT("Change the project settings to allow Touch Interface Designer to display the touch interface correctly"), FSimpleDelegate::CreateRaw(this, &FTouchInterfaceDesignerModule::HandleFixSettings), SNotificationItem::CS_None));
		SuccessNotificationInfo.ButtonDetails.Add(FNotificationButtonInfo(INVTEXT("Reject"), INVTEXT("Do not change the settings, I understand that I have to change the settings by myself"), FSimpleDelegate::CreateRaw(this, &FTouchInterfaceDesignerModule::HandleRejectSettingsModification),SNotificationItem::CS_None));
		FixSettingsNotification = FSlateNotificationManager::Get().AddNotification(SuccessNotificationInfo);
	}
}

void FTouchInterfaceDesignerModule::HandleFixSettings()
{
	if (FixSettingsNotification.IsValid())
	{
		FixSettingsNotification->SetFadeOutDuration(0.2f);
		FixSettingsNotification->Fadeout();
	}

	UInputSettings* InputSettings = GetMutableDefault<UInputSettings>();
	
	// Remove epic touch interface
	InputSettings->DefaultTouchInterface = nullptr;
	
	// Set this to false to not display epic touch interface
	InputSettings->bAlwaysShowTouchInterface = false;
	
	// Enable this to allow interaction with touch interface in desktop platform
	InputSettings->bUseMouseForTouch = true;

	// Save modification in config file
#if ENGINE_MAJOR_VERSION > 4
	InputSettings->TryUpdateDefaultConfigFile();
#else
	InputSettings->UpdateDefaultConfigFile();
#endif

	//Todo: indicate to user that settings is correctly modified
}

void FTouchInterfaceDesignerModule::HandleRejectSettingsModification()
{
	if (FixSettingsNotification.IsValid())
	{
		FixSettingsNotification->SetFadeOutDuration(0.2f);
		FixSettingsNotification->Fadeout();
	}
}

#undef LOCTEXT_NAMESPACE