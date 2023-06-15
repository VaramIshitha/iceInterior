// Copyright Lost in Game Studio, Inc. All Rights Reserved.

#include "TouchInterface/SVirtualControlJoystick.h"

#include "EnhancedInputSubsystems.h"
#include "SlateOptMacros.h"
#include "Settings/TouchInterfaceSettings.h" //Todo: Remove this include. Pass all settings un Slate Argument
#include "Framework/Application/SlateApplication.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SVirtualControlJoystick::Construct(const FArguments& InArgs)
{
	ParentWidget = InArgs._TouchInterface;
	VirtualControl = InArgs._VirtualControl;
	CanvasSlot = InArgs._Slot;
	CurrentOpacity = InArgs._Opacity;
	bAutoPositioning = InArgs._AutoPositioning;
	ScaleFactor = InArgs._ScaleFactor;
	DrawDebug = InArgs._DrawDebug;
	ParentOffset = InArgs._ParentOffset;
	bUseEnhancedInput = InArgs._UseInputAction;
	
	InitDefaultValues();

	bSendOneMoreEvent = false;
	bDragToSprintEnabled = false;
	bDragToSprintEventSent = false;
	bAutoMoveEnabled = false;
	bHoverSprintButton = false;
	bDrawSprintButton = false;
	AutoMoveHoldDuration = 0.0f;
	LastAngleDelta = 0.0f;

	AutoMoveDirection = FVector2D::ZeroVector;
	CurrentDragToSprintOffset = FVector2D::ZeroVector;
	LastNormalizedOffset = FVector2D::ZeroVector;
	LastNormalizedOffsetScaled = FVector2D::ZeroVector;
	ThumbPosition = FVector2D::ZeroVector;
	
	JoystickCurve = VirtualControl.ThumbValueCurve.EditorCurveData;
	DragToSprintTriggerSetting = GetSettings()->DragToSprintTrigger;
}

bool SVirtualControlJoystick::OnPress(const FGeometry& MyGeometry, const FPointerEvent& Event)
{
	bool bBlockInput = SVirtualControl::OnPress(MyGeometry, Event);

	StopDragToSprint();
	StopAutoMove();

	//Todo: Recenter thumbstick
	
	if (VirtualControl.bSendPressAndReleaseEvent && !bUseEnhancedInput)
	{
		ApplyActionInput(VirtualControl.ButtonInputKey, true);
	}

	return bBlockInput;
}

