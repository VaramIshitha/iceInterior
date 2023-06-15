// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VirtualControlSetup.h"
#include "Widgets/SLeafWidget.h"

class UVirtualControlSetup;

/**
 * 
 */
class TOUCHINTERFACE_API SDesignHelper : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SDesignHelper) :
	_StartOpacity(1.0f),
	_DrawInteractionZone(false)
	{}

	SLATE_ARGUMENT(UVirtualControlSetup*, Setup)
	SLATE_ARGUMENT(float, StartOpacity)
	SLATE_ARGUMENT(bool, DrawInteractionZone)
	SLATE_ARGUMENT(FIntPoint, ScreenSize)
	SLATE_ARGUMENT(float, DPIScale)

	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	//Begin SWidget
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	//End SWidget

private:
	virtual void PaintButton(const FVirtualControl& VirtualControl, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const;
	virtual void PaintJoystick(const FVirtualControl& VirtualControl, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const;
	virtual void PaintTouchRegion(const FVirtualControl& VirtualControl, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const;
	
	void DrawLayer(const FVisualLayer& InLayer, const FVector2D InSize, const FVector2D InBrushSize, const FVector2D InOffset, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const;

public:
	void SetSetupAsset(const UVirtualControlSetup* NewSetup);
	void SetScreenSize(const FIntPoint InScreenSize);
	void SetOpacity(const float InOpacity);
	void SetDpiScale(const float InDPIScale) { DPIScale = InDPIScale; }
	void SetDrawInteractionZone(const bool bEnable) { bDrawInteractionZone = bEnable; }

private:
	float GetScaleFactor(const FGeometry& AllottedGeometry) const;
	
	const UVirtualControlSetup* SetupPreviewed;

	FIntPoint ScreenSize;

	float DPIScale;
	
	float Opacity;
	
	uint8 bDrawInteractionZone:1;
};
