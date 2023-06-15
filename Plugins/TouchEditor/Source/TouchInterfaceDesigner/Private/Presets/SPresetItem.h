// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class STouchInterfacePreviewer;
struct FVirtualControl;

DECLARE_DELEGATE_TwoParams(FOnPresetItemSelected, TSharedPtr<class SPresetItem>, FAssetData);

/**
 * 
 */
class SPresetItem : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPresetItem)
		{}
		SLATE_ARGUMENT(FAssetData, AssetData)

		SLATE_EVENT(FOnPresetItemSelected, OnItemClicked)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	FString GetItemNameAsString() const;
	bool IsSelected() const { return bIsSelected; }
	void SetIsSelected(const bool IsSelected) { bIsSelected = IsSelected; }

private:
	TArray<FVirtualControl> GeneratePreviewData(const FAssetData Preset);

	FReply HandleOnClicked(const FGeometry& Geometry, const FPointerEvent& PointerEvent);

	FSlateColor GetBorderColor() const;


	TSharedPtr<STouchInterfacePreviewer> Previewer;

	FAssetData AssetData;

	FOnPresetItemSelected OnItemClicked;

	uint8 bIsSelected:1;
};
