// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SVirtualControl.h"

/**
 * 
 */
class TOUCHINTERFACE_API STouchRegion : public SVirtualControl
{
public:
	SLATE_BEGIN_ARGS(STouchRegion) :
	_TouchInterface(nullptr),
	_Slot(nullptr),
	_AutoPositioning(true),
	_UseInputAction(false),
	_DeltaThresholdSetting(1.0f),
	_ParentOffset(ForceInitToZero)
	{}

	SLATE_ARGUMENT(TSharedPtr<STouchInterface>, TouchInterface)
	SLATE_ARGUMENT(FVirtualControl, VirtualControl)
	SLATE_ARGUMENT(SConstraintCanvas::FSlot*, Slot)
	SLATE_ARGUMENT(bool, AutoPositioning)
	SLATE_ARGUMENT(bool, UseInputAction)
	SLATE_ARGUMENT(float, DeltaThresholdSetting)
	SLATE_ARGUMENT(FVector2D, ParentOffset)
	
	SLATE_ATTRIBUTE(float, ScaleFactor)
	SLATE_ATTRIBUTE(bool, DrawDebug)
	
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	//SVirtualControl
	virtual bool OnPress(const FGeometry& MyGeometry, const FPointerEvent& Event) override;
	virtual void OnMove(const FGeometry& MyGeometry, const FPointerEvent& Event) override;
	virtual void OnRelease(const FGeometry& MyGeometry, const FPointerEvent& Event) override;
	virtual void OnTick(const FGeometry& MyGeometry, const float InScaleFactor, const double InCurrentTime, const float InDeltaTime, const bool InForceUpdate, const bool OrientToLandscape) override;
	//SVirtualControl

	//SWidget
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	//SWidget

private:
	uint8 bConsumeDelta:1;
	uint8 bSendOneMoveEvent:1;
	uint8 bUseInputAction:1;
	uint8 bUseJoystickMode:1;

	float DeltaThresholdSettings;

	FVector2D StartLocation;
	FVector2D CurrentLocation;
	
	FVector2D PreviousLocation;

	FVector2D Delta;
};
