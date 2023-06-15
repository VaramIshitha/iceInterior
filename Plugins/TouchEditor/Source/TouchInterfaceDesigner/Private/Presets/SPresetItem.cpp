// Copyright Lost in Game Studio. All Rights Reserved.

#include "SPresetItem.h"

#include "SlateOptMacros.h"
#include "TouchInterfacePreset.h"
#include "Editor/TouchInterfaceDesignerStyle.h"
#include "Preview/STouchInterfacePreviewer.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SPresetItem::Construct(const FArguments& InArgs)
{
	AssetData = InArgs._AssetData;
	OnItemClicked = InArgs._OnItemClicked;
	
	ChildSlot
	[
		SNew(SBox)
		.HeightOverride(280.25f)
		.WidthOverride(500.0f)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBorder)
			//.ToolTipText(InArgs._TooltipText)
			//.OnMouseButtonDown(this, &SPresetItem::HandleOnClicked)
			.OnMouseButtonUp(this, &SPresetItem::HandleOnClicked)
			.BorderImage(FTouchInterfaceDesignerStyle::Get().GetBrush("RoundedWhiteBox"))
			.BorderBackgroundColor(this, &SPresetItem::GetBorderColor)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Padding(FMargin(1.0f, 1.0f, 1.0f, 0.0f))
			[
				SNew(SVerticalBox)
				.Visibility(EVisibility::HitTestInvisible)
							
				+SVerticalBox::Slot()
				.VAlign(VAlign_Fill)
				.HAlign(HAlign_Fill)
				.FillHeight(1.0f)
				.Padding(2.0f)
				[
					SAssignNew(Previewer, STouchInterfacePreviewer)
					.PresetAssetData(AssetData)
					.BackgroundColor(FLinearColor(0.2f, 0.2f, 0.2f, 1.0f))
				]

				+SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.AutoHeight()
				.Padding(4.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromName(InArgs._AssetData.AssetName))
					.Justification(ETextJustify::Center)
					.WrapTextAt(200.0f)
				]
			]
		]
	];
}

FString SPresetItem::GetItemNameAsString() const
{
	return AssetData.AssetName.ToString();
}

TArray<FVirtualControl> SPresetItem::GeneratePreviewData(const FAssetData Preset)
{
	const UTouchInterfacePreset* TouchDesignerPreset = Cast<UTouchInterfacePreset>(Preset.GetAsset());
	check(TouchDesignerPreset)
	return TouchDesignerPreset->GetControlsSetting();
}

FReply SPresetItem::HandleOnClicked(const FGeometry& Geometry, const FPointerEvent& PointerEvent)
{
	OnItemClicked.ExecuteIfBound(SharedThis(this), AssetData);
	bIsSelected = true;
	return FReply::Handled();
}

FSlateColor SPresetItem::GetBorderColor() const
{
	
#if ENGINE_MAJOR_VERSION > 4
	const ISlateStyle& Style = FAppStyle::Get();
#else
	const ISlateStyle& Style = FEditorStyle::Get();
#endif

	return bIsSelected ? Style.GetSlateColor("SelectionColor") : IsHovered() ? FSlateColor(FLinearColor(0.1f,0.1f,0.1f,1.0f)) : FSlateColor(FLinearColor(0.04f,0.04f,0.04f,1.0f));
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
