// Copyright Lost in Game Studio. All Rights Reserved.


#include "Helpers/SDesignHelper.h"
#include "SlateOptMacros.h"
#include "Settings/TouchInterfaceSettings.h"
#include "Engine/UserInterfaceSettings.h"
#include "Styling/CoreStyle.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SDesignHelper::Construct(const FArguments& InArgs)
{
	SetVisibility(EVisibility::HitTestInvisible);
	SetEnabled(false);
	
	Opacity = InArgs._StartOpacity;
	ScreenSize = InArgs._ScreenSize;
	DPIScale = InArgs._DPIScale;
	bDrawInteractionZone = InArgs._DrawInteractionZone;

	SetSetupAsset(InArgs._Setup);
}

FVector2D SDesignHelper::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	return ScreenSize;
}

int32 SDesignHelper::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	if (SetupPreviewed)
	{
		//Get array of virtual control
		TArray<FVirtualControl> VirtualControls = SetupPreviewed->VirtualControls;
	
		//For loop iterating over each element of array
		for (const FVirtualControl& VirtualControl : VirtualControls)
		{
			switch (VirtualControl.Type)
			{
			case EControlType::Button:
				PaintButton(VirtualControl, AllottedGeometry, OutDrawElements, LayerId, InWidgetStyle);
				break;
			case EControlType::Joystick:
				PaintJoystick(VirtualControl, AllottedGeometry, OutDrawElements, LayerId, InWidgetStyle);
				break;
			case EControlType::TouchRegion:
				PaintTouchRegion(VirtualControl, AllottedGeometry, OutDrawElements, LayerId, InWidgetStyle);
				break;
			}
		}
	}
	
	return LayerId;
}

