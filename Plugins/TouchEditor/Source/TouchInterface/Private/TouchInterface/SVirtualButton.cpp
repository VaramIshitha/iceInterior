// Copyright Lost in Game Studio. All Rights Reserved.

#include "TouchInterface/SVirtualButton.h"

#include "EnhancedInputSubsystems.h"
#include "SlateOptMacros.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/Application/SlateApplication.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SVirtualButton::Construct(const FArguments& InArgs)
{
	ParentWidget = InArgs._TouchInterface;
	VirtualControl = InArgs._VirtualControl;
	CanvasSlot = InArgs._Slot;
	CurrentOpacity = InArgs._Opacity;
	bAutoPositioning = InArgs._AutoPositioning;
	ScaleFactor = InArgs._ScaleFactor;
	DrawDebug = InArgs._DrawDebug;
	bUseEnhancedInput = InArgs._UseInputAction;
	ParentOffset = InArgs._ParentOffset;
	
	InitDefaultValues();
}

bool SVirtualButton::OnPress(const FGeometry& MyGeometry, const FPointerEvent& Event)
{
	bool bBlockInput = SVirtualControl::OnPress(MyGeometry, Event);
	
	if (VirtualControl.PressedSound)
	{
		UGameplayStatics::PlaySound2D(ParentWidget->GetWorldContext(), VirtualControl.PressedSound);
	}
	
	if (!bUseEnhancedInput)
	{
		ApplyActionInput(VirtualControl.ButtonInputKey, true);
	}

	return bBlockInput;
}

void SVirtualButton::OnMove(const FGeometry& MyGeometry, const FPointerEvent& Event)
{
	SVirtualControl::OnMove(MyGeometry, Event);
}

void SVirtualButton::OnRelease(const FGeometry& MyGeometry, const FPointerEvent& Event)
{
	SVirtualControl::OnRelease(MyGeometry, Event);

	if (!bUseEnhancedInput)
	{
		ApplyActionInput(VirtualControl.ButtonInputKey, false);
	}
}

void SVirtualButton::OnTick(const FGeometry& MyGeometry, const float InScaleFactor, const double InCurrentTime, const float InDeltaTime, const bool InForceUpdate, const bool OrientToLandscape)
{
	SVirtualControl::OnTick(MyGeometry, InScaleFactor, InCurrentTime, InDeltaTime, InForceUpdate, OrientToLandscape);
	
	if (bUseEnhancedInput)
	{
		if (VirtualControl.ButtonAction && bIsPressed)
		{
#if ENGINE_MAJOR_VERSION > 4
			GetEnhancedInputSubsystem()->InjectInputForAction(VirtualControl.ButtonAction, FInputActionValue(true), {}, {});
#endif
		}
	}
}

int32 SVirtualButton::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const TArray<FVisualLayer> Layers = VirtualControl.VisualLayers;
	
	for (auto &Layer : Layers)
	{
		if (bIsPressed)
		{
			if (FVisualLayer::Accept(Layer.LayerType, {ELayerType::Background, ELayerType::Pressed}))
			{
				DrawLayer(Layer, AllottedGeometry.GetLocalSize(), AllottedGeometry.GetLocalSize(),FVector2D::ZeroVector, AllottedGeometry, OutDrawElements, LayerId, InWidgetStyle);
			}
		}
		else
		{
			if (FVisualLayer::Accept(Layer.LayerType, {ELayerType::Background, ELayerType::Unpressed}))
			{
				DrawLayer(Layer, AllottedGeometry.GetLocalSize(), AllottedGeometry.GetLocalSize(),FVector2D::ZeroVector, AllottedGeometry, OutDrawElements, LayerId, InWidgetStyle);
			}
		}
	}

	return SVirtualControl::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
