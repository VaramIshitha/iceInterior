// Copyright Lost in Game Studio. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Classes/VirtualControlSetup.h"

class FVirtualControlDesignerEditor;

class SVirtualControlPreviewerTab : public SCompoundWidget
{	
	SLATE_BEGIN_ARGS(SVirtualControlPreviewerTab)	{}
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs, TSharedPtr<FVirtualControlDesignerEditor> InTouchDesignerEditor);

private:
	TSharedRef<SWidget> ConstructToolbar();
	TSharedRef<SWidget> GenerateVirtualControl();
	
public:
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime ) override;

	virtual bool SupportsKeyboardFocus() const override {return false;}
	
	virtual void DrawLayer(const FVisualLayer& InLayer, const FVector2D InSize, const FVector2D InBrushSize, const FVector2D InOffset, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const;

	FVector2D GetCanvasSize() const;
	FVector2D GetThumbOffset() const;

	float GetScaleFactor() const;
	float GetOpacity() const;

	bool GetPressedState() const;
	
	/** Set or Update Data that could be previewed */
	void SetPreviewData(FVirtualControl InControlData);

	/** Clear Preview data, so no controls are previewed */
	void ClearPreview();

private:
	TWeakPtr<FVirtualControlDesignerEditor> Editor;

	TSharedPtr<class SConstraintCanvas> Canvas;

	TSharedPtr<class SVirtualControlEditor> PreviewWidget;
	
	FVirtualControl VirtualControlData;
	
	enum EOpacityState
	{
		Active,
		CountingDownToInactive,
		Inactive,
	};

	/** The current state of all controls */
	EOpacityState State;

	/** Target opacity */
	float CurrentOpacity;

	/* Countdown until next state change */
	float Countdown;

	/** Last used scaling value for  */
	float PreviousScalingFactor = 0.0f;

	FVector2D PreviousGeometrySize;
	
	uint32 bEnablePreview:1;

	uint32 bIsPressed:1;

	float PreviewScaleFactor;

	bool bCaptureMouse;

	FVector2D ThumbOffset;

	FVector2D CorrectedCenter;
	FVector2D VisualCenter;
	FVector2D CorrectedVisualSize;
	FVector2D CorrectedInteractionSize;
	//FVector2D CorrectedThumbSize;
};
