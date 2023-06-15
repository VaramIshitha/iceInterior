// Copyright Lost in Game Studio. All Rights Reserved

#pragma once

#include "AssetTypeActions_Base.h"

class UVirtualControlSetup;

class FVirtualControlSetupAssetTypeActions : public FAssetTypeActions_Base
{
public:
	FVirtualControlSetupAssetTypeActions(EAssetTypeCategories::Type InAssetCategory) : AssetCategory(InAssetCategory) {}
	
	// Begin FAssetTypeActions_Base Interface
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual uint32 GetCategories() override { return AssetCategory; }
	virtual FText GetAssetDescription(const FAssetData& AssetData) const override;

	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;

	// Context menu for TouchDesignerInterface Class (See Editor/VirtualTexturingEditor/Private/RuntimeVirtualTextureAssetTypeActions
	virtual bool HasActions(const TArray<UObject*>& InObjects) const override { return false; }
	virtual void GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) override;
	// End FAssetTypeActions_Base Interface
	

	//TODO: implemente check this class to modify asset thumbnail
	/*virtual UThumbnailInfo* GetThumbnailInfo(UObject* Asset) const override;
	virtual TSharedPtr<SWidget> GetThumbnailOverlay(const FAssetData& AssetData) const override;*/

private:
	void Export(TWeakObjectPtr<UVirtualControlSetup> Object);
	
	EAssetTypeCategories::Type AssetCategory;
};
