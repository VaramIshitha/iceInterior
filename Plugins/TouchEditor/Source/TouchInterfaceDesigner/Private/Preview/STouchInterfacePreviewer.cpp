// Copyright Lost in Game Studio. All Rights Reserved.

#include "STouchInterfacePreviewer.h"

#include "Settings/TouchInterfaceSettings.h"
#include "Engine/UserInterfaceSettings.h"
#include "Presets/TouchInterfacePreset.h"
#include "Viewport/SVirtualControlEditor.h"
#include "Widgets/Layout/SConstraintCanvas.h"

DEFINE_LOG_CATEGORY_STATIC(LogTouchInterfacePreviewer, All, All);

void STouchInterfacePreviewer::Construct(const FArguments& InArgs)
{
	if (InArgs._VirtualControls.Num() > 0)
	{
		VirtualControls = InArgs._VirtualControls;
	}
	else if (InArgs._PresetAssetData.IsValid())
	{
		VirtualControls = GenerateVirtualControlsFromAssetData(InArgs._PresetAssetData);
	}
	else
	{
		UE_LOG(LogTouchInterfacePreviewer, Error, TEXT("Argument not valid"));
		return;
	}
	
	PreviewSize = FIntPoint(1920, 1080);
	CustomScaleFactor = InArgs._CustomScaleFactor;
	PreviousGeometrySize = FVector2D::ZeroVector;
	BackgroundColor = InArgs._BackgroundColor;

	ChildSlot
	[
		SNew(SOverlay)
		+SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(Canvas, SConstraintCanvas)
			.Visibility(EVisibility::HitTestInvisible)
			.Clipping(EWidgetClipping::ClipToBounds)
		]
	];

	RegisterActiveTimer(0.1f, FWidgetActiveTimerDelegate::CreateSP(this, &STouchInterfacePreviewer::AutoRefresh));
}

EActiveTimerReturnType STouchInterfacePreviewer::AutoRefresh(const double InCurrentTime, const float InDeltaTime)
{
	if (GetTickSpaceGeometry().GetLocalSize() == FVector2D::ZeroVector)
	{
		return EActiveTimerReturnType::Continue;
	}

	GenerateVirtualControlWidgets(VirtualControls);
	
	return EActiveTimerReturnType::Stop;
}

TArray<FVirtualControl> STouchInterfacePreviewer::GenerateVirtualControlsFromAssetData(const FAssetData Preset)
{
	const UTouchInterfacePreset* TouchDesignerPreset = Cast<UTouchInterfacePreset>(Preset.GetAsset());
	check(TouchDesignerPreset)
	return TouchDesignerPreset->GetControlsSetting();
}

void STouchInterfacePreviewer::GenerateVirtualControlWidgets(TArray<FVirtualControl> InVirtualControls)
{
	for (const FVirtualControl& Control : InVirtualControls)
	{
		TSharedPtr<SVirtualControlEditor> NewControl = nullptr;
	
		SConstraintCanvas::FSlot* ConstructedSlot = nullptr;
		Canvas->AddSlot()
		//.ZOrder(Control.Type == EControlType::TouchRegion ? 99 : 100)
		.Alignment(FVector2D(0.5f,0.5f))
		.Anchors(FAnchors(0.0f))
		.Expose(ConstructedSlot)
		.AutoSize(true)
		[
			SAssignNew(NewControl, SVirtualControlEditor, nullptr)
			.VirtualControlData(Control)
			.Slot(ConstructedSlot)
			//.StartPosition(Control.LandscapeCenter)
			.CanvasSize(this, &STouchInterfacePreviewer::GetPreviewerSize)
			.ScaleFactor(this, &STouchInterfacePreviewer::GetScaleFactor)
			.ShowDashedOutline(false)
			.HoverEffect(false)
			//.LandscapeOrientation()
		];

		NewControl->SetCanvasSlot(ConstructedSlot);
		NewControl->SetPositionInCanvas(Control.LandscapeCenter, true, false);
		VirtualControlWidgets.Add(NewControl);
	}
}

void STouchInterfacePreviewer::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	
	const FVector2D GeometrySize = AllottedGeometry.GetLocalSize();
	
	const float Scale = ScaleFactor.Get();

	if (GeometrySize != PreviousGeometrySize || Scale != PreviousScaleFactor)
	{
		Refresh();
	}

	PreviousGeometrySize = GeometrySize;
	PreviousScaleFactor = Scale;
}

int32 STouchInterfacePreviewer::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
#if ENGINE_MAJOR_VERSION > 4
	const FSlateBrush* Brush = FAppStyle::Get().GetBrush("WhiteBrush");
#else
	const FSlateBrush* Brush = FEditorStyle::Get().GetBrush("WhiteBrush");
#endif
	//FLinearColor(1.0f, 1.0f, 1.0f, 0.2f)
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), Brush, ESlateDrawEffect::None, BackgroundColor);

	return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

void STouchInterfacePreviewer::UpdateData(TArray<FVirtualControl> NewData, FIntPoint NewPreviewSize)
{
	VirtualControls = NewData;
	PreviewSize = NewPreviewSize;

	Canvas->ClearChildren();
	VirtualControlWidgets.Empty();
	GenerateVirtualControlWidgets(VirtualControls);
}

void STouchInterfacePreviewer::Refresh()
{
	//When geometry change, update virtual control position and visual (scaleFactor)
	for (const TSharedPtr<SVirtualControlEditor> Widget : VirtualControlWidgets)
	{
		Widget->Refresh();
	}
}

float STouchInterfacePreviewer::GetScaleFactor() const
{
	if (CustomScaleFactor)
	{
		return ScaleFactor.Get();
	}
	
	const EScalingMode Mode = GetDefault<UTouchInterfaceSettings>()->ScalingMode;
	const FVector2D Size = GetTickSpaceGeometry().GetLocalSize();
	
	switch (Mode)
	{
	case EScalingMode::NONE:
		return 1;

	case EScalingMode::DPI:
		return GetDefault<UUserInterfaceSettings>(UUserInterfaceSettings::StaticClass())->GetDPIScaleBasedOnSize(FIntPoint(Size.X, Size.Y));
		
	case EScalingMode::DesignSize:
		{
			const float DesiredWidth = GetDefault<UTouchInterfaceSettings>()->DesignWidth; //1024.0f
			return Size.GetMax() / DesiredWidth;
		}
	case EScalingMode::Custom:
		//Todo: Custom scaling mode
		return 1;
	}

	// Default
	return 1;
}

FVector2D STouchInterfacePreviewer::GetPreviewerSize() const
{
	return GetTickSpaceGeometry().GetLocalSize();
}
