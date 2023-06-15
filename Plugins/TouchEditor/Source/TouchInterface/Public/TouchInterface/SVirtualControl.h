// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

//#include "CoreMinimal.h"
//#include "VirtualControlSetup.h"
#include "Widgets/SLeafWidget.h"
//#include "Widgets/DeclarativeSyntaxSupport.h"
//#include "Widgets/Layout/SConstraintCanvas.h"
#include "STouchInterface.h"
//Begin needed for mobile
#include "UObject/StrongObjectPtr.h"
//End needed for mobile

class SBackgroundBlur;
class UTouchInterfaceSettings;
//class STouchInterface;
class UEnhancedInputLocalPlayerSubsystem;

/**
 * 
 */
class TOUCHINTERFACE_API SVirtualControl : public SLeafWidget //Todo: For parenting feature, use SCompoundWidget, add canvas or other and then use ArrangeChildren
{
	//SLATE_DECLARE_WIDGET(SVirtualControl, SLeafWidget)
	
public:
	SLATE_BEGIN_ARGS(SVirtualControl) {}	
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

protected:
	void InitDefaultValues();

	virtual ~SVirtualControl() override;

	//SBackgroundBlur

public:
	/** return true if control block touch region input */
	virtual bool OnPress(const FGeometry& MyGeometry, const FPointerEvent& Event);
	virtual void OnMove(const FGeometry& MyGeometry, const FPointerEvent& Event);
	virtual void OnRelease(const FGeometry& MyGeometry, const FPointerEvent& Event);
	virtual void OnTick(const FGeometry& MyGeometry, const float InScaleFactor, const double InCurrentTime, const float InDeltaTime, const bool InForceUpdate, const bool OrientToLandscape);

	//Begin SWidget Implementation
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	//virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	virtual bool SupportsKeyboardFocus() const override { return false; }
	//End SWidget Implementation

	void SetCanvasSlot(SConstraintCanvas::FSlot* NewSlot) { CanvasSlot = NewSlot; }

	/** Re-calculate position of control */
	virtual void RefreshPosition() { bRefreshPosition = true; }

	virtual void DrawLayer(const FVisualLayer& InLayer, const FVector2D InSize, const FVector2D InBrushSize, const FVector2D InOffset, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const;

	virtual bool IsInside(const FVector2D FingerPosition);

	EControlType GetControlType() const { return VirtualControl.Type; }

	FName GetControlName() const { return VirtualControl.ControlName; }

	FVirtualControl GetData() const { return VirtualControl; }

	FVirtualControl& GetDataByRef() { return VirtualControl; }

	UMaterialInstanceDynamic* GetLayerDynamicMaterialInstance(const FName LayerName, UObject* InOuter);

	/** Apply changes to the structure (FVirtualControl)  */
	bool ApplyModification();
	
	int32 GetPointerIndex() const { return CapturePointerIndex; }

	UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem();

	void HandleOnOrientationChanged(const bool bOrientToLandscape);

	void Reset();

	void ResetChild(const FVector2D ParentPosition);

	virtual void FlushPressedKey();

	bool AddChild(TSharedPtr<SVirtualControl> Other);

	bool RemoveChild(TSharedPtr<SVirtualControl> ChildToRemove);

	bool IsParent() const { return ChildControls.Num() > 0; }

	const TArray<TSharedPtr<SVirtualControl>>& GetLinkedVirtualControls() { return ChildControls; }
	
protected:
	/** Get Virtual control Designer Settings */
	const UTouchInterfaceSettings* GetSettings();

	/** Clamp control position into screen */
	void AlignBoxIntoScreen(FVector2D& Position, const FVector2D& Size, const FVector2D& ScreenSize);

	/** Set Position value based on RelativeTo (Normalized to Local) */
	void ResolveRelativePosition(FVector2D& Position, const FVector2D RelativeTo);

	/** Adjust all virtual control properties based on geometry and ScaleFactor
	 * @param Center Center of control in screen in relative
	 * @param Offset Offset from CorrectedCenter in absolute
	 * @param AllottedGeometry Touch Interface Geometry
	 * @param InScaleFactor ScaleFactor used by Touch Interface
	 */
	virtual void CalculateCorrectedValues(const FVector2D& Center, const FVector2D Offset, const FGeometry& AllottedGeometry, const float InScaleFactor);

	/** Recenter virtual control position at desired position */
	virtual void Recenter(const FVector2D DesiredPosition);

	/** Called by parent control to refresh position of children */
	virtual void RefreshChild(const FGeometry& MyGeometry, const FVector2D ParentCenter, const float InScaleFactor);
	
	virtual void RecenterChild(const FVector2D ParentPosition);

	/** Set position in canvas. If  */
	void SetLocalPosition(const FVector2D NewPosition, const bool IsNormalized);

	void ApplyActionInput(const FKey& InputKey, const bool bIsOnPressed);

	void ApplyAxisInput(const FKey& InputKey, const float Value);
	

	TSharedPtr<STouchInterface> ParentWidget;
	
	int32 CapturePointerIndex;
	
	FVirtualControl VirtualControl;

	SConstraintCanvas::FSlot* CanvasSlot;

	TAttribute<float> CurrentOpacity;
	TAttribute<float> ScaleFactor;
	TAttribute<bool> DrawDebug;
	
	uint8 bIsPressed:1;

	uint8 bRefreshPosition:1;

	uint8 bAutoPositioning:1;
	uint8 bUseLandscapePosition:1;

	uint8 bUseEnhancedInput:1;

	FVector2D AbsoluteCenter;
	FVector2D CorrectedCenter;
	FVector2D CorrectedVisualSize;
	FVector2D CorrectedInteractionSize;
	FVector2D ParentOffset;
	FVector2D CorrectedOffset;
	
	float CorrectedInteractionRadiusSize;

	float CircleHitMaxLenght;

	float CurrentScaleFactor;
	
	float PreviousScaleFactor;

	/** Time since creation */
	double CurrentTime;

	/** Time since last frame */
	float DeltaTime;
	
	/** Duration from press to release */
	float ElapsedTime;

	float LastElapsedTime;

	float DebugOpacity;
	
	UVirtualControlEvent* VirtualControlEventInstance;

private:
	// Set offset in canvas
	void SetOffset(const FVector2D NewPosition, const FVector2D NewVisualSize) const;

	
	const UTouchInterfaceSettings* Settings;
	
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem;

	float TimeToReset;
	
	uint8 bMustBeReset:1;

	TArray<TSharedPtr<SVirtualControl>> ChildControls;
};
