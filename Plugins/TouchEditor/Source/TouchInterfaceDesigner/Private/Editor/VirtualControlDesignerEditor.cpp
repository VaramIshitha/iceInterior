// Copyright Lost in Game Studio. All Rights Reserved.

#include "VirtualControlDesignerEditor.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Modules/ModuleManager.h"
#include "EditorStyleSet.h"

#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Notifications/SNotificationList.h"

#include "Framework/Commands/GenericCommands.h"
#include "Framework/Commands/UIAction.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Notifications/NotificationManager.h"

#include "Viewport/STouchInterfaceDesignerViewport.h"
#include "Preview/SVirtualControlPreviewerTab.h"

#include "IDetailsView.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "IAssetTools.h"
#include "ISettingsModule.h"
#include "TouchInterfaceDesignerModule.h"

#include "TouchInterfaceDesignerStyle.h"
#include "Presets/TouchInterfacePreset.h"
#include "Factories/TouchInterfacePresetFactory.h"
#include "Classes/VirtualControlSetup.h"
#include "VirtualControlDesignerEditorCommands.h"

#include "Hierarchy/SHierarchyTab.h"
#include "Palette/SPaletteTab.h"
#include "DetailsTab/VirtualControlDesignerEditor_DetailsTab.h"
#include "Presets/STouchInterfacePresetWindow.h"
#include "TouchInterfaceDesignerSettings.h"
#include "Presets/TouchInterfacePresetManager.h"
#include "UObject/SavePackage.h"

DEFINE_LOG_CATEGORY_STATIC(LogVirtualControlDesigner, All, All);

#define LOCTEXT_NAMESPACE "VirtualControlDesignerEditor"

const FName TouchDesignerEditorAppName = FName(TEXT("VirtualControlDesignerEditorApp"));

/** Data of control that was copied (Cut/Copy).
 * Check with IsCopyAvailable() before use.
 * This structure can be filled with default value. */
FVirtualControl ControlDataCopied;

struct FTouchDesignerEditorTabs
{
	//Tab identifiers
	static const FName DetailsId;
	static const FName ViewportId;
	static const FName HierarchyId;
	static const FName VirtualControlPreviewerId;
	static const FName PaletteId;
};

const FName FTouchDesignerEditorTabs::DetailsId(TEXT("Details"));
const FName FTouchDesignerEditorTabs::ViewportId(TEXT("Viewport"));
const FName FTouchDesignerEditorTabs::HierarchyId(TEXT("Hierarchy"));
const FName FTouchDesignerEditorTabs::VirtualControlPreviewerId(TEXT("VirtualControlPreviewer"));
const FName FTouchDesignerEditorTabs::PaletteId(TEXT("Palette"));

TSharedPtr<SNotificationItem> FixBlankBrushNotificationItem;

FVirtualControlDesignerEditor::FVirtualControlDesignerEditor()
{
	UEditorEngine* Editor = Cast<UEditorEngine>(GEngine);
	if (Editor != nullptr)
	{
		Editor->RegisterForUndo(this);
	}

	bLandscapeOrientation = true;
	VirtualControlSetupEdited = nullptr;
	DetailTabState = DTS_ShowGeneralProperties;
	bCopyAvailable = false;
}

FVirtualControlDesignerEditor::~FVirtualControlDesignerEditor()
{
	UEditorEngine* Editor = Cast<UEditorEngine>(GEngine);
	if (Editor != nullptr)
	{
		Editor->UnregisterForUndo(this);
	}
	
	VirtualControlSetupEdited = nullptr;
	HierarchyTab = nullptr;

	if (FixBlankBrushNotificationItem.IsValid())
	{
		FixBlankBrushNotificationItem->SetFadeOutDuration(0.2f);
		FixBlankBrushNotificationItem->Fadeout();
	}

	if (PresetManager)
	{
		PresetManager->ShutdownTouchInterfacePresetManager();
		PresetManager = nullptr;
	}
	
	UE_LOG(LogVirtualControlDesigner, Log, TEXT("Destroy Editor"));
}

FName FVirtualControlDesignerEditor::GetToolkitFName() const
{
	return FName("TouchDesignerEditor");
}

FText FVirtualControlDesignerEditor::GetToolkitName() const
{
	return FText::FromString(VirtualControlSetupEdited->GetName());
	//return LOCTEXT("ToolkitName", "TouchDesignerEditor");
}

FText FVirtualControlDesignerEditor::GetBaseToolkitName()const
{
	return LOCTEXT("AppLabel", "Touch Designer Editor");
}

FText FVirtualControlDesignerEditor::GetToolkitToolTipText() const
{
	const UObject* EditingObject = VirtualControlSetupEdited;
	check(EditingObject != nullptr);
	return GetToolTipTextForObject(EditingObject);
	
	//return LOCTEXT("ToolTip", "Touch Designer Editor");
}

FString FVirtualControlDesignerEditor::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "TouchDesignerEditor").ToString();
}

bool FVirtualControlDesignerEditor::ProcessCommandBindings(const FKeyEvent& InKeyEvent) const
{
	return TouchDesignerEditorCommands->ProcessCommandBindings(InKeyEvent);
}

void FVirtualControlDesignerEditor::PostRedo(bool bSuccess)
{
	UE_LOG(LogVirtualControlDesigner, Log, TEXT("Post Redo"));
	FEditorUndoClient::PostRedo(bSuccess);
}

void FVirtualControlDesignerEditor::PostUndo(bool bSuccess)
{
	UE_LOG(LogVirtualControlDesigner, Log, TEXT("Post Undo"));
	FEditorUndoClient::PostUndo(bSuccess);
}

// void FTouchDesignerEditor::AddReferencedObjects(FReferenceCollector& Collector)
// {
// 	Collector.AddReferencedObject(TouchDesignerInterface);
// }
//
// FString FTouchDesignerEditor::GetReferencerName() const
// {
// 	 return TEXT("VirtualControlDesignerEditor");
// }

