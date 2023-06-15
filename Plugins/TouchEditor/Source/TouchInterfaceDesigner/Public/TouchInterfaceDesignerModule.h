// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IAssetTools.h"
#include "Modules/ModuleManager.h"
#include "Toolkits/AssetEditorToolkit.h"

class IAssetTypeActions;
class IAssetTools;
class UVirtualControlSetup;
class FVirtualControlDesignerEditor;
class FVirtualControlDesignerEditorMenuExtender;

extern const FName VirtualControlDesignerEditorAppIdentifier;

class FTouchInterfaceDesignerModule : public IModuleInterface, public IHasMenuExtensibility, public IHasToolBarExtensibility
{
public:
	// Constructor
	//FVirtualControlDesignerEditorModule();
	
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

	virtual TSharedRef<FVirtualControlDesignerEditor> CreateVirtualControlDesignerEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost> &InitToolkitHost, UVirtualControlSetup* TouchDesignerInterface);

	virtual TSharedPtr<FVirtualControlDesignerEditor> GetVirtualControlDesignerEditor() { return TouchDesignerEditorPtr.Pin(); }
	
	/** Gets the extensibility managers for outside entities to extend touch designer interface editor's menu and toolbar */
	virtual TSharedPtr<FExtensibilityManager> GetMenuExtensibilityManager() override { return MenuExtensibilityManager; }

	virtual TSharedPtr<FExtensibilityManager> GetToolBarExtensibilityManager() override { return ToolbarExtensibilityManager; }

	virtual int GetAssetTypeCategory() { return AssetTypeCategory; }

private:
	template<typename T>
	void RegisterAssetTypeAction(IAssetTools& AssetTools);

	void CheckInputSettings();
	void HandleFixSettings();
	void HandleRejectSettingsModification();
	
	TSharedPtr<FExtensibilityManager> MenuExtensibilityManager;
	TSharedPtr<FExtensibilityManager> ToolbarExtensibilityManager;
	
	TArray<TSharedPtr<IAssetTypeActions>> RegisteredAssetTypeActions;

	TWeakPtr<FVirtualControlDesignerEditor> TouchDesignerEditorPtr;

	TSharedPtr<FVirtualControlDesignerEditorMenuExtender> MenuExtender;

	EAssetTypeCategories::Type AssetTypeCategory;
};

IMPLEMENT_MODULE(FTouchInterfaceDesignerModule, TouchInterfaceDesigner)
