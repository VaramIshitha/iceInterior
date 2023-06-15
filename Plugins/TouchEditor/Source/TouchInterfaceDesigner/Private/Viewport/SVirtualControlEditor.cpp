// Copyright Lost in Game Studio. All Rights Reserved.

#include "SVirtualControlEditor.h"

#include "Rendering/DrawElements.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Editor/VirtualControlDesignerEditor.h"
#include "TouchInterfaceDesignerSettings.h"

DEFINE_LOG_CATEGORY_STATIC(LogVirtualControlEditor, All, All);

const float OPACITY_LERP_RATE = 3.f;

void SVirtualControlEditor::Construct(const FArguments& InArgs, TSharedPtr<FVirtualControlDesignerEditor> InDesignerEditorPtr)
{
	//WidgetSlot = InArgs._Slot;

	if (InDesignerEditorPtr) DesignerEditorPtr = InDesignerEditorPtr;
	
	bIsConstrained = InArgs._Constrained;
	VirtualControlData = InArgs._VirtualControlData;
	bLandscapeOrientation = InArgs._LandscapeOrientation;
	ScaleFactor = InArgs._ScaleFactor;
	CurrentOpacity = InArgs._Opacity;
	bShowText = InArgs._ShowText;
	bShowDashedOutline = InArgs._ShowDashedOutline;
	bShowInteractionOutline = InArgs._ShowInteractiveZone;
	bShowPressState = InArgs._ShowPressedState;
	bShowParentLine = InArgs._ShowParentLine;
	CanvasSize = InArgs._CanvasSize;
	ThumbOffset = InArgs._ThumbOffset;
	bEffectEnabled = InArgs._HoverEffect;
	OnHovered = InArgs._Hovered;
	OnUnhovered = InArgs._Unhovered;
	NormalizedPosition = InArgs._StartPosition;

	bIsSelected = false;
}

FVector2D SVirtualControlEditor::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	if (VirtualControlData.Type == EControlType::TouchRegion)
	{
		return VirtualControlData.InteractionSize * ScaleFactor.Get();
	}
	
	return VirtualControlData.VisualSize * ScaleFactor.Get();
}

void SVirtualControlEditor::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SLeafWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	CorrectedThumbOffset = CalculateThumbOffset(AllottedGeometry);
}

