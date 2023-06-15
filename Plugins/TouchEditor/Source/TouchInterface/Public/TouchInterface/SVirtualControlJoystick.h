// Copyright Lost in Game Studio, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SVirtualControl.h"

/**
 * 
 */
class TOUCHINTERFACE_API SVirtualControlJoystick : public SVirtualControl
{
public:
	SLATE_BEGIN_ARGS(SVirtualControlJoystick) :
	_TouchInterface(nullptr),
	_Slot(nullptr),
	_AutoPositioning(true),
	_UseInputAction(false),
	_ParentOffset(ForceInitToZero)
	{}

	SLATE_ARGUMENT(TSharedPtr<STouchInterface>, TouchInterface)
	SLATE_ARGUMENT(FVirtualControl, VirtualControl)
	SLATE_ARGUMENT(SConstraintCanvas::FSlot*, Slot)
	SLATE_ARGUMENT(bool, AutoPositioning)
	SLATE_ARGUMENT(bool, UseInputAction)
	SLATE_ARGUMENT(FVector2D, ParentOffset)
	
	SLATE_ATTRIBUTE(float, Opacity)
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
	
	virtual void CalculateCorrectedValues(const FVector2D& Center, const FVector2D Offset, const FGeometry& AllottedGeometry, const float InScaleFactor) override;
	virtual void Recenter(const FVector2D DesiredPosition) override;
	//SVirtualControl

	//SWidget
	//virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	//SWidget

private:
	void StopAutoMove();
	void StopDragToSprint();
	
	//Send one more joystick update in tick for centering thumbstick
	uint8 bSendOneMoreEvent:1;

	uint8 bDragToSprintEnabled;
	uint8 bDragToSprintEventSent;

	uint8 bAutoMoveEnabled;

	uint8 bHoverSprintButton:1;
	uint8 bDrawSprintButton:1;

	float AutoMoveHoldDuration;

	float LastAngleDelta;

	float DragToSprintTriggerSetting;
	
	FVector2D AutoMoveDirection;
	FVector2D CurrentDragToSprintOffset;	
	
	FVector2D LastNormalizedOffset;
	FVector2D LastNormalizedOffsetScaled;

	FVector2D CorrectedThumbSize;
	FVector2D ThumbPosition;

	FRichCurve JoystickCurve;
};