void FVirtualControlDesignerEditor::InitVirtualControlDesignerEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UVirtualControlSetup* InVirtualControlSetup)
{
	// Set this InVirtualControlSetup as our editing asset
	VirtualControlSetupEdited = InVirtualControlSetup;

	// Bind custom command and button for our Virtual Control Designer
	BindCommand();
	
	// Create the layout of Touch Designer Editor. Do not forget to change the name of the layout with each modification!
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_TouchInterfaceDesigner_v3")
	->AddArea
	(
		// Make Area for Tabs and set orientation to vertical mainly for UE4 (Toolbar tab)
		FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)

#if ENGINE_MAJOR_VERSION < 5
		->Split
		(
			// Add Toolbar tab for UE4
			FTabManager::NewStack()
			//->SetSizeCoefficient(0.3f)
			->SetHideTabWell(false)
			->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
		)
#endif

		->Split
		(
		FTabManager::NewSplitter()->SetOrientation(Orient_Horizontal)

			// Virtual Control Previewer, Palette and Hierarchy
			->Split
			(
			FTabManager::NewSplitter()
				->SetSizeCoefficient(0.3f)
				->SetOrientation(Orient_Vertical)

				->Split
				(
					// Add Virtual Control Previewer Tab
					FTabManager::NewStack()
					->SetSizeCoefficient(0.3f)
					->SetHideTabWell(false)
					->AddTab(FTouchDesignerEditorTabs::VirtualControlPreviewerId, ETabState::OpenedTab)
				)

				->Split
				(
					// Add Palette tab
				FTabManager::NewStack()
					->SetSizeCoefficient(0.3f)
					->SetHideTabWell(false)
					->AddTab(FTouchDesignerEditorTabs::PaletteId, ETabState::OpenedTab)
				)

				->Split
				(
					// Add Hierarchy Tab
					FTabManager::NewStack()
					->SetHideTabWell(false)
					->AddTab(FTouchDesignerEditorTabs::HierarchyId, ETabState::OpenedTab)
				)
			)

			// Viewport Tab
			->Split
			(
				// Add Viewport
				FTabManager::NewStack()
				->SetHideTabWell(false)
				->AddTab(FTouchDesignerEditorTabs::ViewportId, ETabState::OpenedTab)
			)

			// Details Tab
			->Split
			(
				// Add Details Tab
				FTabManager::NewStack()
				->SetSizeCoefficient(0.4f)
				->SetHideTabWell(false)
				->AddTab(FTouchDesignerEditorTabs::DetailsId, ETabState::OpenedTab)
				//->AddTab(FTouchDesignerEditorTabs::BackgroundSettingsId, ETabState::OpenedTab)
			)
		)
	);
	
	constexpr bool bCreateDefaultStandaloneMenu = true;
	constexpr bool bCreateDefaultToolbar = true;
	
	// Initialize our Touch Designer Interface Editor
	InitAssetEditor(Mode, InitToolkitHost, VirtualControlDesignerEditorAppIdentifier, StandaloneDefaultLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, InVirtualControlSetup);

	// Then, regenerate Menu and toolbars
	RegenerateMenusAndToolbars();

	PresetManager = MakeShareable(new FTouchInterfacePresetManager);
	PresetManager->StartupTouchInterfacePresetManager();
	PresetManager->OnPresetSelected.BindSP(this, &FVirtualControlDesignerEditor::HandleOnApplyPreset);

	// When the background settings tab is created, it will be stacked with the detail tab, so highlight the detail tab when launching the editor
	//TabManager->DrawAttention(TabManager->FindExistingLiveTab(FTouchDesignerEditorTabs::DetailsId).ToSharedRef());
}

TSharedRef<SDockTab> FVirtualControlDesignerEditor::SpawnTab_Viewport(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FTouchDesignerEditorTabs::ViewportId);

	// Spawn viewport tab
	return SNew(SDockTab)
	.Label(LOCTEXT("ViewportTab_Title", "Viewport"))
	[
		SAssignNew(ViewportWidget, STouchInterfaceDesignerViewport, SharedThis(this))
	];
}

TSharedRef<SDockTab> FVirtualControlDesignerEditor::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	// Make sure we have the correct tab id
	check(Args.GetTabId() == FTouchDesignerEditorTabs::DetailsId);
	
	// Spawn Details Tab
	return SNew(SDockTab)
	.Label(LOCTEXT("DetailsTab_Title", "Details"))
	[
		SAssignNew(DetailsPanel, SVirtualControlDesignerEditorDetailsPanel, SharedThis(this))
	];
}

TSharedRef<SDockTab> FVirtualControlDesignerEditor::SpawnTab_Hierarchy(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FTouchDesignerEditorTabs::HierarchyId);
	
	// Spawn Control Tab
	return SNew(SDockTab)
	.Label(LOCTEXT("HierarchyTab_Title", "Hierarchy"))
	[
		SAssignNew(HierarchyTab, SHierarchyTab, SharedThis(this))
	];
}

TSharedRef<SDockTab> FVirtualControlDesignerEditor::SpawnTab_VirtualControlPreviewer(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FTouchDesignerEditorTabs::VirtualControlPreviewerId);

	return SNew(SDockTab)
	.Label(LOCTEXT("ControlPreviewTab_Title", "Virtual Control Previewer"))
	[
		SAssignNew(VirtualControlPreviewerTab, SVirtualControlPreviewerTab, SharedThis(this))
	];
}

TSharedRef<SDockTab> FVirtualControlDesignerEditor::SpawnTab_Palette(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FTouchDesignerEditorTabs::PaletteId);

	return SNew(SDockTab)
	.Label(LOCTEXT("PaletteTab_Title", "Palette"))
	[
		SNew(SPaletteTab, SharedThis(this))
	];
}

void FVirtualControlDesignerEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	// Add a new workspace menu category to the tab manager
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_TouchDesignerEditor", "Touch Designer Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();
	
	// We register the tab manager to the asset editor toolkit so we can use it in this editor
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	/** Register the properties tab spawner within our tab manager
	 * we provide the function with the identifier for this tab and a shared pointer to the
	 * SpawnTab_XXX function within this editor class
	 * Additionnaly, we provide a name to be displayed, a category and the tab icon
	 */

	InTabManager->RegisterTabSpawner(FTouchDesignerEditorTabs::HierarchyId, FOnSpawnTab::CreateSP(this, &FVirtualControlDesignerEditor::SpawnTab_Hierarchy))
		.SetDisplayName(LOCTEXT("ControlsTab", "Hierarchy"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FTouchInterfaceDesignerStyle::GetStyleSetName(), "LevelEditor.Tab.Hierarchy"));
	
	InTabManager->RegisterTabSpawner(FTouchDesignerEditorTabs::ViewportId, FOnSpawnTab::CreateSP(this, &FVirtualControlDesignerEditor::SpawnTab_Viewport))
		.SetDisplayName(LOCTEXT("ViewportTab", "Viewport"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FTouchInterfaceDesignerStyle::GetStyleSetName(), "LevelEditor.Tabs.Viewports"));

	InTabManager->RegisterTabSpawner(FTouchDesignerEditorTabs::DetailsId, FOnSpawnTab::CreateSP(this, &FVirtualControlDesignerEditor::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("DetailsTab", "Details"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FTouchInterfaceDesignerStyle::GetStyleSetName(), "LevelEditor.Tabs.Detail"));

	InTabManager->RegisterTabSpawner(FTouchDesignerEditorTabs::VirtualControlPreviewerId, FOnSpawnTab::CreateSP(this, &FVirtualControlDesignerEditor::SpawnTab_VirtualControlPreviewer))
		.SetDisplayName(LOCTEXT("ControlPreviewTab", "Virtual Control Previewer"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FTouchInterfaceDesignerStyle::GetStyleSetName(), "LevelEditor.Tab.Preview"));

	InTabManager->RegisterTabSpawner(FTouchDesignerEditorTabs::PaletteId, FOnSpawnTab::CreateSP(this, &FVirtualControlDesignerEditor::SpawnTab_Palette))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FTouchInterfaceDesignerStyle::GetStyleSetName(), "LevelEditor.Tab.Palette"));
}

void FVirtualControlDesignerEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	// Unregister our tabs from the TabManager, making sure it is cleaned up when the editor gets destroyed
	InTabManager->UnregisterTabSpawner(FTouchDesignerEditorTabs::ViewportId);
	InTabManager->UnregisterTabSpawner(FTouchDesignerEditorTabs::DetailsId);
	InTabManager->UnregisterTabSpawner(FTouchDesignerEditorTabs::HierarchyId);
	InTabManager->UnregisterTabSpawner(FTouchDesignerEditorTabs::VirtualControlPreviewerId);
	InTabManager->UnregisterTabSpawner(FTouchDesignerEditorTabs::PaletteId);
	
	// Unregister the TabManager from the asset editor toolkit
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
}