void SVirtualControlJoystick::OnMove(const FGeometry& MyGeometry, const FPointerEvent& Event)
{
	SVirtualControl::OnMove(MyGeometry, Event);

	const FVector2D LocalCoord = MyGeometry.AbsoluteToLocal(Event.GetScreenSpacePosition());
	
	// figure out position around center
	const FVector2D Offset = LocalCoord - CorrectedCenter;

	if (VirtualControl.bDragToSprint)
	{
		CurrentDragToSprintOffset = Offset;
		
		if (FMath::Abs(Offset.X) <= GetSettings()->DragToSprintThreshold && (-Offset.Y > VirtualControl.VisualSize.Y * 0.5f + 100.0f))
		{
			bDrawSprintButton = true;
			
			if (!bDragToSprintEventSent)
			{
				bDragToSprintEventSent = true;
				//OnDragToSprintChanged.ExecuteIfBound(true);

				if (!bUseEnhancedInput)
				{
					ApplyActionInput(VirtualControl.SprintInputKey, true);
				}
			}

			//Todo: If -Offset.Y >= UserValue, so don't release button because auto sprint
			if (-Offset.Y > GetSettings()->DragToSprintTrigger)
			{
				bHoverSprintButton = true;
			}
			else
			{
				bHoverSprintButton = false;
			}
		}
		else
		{
			bDrawSprintButton = false;
			
			if (bDragToSprintEventSent)
			{
				bHoverSprintButton = false;
				bDragToSprintEventSent = false;
				//OnDragToSprintChanged.ExecuteIfBound(false);

				if (!bUseEnhancedInput)
				{
					ApplyActionInput(VirtualControl.SprintInputKey, false);
				}
			}
		}
	}
	
	// only do work if we aren't at the center
	if (Offset == FVector2D(0, 0))
	{
		ThumbPosition = Offset;
	}
	else
	{
		// clamp to the ellipse of the stick (snaps to the visual size, so, the art should go all the way to the edge of the texture)
		float DistanceToTouchSqr = Offset.SizeSquared();
		float Angle = FMath::Atan2(Offset.Y, Offset.X);

		// length along line to ellipse: L = 1.0 / sqrt(((sin(T)/Rx)^2 + (cos(T)/Ry)^2))
		float CosAngle = FMath::Cos(Angle);
		float SinAngle = FMath::Sin(Angle);
		float XTerm = CosAngle / (CorrectedVisualSize.X * 0.5f);
		float YTerm = SinAngle / (CorrectedVisualSize.Y * 0.5f);
		float DistanceToEdge = FMath::InvSqrt(XTerm * XTerm + YTerm * YTerm);
		
		// only clamp 
		if (DistanceToTouchSqr > FMath::Square(DistanceToEdge))
		{
			ThumbPosition = FVector2D(DistanceToEdge * CosAngle,  DistanceToEdge * SinAngle);
		}
		else
		{
			ThumbPosition = Offset;
		}
	}

	FVector2D AbsoluteThumbPos = ThumbPosition + CorrectedCenter;
	AlignBoxIntoScreen(AbsoluteThumbPos, CorrectedThumbSize, MyGeometry.GetLocalSize());
	ThumbPosition = AbsoluteThumbPos - CorrectedCenter;
}

void SVirtualControlJoystick::OnRelease(const FGeometry& MyGeometry, const FPointerEvent& Event)
{
	SVirtualControl::OnRelease(MyGeometry, Event);
	
	bool bRecenterThumbPosition = true;
	//Todo: Add settings for this ? Let User choose if thumb should be recenter or not.
	//If bAutoMoveEnabled, so do not recenter thumb position
	if (AutoMoveHoldDuration >= GetSettings()->AutoMoveHoldDuration)
	{
		bRecenterThumbPosition = false;
		bAutoMoveEnabled = true;
		AutoMoveDirection = LastNormalizedOffsetScaled;
		//OnAutoMoveBeganEvent.ExecuteIfBound(Control.LastNormalizedOffset);
	}

	bDrawSprintButton = false;
	
	if (bHoverSprintButton)
	{
		bRecenterThumbPosition = false;
		bHoverSprintButton = false;
		bDragToSprintEventSent = false;
		bDragToSprintEnabled = true;
		bAutoMoveEnabled = true;
		AutoMoveDirection = LastNormalizedOffsetScaled;
		//OnAutoMoveBeganEvent.ExecuteIfBound(Control.LastNormalizedOffset);
	}
	else
	{
		bHoverSprintButton = false;
		bDragToSprintEventSent = false;
		//OnDragToSprintChanged.ExecuteIfBound(false);

		if (!bUseEnhancedInput)
		{
			ApplyActionInput(VirtualControl.SprintInputKey, false);
		}
	}

	if (VirtualControl.bSendPressAndReleaseEvent && !bUseEnhancedInput)
	{
		ApplyActionInput(VirtualControl.ButtonInputKey, false);
	}

	if (bRecenterThumbPosition)
	{
		//Release and center the joystick
		ThumbPosition = FVector2D(0.0f);
	}

	if (!bAutoMoveEnabled)
	{
		bDragToSprintEnabled = false;
		bDragToSprintEventSent = false;
		bAutoMoveEnabled = false;
		bHoverSprintButton = false;
		AutoMoveHoldDuration = 0.0f;
		LastAngleDelta = 0.0f;

		AutoMoveDirection = FVector2D::ZeroVector;
		CurrentDragToSprintOffset = FVector2D::ZeroVector;
		LastNormalizedOffset = FVector2D::ZeroVector;
		LastNormalizedOffsetScaled = FVector2D::ZeroVector;

		bSendOneMoreEvent = true;
	}
}

