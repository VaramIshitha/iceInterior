// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once
#include "AssetTypeActions_Base.h"
#include "Classes/VirtualControlEvent.h"

class FVirtualControlEventAssetType : public FAssetTypeActions_Base
{
public:
	FVirtualControlEventAssetType(EAssetTypeCategories::Type InAssetCategory) : AssetCategory(InAssetCategory) {}

	//Begin IAssetTypeAction interface
	virtual FText GetName() const override { return INVTEXT("Virtual Control Event"); }
	virtual UClass* GetSupportedClass() const override { return UVirtualControlEvent::StaticClass(); }
	virtual FColor GetTypeColor() const override { return FColor::Blue; }
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor) override;
	virtual uint32 GetCategories() override { return AssetCategory; }

private:
	EAssetTypeCategories::Type AssetCategory;
};