void FVirtualControlDesignerEditor::BindCommand()
{
	//FTouchDesignerEditorCommands::Register();
	//FTouchDesignerCommands::Register();

	//No need to regenerate the commands
	if (!TouchDesignerEditorCommands.IsValid())
	{
		TouchDesignerEditorCommands = MakeShareable(new FUICommandList);
		
		const FVirtualControlDesignerCommands& Commands = FVirtualControlDesignerCommands::Get();
		
		FTouchInterfaceDesignerModule& Module = FModuleManager::LoadModuleChecked<FTouchInterfaceDesignerModule>("TouchInterfaceDesigner");
		
		TSharedPtr<FExtender> NewToolbarExtender = Module.GetToolBarExtensibilityManager()->GetAllExtenders();
		NewToolbarExtender->AddToolBarExtension("Asset", EExtensionHook::After, TouchDesignerEditorCommands, FToolBarExtensionDelegate::CreateRaw(this, &FVirtualControlDesignerEditor::AddToolbarButton));
		AddToolbarExtender(NewToolbarExtender);
		
		TouchDesignerEditorCommands->MapAction(Commands.CreateButtonCommand, FExecuteAction::CreateRaw(this, &FVirtualControlDesignerEditor::CreateButtonCommand), FCanExecuteAction::CreateRaw(this, &FVirtualControlDesignerEditor::CanCreateNewControl));
		TouchDesignerEditorCommands->MapAction(Commands.CreateJoystickCommand, FExecuteAction::CreateRaw(this, &FVirtualControlDesignerEditor::CreateJoystickCommand), FCanExecuteAction::CreateRaw(this, &FVirtualControlDesignerEditor::CanCreateNewControl));
		TouchDesignerEditorCommands->MapAction(Commands.CreateTouchRegionCommand, FExecuteAction::CreateRaw(this, &FVirtualControlDesignerEditor::CreateTouchRegionCommand), FCanExecuteAction::CreateRaw(this, &FVirtualControlDesignerEditor::CanCreateNewControl));
		
		TouchDesignerEditorCommands->MapAction(Commands.OpenControlDesignerCommand, FExecuteAction::CreateRaw(this, &FVirtualControlDesignerEditor::OpenControlDesigner), FCanExecuteAction());
		TouchDesignerEditorCommands->MapAction(Commands.ShowGeneralSettingsCommand, FExecuteAction::CreateRaw(this, &FVirtualControlDesignerEditor::OnShowGeneralSettingsCommand), FCanExecuteAction());
		TouchDesignerEditorCommands->MapAction(Commands.ShowBackgroundSettingsCommand, FExecuteAction::CreateRaw(this, &FVirtualControlDesignerEditor::OnShowBackgroundSettingsCommand), FCanExecuteAction());
		TouchDesignerEditorCommands->MapAction(Commands.SaveAsPresetCommand, FExecuteAction::CreateRaw(this, &FVirtualControlDesignerEditor::SaveAsPresetCommand), FCanExecuteAction());
		TouchDesignerEditorCommands->MapAction(Commands.RecenterCanvasCommand, FExecuteAction::CreateRaw(this, &FVirtualControlDesignerEditor::OnRecenterCanvasCommand), FCanExecuteAction::CreateRaw(this, &FVirtualControlDesignerEditor::CanExecuteAction));
		TouchDesignerEditorCommands->MapAction(Commands.OpenPresetCommand, FExecuteAction::CreateRaw(this, &FVirtualControlDesignerEditor::HandleOnOpenPresetManager));
		TouchDesignerEditorCommands->MapAction(Commands.GeneratePortraitPositionCommand, FExecuteAction::CreateRaw(this, &FVirtualControlDesignerEditor::HandleOnCalculatePortraitPosition));
		
		TouchDesignerEditorCommands->MapAction(FGenericCommands::Get().Duplicate, FExecuteAction::CreateRaw(this, &FVirtualControlDesignerEditor::OnDuplicateCommand), FCanExecuteAction::CreateRaw(this, &FVirtualControlDesignerEditor::CanExecuteAction));
		TouchDesignerEditorCommands->MapAction(FGenericCommands::Get().Copy, FExecuteAction::CreateRaw(this, &FVirtualControlDesignerEditor::OnCopyCommand), FCanExecuteAction::CreateRaw(this, &FVirtualControlDesignerEditor::CanExecuteAction));
		TouchDesignerEditorCommands->MapAction(FGenericCommands::Get().Cut, FExecuteAction::CreateRaw(this, &FVirtualControlDesignerEditor::OnCutCommand), FCanExecuteAction::CreateRaw(this, &FVirtualControlDesignerEditor::CanExecuteAction));
		TouchDesignerEditorCommands->MapAction(FGenericCommands::Get().Paste, FExecuteAction::CreateRaw(this, &FVirtualControlDesignerEditor::OnPasteCommand), FCanExecuteAction::CreateRaw(this, &FVirtualControlDesignerEditor::CanExecuteAction));
		TouchDesignerEditorCommands->MapAction(FGenericCommands::Get().Delete, FExecuteAction::CreateRaw(this, &FVirtualControlDesignerEditor::OnDeleteCommand), FCanExecuteAction::CreateRaw(this, &FVirtualControlDesignerEditor::CanExecuteAction));
		//TouchDesignerEditorCommands->MapAction(FGenericCommands::Get().Redo)
		//TouchDesignerEditorCommands->MapAction(FGenericCommands::Get().Undo)
		TouchDesignerEditorCommands->MapAction(FGenericCommands::Get().Rename, FExecuteAction::CreateRaw(this, &FVirtualControlDesignerEditor::OnRenameCommand), FCanExecuteAction::CreateRaw(this, &FVirtualControlDesignerEditor::CanExecuteAction));
	}
}

void FVirtualControlDesignerEditor::AddToolbarButton(FToolBarBuilder& Builder)
{
	Builder.BeginSection("Designer");
	//Builder.AddToolBarButton(FTouchDesignerEditorCommands::Get().OpenControlDesignerCommand,"Designer");
	//Builder.AddSeparator();
	Builder.AddToolBarButton(FVirtualControlDesignerCommands::Get().ShowGeneralSettingsCommand, "Designer");
	Builder.AddToolBarButton(FVirtualControlDesignerCommands::Get().ShowBackgroundSettingsCommand, "Designer");
	Builder.EndSection();

	Builder.BeginSection("Preset");
	Builder.AddToolBarButton(FVirtualControlDesignerCommands::Get().SaveAsPresetCommand);
	Builder.AddToolBarButton(FVirtualControlDesignerCommands::Get().OpenPresetCommand);
	Builder.EndSection();

	Builder.BeginSection("Functions");
	Builder.AddToolBarButton(FVirtualControlDesignerCommands::Get().GeneratePortraitPositionCommand);
	Builder.EndSection();
}

void FVirtualControlDesignerEditor::ForceRefresh()
{
	DetailsPanel->Refresh();
}

void FVirtualControlDesignerEditor::AddNewControl(EControlType ControlType, FVector2D CanvasPosition)
{
	DetailTabState = DTS_ShowControlProperties;
		
	const FName ControlName = VirtualControlSetupEdited->GetUniqueName(ControlType);

	FVirtualControl NewVirtualControl = FVirtualControl(ControlType);
	NewVirtualControl.ControlName = ControlName;

	if (IsOrientationInLandscape())
	{
		NewVirtualControl.LandscapeCenter = ViewportWidget->CanvasSpaceToNormalized(CanvasPosition);
	}
	else
	{
		NewVirtualControl.PortraitCenter = ViewportWidget->CanvasSpaceToNormalized(CanvasPosition);
	}
	
	UTexture2D* DefaultBackground = nullptr;
	
	switch (ControlType)
	{
	case EControlType::Button:
		{
			const FSoftObjectPath DefaultBackgroundObject = GetDefault<UTouchInterfaceDesignerSettings>()->DefaultButtonImage;
			if(DefaultBackgroundObject.IsValid())
			{
				DefaultBackground = LoadObject<UTexture2D>(nullptr, *DefaultBackgroundObject.ToString());
			}
			else
			{
				DefaultBackground = Cast<UTexture2D>(FTouchInterfaceDesignerStyle::Get().GetBrush("DefaultBackgroundButton")->GetResourceObject());
			}
			
			//Set default visual layer for button
			NewVirtualControl.VisualLayers = {FVisualLayer(DefaultBackground, 13)};
		}
		break;
	case EControlType::Joystick:
		{
			UTexture2D* DefaultThumb = nullptr;
			
			const FSoftObjectPath DefaultBackgroundObject = GetDefault<UTouchInterfaceDesignerSettings>()->DefaultBackgroundJoystickImage;
			if(DefaultBackgroundObject.IsValid())
			{
				DefaultBackground = LoadObject<UTexture2D>(nullptr, *DefaultBackgroundObject.ToString());
			}
			else
			{
				DefaultBackground = Cast<UTexture2D>(FTouchInterfaceDesignerStyle::Get().GetBrush("DefaultJoystickBackground")->GetResourceObject());
			}
			
			const FSoftObjectPath DefaultThumbObject = GetDefault<UTouchInterfaceDesignerSettings>()->DefaultThumbJoystickImage;
			if (DefaultThumbObject.IsValid())
			{
				DefaultThumb = LoadObject<UTexture2D>(nullptr, *DefaultThumbObject.ToString());
			}
			else
			{
				DefaultThumb = Cast<UTexture2D>(FTouchInterfaceDesignerStyle::Get().GetBrush("DefaultJoystickThumb")->GetResourceObject());
			}

			//Set default visual layer for joystick
			NewVirtualControl.VisualLayers = {FVisualLayer(DefaultBackground, 13), FVisualLayer(DefaultThumb, 14)};
		}
		break;
	case EControlType::TouchRegion:
		break;
	default:
		break;
	}

	VirtualControlSetupEdited->VirtualControls.Add(NewVirtualControl);

	VirtualControlSetupEdited->SelectLastControl();
	VirtualControlSetupEdited->Modify();
	DetailsPanel->Refresh();
	
	ViewportWidget->AddVirtualControlInCanvasSpace(NewVirtualControl, CanvasPosition);

	HierarchyTab->AddNewItem(ControlType, ControlName);
	VirtualControlPreviewerTab->SetPreviewData(NewVirtualControl);
}

