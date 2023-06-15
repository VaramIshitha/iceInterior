// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

struct FDotData;
class UTouchInterfaceSettings;

/**
 * 
 */
class TOUCHINTERFACE_API SVirtualShapeDrawer : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SVirtualShapeDrawer)
	{}
		SLATE_ATTRIBUTE(TArray<FDotData>, UserDrawing)
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs);

	virtual ~SVirtualShapeDrawer() override;
	
	void DrawStarted(const FGeometry& MyGeometry, const FPointerEvent& Event);
	void DrawUpdated(const FGeometry& MyGeometry, const FPointerEvent& Event);
	void DrawEnded(const FGeometry& MyGeometry, const FPointerEvent& Event);

	//Todo: Receive event like InCompute, ShowFindedShape, ClearDraw
	
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override { return FVector2D(100.0f); }
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	
private:	
	void LoadResources();
	
	void ClearDrawData();
	
	const UTouchInterfaceSettings* Settings;
	
	TArray<FDotData> UserDrawing;

	TAttribute<TArray<FDotData>> Draw;
	
	uint8 bIsDrawing:1;
	uint8 bTimerLaunched:1;
	
	float TimerCounter;
	float DelayBetweenEndDrawAndComputation;
	
	FTimerHandle ClearTimerHandle;

	float ShapeDotDistance;
	
	TWeakObjectPtr<UObject> BrushResource;
	
	FSlateBrush PointBrush;

	TAttribute<UWorld*> WorldContext;
};
