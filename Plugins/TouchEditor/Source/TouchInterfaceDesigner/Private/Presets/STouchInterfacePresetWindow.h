// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

#include "Preview/STouchInterfacePreviewer.h"

class SPresetItem;
class SWrapBox;

DECLARE_DELEGATE_ThreeParams(FOnApplyPreset, FAssetData, bool, bool)

class STouchInterfacePresetWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STouchInterfacePresetWindow) {}
	SLATE_EVENT(FOnApplyPreset, OnApply)
	SLATE_EVENT(FSimpleDelegate, OnCancel)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TArray<FAssetData> Presets);

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

private:
	EActiveTimerReturnType RefreshWhenReady(double InCurrentTime, float InDeltaTime);
	
	const ISlateStyle& GetSlateStyle() const;

	TSharedRef<SWidget> CreateSettingsUI();

	EVisibility GetSettingsPanelVisibility() const;

	TArray<FVirtualControl> GeneratePreviewData(const FAssetData Preset);

	void GeneratePresetTiles();

	void HandleOnSearchTextChanged(const FText& Text);

	void HandleOnPresetItemClicked(TSharedPtr<SPresetItem> InItem, const FAssetData AssetData);

	FReply HandleOnSettingClicked(const FGeometry& Geometry, const FPointerEvent& Event);

	void HandleOnApplyGeneralSettingsClicked();
	void HandleOnAddVirtualControlsClicked();
	

	TSharedPtr<SWrapBox> WrapBox;
	TSharedPtr<STouchInterfacePreviewer> InterfacePreviewer;

	TArray<FAssetData> PresetInContentBrowser;
	FAssetData PresetSelected;

	TArray<TSharedPtr<SPresetItem>> PresetItems;

	FOnApplyPreset OnApplyPresetDelegate;
	FSimpleDelegate OnCancelDelegate;

	uint8 bSettingsPanelOpened:1;

	bool bApplyGeneralSettings;
	bool bAddVirtualControls;
};