void FVirtualControlDesignerEditor::AddNewControlFromCopyCache()
{
	if (IsCopyAvailable())
	{
		DetailTabState = DTS_ShowControlProperties;
		
		ControlDataCopied.ControlName = VirtualControlSetupEdited->GetUniqueName(ControlDataCopied.Type);
		ControlDataCopied.ClearLinkData();
		
		VirtualControlSetupEdited->VirtualControls.Add(ControlDataCopied);

		GetVirtualControlSetup()->SelectLastControl();
		
		VirtualControlSetupEdited->Modify();
		DetailsPanel->Refresh();
		
		ViewportWidget->AddVirtualControlInCanvasSpace(ControlDataCopied);
		
		HierarchyTab->AddNewItem(ControlDataCopied.Type, ControlDataCopied.ControlName);
		VirtualControlPreviewerTab->SetPreviewData(ControlDataCopied);

		//Todo: Select last control
		//SelectControl(ControlDataCopied.ControlName);
	}
}

void FVirtualControlDesignerEditor::SelectControl(FName ControlName)
{
	for(int32 Index = 0; Index<VirtualControlSetupEdited->VirtualControls.Num(); Index++)
	{
		if (VirtualControlSetupEdited->VirtualControls[Index].ControlName == ControlName)
		{
			VirtualControlSetupEdited->SetSelectedControlIndex(Index);
			DetailTabState = DTS_ShowControlProperties;
			DetailsPanel->Refresh();
	
			ViewportWidget->SelectWidget(ControlName);
			HierarchyTab->SelectButton(ControlName);
			VirtualControlPreviewerTab->SetPreviewData(VirtualControlSetupEdited->VirtualControls[Index]);
			SelectedControlName = ControlName;
			break;
		}
	}	
}

void FVirtualControlDesignerEditor::UnselectControl()
{
	VirtualControlSetupEdited->SetSelectedControlIndex(-1); // = -1
	DetailTabState = DTS_ShowControlProperties;
	DetailsPanel->Refresh();
	
	ViewportWidget->ClearOutlineEffect();
	HierarchyTab->ClearButtonEffect();
	VirtualControlPreviewerTab->ClearPreview();
	SelectedControlName = NAME_None;
}

void FVirtualControlDesignerEditor::SetControlHoverState(FName ControlName, bool InIsHovered, bool SendToWidget, bool SendToButton) const
{
	if (SendToWidget)
	{
		ViewportWidget->HoverWidget(ControlName, InIsHovered);
	}
	
	if (SendToButton)
	{
		HierarchyTab->SetButtonHoverEffect(ControlName, InIsHovered);
	}
}

void FVirtualControlDesignerEditor::ChangeControlName(FName ControlName, FName NewControlName) const
{
	for (FVirtualControl& VirtualControl : VirtualControlSetupEdited->VirtualControls)
	{
		if (VirtualControl.ControlName == ControlName)
		{
			//If Child, change name in parent
			if (VirtualControl.bIsChild)
			{
				FVirtualControl& ParentControl = VirtualControlSetupEdited->GetVirtualControlRef(VirtualControl.ParentName);
				ParentControl.Children.Remove(ControlName);
				ParentControl.Children.Add(NewControlName);
			}
			
			//If Parent, change name in all children
			if (VirtualControl.IsParent())
			{
				for (const FName ChildName : VirtualControl.Children)
				{
					FVirtualControl& ChildControl = VirtualControlSetupEdited->GetVirtualControlRef(ChildName);
					ChildControl.ParentName = NewControlName;
				}
			}
			
			VirtualControl.ControlName = NewControlName;

			// Update virtual control data in viewport
			ViewportWidget->ChangeControlName(ControlName, NewControlName, VirtualControl);
			break;
		}
	}

	VirtualControlSetupEdited->Modify();
	DetailsPanel->Refresh();
}