void SDesignHelper::PaintButton(const FVirtualControl& VirtualControl, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const
{
	const TArray<FVisualLayer> Layers = VirtualControl.VisualLayers;

	const FVector2D CorrectedVisualSize = VirtualControl.VisualSize * GetScaleFactor(AllottedGeometry);

	FVector2D CorrectedCenter;
	CorrectedCenter.X = VirtualControl.LandscapeCenter.X * AllottedGeometry.GetLocalSize().X - CorrectedVisualSize.X * 0.5f;
	CorrectedCenter.Y = VirtualControl.LandscapeCenter.Y * AllottedGeometry.GetLocalSize().Y - CorrectedVisualSize.Y * 0.5f;
	
	for (auto &Layer : Layers)
	{		
		if (FVisualLayer::Accept(Layer.LayerType, {ELayerType::Background, ELayerType::Unpressed}))
		{
			DrawLayer(Layer, CorrectedVisualSize, CorrectedVisualSize, CorrectedCenter, AllottedGeometry, OutDrawElements, LayerId, InWidgetStyle);
		}
	}

	if (bDrawInteractionZone)
	{
		const FSlateBrush* InteractionZoneBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
		FLinearColor InteractionZoneColor = FLinearColor::Blue;
		InteractionZoneColor.A = 0.5f * Opacity;
		
		const FVector2D InteractionZoneOffset = CorrectedCenter + CorrectedVisualSize * 0.5f - VirtualControl.InteractionSize * 0.5f * GetScaleFactor(AllottedGeometry);

		FSlateDrawElement::MakeBox
		(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(VirtualControl.InteractionSize, FSlateLayoutTransform(GetScaleFactor(AllottedGeometry), InteractionZoneOffset)),
			InteractionZoneBrush,
			ESlateDrawEffect::None,
			InteractionZoneColor
		);
	}
}

void SDesignHelper::PaintJoystick(const FVirtualControl& VirtualControl, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const
{
	const TArray<FVisualLayer> Layers = VirtualControl.VisualLayers;

	const FVector2D CorrectedVisualSize = VirtualControl.VisualSize * GetScaleFactor(AllottedGeometry);
	
	FVector2D CorrectedCenter;
	CorrectedCenter.X = VirtualControl.LandscapeCenter.X * AllottedGeometry.GetLocalSize().X - CorrectedVisualSize.X * 0.5f;
	CorrectedCenter.Y = VirtualControl.LandscapeCenter.Y * AllottedGeometry.GetLocalSize().Y - CorrectedVisualSize.Y * 0.5f;

	const FVector2D CorrectedThumbSize = VirtualControl.ThumbSize * GetScaleFactor(AllottedGeometry);
	
	for (auto &Layer : Layers)
	{
		if (FVisualLayer::Accept(Layer.LayerType, {ELayerType::Background, ELayerType::Unpressed}))
		{
			DrawLayer(Layer,CorrectedVisualSize,CorrectedVisualSize,CorrectedCenter, AllottedGeometry, OutDrawElements, LayerId, InWidgetStyle);
		}

		if (FVisualLayer::Accept(Layer.LayerType, {ELayerType::Thumb, ELayerType::Unpressed}))
		{
			DrawLayer(Layer,CorrectedVisualSize,CorrectedThumbSize,CorrectedCenter, AllottedGeometry, OutDrawElements, LayerId, InWidgetStyle);
		}
	}

	if (bDrawInteractionZone)
	{
		const FSlateBrush* InteractionZoneBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
		FLinearColor InteractionZoneColor = FLinearColor::Blue;
		InteractionZoneColor.A = 0.5f * Opacity;
		
		const FVector2D InteractionZoneOffset = CorrectedCenter + CorrectedVisualSize * 0.5f - VirtualControl.InteractionSize * 0.5f * GetScaleFactor(AllottedGeometry);

		FSlateDrawElement::MakeBox
		(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(VirtualControl.InteractionSize, FSlateLayoutTransform(GetScaleFactor(AllottedGeometry), InteractionZoneOffset)),
			InteractionZoneBrush,
			ESlateDrawEffect::None,
			InteractionZoneColor
		);
	}
}

void SDesignHelper::PaintTouchRegion(const FVirtualControl& VirtualControl, const FGeometry& AllottedGeometry,FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const
{
	const TArray<FVisualLayer> Layers = VirtualControl.VisualLayers;

	const FVector2D CorrectedInteractionSize = VirtualControl.InteractionSize * GetScaleFactor(AllottedGeometry);
	
	FVector2D CorrectedCenter;
	CorrectedCenter.X = VirtualControl.LandscapeCenter.X * AllottedGeometry.GetLocalSize().X - CorrectedInteractionSize.X * 0.5f;
	CorrectedCenter.Y = VirtualControl.LandscapeCenter.Y * AllottedGeometry.GetLocalSize().Y - CorrectedInteractionSize.Y * 0.5f;
	
	
	const FSlateBrush* Brush = FCoreStyle::Get().GetBrush("WhiteBrush");
	FLinearColor BackgroundTint = FLinearColor::Blue;
	BackgroundTint.A = 0.1f * Opacity;
				
	FSlateDrawElement::MakeBox
	(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry
		(
			CorrectedInteractionSize, FSlateLayoutTransform(CorrectedCenter)
		),
		Brush,
		ESlateDrawEffect::None,
		BackgroundTint
	);
}

void SDesignHelper::DrawLayer(const FVisualLayer& InLayer, const FVector2D InSize, const FVector2D InBrushSize, const FVector2D InOffset, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const
{
	const FSlateBrush* Brush = &InLayer.Brush;
	FLinearColor BackgroundTint = Brush->GetTint(InWidgetStyle);
	BackgroundTint.A *= Opacity;

	const FVector2D BrushSize = InLayer.bUseBrushSize ? Brush->ImageSize : InBrushSize;
	const FVector2D BrushOffset = InOffset + (InSize * 0.5 - BrushSize * 0.5 + InLayer.Offset);
				
	FSlateDrawElement::MakeBox
	(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry
		(
			BrushSize, FSlateLayoutTransform(BrushOffset)
		),
		Brush,
		ESlateDrawEffect::None,
		BackgroundTint
	);
}

void SDesignHelper::SetSetupAsset(const UVirtualControlSetup* NewSetup)
{
	SetupPreviewed = NewSetup;
}

void SDesignHelper::SetScreenSize(const FIntPoint InScreenSize)
{
	ScreenSize = InScreenSize;
}

void SDesignHelper::SetOpacity(const float InOpacity)
{
	Opacity = InOpacity;
}

float SDesignHelper::GetScaleFactor(const FGeometry& AllottedGeometry) const
{
	const EScalingMode Mode = GetDefault<UTouchInterfaceSettings>()->ScalingMode;

	switch (Mode)
	{
	case EScalingMode::NONE:
		return 1;
	
	case EScalingMode::DPI:
		//Todo : return 1 if user is using custom size
		// If the user is using a custom size then we disable the DPI scaling logic.
		/*if ( UUserWidget* DefaultWidget = GetDefaultWidget() )
		{
			if ( DefaultWidget->DesignSizeMode == EDesignPreviewSizeMode::Custom || 
				 DefaultWidget->DesignSizeMode == EDesignPreviewSizeMode::Desired )
			{
				return 1.0f;
			}
		}*/
		
		return DPIScale /*GetDefault<UUserInterfaceSettings>(UUserInterfaceSettings::StaticClass())->GetDPIScaleBasedOnSize(ScreenSize)*/;
		
	case EScalingMode::DesignSize:
		const float DesiredWidth = GetDefault<UTouchInterfaceSettings>()->DesignWidth; //1920.0f
		float UndoDPIScaling = 1.0f / DPIScale /*GetDefault<UUserInterfaceSettings>(UUserInterfaceSettings::StaticClass())->GetDPIScaleBasedOnSize(ScreenSize)*/;
		//return ((float)ScreenSize.GetMax() / DesiredWidth) * UndoDPIScaling * GetDefault<UUserInterfaceSettings>(UUserInterfaceSettings::StaticClass())->GetDPIScaleBasedOnSize(ScreenSize);
		return (AllottedGeometry.GetLocalSize().GetMax() / DesiredWidth) * UndoDPIScaling * DPIScale;
	}
	
	// Default
	return 1;

	// float ScaleFactor = 1.0f;
	//
	// switch (Mode)
	// {
	// case EScalingMode::NONE:
	// 	// No Scaling but Unreal Apply DPIScale on all geometry so we undoing this 
	// 	ScaleFactor = 1 / DPIScale/*AllottedGeometry.GetAccumulatedLayoutTransform().GetScale()*/;
	// 	break;
	//
	// case EScalingMode::EpicImplementation:
	// 	{
	// 		const float DesiredWidth = 1024.0f;
	// 		float UndoDPIScaling = 1.0f / DPIScale/*AllottedGeometry.GetAccumulatedLayoutTransform().GetScale()*/;
	// 		ScaleFactor = (AllottedGeometry.GetAbsoluteSize().GetMax() / DesiredWidth) * UndoDPIScaling;
	// 	}
	// 	break;
	//
	// case EScalingMode::DPI:
	// 	// Geometry is already scaled by Unreal so we return 1
	// 	ScaleFactor = 1;
	// 	break;
	// }
	//
	// return ScaleFactor;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
