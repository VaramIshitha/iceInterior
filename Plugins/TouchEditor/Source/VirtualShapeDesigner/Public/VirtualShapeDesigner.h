// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeCategories.h"
#include "Modules/ModuleManager.h"
#include "Toolkits/AssetEditorToolkit.h"

class FVirtualShapeDesignerEditor;
class IAssetTypeActions;
class UVirtualShape;

class FVirtualShapeDesignerModule : public IModuleInterface, public IHasMenuExtensibility, public IHasToolBarExtensibility
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    virtual TSharedRef<FVirtualShapeDesignerEditor> CreateShapeDesignerEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost> &InitToolkitHost, UVirtualShape* VirtualShape);

    virtual TSharedPtr<FVirtualShapeDesignerEditor> GetShapeDesignerEditor() { return ShapeDesignerEditorPtr.Pin(); }
	
    /** Gets the extensibility managers for outside entities to extend touch designer interface editor's menu and toolbar */
    virtual TSharedPtr<FExtensibilityManager> GetMenuExtensibilityManager() override { return MenuExtensibilityManager; }

    virtual TSharedPtr<FExtensibilityManager> GetToolBarExtensibilityManager() override { return ToolbarExtensibilityManager; }

    virtual int GetAssetTypeCategory() { return AssetTypeCategory; }

	static const ISlateStyle& GetSlateStyle();

private:
    TSharedPtr<FExtensibilityManager> MenuExtensibilityManager;
    TSharedPtr<FExtensibilityManager> ToolbarExtensibilityManager;
	
    TArray<TSharedPtr<IAssetTypeActions>> RegisteredAssetTypeActions;

    TWeakPtr<FVirtualShapeDesignerEditor> ShapeDesignerEditorPtr;

    EAssetTypeCategories::Type AssetTypeCategory;
};
