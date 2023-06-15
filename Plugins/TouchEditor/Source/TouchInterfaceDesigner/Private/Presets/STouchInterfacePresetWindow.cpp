// Copyright Lost in Game Studio. All Rights Reserved.

#include "STouchInterfacePresetWindow.h"

#include "SPresetItem.h"
#include "Presets/TouchInterfacePreset.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWrapBox.h"

#define LOCTEXT_NAMESPACE "TouchInterfacePresetWindow"

void STouchInterfacePresetWindow::Construct(const FArguments& InArgs, const TArray<FAssetData> Presets)
{
	OnCancelDelegate = InArgs._OnCancel;
	OnApplyPresetDelegate = InArgs._OnApply;

	PresetInContentBrowser = Presets;

	bAddVirtualControls = true;
	bApplyGeneralSettings = true;
	
	ChildSlot
	[
		SNew(SBox)
		.Padding(5)
		[
			SNew(SVerticalBox)

			+SVerticalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(INVTEXT("Select Preset"))
				.Font(GetSlateStyle().GetFontStyle("Persona.RetargetManager.BoldFont"))
			]

			+SVerticalBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Top)
			.Padding(FMargin(5.0f, 10.0f))
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				+SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.HAlign(HAlign_Fill)
				[
					SNew(SSearchBox)
					.HintText(INVTEXT("Search preset by name"))
					.OnTextChanged(this, &STouchInterfacePresetWindow::HandleOnSearchTextChanged)
				]
					
				+SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(FMargin(10.0f, 0.0f, 0.0f, 0.0f))
				[
#if ENGINE_MAJOR_VERSION > 4
				
					SNew(SComboButton)
					.HasDownArrow(false)
					.MenuPlacement(EMenuPlacement::MenuPlacement_ComboBoxRight)
					.ContentPadding(0)
					.ForegroundColor(FSlateColor::UseForeground())
					.ButtonStyle(GetSlateStyle(), "SimpleButton" )
					.MenuContent()
					[
						CreateSettingsUI()
					]
					.ButtonContent()
					[
						SNew(SImage)
						.ColorAndOpacity(FSlateColor::UseForeground())
						.Image(GetSlateStyle().GetBrush("DetailsView.ViewOptions"))
					]
					
#else

					SNew(SComboButton)
					.MenuPlacement(EMenuPlacement::MenuPlacement_ComboBoxRight)
					.ContentPadding(0)
					.ForegroundColor(FSlateColor::UseForeground())
					.ButtonStyle(GetSlateStyle(), "ToggleButton" )
					.MenuContent()
					[
						CreateSettingsUI()
					]
					.ButtonContent()
					[
						SNew(SImage)
						.Image(GetSlateStyle().GetBrush("GenericViewButton"))
					]
					
#endif
					
				]
			]
			
			+SVerticalBox::Slot()
			.FillHeight(1)
			[
				SNew(SBox)
				.Padding(FMargin(10.0f))
				.Visibility(EVisibility::SelfHitTestInvisible)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SScrollBox)
					.Orientation(Orient_Vertical)
					.ScrollBarAlwaysVisible(false)

					+SScrollBox::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Top)
					[
						SAssignNew(WrapBox, SWrapBox)
						.Visibility(EVisibility::SelfHitTestInvisible)
						.Orientation(Orient_Horizontal)
						.InnerSlotPadding(FVector2D(10.0f))
			#if ENGINE_MAJOR_VERSION > 4
						.HAlign(HAlign_Left)
			#endif

						.UseAllottedSize(true)
						.UseAllottedWidth(true)
					]
				]
			]
			
			+SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.ContentPadding(FMargin(2))
					.Text(INVTEXT("Cancel"))
					.OnClicked(FOnClicked::CreateLambda([this]()
					{
						if (OnCancelDelegate.IsBound())
						{
							OnCancelDelegate.Execute();
							return FReply::Handled();
						}
						return FReply::Unhandled();
					}))
				]
				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.ContentPadding(FMargin(2))
					.Text(INVTEXT("Apply"))
					.IsEnabled_Lambda([this]()
					{
						return PresetSelected.IsValid();
					})
					.OnClicked(FOnClicked::CreateLambda([this]()
					{
						if (OnApplyPresetDelegate.IsBound())
						{
							OnApplyPresetDelegate.Execute(PresetSelected, bAddVirtualControls, bApplyGeneralSettings);
							return FReply::Handled();
						}
						return FReply::Unhandled();
					}))
				]
			]
		]
	];

	RegisterActiveTimer(0.0f, FWidgetActiveTimerDelegate::CreateSP(this, &STouchInterfacePresetWindow::RefreshWhenReady));
}

int32 STouchInterfacePresetWindow::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const FSlateBrush* BackgroundImage = GetSlateStyle().GetBrush(TEXT("Graph.Panel.SolidBackground"));
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), BackgroundImage);
	
	return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

FReply STouchInterfacePresetWindow::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (PresetSelected.IsValid())
	{
		PresetSelected = nullptr;
		for (const TSharedPtr<SPresetItem> Item : PresetItems)
		{
			Item->SetIsSelected(false);
		}

		return FReply::Handled();
	}

	return FReply::Unhandled();
}

