// Copyright Lost in Game Studio. All Rights Reserved.

#include "VirtualShapeDesignerQuickSettings.h"

#include "VirtualShapeDesignerEditorStyle.h"
#include "Settings/TouchInterfaceSettings.h"
#include "Widgets/Input/SNumericEntryBox.h"

void SVirtualShapeDesignerQuickSettings::Construct(const FArguments& InArgs)
{
	RuntimeSettings = GetMutableDefault<UTouchInterfaceSettings>();
	
	ChildSlot
	[
		SNew(SVerticalBox)
		
		+SVerticalBox::Slot()
		.FillHeight(1.0f)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Top)
		[
			SNew(SHorizontalBox)

			+SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0, 0.0f, 5.0f, 0.0f)
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			[
				SNew(SImage)
				.Image(FVirtualShapeDesignerEditorStyle::Get().GetBrush("VirtualShapeDesigner.CornerThreshold"))
			]

			+SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(100.0f)
				[
					SNew(SSpinBox<float>)
					//.Delta(0.01f)
					//.SliderExponent(0.01f)
					.Value(GetDefault<UTouchInterfaceSettings>()->CornerDetectionThreshold)
					.MinValue(0.1f)
					.MaxValue(1.0f)
					.MinSliderValue(0.1f)
					.MaxSliderValue(1.0f)
					.OnValueCommitted(FOnFloatValueCommitted::CreateSP(this, &SVirtualShapeDesignerQuickSettings::HandleOnSliderValueCommitted))
				]
			]
		]

		

		/*+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(GetSlateStyle().GetMargin(ISlateStyle::Join(FName("ToolBar"),".Label.Padding")))
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Visibility(EVisibility::HitTestInvisible)
			.Text(FText::FromString("Corner Threshold"))
			.TextStyle(GetSlateStyle(), ISlateStyle::Join(FName("ToolBar"),".Label"))	// Smaller font for tool tip labels
			.ShadowOffset(FVector2D::UnitVector)
		]*/
	];
}

const ISlateStyle& SVirtualShapeDesignerQuickSettings::GetSlateStyle() const
{
	
#if ENGINE_MAJOR_VERSION > 4
	return FAppStyle::Get();
#else
	return FEditorStyle::Get();
#endif
	
}

void SVirtualShapeDesignerQuickSettings::HandleOnSliderValueCommitted(float Value, ETextCommit::Type CommitType) const
{
	RuntimeSettings->CornerDetectionThreshold = Value;

#if ENGINE_MAJOR_VERSION > 4
	RuntimeSettings->TryUpdateDefaultConfigFile();
#else
	RuntimeSettings->UpdateDefaultConfigFile();
#endif

}
