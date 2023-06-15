// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/IToolkitHost.h"
#include "Toolkits/AssetEditorToolkit.h"

class UTouchInterfacePreset;
class FTouchInterfacePresetManager;
class IDetailsView;
class SDockableTab;
class UVirtualControlSetup;
class STouchInterfaceDesignerViewport;
class SHierarchyTab;
class SVirtualControlPreviewerTab;
class SVirtualControlDesignerEditorDetailsPanel;
class FTouchDesignerEditor_DetailsTab;

enum class EControlType : uint8;

UENUM()
enum EDetailTabState
{
	DTS_ShowGeneralProperties,
	DTS_ShowBackgroundProperties,
	DTS_ShowControlProperties
};

/**
 * Class that create layout and manage all VirtualControlSetup modifications
 */
class FVirtualControlDesignerEditor : public FAssetEditorToolkit, public FEditorUndoClient/*, FGCObject*/
{
public:
	// Constructor
	FVirtualControlDesignerEditor();

	// Destructor
	virtual ~FVirtualControlDesignerEditor();

	// Begin IToolkit Interface
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FText GetToolkitName() const override;
	virtual FText GetToolkitToolTipText() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override { return FColor::Orange; }
	virtual bool IsAssetEditor() const override { return true; }
	virtual bool ProcessCommandBindings(const FKeyEvent& InKeyEvent) const override;
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	// End IToolkit Interface

	// Begin FEditorUndoClient Interface
	virtual void PostRedo(bool bSuccess) override;
	virtual void PostUndo(bool bSuccess) override;
	// End FEditorUndoClient Interface

	// Begin IAssetEditorInstance Interface
	//virtual bool IsPrimaryEditor() const override;
	// End IAssetEditorInstance Interface

	// Begin FGCObject Interface
	//virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	//virtual FString GetReferencerName() const override;
	// End FGCObject Interface
	
	void InitVirtualControlDesignerEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost> &InitToolkitHost, UVirtualControlSetup* InVirtualControlSetup);

private:
	TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Viewport(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Hierarchy(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_VirtualControlPreviewer(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Palette(const FSpawnTabArgs& Args);

	// Binds commands to Touch Interface Designer Editor's
	void BindCommand();

	// Setup Toolbar button for Touch Designer Editor
	void AddToolbarButton(FToolBarBuilder& Builder);
	
public:
	// Force refresh of DetailTab
	virtual void ForceRefresh();

	// Add new virtual control to virtual control setup and create widget in designer surface
	void AddNewControl(EControlType ControlType, FVector2D CanvasPosition);

	// Add new virtual control using data from copy cache
	void AddNewControlFromCopyCache();
	
	void SelectControl(FName ControlName);

	// Call this function to clear select and hover effect on button and widget
	void UnselectControl();
	
	void SetControlHoverState(FName ControlName, bool InIsHovered, bool SendToWidget, bool SendToButton) const;
	void ChangeControlName(FName ControlName, FName NewControlName) const;
	void RemoveControl(const FName ControlName, const EControlType Type) const;
	
	void CutSelected();
	void CopySelected();
	void PasteCopiedControl();
	void DuplicateSelected();
	void RemoveSelected() const;

	void LinkControl(const FName ParentControl, const FName ChildControl, const FVector2D Offset, const bool ForceRefresh) const;
	void UnlinkControl(const FName ChildControl, const bool ForceRefresh) const;
	void UnlinkAllControl(const FName ParentControl) const;

	// Return true if a virtual control was copied before and data is available
	bool IsCopyAvailable() const { return bCopyAvailable; }

	/** Set control position to virtual control setup.
	 * Warning! Do not call this function each frame.
	 */
	void NotifyControlPositionChanged(const FName ControlName, FVector2D NewPosition) const;
	
	// Propagate opacity change in canvas (not in detail tab because this is automatic)
	void NotifyOpacityChange() const;

	// Propagate Control change in canvas and other
	void NotifyControlChange() const;

	// Notify editor that designer surface orientation has changed
	void NotifyOrientationChanged(const bool InLandscapeMode);


	// GETTERS
	
	UVirtualControlSetup* GetVirtualControlSetup() const { return VirtualControlSetupEdited; }

	TSharedPtr<STouchInterfaceDesignerViewport> GetViewportWidget() { return ViewportWidget; }

	EDetailTabState GetDetailTabState() const { return DetailTabState; }

	TSharedPtr<FUICommandList> GetCommands();

	
	// AUTOMATIONS
	
	void HandleOnCalculatePortraitPosition();
	void HandleOnOpenPresetManager();

	
	// UTILITIES
	
	bool VerifyTextCommitted(const FText& Text, FText& OtherText) const;

	static bool ContainAnySpace(const FText TextToVerify);
	static bool ContainAnySpace(const FString TextToVerify);
	
	bool IsOrientationInLandscape() const { return bLandscapeOrientation; }

	FReply HandleDPISettingsClicked() const;

	FReply HandleTouchInterfaceSettingsClicked() const;

private:
	// All commands for Touch Interface Designer
	void OnCutCommand();
	void OnCopyCommand();
	void OnPasteCommand();
	void OnDuplicateCommand();
	void OnDeleteCommand() const;
	void OnRecenterCanvasCommand() const;
	void OnRenameCommand() const;
	void OnSaveCommand() const;

	
	// Cache data of selected virtual control
	bool CopySelectedControlData();

	// Clear copy operation
	void ClearCopyCache();
	
	
	// Shortcut commands for Touch Interface Designer
	void CreateButtonCommand();
	void CreateJoystickCommand();
	void CreateTouchRegionCommand();
	
	// FCanExecuteAction
	bool CanCreateNewControl() const;
	bool CanExecuteAction() const;
	
	// Toolbar Commands for Touch Interface Designer
	void OpenControlDesigner() const;
	void OnShowGeneralSettingsCommand();
	void OnShowBackgroundSettingsCommand();
	void SaveAsPresetCommand();
	
	
	// PRESET
	
	void HandleOnApplyPreset(const UTouchInterfacePreset* PresetSelected, const bool bAddVirtualControls, const bool bApplySettings);


	//VARIABLES

	// Touch Designer Editor Tabs
	TSharedPtr<STouchInterfaceDesignerViewport> ViewportWidget;
	TSharedPtr<SHierarchyTab> HierarchyTab;
	TSharedPtr<SVirtualControlDesignerEditorDetailsPanel> DetailsPanel;
	TSharedPtr<SVirtualControlPreviewerTab> VirtualControlPreviewerTab;
	
	// The TouchDesignerInterface open within this editor
	UVirtualControlSetup* VirtualControlSetupEdited;

	// Command list for Touch Designer Editor
	TSharedPtr<FUICommandList> TouchDesignerEditorCommands;

	// Instance of preset manager
	TSharedPtr<FTouchInterfacePresetManager> PresetManager;

	// Name of current selected control. NAME_NONE if there is no control selected
	FName SelectedControlName;

	// Is equal to true if any operation copied a control and data is available
	bool bCopyAvailable;

	// True if designer surface is in landscape mode
	uint8 bLandscapeOrientation:1;

	// Current State of DetailTab in Touch Designer Editor
	EDetailTabState DetailTabState;
};