int32 SVirtualControlEditor::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	float DPIScale = ScaleFactor.Get();
	
	const FName ControlName = VirtualControlData.ControlName;

	//Todo: Check why OnPaint is called twice by frame
	//UE_LOG(LogVirtualControlEditor, Log, TEXT("Name = %s | ScaleFactor = %f"), *ControlName.ToString(), DPIScale);
	
	const TArray<FVisualLayer> Layers = VirtualControlData.VisualLayers;

	switch (VirtualControlData.Type)
	{
	case EControlType::Button:
		{
			for (auto &Layer : Layers)
			{
				if (bShowPressState.Get())
				{
					if (FVisualLayer::Accept(Layer.LayerType, {ELayerType::Background, ELayerType::Pressed}))
					{
						DrawLayer(Layer, AllottedGeometry.GetLocalSize(), AllottedGeometry.GetLocalSize(), FVector2D::ZeroVector, AllottedGeometry, OutDrawElements, LayerId, InWidgetStyle);
					}
				}
				else
				{
					if (FVisualLayer::Accept(Layer.LayerType, {ELayerType::Background, ELayerType::Unpressed}))
					{
						DrawLayer(Layer, AllottedGeometry.GetLocalSize(), AllottedGeometry.GetLocalSize(), FVector2D::ZeroVector, AllottedGeometry, OutDrawElements, LayerId, InWidgetStyle);
					}
				}
			}
		}
		break;
	case EControlType::Joystick:
		{
			FVector2D CorrectedThumbSize = VirtualControlData.ThumbSize * DPIScale;

			for (auto &Layer : Layers)
			{
				if (bShowPressState.Get())
				{
					if (FVisualLayer::Accept(Layer.LayerType, {ELayerType::Background, ELayerType::Pressed}))
					{
						DrawLayer(Layer,AllottedGeometry.GetLocalSize(),AllottedGeometry.GetLocalSize(),FVector2D::ZeroVector, AllottedGeometry, OutDrawElements, LayerId, InWidgetStyle);
					}

					if (FVisualLayer::Accept(Layer.LayerType, {ELayerType::Thumb, ELayerType::Pressed}))
					{
						DrawLayer(Layer,AllottedGeometry.GetLocalSize(),CorrectedThumbSize,CorrectedThumbOffset, AllottedGeometry, OutDrawElements, LayerId, InWidgetStyle);
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
						DrawLayer(Layer,AllottedGeometry.GetLocalSize(),CorrectedThumbSize,CorrectedThumbOffset, AllottedGeometry, OutDrawElements, LayerId, InWidgetStyle);
					}
				}
			}
		}
		break;
	case EControlType::TouchRegion:
		{
			const FSlateBrush* Brush = GetSlateStyle().GetBrush("FocusRectangle");
			FLinearColor BackgroundTint = Brush->GetTint(InWidgetStyle);
			//BackgroundTint.A *= CurrentOpacity;
						
			FSlateDrawElement::MakeBox
			(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry
				(
					AllottedGeometry.GetLocalSize(), FSlateLayoutTransform()
				),
				Brush,
				ESlateDrawEffect::None,
				BackgroundTint
			);
		}		
		break;
	}
	
	if (bShowInteractionOutline.Get())
	{
		FLinearColor InteractionColor = GetDefault<UTouchInterfaceDesignerSettings>()->InteractionVisualizationColor;
		
		if (VirtualControlData.Type == EControlType::TouchRegion)
		{
			// Interaction Size
			FSlateDrawElement::MakeBox
			(
				OutDrawElements,
				LayerId+2,
				AllottedGeometry.ToPaintGeometry
				(
					VirtualControlData.InteractionSize, FSlateLayoutTransform(DPIScale,FVector2D::ZeroVector)
				),
				GetSlateStyle().GetBrush("DashedBorder"),
				ESlateDrawEffect::None,
				InteractionColor);
		}
		else
		{
			switch (VirtualControlData.InteractionShape)
			{
			case EHitTestType::Square:
				// Square Interaction Size
				FSlateDrawElement::MakeBox
				(
					OutDrawElements,
					LayerId+2,
					AllottedGeometry.ToPaintGeometry
					(
						VirtualControlData.InteractionSize, FSlateLayoutTransform(DPIScale,(VirtualControlData.VisualSize - VirtualControlData.InteractionSize) * 0.5f * DPIScale)
					),
					GetSlateStyle().GetBrush("DashedBorder"),
					ESlateDrawEffect::None,
					InteractionColor
				);
				break;

			case EHitTestType::Circle:
					FVector2D StartPoint;
					StartPoint.X = VirtualControlData.VisualSize.X * 0.5f * DPIScale;
					StartPoint.Y = (VirtualControlData.VisualSize.Y * 0.5 - VirtualControlData.InteractionRadiusSize) * DPIScale;

					FVector2D StartDirection;
					StartDirection.X = VirtualControlData.InteractionRadiusSize * 4.0f * DPIScale;
					StartDirection.Y = 0.0f;

					FVector2D EndPoint;
					EndPoint.X = VirtualControlData.VisualSize.X * 0.5f * DPIScale;
					EndPoint.Y = (VirtualControlData.VisualSize.Y * 0.5 + VirtualControlData.InteractionRadiusSize) * DPIScale;

					FVector2D EndDirection;
					EndDirection.X = -StartDirection.X;
					EndDirection.Y = 0.0f;
					
					FSlateDrawElement::MakeSpline
					(
						OutDrawElements,
						LayerId+2,
						AllottedGeometry.ToPaintGeometry(),
						StartPoint,
						StartDirection,
						EndPoint,
						EndDirection,
						1.0f * DPIScale,
						ESlateDrawEffect::None,
						InteractionColor
					);

					FSlateDrawElement::MakeSpline
					(
						OutDrawElements,
						LayerId+2,
						AllottedGeometry.ToPaintGeometry(),
						EndPoint,
						EndDirection,
						StartPoint,
						StartDirection,
						1.0f * DPIScale,
						ESlateDrawEffect::None,
						InteractionColor
					);
				
					break;
			}
	
			//const FLinearColor* BorderColor = bIsSelected ? &FLinearColor::Green : &FLinearColor::White;
		}
	}
	
	if (bShowDashedOutline.Get())
	{
		// Dashed border
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId+3, AllottedGeometry.ToPaintGeometry(), GetSlateStyle().GetBrush("DashedBorder")/*, ESlateDrawEffect::None, *BorderColor*/);
	}

	//Virtual Control Name
	if (bShowText.Get())
	{
		FSlateDrawElement::MakeText
		(
			OutDrawElements,
			LayerId+4,
			AllottedGeometry.ToPaintGeometry
			(
				FSlateLayoutTransform(DPIScale, FVector2D(0.0f, AllottedGeometry.GetLocalSize().Y + 10.0f))
			),
			VirtualControlData.ControlName.ToString(),
			GetSlateStyle().GetFontStyle("NormalText.Important")
		);
	}

	//Show position on canvas when moved
	if (bIsMoving)
	{
		//Todo: use this bellow for 0-1 position on canvas ?
		//AllottedGeometry.GetLocalPositionAtCoordinates()
	
		FVector2D InPosition = GetParentWidget()->GetTickSpaceGeometry().AbsoluteToLocal(GetTickSpaceGeometry().GetAbsolutePosition()) + (VirtualControlData.VisualSize * 0.5f * DPIScale);
		float CorrectedPositionX = FMath::RoundToFloat(InPosition.X);
		float CorrectedPositionY = FMath::RoundToFloat(InPosition.Y);
		
		FString TextPosition = FVector2D(CorrectedPositionX, CorrectedPositionY).ToString();
		//float TextSize = TextPosition.Len() * 6.0f;
		FSlateDrawElement::MakeText
		(
			OutDrawElements,
			LayerId+4,
			AllottedGeometry.ToPaintGeometry
			(
				//Todo: Use DrawSize and remove scale from slateLayoutTransform
				VirtualControlData.VisualSize,
				FSlateLayoutTransform(DPIScale,FVector2D(0.0f, -50.0f))
			),
			TextPosition,
			GetSlateStyle().GetFontStyle("NormalText.Important")
		);
	}

	//Draw Hover effect
	if (bIsHovered && !bIsSelected)
	{
		const FVector2D OutlinePixelSize = FVector2D(2.0f) / FVector2D(AllottedGeometry.GetAccumulatedRenderTransform().GetMatrix().GetScale().GetVector());
		FPaintGeometry SelectionGeometry = AllottedGeometry.ToInflatedPaintGeometry(OutlinePixelSize);
		FSlateClippingZone SelectionZone(SelectionGeometry);
		
		TArray<FVector2D> Points;
		Points.Add(FVector2D(SelectionZone.TopLeft));
		Points.Add(FVector2D(SelectionZone.TopRight));
		Points.Add(FVector2D(SelectionZone.BottomRight));
		Points.Add(FVector2D(SelectionZone.BottomLeft));
		Points.Add(FVector2D(SelectionZone.TopLeft));
	
		const FLinearColor HoverColor(0,0.5,1,1);
	
		FSlateDrawElement::MakeLines(OutDrawElements,LayerId,FPaintGeometry(),Points, ESlateDrawEffect::None,HoverColor, false, 2.0f);
	}
	//Draw Selected effect
	else if (bIsSelected)
	{
		//Todo: Use AllottedGeometry.GetLocalSize() instead and make FSlateRect based on this
		const FVector2D OutlinePixelSize = FVector2D(2.0f) / FVector2D(AllottedGeometry.GetAccumulatedRenderTransform().GetMatrix().GetScale().GetVector());
		FPaintGeometry SelectionGeometry = AllottedGeometry.ToInflatedPaintGeometry(OutlinePixelSize);
		FSlateClippingZone SelectionZone(SelectionGeometry);

		//FSlateRect Rect = FSlateRect()
		
		TArray<FVector2D> Points;
		Points.Add(FVector2D(SelectionZone.TopLeft));
		Points.Add(FVector2D(SelectionZone.TopRight));
		Points.Add(FVector2D(SelectionZone.BottomRight));
		Points.Add(FVector2D(SelectionZone.BottomLeft));
		Points.Add(FVector2D(SelectionZone.TopLeft));
	
		FSlateDrawElement::MakeLines(OutDrawElements,LayerId,FPaintGeometry(),Points, ESlateDrawEffect::None,FLinearColor::Green, false, 2.0f);
	}
	
	return LayerId;
}

