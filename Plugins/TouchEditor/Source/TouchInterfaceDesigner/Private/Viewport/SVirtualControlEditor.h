// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VirtualControlSetup.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/SLeafWidget.h"

class FVirtualControlDesignerEditor;
class STouchInterfaceDesignerViewport;

DECLARE_DELEGATE_OneParam(FOnVirtualControlEffect, FName);

class SVirtualControlEditor : public SLeafWidget
{	
	SLATE_BEGIN_ARGS(SVirtualControlEditor) :
	_Slot(nullptr),
	_HoverEffect(true),
	_LandscapeOrientation(true),
	_ScaleFactor(1.0f),
	_Opacity(1.0f),
	_Constrained(false),
	_ShowDashedOutline(true),
	_ShowInteractiveZone(false),
	_ShowParentLine(false),
	_ShowText(false),
	_ShowPressedState(false),
	_ThumbOffset(ForceInitToZero)
	{}
	SLATE_ARGUMENT(FVirtualControl, VirtualControlData)
	SLATE_ARGUMENT(SConstraintCanvas::FSlot*, Slot)
	SLATE_ARGUMENT(FVector2D, StartPosition)
	SLATE_ARGUMENT(bool, HoverEffect)

	SLATE_ATTRIBUTE(bool, LandscapeOrientation)
	SLATE_ATTRIBUTE(FVector2D, CanvasSize)
	SLATE_ATTRIBUTE(float, ScaleFactor)
	SLATE_ATTRIBUTE(float, Opacity)
	SLATE_ATTRIBUTE(bool, Constrained)
	SLATE_ATTRIBUTE(bool, ShowDashedOutline)
	SLATE_ATTRIBUTE(bool, ShowInteractiveZone)
	SLATE_ATTRIBUTE(bool, ShowParentLine)
	SLATE_ATTRIBUTE(bool, ShowText)
	SLATE_ATTRIBUTE(bool, ShowPressedState)
	SLATE_ATTRIBUTE(FVector2D, ThumbOffset)

	SLATE_EVENT(FOnVirtualControlEffect, Hovered)
	SLATE_EVENT(FOnVirtualControlEffect, Unhovered)
		
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs, TSharedPtr<FVirtualControlDesignerEditor> InDesignerEditorPtr);

	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	virtual int32 OnPaint( const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled ) const override;
	virtual bool SupportsKeyboardFocus() const override { return false; }
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;

	virtual void DrawLayer(const FVisualLayer& InLayer, const FVector2D InSize, const FVector2D InBrushSize, const FVector2D InOffset, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const;

	void SetCanvasSlot(SConstraintCanvas::FSlot* NewSlot) { WidgetSlot = NewSlot; }
	
	/** Update data of virtual control */
	void Update(const FVirtualControl ControlData, const bool ForceRefresh);

	/** Refresh virtual control with existing data */
	void Refresh();

	bool PositionIsInside(const FVector2D InPosition);
	
	void SetIsMoving(const bool InEnable) { bIsMoving = InEnable; }

	/** Set position in designer surface (canvas space) */
	FVector2D SetPositionInCanvas(FVector2D InPosition, const bool IsNormalized, const bool IncludeChild);

	FVector2D GetPositionInCanvas() const;

	void SetOffsetFromParent(const FVector2D OffsetFromParent, const bool ForceRefresh);

	void SetIsSelected(bool InIsSelected) {bIsSelected = InIsSelected;}
	void SetIsHovered(bool InIsHovered) {bIsHovered = InIsHovered;}
	
	FName GetControlName() const { return VirtualControlData.ControlName; }
	void SetControlName(const FName NewName);

	bool AddChild(const FName ChildName, TSharedPtr<SVirtualControlEditor>);
	bool RemoveChild(const FName ChildName);
	bool IsParent() const { return Children.Num() > 0; }

	void SetIsChild(const bool IsChild);
	bool IsChild() const { return VirtualControlData.bIsChild; }

	void MoveChild(FVector2D ParentAbsolutePosition);

	void SetParentName(const FName NewParentName);
	void RenameChild(const FName ChildName, const FName NewChildName);

	TArray<FName> GetChildName() const;

private:
	bool AcceptLayer(int32 LayerType, TArray<ELayerType> TestLayer) const;

	FVector2D CalculateThumbOffset(const FGeometry Geometry) const;

	const ISlateStyle& GetSlateStyle() const;

	void SetOffset(const FVector2D NewPosition, const FVector2D NewVisualSize) const;
	
	TWeakPtr<FVirtualControlDesignerEditor> DesignerEditorPtr;

	FVirtualControl VirtualControlData;

	TMap<FName, TSharedPtr<SVirtualControlEditor>> Children;

	TAttribute<FVector2D> CanvasSize;
	TAttribute<FVector2D> ThumbOffset;
	
	TAttribute<float> ScaleFactor;
	TAttribute<float> CurrentOpacity;

	TAttribute<bool> bLandscapeOrientation;
	TAttribute<bool> bShowDashedOutline;
	TAttribute<bool> bShowInteractionOutline;
	TAttribute<bool> bShowText;
	TAttribute<bool> bShowPressState;
	TAttribute<bool> bShowParentLine;
	TAttribute<bool> bIsConstrained;

	FOnVirtualControlEffect OnHovered;
	FOnVirtualControlEffect OnUnhovered;
	
	FVector2D VisualSize;
	FVector2D ThumbSize;
	FVector2D InteractionSize;
	FVector2D NormalizedPosition;
	FVector2D Position;
	FVector2D ParentPosition;
	FVector2D PortraitPosition;
	FVector2D ParentOffset;

	FVector2D CorrectedVisualSize;
	//FVector2D CorrectedInteractionSize;
	FVector2D CorrectedThumbOffset;
	
	uint8 bIsSelected:1;
	uint8 bIsHovered:1;
	uint8 bEffectEnabled:1;
	uint8 bIsParent:1;
	uint8 bIsMoving:1;
	
	float TargetOpacity;

	SConstraintCanvas::FSlot* WidgetSlot;	
};