void FVirtualControlDesignerEditor::RemoveControl(const FName ControlName, const EControlType Type) const
{
	ViewportWidget->RemoveControl(ControlName);
	HierarchyTab->RemoveItem(ControlName);
	
	int32 IndexToRemove = -1;
	for (int32 Index = 0; Index < VirtualControlSetupEdited->VirtualControls.Num(); Index++)
	{
		if (VirtualControlSetupEdited->VirtualControls[Index].ControlName.IsEqual(ControlName))
		{
			IndexToRemove = Index;
			break;
		}
	}
	if (IndexToRemove >= 0)
	{
		VirtualControlSetupEdited->VirtualControls.RemoveAt(IndexToRemove);
		VirtualControlSetupEdited->SetSelectedControlIndex(VirtualControlSetupEdited->VirtualControls.Num()-1);
		VirtualControlSetupEdited->Modify();
		DetailsPanel->Refresh();

		//Todo: Select Last Control in array
	}
}

void FVirtualControlDesignerEditor::LinkControl(const FName ParentControl, const FName ChildControl, const FVector2D Offset, const bool ForceRefresh) const
{
	for (FVirtualControl& VirtualControl : VirtualControlSetupEdited->VirtualControls)
	{
		if (VirtualControl.ControlName == ChildControl)
		{
			VirtualControl.bIsChild = true;
			VirtualControl.ParentName = ParentControl;
			VirtualControl.ParentOffset = Offset;
		}

		if (VirtualControl.ControlName == ParentControl)
		{
			VirtualControl.Children.Add(ChildControl);
		}
	}
	
	if (ForceRefresh)
	{
		VirtualControlSetupEdited->Modify();
		DetailsPanel->Refresh();
	}
	
	ViewportWidget->Link(ParentControl, ChildControl, Offset);
}

void FVirtualControlDesignerEditor::UnlinkControl(const FName ChildControl, const bool ForceRefresh) const
{
	FName ParentName = NAME_None;

	for (FVirtualControl& VirtualControl : VirtualControlSetupEdited->VirtualControls)
	{
		if (VirtualControl.ControlName == ChildControl)
		{
			VirtualControl.bIsChild = false;
			ParentName = VirtualControl.ParentName;
			VirtualControl.ParentName = NAME_None;
			VirtualControl.ParentOffset = FVector2D::ZeroVector;

			if (IsOrientationInLandscape())
			{
				VirtualControl.LandscapeCenter = ViewportWidget->GetVirtualControlPosition(ChildControl, true);
			}
			else
			{
				VirtualControl.PortraitCenter = ViewportWidget->GetVirtualControlPosition(ChildControl, true);
			}
			
			break;
		}
	}

	if (!ParentName.IsNone())
	{
		for (FVirtualControl& VirtualControl : VirtualControlSetupEdited->VirtualControls)
		{
			if (VirtualControl.ControlName == ParentName)
			{
				VirtualControl.Children.Remove(ChildControl);
				ViewportWidget->Unlink(ParentName, ChildControl);
				break;
			}
		}
	}

	if (ForceRefresh)
	{
		VirtualControlSetupEdited->Modify();
		DetailsPanel->Refresh();
	}
}

void FVirtualControlDesignerEditor::UnlinkAllControl(const FName ParentControl) const
{
	for (FVirtualControl& VirtualControl : VirtualControlSetupEdited->VirtualControls)
	{
		if (VirtualControl.ControlName == ParentControl)
		{
			TArray<FName> ChildrenName = VirtualControl.Children;
			for (const FName ChildName : ChildrenName)
			{
				UnlinkControl(ChildName, false);
			}
		}
	}
	
	VirtualControlSetupEdited->Modify();
	DetailsPanel->Refresh();
}

void FVirtualControlDesignerEditor::CutSelected()
{
	//Todo : Make better cut. Widget appear disabled instead of removed, if user paste, remove widget and create new. If user copy other, clear disabled appearance 
	if (CopySelectedControlData()) RemoveSelected();
}

void FVirtualControlDesignerEditor::CopySelected()
{
	CopySelectedControlData();
}

void FVirtualControlDesignerEditor::PasteCopiedControl()
{
	if (IsCopyAvailable())
	{
		const FVector2D CursorPosition = FSlateApplication::Get().GetCursorPos();

		const FVector2D CanvasPosition = ViewportWidget->AbsoluteToNormalize(CursorPosition);

		if (IsOrientationInLandscape())
		{
			//ControlDataCopied.LandscapeCenter = ViewportWidget->DeprecatedAbsoluteToRelative(CursorPosition);
			ControlDataCopied.LandscapeCenter = CanvasPosition;
		}
		else
		{
			//ControlDataCopied.PortraitCenter = ViewportWidget->DeprecatedAbsoluteToRelative(CursorPosition);
			ControlDataCopied.PortraitCenter = CanvasPosition;
		}

		AddNewControlFromCopyCache();

		bCopyAvailable = false;
	}
}

void FVirtualControlDesignerEditor::DuplicateSelected()
{	
	CopySelectedControlData();

	if (IsCopyAvailable())
	{
		if (ViewportWidget->IsInLandscapeOrientation())
		{
			ControlDataCopied.LandscapeCenter += FVector2D(0.05f, 0.05f);
		}
		else
		{
			ControlDataCopied.PortraitCenter += FVector2D(0.05f, 0.05f);
		}
		
		AddNewControlFromCopyCache();
	}
}

void FVirtualControlDesignerEditor::RemoveSelected() const
{	
	if (!SelectedControlName.IsNone())
	{
		const FVirtualControl Control = VirtualControlSetupEdited->GetSelectedControl();

		if (Control.IsParent())
		{
			UnlinkAllControl(Control.ControlName);
		}
		else if (Control.bIsChild)
		{
			UnlinkControl(Control.ControlName, false);
		}
		
		ViewportWidget->RemoveControl(Control.ControlName);
		HierarchyTab->RemoveItem(Control.ControlName);
	
		int32 IndexToRemove = -1;
		for (int32 Index = 0; Index < VirtualControlSetupEdited->VirtualControls.Num(); Index++)
		{
			if (VirtualControlSetupEdited->VirtualControls[Index].ControlName.IsEqual(Control.ControlName))
			{
				IndexToRemove = Index;
				break;
			}
		}
		if (IndexToRemove >= 0)
		{
			VirtualControlSetupEdited->VirtualControls.RemoveAt(IndexToRemove);
			VirtualControlSetupEdited->SetSelectedControlIndex(-1);

			//Todo: Select Last Control in array
			//Select last control or nothing if there is no control
			//VirtualControlSetup->SelectLastControl();

			VirtualControlSetupEdited->Modify();
			DetailsPanel->Refresh();
		}
	}
}

