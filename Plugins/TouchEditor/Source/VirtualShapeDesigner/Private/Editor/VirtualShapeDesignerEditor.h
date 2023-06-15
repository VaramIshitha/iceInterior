// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

#include "VirtualShapeDesignerEditor.h"

class SVirtualShapeDesignerCompileResult;
class FVirtualShapeDesignerCommands;
class SVirtualShapeDesignerShapeInfo;
class SVirtualShapeDesignerDrawer;
class SVirtualShapeDesignerDetailsPanel;
class SVirtualShapeDesignerPalette;
class SVirtualShapeDesignerViewport;
class UVirtualShape;

//Todo: For Drawer and tester mode see WorkflowCentricApplication.cpp line 64, FWidgetBlueprintEditorToolbar and BlueprintEditor.cpp line 1948

class FVirtualShapeDesignerEditor : public FAssetEditorToolkit, public FEditorUndoClient
{
public:
	FVirtualShapeDesignerEditor();

	virtual ~FVirtualShapeDesignerEditor();

	// Begin IToolkit Interface
	virtual FName GetToolkitFName() const override { return FName("ShapeDesignerEditor"); }
	virtual FText GetBaseToolkitName() const override { return INVTEXT("Shape Designer Editor"); }
	virtual FString GetWorldCentricTabPrefix() const override { return FString("Shape Designer Editor"); }
	virtual FLinearColor GetWorldCentricTabColorScale() const override { return FColor::Purple; }
	virtual bool IsAssetEditor() const override { return true; }
	
	virtual FText GetToolkitName() const override;
	virtual FText GetToolkitToolTipText() const override;
	
	virtual bool ProcessCommandBindings(const FKeyEvent& InKeyEvent) const override;
	// End IToolkit Interface
	

	// Begin FEditorUndoClient Interface
	virtual void PostRedo(bool bSuccess) override;
	virtual void PostUndo(bool bSuccess) override;
	// End FEditorUndoClient Interface
	

	void InitEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost> &InitToolkitHost, UVirtualShape* InVirtualShape);
	
	
	// Begin IToolkit Interface
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	// End IToolkit Interface
	
private:
	TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Viewport(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Drawer(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Palette(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_ShapeInfo(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_CompileResult(const FSpawnTabArgs& Args);

	// Binds commands to Shape Designer Editor's
	void BindCommand();

	// Setup Toolbar button for Shape Designer Editor
	void AddToolbarButton(FToolBarBuilder& Builder);

	TSharedRef<SWidget> GetComputationModeMenuContent();

	FText GetCompileButtonText() const;
	FSlateIcon GetCompileButtonIcon() const;

	void HandleCompileCommand();
	void HandleSimplifyCommand();

	const ISlateStyle& GetSlateStyle() const;

public:
	void ForceRefresh();

	UVirtualShape* GetVirtualShape() const { return VirtualShapeEdited; }

	TSharedPtr<FUICommandList> GetCommandList() const { return VirtualShapeDesignerCommands; }

	TSharedPtr<SVirtualShapeDesignerViewport> GetViewport() { return ViewportWidget; }
	TSharedPtr<SVirtualShapeDesignerShapeInfo> GetInfoWidget() { return ShapeInfoWidget; }

	void SetIsDirty();

private:
	//Virtual Shape Designer Tabs
	TSharedPtr<SVirtualShapeDesignerViewport> ViewportWidget;
	TSharedPtr<SVirtualShapeDesignerDrawer> DrawerWidget;
	TSharedPtr<SVirtualShapeDesignerPalette> PaletteWidget;
	TSharedPtr<SVirtualShapeDesignerDetailsPanel> DetailsPanel;
	TSharedPtr<SVirtualShapeDesignerShapeInfo> ShapeInfoWidget;
	TSharedPtr<SVirtualShapeDesignerCompileResult> CompileWidget;
	
	UVirtualShape* VirtualShapeEdited;

	TSharedPtr<FUICommandList> VirtualShapeDesignerCommands;

	uint8 bIsDirty:1;
};