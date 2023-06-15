// Copyright Lost in Game Studio. All Right Reserved

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SLeafWidget.h"

enum class EControlType : uint8;

/**
 * 
 */
class SPaletteItemDraggedWidget : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SPaletteItemDraggedWidget) :
	_ScaleFactor(1.0f)
	{}
	SLATE_ARGUMENT(EControlType, Type)

	SLATE_ATTRIBUTE(float, ScaleFactor)

	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	
private:
	
	FSlateBrush BackgroundToPaint;
	FSlateBrush ThumbToPaint;

	FVector2D DesiredSize;

	TAttribute<float> ScaleFactor;

	EControlType CurrentType;
};