void SVirtualControlJoystick::OnTick(const FGeometry& MyGeometry, const float InScaleFactor, const double InCurrentTime, const float InDeltaTime, const bool InForceUpdate, const bool OrientToLandscape)
{
	SVirtualControl::OnTick(MyGeometry, InScaleFactor, InCurrentTime, InDeltaTime, InForceUpdate, OrientToLandscape);

	if (bIsPressed || bAutoMoveEnabled)
	{
		// Get the corrected thumb offset scale (now allows ellipse instead of assuming square)
		FVector2D ThumbScaledOffset = FVector2D(ThumbPosition.X * 2.0f / CorrectedVisualSize.X, ThumbPosition.Y * 2.0f / CorrectedVisualSize.Y);
		float ThumbSquareSum = ThumbScaledOffset.X * ThumbScaledOffset.X + ThumbScaledOffset.Y * ThumbScaledOffset.Y;
		float ThumbMagnitude = FMath::Sqrt(ThumbSquareSum);
		FVector2D ThumbNormalized = FVector2D(0.f, 0.f);
		if (ThumbSquareSum > SMALL_NUMBER)
		{
			const float Scale = 1.0f / ThumbMagnitude;
			ThumbNormalized = FVector2D(ThumbScaledOffset.X * Scale, ThumbScaledOffset.Y * Scale);
		}

		// Find the scale to apply to ThumbNormalized vector to project onto unit square
		float ToSquareScale = fabs(ThumbNormalized.Y) > fabs(ThumbNormalized.X) ? FMath::Sqrt((ThumbNormalized.X * ThumbNormalized.X) / (ThumbNormalized.Y * ThumbNormalized.Y) + 1.0f)
			: ThumbNormalized.X == 0.0f ? 1.0f : FMath::Sqrt((ThumbNormalized.Y * ThumbNormalized.Y) / (ThumbNormalized.X * ThumbNormalized.X) + 1.0f);

		// Apply proportional offset corrected for projection to unit square
		FVector2D NormalizedOffset = ThumbNormalized * ThumbMagnitude * ToSquareScale;
	
		LastAngleDelta = FVector2D::DotProduct(LastNormalizedOffset.GetSafeNormal(), NormalizedOffset.GetSafeNormal());
	
		FVector2D CurveSensibility;

		CurveSensibility.X = JoystickCurve.Eval(FMath::Abs(NormalizedOffset.X));
		CurveSensibility.Y = JoystickCurve.Eval(FMath::Abs(NormalizedOffset.Y));

		FVector2D Invert;
		Invert.X = NormalizedOffset.X < 0 ? -1 : 1;
		Invert.Y = NormalizedOffset.Y < 0 ? -1 : 1;
	
		const FVector2D NormalizedOffsetScaled = CurveSensibility * VirtualControl.InputScale * Invert;

		LastNormalizedOffset = NormalizedOffset;
		LastNormalizedOffsetScaled = NormalizedOffsetScaled;
		
		if (VirtualControl.bAutoMove && GetSettings()->AutoMoveHoldDuration > 0.0f)
		{
			if (!bAutoMoveEnabled)
			{
				//Todo: Doesn't work correctly -> lastAngleDelta
				if (FMath::Abs(LastNormalizedOffset.Size()) >= GetSettings()->AutoMoveThreshold && LastAngleDelta >= GetSettings()->AutoMoveDirectionThreshold)
				{
					AutoMoveHoldDuration += InDeltaTime;
				}
				else
				{
					AutoMoveHoldDuration = 0.0f;
				}
			}
		}
		
		if (bUseEnhancedInput)
		{
			
#if ENGINE_MAJOR_VERSION > 4
			if (VirtualControl.JoystickAction)
			{
				GetEnhancedInputSubsystem()->InjectInputVectorForAction(VirtualControl.JoystickAction, bAutoMoveEnabled ? FVector(AutoMoveDirection, 0.0f) : FVector(LastNormalizedOffsetScaled, 0.0f), {}, {});
			}

			if (bDragToSprintEventSent || bDragToSprintEnabled)
			{
				GetEnhancedInputSubsystem()->InjectInputForAction(VirtualControl.SprintAction, FInputActionValue(true), {}, {});
			}
#endif
			
		}
		else
		{
			float XValue = bAutoMoveEnabled ? AutoMoveDirection.X : LastNormalizedOffsetScaled.X;
			if (VirtualControl.bUseTurnRate)
			{
				XValue = XValue * (VirtualControl.TurnRate * InDeltaTime);
			}
			ApplyAxisInput(VirtualControl.HorizontalInputKey, XValue);
		
		
			float YValue = bAutoMoveEnabled ? -AutoMoveDirection.Y : -LastNormalizedOffsetScaled.Y;
			if (VirtualControl.bUseTurnRate)
			{
				YValue = YValue * (VirtualControl.TurnRate * InDeltaTime);
			}
			ApplyAxisInput(VirtualControl.VerticalInputKey, YValue);
		}
	}

	if (bSendOneMoreEvent)
	{
		bSendOneMoreEvent = false;
		
		if (bUseEnhancedInput)
		{
			
#if ENGINE_MAJOR_VERSION > 4
			if (VirtualControl.JoystickAction)
			{
				GetEnhancedInputSubsystem()->InjectInputVectorForAction(VirtualControl.JoystickAction, bAutoMoveEnabled ? FVector(AutoMoveDirection, 0.0f) : FVector(LastNormalizedOffsetScaled, 0.0f), {}, {});
			}
#endif
			
		}
		else
		{
			ApplyAxisInput(VirtualControl.HorizontalInputKey, bAutoMoveEnabled ? AutoMoveDirection.X : LastNormalizedOffsetScaled.X);
			ApplyAxisInput(VirtualControl.VerticalInputKey, bAutoMoveEnabled ? -AutoMoveDirection.Y : -LastNormalizedOffsetScaled.Y);
		}
	}
}

