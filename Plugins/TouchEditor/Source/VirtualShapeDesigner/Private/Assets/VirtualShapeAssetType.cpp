// Fill out your copyright notice in the Description page of Project Settings.


#include "VirtualShapeAssetType.h"

#include "VirtualShapeDesigner.h"
#include "Classes/VirtualShape.h"


UClass* FVirtualShapeAssetType::GetSupportedClass() const
{
	return UVirtualShape::StaticClass();
}

void FVirtualShapeAssetType::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	//FAssetTypeActions_Base::OpenAssetEditor(InObjects, EditWithinLevelEditor);
	
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		UVirtualShape* VirtualShape = Cast<UVirtualShape>(*ObjIt);
		if (VirtualShape != nullptr)
		{
			FVirtualShapeDesignerModule* ShapeDesignerModule = &FModuleManager::LoadModuleChecked<FVirtualShapeDesignerModule>("VirtualShapeDesigner");
			ShapeDesignerModule->CreateShapeDesignerEditor(Mode, EditWithinLevelEditor, VirtualShape);
		}
	}
}