EActiveTimerReturnType STouchInterfacePresetWindow::RefreshWhenReady(double InCurrentTime, float InDeltaTime)
{
	if (GetTickSpaceGeometry().GetLocalSize() == FVector2D::ZeroVector)
	{
		return EActiveTimerReturnType::Continue;
	}

	GeneratePresetTiles();
	return EActiveTimerReturnType::Stop;
}

const ISlateStyle& STouchInterfacePresetWindow::GetSlateStyle() const
{
#if ENGINE_MAJOR_VERSION > 4
	return FAppStyle::Get();
#else
	return FEditorStyle::Get();
#endif
}

TSharedRef<SWidget> STouchInterfacePresetWindow::CreateSettingsUI()
{
	FMenuBuilder SettingsViewOptions( true, nullptr);
	
	SettingsViewOptions.AddMenuEntry( 
		INVTEXT("Add virtual controls"),
		INVTEXT("Add virtual controls in preset to your touch interface (Warning! Erase already added)"),
		FSlateIcon(),
		FUIAction( 
			FExecuteAction::CreateSP(this, &STouchInterfacePresetWindow::HandleOnAddVirtualControlsClicked),
			FCanExecuteAction(),
			FIsActionChecked::CreateLambda([this](){ return bAddVirtualControls; })
		),
		NAME_None,
		EUserInterfaceActionType::Check 
	);

	SettingsViewOptions.AddMenuEntry(
		INVTEXT("Apply General Settings"),
		INVTEXT("Apply preset general settings to your touch interface"),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateSP(this, &STouchInterfacePresetWindow::HandleOnApplyGeneralSettingsClicked),
			FCanExecuteAction(),
			FIsActionChecked::CreateLambda([this](){ return bApplyGeneralSettings; })
		),
			NAME_None,
			EUserInterfaceActionType::Check
		);
	
	return SettingsViewOptions.MakeWidget();
}

EVisibility STouchInterfacePresetWindow::GetSettingsPanelVisibility() const
{
	return bSettingsPanelOpened ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed;
}

TArray<FVirtualControl> STouchInterfacePresetWindow::GeneratePreviewData(const FAssetData Preset)
{
	const UTouchInterfacePreset* TouchDesignerPreset = Cast<UTouchInterfacePreset>(Preset.GetAsset());

	check(TouchDesignerPreset)
	
	return TouchDesignerPreset->GetControlsSetting();
}

void STouchInterfacePresetWindow::GeneratePresetTiles()
{
	for (const FAssetData& AssetData : PresetInContentBrowser)
	{
		TSharedPtr<SPresetItem> Item;

		WrapBox->AddSlot()
		[
			SAssignNew(Item, SPresetItem)
			.AssetData(AssetData)
			.OnItemClicked(this, &STouchInterfacePresetWindow::HandleOnPresetItemClicked)
		];

		PresetItems.Add(Item);
	}
}

void STouchInterfacePresetWindow::HandleOnSearchTextChanged(const FText& Text)
{
	if (Text.IsEmpty())
	{
		for (const TSharedPtr<SPresetItem> Item : PresetItems)
		{
			Item->SetVisibility(EVisibility::Visible);
		}
		return;
	}

	if (PresetSelected.IsValid())
	{
		PresetSelected = nullptr;
		for (const TSharedPtr<SPresetItem> Item : PresetItems)
		{
			Item->SetIsSelected(false);
		}
	}

	FString StringWithoutWhitespace = Text.ToString();
	StringWithoutWhitespace.RemoveSpacesInline();
	
	for (const TSharedPtr<SPresetItem> Item : PresetItems)
	{
		if (Item->GetItemNameAsString().Contains(StringWithoutWhitespace))
		{
			Item->SetVisibility(EVisibility::Visible);
		}
		else
		{
			Item->SetVisibility(EVisibility::Collapsed);
		}
	}
}

void STouchInterfacePresetWindow::HandleOnPresetItemClicked(TSharedPtr<SPresetItem> InItem, const FAssetData AssetData)
{
	UE_LOG(LogTemp, Log, TEXT("Preset Selected"));
	
	PresetSelected = AssetData;

	for (TSharedPtr<SPresetItem> Item : PresetItems)
	{
		if (Item != InItem)
		{
			Item->SetIsSelected(false);
		}
	}
}

FReply STouchInterfacePresetWindow::HandleOnSettingClicked(const FGeometry& Geometry, const FPointerEvent& Event)
{
	bSettingsPanelOpened = !bSettingsPanelOpened;
	return FReply::Handled();
}

void STouchInterfacePresetWindow::HandleOnApplyGeneralSettingsClicked()
{
	bApplyGeneralSettings = !bApplyGeneralSettings;
}

void STouchInterfacePresetWindow::HandleOnAddVirtualControlsClicked()
{
	bAddVirtualControls = !bAddVirtualControls;
}

#undef LOCTEXT_NAMESPACE