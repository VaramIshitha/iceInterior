// Copyright Lost in Game Studio. All Rights Reserved

#include "PaletteItemDragDropOp.h"

#include "Editor/TouchInterfaceDesignerStyle.h"
#include "Widgets/Layout/SSeparator.h"

void FPaletteItemDragDropOp::OnDragged(const FDragDropEvent& DragDropEvent)
{
	if (CursorDecoratorWindow.IsValid())
	{
		FSlateApplicationBase& SlateApplication = FSlateApplicationBase::Get();
						
		FVector2D Position = DragDropEvent.GetScreenSpacePosition() + FSlateApplicationBase::Get().GetCursorSize();

		if (Position != CursorDecoratorWindow->GetPositionInScreen())
		{
			FSlateRect Anchor(Position.X, Position.Y, Position.X, Position.Y);
			Position = SlateApplication.CalculateTooltipWindowPosition(Anchor, CursorDecoratorWindow->GetDesiredSizeDesktopPixels(), false);
		}

		CurrentPosition = Position;
		CursorDecoratorWindow->MoveWindowTo(Position);
	}
}

TSharedPtr<SWidget> FPaletteItemDragDropOp::GetDefaultDecorator() const
{	
	return SNew(SBorder)
#if ENGINE_MAJOR_VERSION > 4
	.BorderImage(FAppStyle::Get().GetBrush("WhiteBrush"))
#else
	.BorderImage(FEditorStyle::Get().GetBrush("WhiteBrush"))
#endif
	
	.BorderBackgroundColor(FLinearColor(0.01f, 0.01f, 0.01f, 1.0f))
	.Padding(0.0f)
	[
		SNew(SHorizontalBox)

		//Icon
		+SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBox)
			.WidthOverride(60.0f)
			.HeightOverride(60.0f)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SOverlay)
				+SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SImage)
					.Image(FTouchInterfaceDesignerStyle::Get().GetBrush(IconStyleName))
				]
				+SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Bottom)
				[
					SNew(SSeparator)
					.Orientation(Orient_Horizontal)
					.Thickness(2.0f)
					.SeparatorImage(FTouchInterfaceDesignerStyle::Get().GetBrush("BlankBrush"))
				]
				
			]
		]

		//Text
		+SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(ItemName)
			.Justification(ETextJustify::Center)
			.Margin(FMargin(10.0f))
		]
	];
}

FVector2D FPaletteItemDragDropOp::GetDecoratorPosition() const
{
	return CurrentPosition;
}

FCursorReply FPaletteItemDragDropOp::OnCursorQuery()
{
	return FCursorReply::Cursor(MouseCursor);
}

void FPaletteItemDragDropOp::SetCursor(EMouseCursor::Type Cursor)
{
	MouseCursor = Cursor;
}

TSharedRef<FPaletteItemDragDropOp> FPaletteItemDragDropOp::New(const TSharedPtr<SWidget>& InDraggedWidget, const EControlType InType, const FName InIconName, const FText InName)
{
	TSharedRef<FPaletteItemDragDropOp> Operation = MakeShareable(new FPaletteItemDragDropOp());
	Operation->ControlType = InType;
	Operation->DraggedWidget = InDraggedWidget;
	Operation->IconStyleName = InIconName;
	Operation->ItemName = InName;
	Operation->MouseCursor = EMouseCursor::GrabHandClosed;
	Operation->Construct();

	return Operation;
}