void FVirtualControlDesignerEditor::NotifyControlPositionChanged(const FName ControlName, FVector2D NewPosition) const
{
	const FVector2D NormalizedPosition = ViewportWidget->CanvasSpaceToNormalized(NewPosition);
	
	FVirtualControl& VirtualControl = VirtualControlSetupEdited->GetVirtualControlRef(ControlName);

	if (bLandscapeOrientation)
	{
		VirtualControl.LandscapeCenter = NormalizedPosition;
	}
	else
	{
		VirtualControl.PortraitCenter = NormalizedPosition;
	}

	if (VirtualControl.bIsChild)
	{
		VirtualControl.ParentOffset = ViewportWidget->CalculateOffsetBetweenControl(VirtualControl.ParentName, VirtualControl.ControlName);
	}
	
	VirtualControlSetupEdited->Modify();
	DetailsPanel->Refresh();
	ViewportWidget->UpdateSelectedControl(VirtualControl);
}

void FVirtualControlDesignerEditor::NotifyOpacityChange() const
{
	ViewportWidget->ChangeOpacity(VirtualControlSetupEdited->ActiveOpacity, VirtualControlSetupEdited->InactiveOpacity);
}

void FVirtualControlDesignerEditor::NotifyControlChange() const
{	
	const FVirtualControl VirtualControl = VirtualControlSetupEdited->GetSelectedControl();
	ViewportWidget->UpdateSelectedControl(VirtualControl);

	if (VirtualControl.bIsChild)
	{
		const FVirtualControl ParentVirtualControl = VirtualControlSetupEdited->GetVirtualControlRef(VirtualControl.ParentName);
		ViewportWidget->UpdateSelectedControl(ParentVirtualControl);
	}
	
	VirtualControlPreviewerTab->SetPreviewData(VirtualControl);
}

void FVirtualControlDesignerEditor::NotifyOrientationChanged(const bool InLandscapeMode)
{
	//Refresh detail tab
	ForceRefresh();

	bLandscapeOrientation = InLandscapeMode;

	//Clear copy data
	bCopyAvailable = false;
}

TSharedPtr<FUICommandList> FVirtualControlDesignerEditor::GetCommands()
{
	if (TouchDesignerEditorCommands.IsValid()) return TouchDesignerEditorCommands;
	
	return nullptr;
}

void FVirtualControlDesignerEditor::HandleOnCalculatePortraitPosition()
{
	// Todo: Add options to adjust the calculation such as using only the lower half of the screen

	if (VirtualControlSetupEdited->VirtualControls.Num() > 0)
	{
		/*for (int32 Itr = 0; Itr < VirtualControlSetupEdited->VirtualControls.Num(); ++Itr)
		{
			const FVector2D LandscapePosition = VirtualControlSetupEdited->VirtualControls[Itr].LandscapeCenter;
			VirtualControlSetupEdited->VirtualControls[Itr].PortraitCenter = LandscapePosition;
		}*/

		for (FVirtualControl& Control : VirtualControlSetupEdited->VirtualControls)
		{
			const FVector2D LandscapePosition = Control.LandscapeCenter;
			Control.PortraitCenter = LandscapePosition;
		}
		
		ViewportWidget->Refresh();
	
		VirtualControlSetupEdited->Modify();
		ForceRefresh();

		FNotificationInfo NotificationInfo(LOCTEXT("CalculatePortraitMsgKey", "Portrait layout successfully generated"));
		NotificationInfo.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(NotificationInfo);
	}
	else
	{
		FNotificationInfo NotificationInfo(LOCTEXT("CalculatePortraitMsgKey", "Fail! There is no control, add a control first"));
		NotificationInfo.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(NotificationInfo);
	}
}

void FVirtualControlDesignerEditor::HandleOnOpenPresetManager()
{
	if (!PresetManager)
	{
		UE_LOG(LogVirtualControlDesigner, Error, TEXT("Instance of Preset Manager not found"));
		return;
	}

	PresetManager->OpenPresetWindow();
}

bool FVirtualControlDesignerEditor::VerifyTextCommitted(const FText& Text, FText& OtherText) const
{
	//Todo: check if Text contains whitespace
	if (Text.IsEmpty())
	{
		OtherText = FText(INVTEXT("Fill control name"));
		return false;
	}
	
	for (const FVirtualControl& ControlItem : VirtualControlSetupEdited->VirtualControls)
	{
		if (Text.EqualTo(FText::FromName(ControlItem.ControlName)))
		{
			OtherText = FText(INVTEXT("This name already exist"));
			return false;
		}
	}

	return true;
}

bool FVirtualControlDesignerEditor::ContainAnySpace(const FText TextToVerify)
{
	return ContainAnySpace(TextToVerify.ToString());
}

bool FVirtualControlDesignerEditor::ContainAnySpace(const FString TextToVerify)
{
	return TextToVerify.Contains(" ");
}

FReply FVirtualControlDesignerEditor::HandleDPISettingsClicked() const
{
	FModuleManager::LoadModuleChecked<ISettingsModule>("Settings").ShowViewer("Project", "Engine", "UI");
	return FReply::Handled();
}

FReply FVirtualControlDesignerEditor::HandleTouchInterfaceSettingsClicked() const
{
	FModuleManager::LoadModuleChecked<ISettingsModule>("Settings").ShowViewer("Project", "Plugins", "Touch Interface Designer");
	return FReply::Handled();
}

