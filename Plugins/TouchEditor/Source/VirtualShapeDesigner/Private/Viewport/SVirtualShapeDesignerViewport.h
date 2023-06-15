// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

class SVirtualShapeDesignerShapeInfo;
struct FDotData;
class FVirtualShapeDesignerCommands;
class SConstraintCanvas;
class STouchInterfaceDesignerRuler;
struct FShapeLine;
struct FShapeAngle;
class UVirtualShape;
class UTouchInterfaceSettings;
class UTouchInterfaceDesignerSettings;
class FVirtualShapeDesignerEditor;

enum class EDrawTools
{
	FreeDraw,
	//DrawBezierCurve,
	//EditBezierCurve
	AddPoint,
	SelectPoint,
	RemovePoint,
	ClearPoint,
	FlattenHorizontally,
	FlattenVertically
	//BendPoints
};

struct FBoxSelection
{
	void UpdatePointerLocation(const FVector2D InPointerLocation);
	FVector2D GetOrigin() const;
	FVector2D GetBoxCenter() const { return Center; }
	FVector2D GetBoxExtend() const { return Extend.GetAbs(); }
	FVector2D GetBoxSize() const { return Extend.GetAbs() * 2.0f; }
	FVector2D GetBoxMin() const { return GetOrigin(); }
	FVector2D GetBoxMax() const { return GetOrigin() + GetBoxSize(); }
	
	FBoxSelection()
	: Origin(ForceInitToZero)
	, Center(ForceInitToZero)
	, Extend(ForceInitToZero)
	{
		
	}

	explicit FBoxSelection(const FVector2D InOrigin)
	: Origin(InOrigin)
	, Center(ForceInitToZero)
	, Extend(ForceInitToZero)
	{
		
	}

private:
	FVector2D Origin;
	FVector2D PointerLocation;
	FVector2D Center;
	FVector2D Extend;
};

class SVirtualShapeDesignerViewport : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SVirtualShapeDesignerViewport)
	: _CommandList(nullptr)
	{}
	SLATE_ATTRIBUTE(TSharedPtr<FUICommandList>, CommandList)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedPtr<FVirtualShapeDesignerEditor> InVirtualShapeDesignerEditor);

private:
	void BindCommand();
	void GenerateDataFromVirtualShapeAsset();
	
protected:
	//SWidget Implementation
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;

	virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override;
	//SWidget Implementation

private:
	FReply OnFreeDrawStarted(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
	FReply OnFreeDrawMoved(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
	FReply OnFreeDrawEnded(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);

	FReply OnSelectStarted(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
	FReply OnSelectMoved(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
	FReply OnSelectEnded(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
	
	FReply OnAddEnded(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);

protected:	
	//SWidget Implementation
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	//SWidget Implementation
	
	/** Paint Background Lines of viewport (Very similar to Editor/UMGEditor/Private/Designer/SDesignSurface) */
	void PaintViewportGrid(const FSlateBrush* BackgroundImage, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32& DrawLayerId) const;

	/** Paint Image Reference */
	void PaintImageReference(const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const;

	/** Paint Shape */
	void PaintShape(const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const;

	/** Paint selection box */
	void PaintSelectionBox(const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const;


public:
	//Todo: calculate bound. Used for normalize shape for data comparison
	//Todo: Allow user to define start order for line that not connected to any other

	void ChangeToolMode(EDrawTools Mode);

	// Remove point and line from viewport
	void ClearDraw();

	void Flatten(bool bHorizontally = true);

	// Unselect all points
	void ClearDotSelection();

	// Try to simplify the current drawing
	void SimplifyShape();

	// Calculate all data
	void GenerateData(const bool SaveToAsset = false);
	
private:
	TSharedRef<SWidget> OnGetVisualizationMenuContent();
	
	const ISlateStyle& GetSlateStyle() const;

	EVisibility GetDesignerOutlineVisibility() const;

	EVisibility GetRulerVisibility() const;

	void HandleOnGridSnappingChanged(bool bIsEnabled);
	void HandleOnGridSpacingChanged(int32 Spacing);

#if WITH_EDITOR
	void HandleOnSettingsChanged(UObject* Object, FPropertyChangedEvent& Property);
#endif

	float ComputeCornerAngle(const FVector2D VectorA, const FVector2D VectorB, const float NormalizeTolerance = 1.e-8f);

	//Todo: Regenerate partial section

	void RemovePoint(const FVector2D LocalCoord);

	FVector2D GetSnappingPosition(const FVector2D LocalCoord, int32 Multiple) const;

	void SaveModification();

	void ToggleDirectionDebug() { bShowDrawDirection = !bShowDrawDirection; }
	void ToggleLenghtDebug() { bShowDrawLenght = !bShowDrawLenght; }
	void ToggleAngleDebug() { bShowDrawAngle = !bShowDrawAngle; }
	void ToggleBoundDebug() { bShowDrawBound = !bShowDrawBound; }
	void ToggleOrderDebug() { bShowOrder = !bShowOrder; }

	FName GetDotStyleName(const FDotData& Dot) const;

	bool DotIsUnderCursor(const FVector2D DotLocation) const;
	bool DotIsUnderLocation(const FVector2D DotLocation, const FVector2D Location) const;
	bool DotInUnderBoxSelection(const FVector2D DotLocation, const FBoxSelection& BoxSelection) const;


	TWeakPtr<FVirtualShapeDesignerEditor> EditorPtr;

	TSharedPtr<SVirtualShapeDesignerShapeInfo> ShapeInfoWidget;
	
	TSharedPtr<FUICommandList> CommandList;
	
	UVirtualShape* VirtualShapeEdited;
	
	const UTouchInterfaceDesignerSettings* EditorSettings;
	const UTouchInterfaceSettings* RuntimeSettings;

	TArray<FDotData> DotData;
	
	TArray<FVector2D> DotPositions;

	TArray<FVector2D> Deprecated_Lines;

	TArray<FShapeLine> ShapeLines;
	TArray<FShapeAngle> ShapeAngles;

	TArray<FShapeAngle> Angles;

	float ShapeDotDistance;

	float LastCornerAngle;

	uint8 bIsDrawing:1;
	uint8 bIsPanning:1;
	uint8 bIsHoverDot:1;
	uint8 bDotSelected:1;
	uint8 bBoxSelection:1;
	uint8 bAdditiveSelection:1;
	uint8 bSubtractiveSelection:1;
	uint8 bShowNextLine:1;

	TSharedPtr<SConstraintCanvas> Canvas;
	TSharedPtr<STouchInterfaceDesignerRuler> TopRuler;
	TSharedPtr<STouchInterfaceDesignerRuler> SideRuler;

	bool bShowDrawDirection;
	bool bShowDrawLenght;
	bool bShowDrawAngle;
	bool bShowDrawBound;
	bool bShowOrder;

	bool bGridSnappingEnabled;
	
	int32 GridSpacingValue;

	EDrawTools SelectedTool;

	FBoxSelection BoxSelectionData;

	FVector2D MousePosition;
	FVector2D PanningStartLocation;
	FVector2D OldMouseLocation;
};
