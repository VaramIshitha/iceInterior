// Copyright Lost in Game Studio. All Rights Reserved.

#include "VirtualShapeDesignerEditor.h"

#include "SVirtualShapeDesignerDetailsPanel.h"
#include "SVirtualShapeDesignerDrawer.h"
#include "SVirtualShapeDesignerPalette.h"
#include "SVirtualShapeDesignerViewport.h"
#include "VirtualShapeDesigner.h"
#include "VirtualShapeDesignerCommands.h"
#include "VirtualShapeDesignerEditorStyle.h"
#include "VirtualShapeDesignerQuickSettings.h"
#include "Classes/VirtualShape.h"
#include "Debug/SVirtualShapeDesignerShapeInfo.h"
#include "Debug/SVirtualShapeDesignerCompileResult.h"

DEFINE_LOG_CATEGORY_STATIC(LogVirtualShapeDesigner, All, All);

#define LOCTEXT_NAMESPACE "VirtualShapeDesignerEditor"

const FName VirtualShapeDesignerAppName = FName(TEXT("VirtualShapeDesignerEditorApp"));

struct FVirtualShapeDesignerTabs
{
	//Tab identifiers
	static const FName DetailsId;
	static const FName ViewportId;
	static const FName DrawerId;
	static const FName PaletteId;
	static const FName ShapeInfoId;
	static const FName CompileResultId;
};

const FName FVirtualShapeDesignerTabs::DetailsId(TEXT("Details"));
const FName FVirtualShapeDesignerTabs::ViewportId(TEXT("Viewport"));
const FName FVirtualShapeDesignerTabs::DrawerId(TEXT("Drawer"));
const FName FVirtualShapeDesignerTabs::PaletteId(TEXT("Palette"));
const FName FVirtualShapeDesignerTabs::ShapeInfoId(TEXT("ShapeInfo"));
const FName FVirtualShapeDesignerTabs::CompileResultId(TEXT("CompileResult"));


FVirtualShapeDesignerEditor::FVirtualShapeDesignerEditor()
: VirtualShapeEdited(nullptr)
{
	bIsDirty = false;
}

FVirtualShapeDesignerEditor::~FVirtualShapeDesignerEditor()
{
	
}

FText FVirtualShapeDesignerEditor::GetToolkitName() const
{
	return FText::FromString(VirtualShapeEdited->GetName());
}

FText FVirtualShapeDesignerEditor::GetToolkitToolTipText() const
{
	const UObject* EditingObject = VirtualShapeEdited;
	check(EditingObject != nullptr);
	return GetToolTipTextForObject(EditingObject);
}

bool FVirtualShapeDesignerEditor::ProcessCommandBindings(const FKeyEvent& InKeyEvent) const
{
	//return ShapeDesignerEditorCommands->ProcessCommandBindings(InKeyEvent);
	return FAssetEditorToolkit::ProcessCommandBindings(InKeyEvent);
}

void FVirtualShapeDesignerEditor::PostRedo(bool bSuccess)
{
	UE_LOG(LogVirtualShapeDesigner, Log, TEXT("Post Redo"));
	FEditorUndoClient::PostRedo(bSuccess);
}

void FVirtualShapeDesignerEditor::PostUndo(bool bSuccess)
{
	UE_LOG(LogVirtualShapeDesigner, Log, TEXT("Post Undo"));
	FEditorUndoClient::PostUndo(bSuccess);
}

