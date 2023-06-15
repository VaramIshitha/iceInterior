// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

class FVirtualShapeDesignerEditor;
struct FDotData;
class UTouchInterfaceDesignerSettings;

class SVirtualShapeDesignerDrawer : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SVirtualShapeDesignerDrawer){}
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs, TSharedPtr<FVirtualShapeDesignerEditor> InVirtualShapeDesignerEditor);

	//Todo: Recognize only shape being edited or all shapes in project ?
	//Todo: Draw touch interface surface for help only (option ?)
	//Todo: Show matching score
	//Todo: Cool Effect : Show process of comparison slowly (dot location comparison, line lenght and direction, corner, adjustment..)
	//Todo: Get config class for drawer appearance in settings. If not valid, use default
	
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override { return FVector2D(100.0f); }

	//Try to recognize the shape
	void ComputeRecognition();

private:

#if WITH_EDITOR
	void HandleOnSettingsChanged(UObject* Object, FPropertyChangedEvent& Property);
#endif

	void CalculateDataBasedOnBound(const FVector2D& BoundCenter);

	//Todo: Return index
	void AddNewDrawDot(FVector2D Location);
	void AddNewDrawDot(TArray<FVector2D> Locations);

	void AddNewDrawLine(FVector2D StartLocation, FVector2D EndLocation);
	void AddNewDrawLines(TArray<FVector2D> Locations);

	void ClearDrawDots();
	void ClearDrawLines();

	//Todo: Remove index from draw dot or line

	const UTouchInterfaceDesignerSettings* EditorSettings;

	TWeakPtr<FVirtualShapeDesignerEditor> EditorPtr;
	
	TArray<FVector2D> DotPositions;

	TArray<FDotData> UserDrawing;
	TArray<FDotData> DotData;

	TArray<FVector2D> DrawDots;
	TArray<TArray<FVector2D>> DrawLines;

	float ShapeDotDistance;

	uint8 bIsDrawing:1;

	uint8 bIsComputing:1;

	uint8 bLaunchTimer:1;
	
	float AllowedDelayBetweenSection;
	float LastTouchTimer;
};
