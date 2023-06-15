// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AssetTypeActions_Base.h"

class FVirtualShapeAssetType : public FAssetTypeActions_Base
{
public:
	FVirtualShapeAssetType(EAssetTypeCategories::Type InAssetCategory) : AssetCategory(InAssetCategory) {}

	// Begin FAssetTypeActions_Base Interface
	virtual FText GetName() const override { return INVTEXT("Virtual Shape"); }
	virtual FColor GetTypeColor() const override { return FColor::Purple; }
	virtual uint32 GetCategories() override { return AssetCategory; }
	virtual FText GetAssetDescription(const FAssetData& AssetData) const override { return INVTEXT("Contains all data needed by the Shape Manager for recognition"); }
	
	virtual UClass* GetSupportedClass() const override;
	
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor) override;
	// End FAssetTypeActions_Base Interface
	
private:
	EAssetTypeCategories::Type AssetCategory;
};
