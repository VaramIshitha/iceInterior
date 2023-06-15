// Copyright Lost in Game Studio. All Rights Reserved.

#include "SVirtualShapeDesignerShapeInfo.h"

#include "Widgets/Layout/SScrollBox.h"

void SVirtualShapeDesignerShapeInfo::Construct(const FArguments& InArgs)
{
	VirtualShapeEdited = InArgs._VirtualShapeEdited;
	
	ChildSlot
	[
		SNew(SScrollBox)
		.Orientation(Orient_Vertical)

		+SScrollBox::Slot()
		.Padding(5.0f)
		//.Style()
		[
			ConstructViewportInfo()
		]
	];
}

int32 SVirtualShapeDesignerShapeInfo::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const FSlateBrush* BackgroundImage = GetSlateStyle().GetBrush(TEXT("Graph.Panel.SolidBackground")); //"Graph.DelegatePin.Connected"
	// Draw Background
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), BackgroundImage);
	
	return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

void SVirtualShapeDesignerShapeInfo::UpdateInfo()
{
	
}

TSharedRef<SWidget> SVirtualShapeDesignerShapeInfo::ConstructViewportInfo()
{
	return SNew(SVerticalBox)

	+SVerticalBox::Slot()
	.AutoHeight()
	[
		GenerateTitle(INVTEXT("Virtual Shape Points"))
	]

	// Point data
	+SVerticalBox::Slot()
	.AutoHeight()
	[
		SNew(STextBlock)
		.Text(INVTEXT("2 points"))
	]

	+SVerticalBox::Slot()
	.AutoHeight()
	[
		GenerateTitle(INVTEXT("Virtual Shape Lines"))
	]

	+SVerticalBox::Slot()
	.AutoHeight()
	[
		GenerateTitle(INVTEXT("Virtual Shape Angles"))
	];
}

TSharedRef<SWidget> SVirtualShapeDesignerShapeInfo::GenerateTitle(const FText Title)
{
	return SNew(SHorizontalBox)
	+SHorizontalBox::Slot()
	.AutoWidth()
	[
		SNew(SImage)
	]

	+SHorizontalBox::Slot()
	.FillWidth(1.0f)
	[
		SNew(STextBlock)
		.Text(Title)
		.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		.TextStyle(GetSlateStyle(), "Log.Normal")
	];
}

const ISlateStyle& SVirtualShapeDesignerShapeInfo::GetSlateStyle() const
{
	
#if ENGINE_MAJOR_VERSION > 4
	return FAppStyle::Get();
#else
	return FEditorStyle::Get();
#endif
	
}
