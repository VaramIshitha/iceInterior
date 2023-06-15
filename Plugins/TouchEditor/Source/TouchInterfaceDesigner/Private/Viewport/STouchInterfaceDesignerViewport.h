// Copyright Lost in Game Studio. All Right Reserved

#pragma once

#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Layout/SConstraintCanvas.h"

class STouchInterfaceDesignerRuler;
class FVirtualControlDesignerEditor;
class UVirtualControlSetup;
class STouchDesignerEditor_ControlCanvas;
class SVirtualControlEditor;

struct FVirtualControl;

struct FControlWidget
{
	TSharedPtr<SVirtualControlEditor> Widget;
	SConstraintCanvas::FSlot* Slot;

	FControlWidget()
	{
		Slot = nullptr;
	}

	FControlWidget(TSharedPtr<SVirtualControlEditor> InWidget, SConstraintCanvas::FSlot* InSlot) :
	Widget(InWidget),
	Slot(InSlot)
	{
		
	}
};

class STouchInterfaceDesignerViewport : public SCompoundWidget
{
	class STouchDesignerEditorDetailsPanel;
	
	SLATE_BEGIN_ARGS(STouchInterfaceDesignerViewport)
		//:_ShowGraphStateOverlay(true)
		//,_IsEditable(true)
		{}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedPtr<FVirtualControlDesignerEditor> InTouchDesignerEditor);

private:
	// Recenter designer surface after creation. Need this because geometry is not ready immediately
	EActiveTimerReturnType AutoRecenter(double InCurrentTime, float InDeltaTime);

public:
	// Recenter Designer Surface
	void RecenterDesigner();

private:
	// Add multiple new control to Designer Surface
	void GenerateVirtualControlWidgets(TArray<FVirtualControl> VirtualControls);

public:
	// Add new virtual control to designer surface.
	TSharedPtr<SVirtualControlEditor> AddVirtualControlInCanvasSpace(const FVirtualControl& ControlAdded);
	
	/** Add new virtual control to designer surface at desired position.
	 * @param ControlAdded Struct of virtual control to add
	 * @param Position Position of virtual control in canvas space
	 */
	TSharedPtr<SVirtualControlEditor> AddVirtualControlInCanvasSpace(const FVirtualControl& ControlAdded, const FVector2D Position);

	/** Remove Control from Designer Surface based on Control Name */
	void RemoveControl(const FName ControlName);

	/** Remove all control from Designer Surface and regenerate virtual control */
	void Refresh();
	
	//Begin SWidget Implementation
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	//End SWidget Implementation

