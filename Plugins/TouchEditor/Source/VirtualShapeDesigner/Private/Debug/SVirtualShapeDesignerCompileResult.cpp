// Copyright Lost in Game Studio. All Rights Reserved.

#include "SVirtualShapeDesignerCompileResult.h"

void SVirtualShapeDesignerCompileResult::Construct(const FArguments& InArgs)
{
	
}

int32 SVirtualShapeDesignerCompileResult::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const FSlateBrush* BackgroundImage = GetSlateStyle().GetBrush(TEXT("Graph.Panel.SolidBackground")); //"Graph.DelegatePin.Connected"
	// Draw Background
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), BackgroundImage);
	
	return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

void SVirtualShapeDesignerCompileResult::UpdateCompileResult()
{

}

const ISlateStyle& SVirtualShapeDesignerCompileResult::GetSlateStyle() const
{
#if ENGINE_MAJOR_VERSION > 4
	return FAppStyle::Get();
#else
	return FEditorStyle::Get();
#endif
}