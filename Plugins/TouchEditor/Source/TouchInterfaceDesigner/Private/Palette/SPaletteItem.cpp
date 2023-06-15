// Copyright Lost in Game Studio. All Right Reserved


#include "SPaletteItem.h"

#include "DragAndDrop/PaletteItemDragDropOp.h"
#include "SlateOptMacros.h"
#include "DragAndDrop/SPaletteItemDraggedWidget.h"
#include "Editor/TouchInterfaceDesignerStyle.h"
#include "VirtualControlSetup.h"
#include "Widgets/Layout/SSeparator.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SPaletteItem::Construct(const FArguments& InArgs, TSharedPtr<SPaletteTab> PaletteTab)
{
	OnClicked = InArgs._OnClicked;
	Type_Internal = InArgs._Type;

	ItemName = InArgs._ItemName;
	ImageName = InArgs._ImageName;
	
	ChildSlot
	[
		SNew(SBox)
		.HeightOverride(100.0f)
		.WidthOverride(75.0f)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBorder)
			.ToolTipText(InArgs._TooltipText)
			.OnMouseButtonDown(this, &SPaletteItem::HandleItemClicked)
			.BorderImage(FTouchInterfaceDesignerStyle::Get().GetBrush("RoundedWhiteBox"))
			.BorderBackgroundColor(this, &SPaletteItem::GetBorderColor)
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
				.Padding(0.0f)
				[
					SNew(SBorder)
					.BorderImage(FTouchInterfaceDesignerStyle::Get().GetBrush("TopRoundedWhiteBox"))
					.BorderBackgroundColor(FLinearColor(0.01f,0.01f,0.01f,1.0f))
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.Padding(0.0f)
					[
						//Icon
						SNew(SBox)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.HeightOverride(40.0f)
						.WidthOverride(40.0f)
						[
							SNew(SImage)
							.Image(FTouchInterfaceDesignerStyle::Get().GetBrush(InArgs._ImageName))
						]
					]
				]

				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SSeparator)
					.Orientation(Orient_Horizontal)
					.SeparatorImage(FTouchInterfaceDesignerStyle::Get().GetBrush("BlankBrush"))
					.Thickness(2.0f)
					//.ColorAndOpacity(FColor::White)
				]

				+SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.AutoHeight()
				.Padding(4.0f)
				[
					SNew(STextBlock)
					.Text(InArgs._ItemName)
					.Justification(ETextJustify::Center)
					.WrapTextAt(50.0f)
				]
			]
		]
	];
}

FReply SPaletteItem::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const TSharedPtr<SPaletteItemDraggedWidget> DraggedWidget = SNew(SPaletteItemDraggedWidget).Type(Type_Internal);
	
	return FReply::Handled().BeginDragDrop(FPaletteItemDragDropOp::New(DraggedWidget.ToSharedRef(), Type_Internal, ImageName, ItemName));
}

FCursorReply SPaletteItem::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const
{
	if (IsHovered())
	{
		return FCursorReply::Cursor(EMouseCursor::GrabHand);
	}
	return FCursorReply::Cursor(EMouseCursor::Default);
}

FReply SPaletteItem::HandleItemClicked(const FGeometry& Geometry, const FPointerEvent& Event)
{
	if (OnClicked.IsBound())
	{
		OnClicked.Execute();
	}
	
	return FReply::Handled().DetectDrag(SharedThis(this), EKeys::LeftMouseButton);
}

const FSlateBrush* SPaletteItem::GetBorderImage() const
{
#if ENGINE_MAJOR_VERSION > 4
	const ISlateStyle& SlateStyle = FAppStyle::Get();
#else
	const ISlateStyle& SlateStyle = FEditorStyle::Get();
#endif
	
	return IsHovered() ? SlateStyle.GetBrush("DetailView.CategoryTop_Hovered") : SlateStyle.GetBrush("DetailView.CategoryTop");
}

FSlateColor SPaletteItem::GetBorderColor() const
{
	return IsHovered() ? FSlateColor(FLinearColor(0.1f,0.1f,0.1f,1.0f)) : FSlateColor(FLinearColor(0.04f,0.04f,0.04f,1.0f));
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