void FVirtualShapeDesignerEditor::InitEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UVirtualShape* InVirtualShape)
{
	//Set this InVirtualShape as our editing asset
	VirtualShapeEdited = InVirtualShape;

	//Bind custom command and button for our Virtual Shape Designer
	BindCommand();

	const TSharedRef<FTabManager::FLayout> VirtualShapeDesignerLayout = FTabManager::NewLayout("Standalone_VirtualShapeDesigner_v1_dev10")
	->AddArea
	(
		// Make Area for Tabs and set orientation to vertical mainly for UE4 (Toolbar tab)
		FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)

#if ENGINE_MAJOR_VERSION < 5
		->Split
		(
			//Add Toolbar tab for UE4
			FTabManager::NewStack()
			->SetHideTabWell(false)
			->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
		)
#endif

		->Split
		(
			FTabManager::NewSplitter()->SetOrientation(Orient_Horizontal)

			->Split
			(
				//Add palette tab
				FTabManager::NewStack()
				->SetSizeCoefficient(0.1f)
				->SetHideTabWell(true)
				->AddTab(FVirtualShapeDesignerTabs::PaletteId, ETabState::OpenedTab)
			)

			->Split
			(
				FTabManager::NewSplitter()->SetOrientation(Orient_Vertical)

				->Split
				(
					//Add viewport tab
					FTabManager::NewStack()
					->SetHideTabWell(true)
					->AddTab(FVirtualShapeDesignerTabs::ViewportId, ETabState::OpenedTab)
					->AddTab(FVirtualShapeDesignerTabs::DrawerId, ETabState::OpenedTab)
				)

				->Split
				(
					//Add compilation result tab
					FTabManager::NewStack()
					->SetSizeCoefficient(0.2f)
					->SetHideTabWell(false)
					->AddTab(FVirtualShapeDesignerTabs::CompileResultId, ETabState::OpenedTab)
				)
			)

			->Split
			(
				FTabManager::NewSplitter()->SetOrientation(Orient_Vertical)
				->SetSizeCoefficient(0.3f)
				
				->Split
				(
					//Add detail tab
					FTabManager::NewStack()
					->SetHideTabWell(false)
					->AddTab(FVirtualShapeDesignerTabs::DetailsId, ETabState::OpenedTab)
				)

				->Split
				(
					//Add virtual shape info
					FTabManager::NewStack()
					->SetSizeCoefficient(0.5f)
					->SetHideTabWell(true)
					->AddTab(FVirtualShapeDesignerTabs::ShapeInfoId, ETabState::OpenedTab)
				)
			)
		)
	);

	constexpr bool bCreateDefaultStandaloneMenu = true;
	constexpr bool bCreateDefaultToolbar = true;

	//Initialize our Virtual Shape Designer interface editor
	InitAssetEditor(Mode, InitToolkitHost, VirtualShapeDesignerAppName, VirtualShapeDesignerLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, InVirtualShape);

	//Then, regenerate menu and toolbars
	RegenerateMenusAndToolbars();

	// Notify editor when active tab has changed
	//FGlobalTabmanager::Get()->OnActiveTabChanged_Subscribe(FOnActiveTabChanged::FDelegate::CreateSP())
}

void FVirtualShapeDesignerEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	//Add a new workspace menu category to the tab manager
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(INVTEXT("Virtual Shape Designer Editor"));
	const TSharedRef<FWorkspaceItem> WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	//We register the tab manager to the asset editor toolkit so we can use it in this editor
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	/** Register the properties tab spawner within our tab manager
	 * we provide the function with the identifier for this tab and a shared pointer to the
	 * SpawnTab_XXX function within this editor class
	 * Additionnaly, we provide a name to be displayed, a category and the tab icon
	 */

	InTabManager->RegisterTabSpawner(FVirtualShapeDesignerTabs::PaletteId, FOnSpawnTab::CreateSP(this, &FVirtualShapeDesignerEditor::SpawnTab_Palette))
	.SetDisplayName(INVTEXT("Tools"))
	.SetGroup(WorkspaceMenuCategoryRef);
	//.SetIcon(FSlateIcon(FVirtualControlDesignerEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(FVirtualShapeDesignerTabs::ViewportId, FOnSpawnTab::CreateSP(this, &FVirtualShapeDesignerEditor::SpawnTab_Viewport))
	.SetDisplayName(INVTEXT("Viewport"))
	.SetGroup(WorkspaceMenuCategoryRef);

	InTabManager->RegisterTabSpawner(FVirtualShapeDesignerTabs::DrawerId, FOnSpawnTab::CreateSP(this, &FVirtualShapeDesignerEditor::SpawnTab_Drawer))
	.SetDisplayName(INVTEXT("Drawer"))
	.SetGroup(WorkspaceMenuCategoryRef);

	InTabManager->RegisterTabSpawner(FVirtualShapeDesignerTabs::DetailsId, FOnSpawnTab::CreateSP(this, &FVirtualShapeDesignerEditor::SpawnTab_Details))
	.SetDisplayName(INVTEXT("Details"))
	.SetGroup(WorkspaceMenuCategoryRef);

	InTabManager->RegisterTabSpawner(FVirtualShapeDesignerTabs::ShapeInfoId, FOnSpawnTab::CreateSP(this, &FVirtualShapeDesignerEditor::SpawnTab_ShapeInfo))
	.SetDisplayName(INVTEXT("Virtual Shape Info"))
	.SetGroup(WorkspaceMenuCategoryRef);

	InTabManager->RegisterTabSpawner(FVirtualShapeDesignerTabs::CompileResultId, FOnSpawnTab::CreateSP(this, &FVirtualShapeDesignerEditor::SpawnTab_CompileResult))
	.SetDisplayName(INVTEXT("Compilation"))
	.SetGroup(WorkspaceMenuCategoryRef);
}

void FVirtualShapeDesignerEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	InTabManager->UnregisterAllTabSpawners();

	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
}

TSharedRef<SDockTab> FVirtualShapeDesignerEditor::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FVirtualShapeDesignerTabs::DetailsId);
	
	return SNew(SDockTab)
	.Label(INVTEXT("Details"))
	[
		SAssignNew(DetailsPanel, SVirtualShapeDesignerDetailsPanel, SharedThis(this))
	];
}

TSharedRef<SDockTab> FVirtualShapeDesignerEditor::SpawnTab_Viewport(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FVirtualShapeDesignerTabs::ViewportId);
	
	return SNew(SDockTab)
	.Label(INVTEXT("Viewport"))
	[
		SAssignNew(ViewportWidget, SVirtualShapeDesignerViewport, SharedThis(this)).CommandList(this, &FVirtualShapeDesignerEditor::GetCommandList)
	];
}

TSharedRef<SDockTab> FVirtualShapeDesignerEditor::SpawnTab_Drawer(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FVirtualShapeDesignerTabs::DrawerId);

	return SNew(SDockTab)
	.Label(INVTEXT("Drawer"))
	[
		SAssignNew(DrawerWidget, SVirtualShapeDesignerDrawer, SharedThis(this))
	];
}

TSharedRef<SDockTab> FVirtualShapeDesignerEditor::SpawnTab_Palette(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FVirtualShapeDesignerTabs::PaletteId);
	
	return SNew(SDockTab)
	.Label(INVTEXT("Tools"))
	[
		SAssignNew(PaletteWidget, SVirtualShapeDesignerPalette, SharedThis(this))
	];
}

TSharedRef<SDockTab> FVirtualShapeDesignerEditor::SpawnTab_ShapeInfo(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FVirtualShapeDesignerTabs::ShapeInfoId);
	
	return SNew(SDockTab)
	.Label(INVTEXT("Shape Info"))
	[
		SAssignNew(ShapeInfoWidget, SVirtualShapeDesignerShapeInfo).VirtualShapeEdited(this, &FVirtualShapeDesignerEditor::GetVirtualShape)
	];
}

TSharedRef<SDockTab> FVirtualShapeDesignerEditor::SpawnTab_CompileResult(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FVirtualShapeDesignerTabs::CompileResultId);
	
	return SNew(SDockTab)
	.Label(INVTEXT("Compile"))
	[
		SAssignNew(CompileWidget, SVirtualShapeDesignerCompileResult)
	];
}

void FVirtualShapeDesignerEditor::BindCommand()
{
	if (VirtualShapeDesignerCommands.IsValid()) return;

	VirtualShapeDesignerCommands = MakeShareable(new FUICommandList);

	const FVirtualShapeDesignerCommands& Commands = FVirtualShapeDesignerCommands::Get();

	FVirtualShapeDesignerModule& Module = FModuleManager::LoadModuleChecked<FVirtualShapeDesignerModule>("VirtualShapeDesigner");

	TSharedPtr<FExtender> NewToolbarExtender = Module.GetToolBarExtensibilityManager()->GetAllExtenders();
	NewToolbarExtender->AddToolBarExtension("Asset", EExtensionHook::After, VirtualShapeDesignerCommands, FToolBarExtensionDelegate::CreateRaw(this, &FVirtualShapeDesignerEditor::AddToolbarButton));
	AddToolbarExtender(NewToolbarExtender);

	VirtualShapeDesignerCommands->MapAction(Commands.CompileCommand, FExecuteAction::CreateRaw(this, &FVirtualShapeDesignerEditor::HandleCompileCommand));
}

