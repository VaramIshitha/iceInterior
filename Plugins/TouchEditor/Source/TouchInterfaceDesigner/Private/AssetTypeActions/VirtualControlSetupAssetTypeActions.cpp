// Copyright Lost in Game Studio. All Rights Reserved

#include "VirtualControlSetupAssetTypeActions.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "TouchInterfaceDesignerModule.h"
#include "Classes/VirtualControlSetup.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions_VirtualControlSetup"

FText FVirtualControlSetupAssetTypeActions::GetName() const
{
	return LOCTEXT("AssetName", "Virtual Control Setup");
}

FColor FVirtualControlSetupAssetTypeActions::GetTypeColor() const
{
	//Todo: Make better orange!
	//return FColor::FromHex();
	return FColor::Orange;
}

UClass* FVirtualControlSetupAssetTypeActions::GetSupportedClass() const
{
	return UVirtualControlSetup::StaticClass();
}

/*uint32 FVirtualControlSetupAssetTypeActions::GetCategories()
{
	FVirtualControlDesignerEditorModule* TouchDesignerModule = &FModuleManager::LoadModuleChecked<FVirtualControlDesignerEditorModule>("VirtualControlDesignerEditor");
	return TouchDesignerModule->GetAssetTypeCategory();
}*/

FText FVirtualControlSetupAssetTypeActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT("AssetTypeActions_VirtualControlSetupDescription","Contains all the data needed to display the virtual controls in Touch Interface");
}

void FVirtualControlSetupAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		UVirtualControlSetup* VirtualControlSetup = Cast<UVirtualControlSetup>(*ObjIt);
		if(VirtualControlSetup != nullptr)
		{
			FTouchInterfaceDesignerModule* TouchDesignerModule = &FModuleManager::LoadModuleChecked<FTouchInterfaceDesignerModule>("TouchInterfaceDesigner");
			TouchDesignerModule->CreateVirtualControlDesignerEditor(Mode, EditWithinLevelEditor, VirtualControlSetup);
		}
	}
}

void FVirtualControlSetupAssetTypeActions::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	if (InObjects.Num() == 1)
	{
		TArray<TWeakObjectPtr<UVirtualControlSetup>> VirtualControlSetupArray = GetTypedWeakObjectPtrs<UVirtualControlSetup>(InObjects);

		MenuBuilder.AddMenuEntry
		(
			LOCTEXT("TouchDesignerInterface_Export", "Export..."),
			LOCTEXT("TouchDesignerInterface_ExportTooltip", "Export Touch Designer Interface to Touch Interface format"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &FVirtualControlSetupAssetTypeActions::Export, VirtualControlSetupArray[0]), FCanExecuteAction())
		);
	}
}

void FVirtualControlSetupAssetTypeActions::Export(TWeakObjectPtr<UVirtualControlSetup> Object)
{
	
}

#undef LOCTEXT_NAMESPACE