void SVirtualControlEditor::DrawLayer(const FVisualLayer& InLayer, const FVector2D InSize, const FVector2D InBrushSize, const FVector2D InOffset, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const
{	
	const FSlateBrush* Brush = &InLayer.Brush;
	FLinearColor BackgroundTint = Brush->GetTint(InWidgetStyle);
	BackgroundTint.A *= CurrentOpacity.Get();

	const FVector2D BrushSize = InLayer.bUseBrushSize ? Brush->ImageSize * ScaleFactor.Get() : InBrushSize;
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

void SVirtualControlEditor::Update(const FVirtualControl ControlData, const bool ForceRefresh)
{
	VirtualControlData = ControlData;

	if (ForceRefresh)
	{
		Refresh();
	}
}

void SVirtualControlEditor::Refresh()
{	
	if (Children.Num() > 0)
	{
		TArray<TSharedPtr<SVirtualControlEditor>> VirtualControlEditors;
		Children.GenerateValueArray(VirtualControlEditors);
		for (const TSharedPtr<SVirtualControlEditor> VirtualControlEditor : VirtualControlEditors)
		{
			VirtualControlEditor->SetParentName(VirtualControlData.ControlName);
		}
	}
	
	SetPositionInCanvas(bLandscapeOrientation.Get() ? VirtualControlData.LandscapeCenter : VirtualControlData.PortraitCenter, true, true);
	
	//Todo: Refresh Thumb Position
}

bool SVirtualControlEditor::PositionIsInside(const FVector2D InPosition)
{
	const FVector2D CurrentPosition = /*Position*/ /*WidgetSlot->GetOffset().GetTopLeft()*/ GetTickSpaceGeometry().GetAbsolutePosition() + VirtualControlData.VisualSize * ScaleFactor.Get() * 0.5f;
	const FVector2D CorrectedInteractionSize = VirtualControlData.InteractionSize * ScaleFactor.Get();

	return
	InPosition.X >= CurrentPosition.X - CorrectedInteractionSize.X * 0.5f &&
	InPosition.X <= CurrentPosition.X + CorrectedInteractionSize.X * 0.5f &&
	InPosition.Y >= CurrentPosition.Y - CorrectedInteractionSize.Y * 0.5f &&
	InPosition.Y <= CurrentPosition.Y + CorrectedInteractionSize.Y * 0.5f;
}

FVector2D SVirtualControlEditor::SetPositionInCanvas(FVector2D InPosition, const bool IsNormalized, const bool IncludeChild)
{
	const FVector2D DesignerSize = CanvasSize.Get();
	const float DPIScale = ScaleFactor.Get();

	if (IsNormalized)
	{
		Position.X = InPosition.X * DesignerSize.X;
		Position.Y = InPosition.Y * DesignerSize.Y;
	}
	else
	{
		Position = InPosition;
	}
	
	if (bIsConstrained.Get())
	{
		Position.X = FMath::Clamp(Position.X, 0.0f, DesignerSize.X);
		Position.Y = FMath::Clamp(Position.Y, 0.0f, DesignerSize.Y);
	}
	
	SetOffset(Position, VisualSize * DPIScale);
	//WidgetSlot->SetOffset(FMargin(Position.X,Position.Y, VisualSize.X * DPIScale, VisualSize.Y * DPIScale));
	
	if (IncludeChild)
	{
		if (Children.Num() > 0)
		{
			TArray<TSharedPtr<SVirtualControlEditor>> VirtualControlEditors;
			Children.GenerateValueArray(VirtualControlEditors);
			for (const TSharedPtr<SVirtualControlEditor> VirtualControlEditor : VirtualControlEditors)
			{
				VirtualControlEditor->MoveChild(Position);
			}
		}
	}

	return Position;
}

FVector2D SVirtualControlEditor::GetPositionInCanvas() const
{
	return Position;
}

void SVirtualControlEditor::SetOffsetFromParent(const FVector2D OffsetFromParent, const bool ForceRefresh)
{
	VirtualControlData.ParentOffset = OffsetFromParent;

	if (ForceRefresh)
	{
		// Todo: Use ScaleFactor or Based on designer size like 1920*1080, scale will be 1.0f.
		const FVector2D CanvasPosition = ParentPosition + OffsetFromParent * ScaleFactor.Get();
		const FVector2D NewVisualSize = VirtualControlData.VisualSize * ScaleFactor.Get();
		
		SetOffset(CanvasPosition, NewVisualSize);
		//WidgetSlot->SetOffset(FMargin(CanvasPosition.X, CanvasPosition.Y, NewVisualSize.X, NewVisualSize.Y));
	}
}

void SVirtualControlEditor::SetControlName(const FName NewName)
{
	VirtualControlData.ControlName = NewName;
	
	if (Children.Num() > 0)
	{
		TArray<TSharedPtr<SVirtualControlEditor>> VirtualControlEditors;
		Children.GenerateValueArray(VirtualControlEditors);
		for (const TSharedPtr<SVirtualControlEditor> VirtualControlEditor : VirtualControlEditors)
		{
			VirtualControlEditor->SetParentName(NewName);
		}
	}
}

bool SVirtualControlEditor::AddChild(const FName ChildName, TSharedPtr<SVirtualControlEditor> ChildControl)
{
	Children.Add(ChildName, ChildControl);
	//Todo: Show parent icon
	return true;
}

bool SVirtualControlEditor::RemoveChild(const FName ChildName)
{
	Children.FindAndRemoveChecked(ChildName);
	return true;

	/*if (Children.Num() <= 0)
	{
		//Todo: Hide Parent icon and link
	}*/
	
	/*if (Children.Contains(ChildName))
	{
		Children.Remove(ChildName);
		if (Children.Num() <= 0)
		{
			//Todo: Hide Parent icon and link
		}
		return true;
	}
	return false;*/
}

void SVirtualControlEditor::SetIsChild(const bool IsChild)
{
	VirtualControlData.bIsChild = IsChild;
}

void SVirtualControlEditor::MoveChild(FVector2D ParentAbsolutePosition)
{
	const float CurrentScaleFactor = ScaleFactor.Get();
	ParentPosition = ParentAbsolutePosition;
	Position = ParentAbsolutePosition + VirtualControlData.ParentOffset * CurrentScaleFactor;
	SetOffset(Position, VisualSize * CurrentScaleFactor);
	//WidgetSlot->SetOffset(FMargin(Position.X,Position.Y, VisualSize.X * CurrentScaleFactor, VisualSize.Y * CurrentScaleFactor));
}

void SVirtualControlEditor::SetParentName(const FName NewParentName)
{
	VirtualControlData.ParentName = NewParentName;
}

void SVirtualControlEditor::RenameChild(const FName ChildName, const FName NewChildName)
{
	check(Children.Contains(ChildName))

	const TSharedPtr<SVirtualControlEditor> Child = Children.FindAndRemoveChecked(ChildName);
	Children.Add(NewChildName, Child);
}

TArray<FName> SVirtualControlEditor::GetChildName() const
{
	TArray<FName> ChildrenName;
	Children.GenerateKeyArray(ChildrenName);
	return ChildrenName;
}

bool SVirtualControlEditor::AcceptLayer(int32 LayerType, TArray<ELayerType> TestLayer) const
{	
	if (LayerType != (int32)ELayerType::None)
	{
		for (ELayerType Type : TestLayer)
		{
			if (LayerType & (int32)Type)
			{
				
			}
			else
			{
				return false;
			}
		}
		return true;
	}
	return false;
}

FVector2D SVirtualControlEditor::CalculateThumbOffset(const FGeometry Geometry) const
{
	FVector2D Offset = ThumbOffset.Get();
	
	// only do work if we aren't at the center
	if (Offset != FVector2D(0.5f))
	{
		// clamp to the ellipse of the stick (snaps to the visual size, so, the art should go all the way to the edge of the texture)
		float DistanceToTouchSqr = Offset.SizeSquared();
		float Angle = FMath::Atan2(Offset.Y, Offset.X);

		// length along line to ellipse: L = 1.0 / sqrt(((sin(T)/Rx)^2 + (cos(T)/Ry)^2))
		float CosAngle = FMath::Cos(Angle);
		float SinAngle = FMath::Sin(Angle);
		float XTerm = CosAngle / (Geometry.GetLocalSize().X * 0.5f);
		float YTerm = SinAngle / (Geometry.GetLocalSize().Y * 0.5f);
		float DistanceToEdge = FMath::InvSqrt(XTerm * XTerm + YTerm * YTerm);
		
		// only clamp 
		if (DistanceToTouchSqr > FMath::Square(DistanceToEdge))
		{
			Offset = FVector2D(DistanceToEdge * CosAngle,  DistanceToEdge * SinAngle);
		}
	}

	return Offset;

	//FVector2D AbsoluteThumbPos = ThumbPosition + CorrectedCenter;
	//AlignBoxIntoScreen(AbsoluteThumbPos, CorrectedThumbSize, MyGeometry.GetLocalSize());
	//ThumbPosition = AbsoluteThumbPos - CorrectedCenter;
}

const ISlateStyle& SVirtualControlEditor::GetSlateStyle() const
{
#if ENGINE_MAJOR_VERSION > 4
	return FAppStyle::Get();
#else
	return FEditorStyle::Get();
#endif
}

void SVirtualControlEditor::SetOffset(const FVector2D NewPosition, const FVector2D NewVisualSize) const
{
#if ENGINE_MAJOR_VERSION > 4
	WidgetSlot->SetOffset(FMargin(NewPosition.X, NewPosition.Y, NewVisualSize.X, NewVisualSize.Y));
#else
	WidgetSlot->Offset(FMargin(NewPosition.X,NewPosition.Y, NewVisualSize.X, NewVisualSize.Y));	
#endif
}

void SVirtualControlEditor::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (bEffectEnabled)
	{
		bIsHovered = true;
		if (OnHovered.IsBound()) OnHovered.Execute(VirtualControlData.ControlName);
		
		DesignerEditorPtr.Pin()->SetControlHoverState(VirtualControlData.ControlName, true, false, true);
	}

	SLeafWidget::OnMouseEnter(MyGeometry, MouseEvent);
}

void SVirtualControlEditor::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	if (bEffectEnabled)
	{
		bIsHovered = false;
		if (OnUnhovered.IsBound()) OnUnhovered.Execute(VirtualControlData.ControlName);
		
		DesignerEditorPtr.Pin()->SetControlHoverState(VirtualControlData.ControlName, false, false, true);
	}
	
	SLeafWidget::OnMouseLeave(MouseEvent);
}