void FVirtualShapeDesignerEditor::AddToolbarButton(FToolBarBuilder& Builder)
{
	Builder.BeginSection("Compilation");
	Builder.AddToolBarButton(FVirtualShapeDesignerCommands::Get().CompileCommand, "Compilation", INVTEXT("Compile"),
		TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &FVirtualShapeDesignerEditor::GetCompileButtonText)),
		TAttribute<FSlateIcon>::Create(TAttribute<FSlateIcon>::FGetter::CreateSP(this, &FVirtualShapeDesignerEditor::GetCompileButtonIcon)));
	Builder.EndSection();
	
	Builder.BeginSection("Settings");
	Builder.AddToolBarWidget(SNew(SVirtualShapeDesignerQuickSettings), INVTEXT("Corner Threshold"));

	//FUIAction ComputationMode = FUIAction();
	//Builder.AddComboButton(FUIAction(), FOnGetContent::CreateSP(this, &FVirtualShapeDesignerEditor::GetComputationModeMenuContent));
	Builder.EndSection();

	Builder.BeginSection("Tools");
	Builder.AddToolBarButton(FVirtualShapeDesignerCommands::Get().SimplifyCommand);
	Builder.EndSection();
}

TSharedRef<SWidget> FVirtualShapeDesignerEditor::GetComputationModeMenuContent()
{
	FMenuBuilder MenuBuilder(false, VirtualShapeDesignerCommands);

	// FUIAction OutlineAction(FExecuteAction::CreateRaw(this, &SVirtualShapeDesignerViewport::ToggleDirectionDebug), FCanExecuteAction(), FIsActionChecked::CreateLambda([this](){ return bShowDrawDirection; }));
	// MenuBuilder.AddMenuEntry(INVTEXT("Timer"),FText::GetEmpty(), FSlateIcon(), OutlineAction, NAME_None, EUserInterfaceActionType::CollapsedButton);
	//
	// FUIAction ControlNameAction(FExecuteAction::CreateRaw(this, &SVirtualShapeDesignerViewport::ToggleLenghtDebug), FCanExecuteAction(), FIsActionChecked::CreateLambda([this](){ return bShowDrawLenght; }));
	// MenuBuilder.AddMenuEntry(INVTEXT("Right Click"),FText::GetEmpty(), FSlateIcon(), ControlNameAction, NAME_None, EUserInterfaceActionType::CollapsedButton);
	//
	// FUIAction InteractionSizeAction(FExecuteAction::CreateRaw(this, &SVirtualShapeDesignerViewport::ToggleAngleDebug), FCanExecuteAction(), FIsActionChecked::CreateLambda([this](){ return bShowDrawAngle; }));
	// MenuBuilder.AddMenuEntry(INVTEXT("Button"),FText::GetEmpty(), FSlateIcon(), InteractionSizeAction, NAME_None, EUserInterfaceActionType::CollapsedButton);

	return MenuBuilder.MakeWidget();
}

FText FVirtualShapeDesignerEditor::GetCompileButtonText() const
{
	return bIsDirty ? FText::FromString("Is Dirty") : FText::FromString("Good to go");
}

FSlateIcon FVirtualShapeDesignerEditor::GetCompileButtonIcon() const
{
	const FName IconName = bIsDirty ? FName("Kismet.Status.Unknown") : FName("Kismet.Status.Good");
	return FSlateIcon(GetSlateStyle().GetStyleSetName(), IconName);
}

void FVirtualShapeDesignerEditor::HandleCompileCommand()
{
	bIsDirty = false;
	ViewportWidget->GenerateData(true);
}

void FVirtualShapeDesignerEditor::HandleSimplifyCommand()
{
	
}

const ISlateStyle& FVirtualShapeDesignerEditor::GetSlateStyle() const
{
#if ENGINE_MAJOR_VERSION > 4
	return FAppStyle::Get();
#else
	return FEditorStyle::Get();
#endif
}

void FVirtualShapeDesignerEditor::ForceRefresh()
{
	DetailsPanel->Refresh();
}

void FVirtualShapeDesignerEditor::SetIsDirty()
{
	bIsDirty = true;
	VirtualShapeEdited->Modify();
}

#undef LOCTEXT_NAMESPACE
