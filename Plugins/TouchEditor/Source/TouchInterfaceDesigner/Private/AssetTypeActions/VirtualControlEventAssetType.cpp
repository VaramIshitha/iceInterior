// Copyright Lost in Game Studio. All Rights Reserved.

#include "VirtualControlEventAssetType.h"

void FVirtualControlEventAssetType::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	FAssetTypeActions_Base::OpenAssetEditor(InObjects, EditWithinLevelEditor);
}