void FVirtualControlDesignerEditor::CreateButtonCommand()
{
	const FVector2D CanvasPosition = ViewportWidget->AbsoluteToCanvasSpace(FSlateApplication::Get().GetCursorPos());

	//Todo: Remove LeftMouseButton ?
	if (FSlateApplication::Get().GetPressedMouseButtons().Contains(EKeys::LeftMouseButton))
	{
		AddNewControl(EControlType::Button, CanvasPosition);
	}
}

void FVirtualControlDesignerEditor::CreateJoystickCommand()
{
	const FVector2D CanvasPosition = ViewportWidget->AbsoluteToCanvasSpace(FSlateApplication::Get().GetCursorPos());

	if (FSlateApplication::Get().GetPressedMouseButtons().Contains(EKeys::LeftMouseButton))
	{
		AddNewControl(EControlType::Joystick, CanvasPosition);
	}
}

void FVirtualControlDesignerEditor::CreateTouchRegionCommand()
{
	const FVector2D CanvasPosition = ViewportWidget->AbsoluteToCanvasSpace(FSlateApplication::Get().GetCursorPos());

	if (FSlateApplication::Get().GetPressedMouseButtons().Contains(EKeys::LeftMouseButton))
	{
		AddNewControl(EControlType::TouchRegion, CanvasPosition);
	}
}

bool FVirtualControlDesignerEditor::CanCreateNewControl() const
{
	if (ViewportWidget.IsValid())
	{
		return ViewportWidget->CanCreateNewControl();
	}
	
	return false;
}

bool FVirtualControlDesignerEditor::CanExecuteAction() const
{
	if (ViewportWidget.IsValid())
	{
		return !ViewportWidget->IsDragControl();
	}
	
	return false;
}

void FVirtualControlDesignerEditor::OnDuplicateCommand()
{
	DuplicateSelected();
}

void FVirtualControlDesignerEditor::OnCopyCommand()
{
	CopySelected();
}

void FVirtualControlDesignerEditor::OnCutCommand()
{
	CutSelected();
}

void FVirtualControlDesignerEditor::OnPasteCommand()
{
	PasteCopiedControl();
}

void FVirtualControlDesignerEditor::OnDeleteCommand() const
{
	RemoveSelected();
}

void FVirtualControlDesignerEditor::OnRecenterCanvasCommand() const
{
	ViewportWidget->RecenterDesigner();
}

void FVirtualControlDesignerEditor::OnRenameCommand() const
{
	if (!SelectedControlName.IsNone())
	{
		HierarchyTab->EditItemText(SelectedControlName);
	}
}

void FVirtualControlDesignerEditor::OnSaveCommand() const
{
#if ENGINE_MAJOR_VERSION > 4
	FSavePackageArgs SavePackageArgs;
	SavePackageArgs.TopLevelFlags = RF_Standalone;
	GEditor->SavePackage(VirtualControlSetupEdited->GetPackage(), VirtualControlSetupEdited, *VirtualControlSetupEdited->GetName(), SavePackageArgs);
#endif
}

void FVirtualControlDesignerEditor::OpenControlDesigner() const
{
	UE_LOG(LogVirtualControlDesigner, Log, TEXT("Open Control Designer"));
}

void FVirtualControlDesignerEditor::OnShowGeneralSettingsCommand()
{
	//Todo: Clear Selected Widget
	UnselectControl();
	DetailTabState = DTS_ShowGeneralProperties;
	ForceRefresh();
}

void FVirtualControlDesignerEditor::OnShowBackgroundSettingsCommand()
{
	UnselectControl();
	DetailTabState = DTS_ShowBackgroundProperties;
	ForceRefresh();
}

void FVirtualControlDesignerEditor::HandleOnApplyPreset(const UTouchInterfacePreset* PresetSelected, const bool bAddVirtualControls, const bool bApplySettings)
{
	UE_LOG(LogVirtualControlDesigner, Log, TEXT("Apply Preset"));

	if (bAddVirtualControls)
	{
		// Add Preset virtual controls to Virtual Control Setup
		VirtualControlSetupEdited->VirtualControls = PresetSelected->GetControlsSetting();
	}

	if (bApplySettings)
	{
		// Add preset general settings to Virtual Control Setup
		const FInterfaceSettings GeneralSettings = PresetSelected->GetGeneralSettings();
	
		VirtualControlSetupEdited->ActiveOpacity = GeneralSettings.ActiveOpacity;
		VirtualControlSetupEdited->InactiveOpacity = GeneralSettings.InactiveOpacity;
		VirtualControlSetupEdited->ActivationDelay = GeneralSettings.ActivationDelay;
		VirtualControlSetupEdited->StartupDelay = GeneralSettings.StartupDelay;
		VirtualControlSetupEdited->TimeUntilDeactivated = GeneralSettings.TimeUntilDeactivated;
		VirtualControlSetupEdited->TimeUntilReset = GeneralSettings.TimeUntilReset;
	}
	
	VirtualControlSetupEdited->Modify();
	ForceRefresh();
	
	// Force refresh of Viewport and ControlsTab
	ViewportWidget->Refresh();
	HierarchyTab->Refresh();

	FNotificationInfo NotificationInfo(INVTEXT("Preset was successfully applied"));
	NotificationInfo.ExpireDuration = 3.0f;
	FSlateNotificationManager::Get().AddNotification(NotificationInfo);
}

bool FVirtualControlDesignerEditor::CopySelectedControlData()
{
	if (!SelectedControlName.IsNone())
	{
		ControlDataCopied = VirtualControlSetupEdited->GetSelectedControl();
		bCopyAvailable = true;
		return true;
	}

	bCopyAvailable = false;
	return false;
}

void FVirtualControlDesignerEditor::ClearCopyCache()
{
	bCopyAvailable = false;
	ControlDataCopied = FVirtualControl();
}

void FVirtualControlDesignerEditor::SaveAsPresetCommand()
{
	if (PresetManager)
	{
		PresetManager->SavePreset(VirtualControlSetupEdited);
	}
}

#undef LOCTEXT_NAMESPACE
