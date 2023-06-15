// Copyright Lost in Game Studio. All Rights Reserved

#pragma once
#include "Classes/VirtualControlSetup.h"

class SVirtualControlEditor;
class SConstraintCanvas;

struct FPreviewData
{
	/** The actual center of the control (should be normalized to 0-1)*/
	FVector2D Center;

	/** The size of a control */
	FVector2D VisualSize;

	/** The size of the thumb */
	FVector2D ThumbSize;

	FSlateBrush Background;
	FSlateBrush Thumb;
	
	/** The brush to use to draw the background for joysticks or buttons */
	//TSharedPtr<ISlateBrushSource> BackgroundImage;

	/** The brush to use to draw the thumb for joysticks */
	//TSharedPtr<ISlateBrushSource> ThumbImage;

	// Default Constructor
	FPreviewData() :
	Center(FVector2D(0.5f)),
	VisualSize(FVector2D(100)),
	ThumbSize(ForceInitToZero)
	//BackgroundImage(nullptr),
	//ThumbImage(nullptr)
	{
		
	}

	FPreviewData(FVector2D InCenter, FVector2D InVisualSize, FVector2D InThumbSize, TSharedPtr<ISlateBrushSource> InBackgroundImage, TSharedPtr<ISlateBrushSource> InThumbImage = nullptr) :
	Center(InCenter),
	VisualSize(InVisualSize),
	ThumbSize(InThumbSize)
	//BackgroundImage(InBackgroundImage),
	//ThumbImage(InThumbImage)
	{
		
	}
};

struct FConvertSettings;

//Todo: Use this as canvas of virtual controls. This can be used by SVirtualControlPreviewTab, STouchInterfacePresetWindow and STouchInterfaceConverter
class STouchInterfacePreviewer : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STouchInterfacePreviewer)
	: _CustomScaleFactor(false)
	, _BackgroundColor(FLinearColor::Gray)
	{}
	SLATE_ARGUMENT(TArray<FVirtualControl>, VirtualControls)
	SLATE_ARGUMENT(FAssetData, PresetAssetData)
	SLATE_ARGUMENT(bool, CustomScaleFactor)
	SLATE_ARGUMENT(FLinearColor, BackgroundColor)

	SLATE_ATTRIBUTE(float, ScaleFactor)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	EActiveTimerReturnType AutoRefresh(const double InCurrentTime, const float InDeltaTime);
	TArray<FVirtualControl> GenerateVirtualControlsFromAssetData(const FAssetData Preset);
	void GenerateVirtualControlWidgets(TArray<FVirtualControl> InVirtualControls);

public:
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	void UpdateData(TArray<FVirtualControl> NewData, FIntPoint NewPreviewSize);
	void Refresh();

private:	
	float GetScaleFactor() const;

	FVector2D GetPreviewerSize() const;
	
	TSharedPtr<SConstraintCanvas> Canvas;

	TArray<FVirtualControl> VirtualControls;

	TArray<TSharedPtr<SVirtualControlEditor>> VirtualControlWidgets;

	FVector2D PreviousGeometrySize;

	FIntPoint PreviewSize;	

	TAttribute<float> ScaleFactor;

	float PreviousScaleFactor;

	uint8 CustomScaleFactor:1;

	FLinearColor BackgroundColor;
};
