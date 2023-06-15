// Copyright Lost in Game Studio. All Rights Reserved.

#include "TouchInterface/STouchRegion.h"

#include "EnhancedInputSubsystems.h"
#include "SlateOptMacros.h"
#include "Framework/Application/SlateApplication.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void STouchRegion::Construct(const FArguments& InArgs)
{
	ParentWidget = InArgs._TouchInterface;
	VirtualControl = InArgs._VirtualControl;
	CanvasSlot = InArgs._Slot;
	bAutoPositioning = InArgs._AutoPositioning;
	ScaleFactor = InArgs._ScaleFactor;
	DrawDebug = InArgs._DrawDebug;
	bUseInputAction = InArgs._UseInputAction;
	bUseEnhancedInput = InArgs._UseInputAction;
	DeltaThresholdSettings = InArgs._DeltaThresholdSetting;
	ParentOffset = InArgs._ParentOffset;
	
	InitDefaultValues();

	bConsumeDelta = false;
	bSendOneMoveEvent = false;
	Delta = FVector2D::ZeroVector;
	bUseJoystickMode = VirtualControl.bJoystickMode;
}

bool STouchRegion::OnPress(const FGeometry& MyGeometry, const FPointerEvent& Event)
{
	SVirtualControl::OnPress(MyGeometry, Event);
	StartLocation = Event.GetScreenSpacePosition();
	
	return false;
}

void STouchRegion::OnMove(const FGeometry& MyGeometry, const FPointerEvent& Event)
{
	SVirtualControl::OnMove(MyGeometry, Event);

	if (bUseJoystickMode)
	{
		Delta = Event.GetScreenSpacePosition() - StartLocation;
	}
	else
	{
		if (!bConsumeDelta)
		{		
			if (FMath::Abs(Event.GetCursorDelta().X) > DeltaThresholdSettings || FMath::Abs(Event.GetCursorDelta().Y) > DeltaThresholdSettings)
			{
				Delta = Event.GetCursorDelta();
				bConsumeDelta = true;
			}
		}	
	}
}

void STouchRegion::OnRelease(const FGeometry& MyGeometry, const FPointerEvent& Event)
{
	SVirtualControl::OnRelease(MyGeometry, Event);

	Delta = FVector2D::ZeroVector;
	bSendOneMoveEvent = true;
}

void STouchRegion::OnTick(const FGeometry& MyGeometry, const float InScaleFactor, const double InCurrentTime, const float InDeltaTime, const bool InForceUpdate, const bool OrientToLandscape)
{
	SVirtualControl::OnTick(MyGeometry, InScaleFactor, InCurrentTime, InDeltaTime, InForceUpdate, OrientToLandscape);

	if (bUseJoystickMode)
	{
		if (bIsPressed)
		{
			const FVector2D AxisValue = Delta * VirtualControl.InputScale /*** InDeltaTime*/;
			
			if (bUseInputAction)
			{
#if ENGINE_MAJOR_VERSION > 4
				GetEnhancedInputSubsystem()->InjectInputVectorForAction(VirtualControl.JoystickAction, FVector(AxisValue, 0.0f), {}, {});
#endif			
			}
			else
			{
				ApplyAxisInput(VirtualControl.HorizontalInputKey, AxisValue.X);
				ApplyAxisInput(VirtualControl.VerticalInputKey, -AxisValue.Y);
			}
		}

		if (bSendOneMoveEvent)
		{
			bSendOneMoveEvent = false;
		
			if (bUseInputAction)
			{
#if ENGINE_MAJOR_VERSION > 4
				GetEnhancedInputSubsystem()->InjectInputVectorForAction(VirtualControl.JoystickAction, FVector(0.0f), {}, {});
#endif
			}
			else
			{
				ApplyAxisInput(VirtualControl.HorizontalInputKey, 0.0f);
				ApplyAxisInput(VirtualControl.VerticalInputKey, 0.0f);
			}
		}
		
		return;
	}
	
	if (bConsumeDelta)
	{
		bConsumeDelta = false;
		
		const FVector2D AxisValue = Delta * VirtualControl.InputScale * InDeltaTime;

		if (bUseInputAction)
		{
#if ENGINE_MAJOR_VERSION > 4
			GetEnhancedInputSubsystem()->InjectInputVectorForAction(VirtualControl.JoystickAction, FVector(AxisValue, 0.0f), {}, {});
#endif			
		}
		else
		{
			ApplyAxisInput(VirtualControl.HorizontalInputKey, AxisValue.X);
			ApplyAxisInput(VirtualControl.VerticalInputKey, AxisValue.Y);
		}

		Delta = FVector2D::ZeroVector;
	}
	else
	{
		if (bUseInputAction)
		{
#if ENGINE_MAJOR_VERSION > 4
			GetEnhancedInputSubsystem()->InjectInputVectorForAction(VirtualControl.JoystickAction, FVector(0.0f), {}, {});
#endif
		}
		else
		{
			ApplyAxisInput(VirtualControl.HorizontalInputKey, 0.0f);
			ApplyAxisInput(VirtualControl.VerticalInputKey, 0.0f);
		}
	}

	if (bSendOneMoveEvent)
	{
		bSendOneMoveEvent = false;
		
		if (bUseInputAction)
		{
#if ENGINE_MAJOR_VERSION > 4
			GetEnhancedInputSubsystem()->InjectInputVectorForAction(VirtualControl.JoystickAction, FVector(0.0f), {}, {});
#endif
		}
		else
		{
			ApplyAxisInput(VirtualControl.HorizontalInputKey, 0.0f);
			ApplyAxisInput(VirtualControl.VerticalInputKey, 0.0f);
		}
	}
}

int32 STouchRegion::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	if (DrawDebug.Get())
	{
		const FSlateBrush* InteractionZoneBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
		FLinearColor InteractionZoneColor = FLinearColor::Blue;
		InteractionZoneColor.A = DebugOpacity;
		
		FSlateDrawElement::MakeBox(OutDrawElements,LayerId, AllottedGeometry.ToPaintGeometry(), InteractionZoneBrush, ESlateDrawEffect::None, InteractionZoneColor);
		
		TSharedPtr<const FCompositeFont> Font;
		FSlateDrawElement::MakeText(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(CorrectedInteractionSize, FSlateLayoutTransform(FVector2D(0.0f))), VirtualControl.ControlName.ToString(), FCoreStyle::Get().GetFontStyle("NormalFont"));
	}

	return LayerId;
}

FVector2D STouchRegion::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	return CorrectedInteractionSize;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