void SVirtualControlJoystick::CalculateCorrectedValues(const FVector2D& Center, const FVector2D Offset, const FGeometry& AllottedGeometry, const float InScaleFactor)
{
	SVirtualControl::CalculateCorrectedValues(Center, Offset , AllottedGeometry, InScaleFactor);

	CorrectedThumbSize = VirtualControl.ThumbSize * InScaleFactor;
}

void SVirtualControlJoystick::Recenter(const FVector2D DesiredPosition)
{
	SVirtualControl::Recenter(DesiredPosition);

	ThumbPosition = FVector2D::ZeroVector;
}

int32 SVirtualControlJoystick::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{	
	const TArray<FVisualLayer> Layers = VirtualControl.VisualLayers;
	
	for (auto &Layer : Layers)
	{
		if (bIsPressed)
		{
			if (FVisualLayer::Accept(Layer.LayerType, {ELayerType::Background, ELayerType::Pressed}))
			{
				DrawLayer(Layer,AllottedGeometry.GetLocalSize(),AllottedGeometry.GetLocalSize(),FVector2D::ZeroVector, AllottedGeometry, OutDrawElements, LayerId, InWidgetStyle);
			}

			if (FVisualLayer::Accept(Layer.LayerType, {ELayerType::Thumb, ELayerType::Pressed}))
			{
				DrawLayer(Layer,AllottedGeometry.GetLocalSize(),CorrectedThumbSize,ThumbPosition, AllottedGeometry, OutDrawElements, LayerId, InWidgetStyle);
			}
		}
		else
		{
			if (FVisualLayer::Accept(Layer.LayerType, {ELayerType::Background, ELayerType::Unpressed}))
			{
				DrawLayer(Layer,AllottedGeometry.GetLocalSize(),AllottedGeometry.GetLocalSize(),FVector2D::ZeroVector, AllottedGeometry, OutDrawElements, LayerId, InWidgetStyle);
			}

			if (FVisualLayer::Accept(Layer.LayerType, {ELayerType::Thumb, ELayerType::Unpressed}))
			{
				DrawLayer(Layer,AllottedGeometry.GetLocalSize(),CorrectedThumbSize,ThumbPosition, AllottedGeometry, OutDrawElements, LayerId, InWidgetStyle);
			}
		}
	}

	if (bDrawSprintButton)
	{		
		const TArray<FVisualLayer> SprintButtonLayers = VirtualControl.SprintButton;
		
		const FVector2D SprintButtonOffset = FVector2D(0.0f,-DragToSprintTriggerSetting - VirtualControl.SprintButtonVisualSize.Y * 0.5f);
		
		for(const FVisualLayer& Layer : SprintButtonLayers)
		{
			if (bHoverSprintButton)
			{
				if (FVisualLayer::Accept(Layer.LayerType, {ELayerType::Background, ELayerType::Hovered}))
				{
					DrawLayer(Layer,AllottedGeometry.GetLocalSize(),VirtualControl.SprintButtonVisualSize, SprintButtonOffset, AllottedGeometry, OutDrawElements, LayerId, InWidgetStyle);
				}
			}
			else
			{
				if (FVisualLayer::Accept(Layer.LayerType, {ELayerType::Background, ELayerType::UnHovered}))
				{
					DrawLayer(Layer,AllottedGeometry.GetLocalSize(),VirtualControl.SprintButtonVisualSize, SprintButtonOffset, AllottedGeometry, OutDrawElements, LayerId, InWidgetStyle);
				}
			}
		}
	}

	if (bAutoMoveEnabled || bDragToSprintEnabled)
	{
		const FSlateBrush LockBrush = VirtualControl.LockIcon;
		FLinearColor LockBrushTint = LockBrush.GetTint(InWidgetStyle);
		LockBrushTint.A *= CurrentOpacity.Get();

		FSlateDrawElement::MakeBox
		(
			OutDrawElements,
			LayerId++,
			AllottedGeometry.ToPaintGeometry(CorrectedThumbSize, FSlateLayoutTransform(FVector2D(CorrectedVisualSize.X * 0.5f - CorrectedThumbSize.X * 0.5f, CorrectedVisualSize.Y * 0.5f - CorrectedThumbSize.Y * 0.5f))),
			&LockBrush,
			ESlateDrawEffect::None,
			LockBrushTint
		);
	}
	
	return SVirtualControl::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

FVector2D SVirtualControlJoystick::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	return CorrectedVisualSize;
}

void SVirtualControlJoystick::StopAutoMove()
{
	if (bAutoMoveEnabled)
	{
		bAutoMoveEnabled = false;
		//OnAutoMoveEndedEvent.ExecuteIfBound(Control.LastNormalizedOffset);
		AutoMoveDirection = FVector2D::ZeroVector;
		AutoMoveHoldDuration = 0.0f;
		LastNormalizedOffset = FVector2D::ZeroVector;
		LastNormalizedOffsetScaled = FVector2D::ZeroVector;
	}
}

void SVirtualControlJoystick::StopDragToSprint()
{
	if (bDragToSprintEnabled)
	{
		//if (OnDragToSprintChanged.IsBound()) OnDragToSprintChanged.Execute(false);
		bDragToSprintEnabled = false;
		bDragToSprintEventSent = false;

		if (VirtualControl.SprintInputKey.IsValid() && !bUseEnhancedInput)
		{
			const FGamepadKeyNames::Type KeyName = VirtualControl.SprintInputKey.GetFName();
			ApplyActionInput(VirtualControl.SprintInputKey, false);
		}
	}
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