private:
	/** Paint Background Lines of viewport (Very similar to Editor/UMGEditor/Private/Designer/SDesignSurface) */
	void PaintViewportBackground(const FSlateBrush* BackgroundImage, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32& DrawLayerId) const;

	/** Paint device Mockup */
	void PaintDeviceMockup(const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId) const;

	/** Paint Designer Surface Background */
	void PaintDesignerBackground(const FGeometry& DesignerGeometry, const FSlateRect& DesignerCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const;
	
	/** Paint Selection and/or Hover Outline effect */
	void DrawSelectionAndHoverOutline(const FGeometry& DesignerSurfaceGeometry, const FSlateRect& DesignerSurfaceRect, FSlateWindowElementList& OutDrawElements, int32 LayerId) const;

	/** Paint linked virtual control */
	void PaintLinkedVirtualControl(const FGeometry& DesignerSurfaceGeometry, const FSlateRect& DesignerSurfaceRect, FSlateWindowElementList& OutDrawElements, int32 LayerId) const;

public:
	//Begin SWidget Implementation
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual void OnDragLeave(const FDragDropEvent& DragDropEvent) override;
	virtual FReply OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	
	virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override;
	//End SWidget Implementation

	
	/** WIDGET FUNCTIONS */

	/** Set manually Widget that is selected */
	void SelectWidget(const FName ControlName);

	/** Set Manually Widget that is Hovered by ControlName */
	void HoverWidget(const FName ControlName, bool Enable);

	/** Clear Outline effect on all Widgets in Designer Surface */
	void ClearOutlineEffect() const;

	/** Link Child Control to other control */
	void Link(const FName ParentControl, const FName ChildControl, const FVector2D OffsetFromParent) const;

	/** Unlink Child control from other control */
	void Unlink(const FName ParentControl, const FName ChildControl) const;

	/** Update all data of selected control */
	void UpdateSelectedControl(const FVirtualControl ControlData) const;

	/** Update Opacity Data on All Controls that exist in Designer Surface */
	void ChangeOpacity(const float Active, const float Inactive);

	/** Update Name of Selected Control */
	void ChangeControlName(const FName ControlName, const FName NewName, const FVirtualControl& UpdatedData);

	bool IsDragControl() const { return bDragControl; }

	bool CanCreateNewControl() const { return !bDragControl; }

	FVector2D GetVirtualControlPosition(const FName ControlName, const bool InNormalized) const;
	
	//void UpdateSelectedWidgetPosition(const FTouchDesignerControl ControlData);

	
	/** UTILITIES */

	/** Transform canvas space (Designer Surface) to normalized position (0-1) */
	FVector2D CanvasSpaceToNormalized(const FVector2D CanvasSpacePosition) const;

	/** Transform normalized position (0-1) to canvas space (Designer Surface) position */
	FVector2D NormalizedToCanvasSpace(const FVector2D NormalizedPosition) const;

	/** Transform absolute (screen space position) to canvas space */
	FVector2D AbsoluteToCanvasSpace(const FVector2D ScreenSpacePosition) const;

	/** Transform absolute (screen space position) to normalized position */
	FVector2D AbsoluteToNormalize(const FVector2D ScreenSpacePosition) const;
	
	/** Return size of Designer Surface */
	FVector2D GetDesignerSize() const { return FVector2D(DesignerSize); }

	/** Return ScaleFactor based on user chosen scaling mode */
	float GetScaleFactor() const { return CurrentScaleFactor; }

	/** Return current opacity value */
	float GetOpacityStateValue() const { return CurrentOpacity; }

	bool IsInLandscapeOrientation() { return bIsInLandscapeMode; }

	FVector2D CalculateOffsetBetweenControl(const FName Parent, const FName Child) const;
	
private:
	/** EDITOR STATE */
	EVisibility GetDesignerOutlineVisibility() const;
	EVisibility GetDesignerVisibility() const;
	
	/** TOP TOOLBAR FEATURES */

	void BindCommands();
	
	/** Construct MockupScreenSize ComboBox Menu */
	TSharedRef<SWidget> OnGetScreenSizeMenuContent();

	void CreateScreenSizeSubMenu(FMenuBuilder& MenuBuilder, FName Category);
	void OnScreenSizeButtonClicked(const FString ProfileName);

	TSharedRef<SWidget> CreateCustomScreenSizeButtons();
	void OnCustomScreenSizeButtonClicked();
	void SetCustomWidth(const int32 Value);
	void SetCustomHeight(const int32 Value);
	void UpdateCustomScreenSize(const int32 Width, const int32 Height);

	/** Get Brush based on Designer Orientation */
	const FSlateBrush* GetOrientationModeBrush() const;

	TSharedRef<SWidget> MakeViewportToolbar() const;

	void ToggleOpacityState();
	void ToggleDashedOutline();
	void ToggleOrientation();
	void ToggleConstraint();
	void TogglePressedState();

	bool IsShowingDashedOutline() const { return bEnableDashedOutline; }
	bool IsShowingActiveOpacity() const { return bOpacityInActiveState; }
	bool IsConstraintActive() const { return bEnableConstraintControl; }
	bool IsInLandscapeOrientation() const { return bIsInLandscapeMode; }

	bool IsShowingControlName() const { return bEnableText; }
	bool IsShowPressedState() const { return bShowControlPressedState; }
	bool IsShowInteractiveZone() const { return bEnableInteractionOutline; }

	bool IsShowParentingLine() const { return bEnableParentingLine; }
	
	/** Construct Visualization ComboBox Menu */
	TSharedRef<SWidget> OnGetVisualizationMenuContent();

	void ToggleText();
	void ToggleOutline();
	void ToggleInteractionOutline();
	void ToggleClipping();

	void ToggleParentingLine();

	FReply HandleOnScaleSettingClicked() const;
	
	/** WIDGET */

	void HandleLinkControl();
	void HandleUnLinkControl() const;
	void HandleUnLinkAllControl() const;
	
	void HandleCancelLinkControl();
	void HandleApplyLinkControl(const FName ParentControl);
	

	/** UTILITIES */
	
	float GetDesignerScale() const { return DesignerScale; }
	
	FVector2D GetDesignerOffset() const { return PanningOffset / DesignerScale; }
	
	FOptionalSize GetDesignerWidth() const {return DesignerSize.X;}
	FOptionalSize GetDesignerHeight() const {return DesignerSize.Y;}

	EWidgetClipping GetClippingState() const;

	FText GetZoomText() const;

	FText GetScaleFactorAsText() const;

	/** Return all control widget added in canvas */
	TArray<TSharedPtr<SVirtualControlEditor>> GetAllWidgetInCanvas() const;

	/** Return true if an widget is hovered. OutWidget is first widget hovered */
	bool GetControlWidgetHovered(TSharedPtr<SVirtualControlEditor>& OutWidget) const;

	TSharedPtr<SVirtualControlEditor> GetControlWidgetHovered();

	const ISlateStyle& GetSlateStyle() const;

	//VARIABLES

	TWeakPtr<FVirtualControlDesignerEditor> TouchDesignerEditorPtr;
	
	UVirtualControlSetup* VirtualControlSetupEdited;

	TSharedPtr<SConstraintCanvas> DesignerSurface;
	TSharedPtr<STouchInterfaceDesignerRuler> TopRuler;
	TSharedPtr<STouchInterfaceDesignerRuler> SideRuler;

	TSharedPtr<SWidget> CustomSizeWidget;

	TSharedPtr<SSpinBox<int32>> WidthSpinBox;
	TSharedPtr<SSpinBox<int32>> HeightSpinBox;

	TSharedPtr<class SDeviceProfileInfo> DeviceProfileInfo;
	
	TSharedPtr<FActiveTimerHandle> RecenterHandle;
	
	TSharedPtr<SVirtualControlEditor> WidgetHovered;
	TSharedPtr<SVirtualControlEditor> WidgetSelected;
	TSharedPtr<SVirtualControlEditor> OldWidgetSelected;

	//Multi-Selection
	TArray<TSharedPtr<SVirtualControlEditor>> WidgetsSelected;
	TArray<TSharedPtr<SVirtualControlEditor>> OldWidgetsSelected;

	TSharedPtr<class SPaletteItemDraggedWidget> DraggedWidgetInstance;
	SConstraintCanvas::FSlot* DraggedWidgetSlot;

	TMap<FName, FControlWidget> ControlWidgets;

	const class UTouchInterfaceSettings* TouchInterfaceSettings;
#if ENGINE_MAJOR_VERSION > 4
	TObjectPtr<class UCustomTouchInterfaceScaling> CustomTouchInterfaceScaling;
#else
	class UCustomTouchInterfaceScaling* CustomTouchInterfaceScaling;
#endif
	const class UTouchInterfaceDesignerSettings* TouchInterfaceDesignerSettings;
	
	FVector2D PanningOffset;
	FVector2D PanningOffsetStart;
	FVector2D AbsoluteMousePositionStart;
	FVector2D CursorOffset;
	FVector2D MouseRelativeToViewportPosition;

	/** Current size of Designer Surface */
	FIntPoint DesignerSize;

	float DesignerScale;
	float PreviousDesignerScale;
	float CurrentScaleFactor;

	float CurrentOpacity;
	float ActiveOpacity;
	float InactiveOpacity;

	bool bIsPanning;
	bool bDragControl;
	bool bOpacityInActiveState;
	bool bIsInLandscapeMode;
	bool bEnableConstraintControl;
	bool bEnableOutline;
	bool bEnableDashedOutline;
	bool bEnableInteractionOutline;
	bool bEnableText;
	bool bEnableClipping;
	bool bEnableParentingLine;
	bool bIsDraggingItem;
	bool bDropAllowed;
	bool bShowControlPressedState;

	//Link/Unlink function
	uint8 bWaitForParentSelection:1;
	uint8 bDesignerHasCustomSize:1;

	int32 CustomWidth;
	int32 CustomHeight;

	FName ChildControlSelected;
	FName ParentControlSelected;
};
