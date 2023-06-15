// Copyright Lost in Game Studio. All Rights Reserved.

#include "VirtualShapeDesigner.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "SVirtualShapeDesignerDetailsPanel.h"
#include "TouchInterfaceDesignerSettings.h"
#include "VirtualShapeDesignerEditor.h"
#include "VirtualShapeAssetType.h"
#include "VirtualShapeDesignerCommands.h"
#include "VirtualShapeDesignerEditorStyle.h"

#define LOCTEXT_NAMESPACE "VirtualShapeDesignerModule"

void FVirtualShapeDesignerModule::StartupModule()
{
	if (!GetDefault<UTouchInterfaceDesignerSettings>()->bEnableVirtualShapeEditor)
	{
		return;
	}
	
	FVirtualShapeDesignerEditorStyle::RegisterStyle();

	FVirtualShapeDesignerCommands::Register();
	
	MenuExtensibilityManager = MakeShareable(new FExtensibilityManager);
	ToolbarExtensibilityManager = MakeShareable(new FExtensibilityManager);

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	// Get same asset type category defined in Touch Interface Designer module
	AssetTypeCategory = AssetTools.RegisterAdvancedAssetCategory(FName("TouchInterfaceDesigner"), INVTEXT("Touch Interface Designer"));
	
	// Register Shape asset type
	TSharedRef<IAssetTypeActions> Action = MakeShared<FVirtualShapeAssetType>(AssetTypeCategory);
	AssetTools.RegisterAssetTypeActions(Action);
	RegisteredAssetTypeActions.Add(Action);

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.RegisterCustomClassLayout("VirtualShape", FOnGetDetailCustomizationInstance::CreateStatic(&FVirtualShapeDesignerDetailsPanelCustomization::MakeInstance));
	PropertyEditorModule.NotifyCustomizationModuleChanged();
}

void FVirtualShapeDesignerModule::ShutdownModule()
{
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
	
	FVirtualShapeDesignerEditorStyle::UnregisterStyle();

	FVirtualShapeDesignerCommands::Unregister();
	
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.UnregisterCustomClassLayout("VirtualShape");
	PropertyEditorModule.NotifyCustomizationModuleChanged();

	ShapeDesignerEditorPtr = nullptr;
}

TSharedRef<FVirtualShapeDesignerEditor> FVirtualShapeDesignerModule::CreateShapeDesignerEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UVirtualShape* VirtualShape)
{
	TSharedRef<FVirtualShapeDesignerEditor> NewShapeDesignerEditor(new FVirtualShapeDesignerEditor());
	ShapeDesignerEditorPtr = NewShapeDesignerEditor;
	NewShapeDesignerEditor->InitEditor(Mode, InitToolkitHost, VirtualShape);
	return NewShapeDesignerEditor;
}

const ISlateStyle& FVirtualShapeDesignerModule::GetSlateStyle()
{
#if ENGINE_MAJOR_VERSION > 4
	return FAppStyle::Get();
#else
	return FEditorStyle::Get();
#endif
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FVirtualShapeDesignerModule, VirtualShapeDesigner)