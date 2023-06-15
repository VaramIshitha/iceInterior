// Copyright Lost in Game Studio. All Rights Reserved

#include "SVirtualControlPreviewerTab.h"
#include "Editor/VirtualControlDesignerEditor.h"
#include "Editor/VirtualControlDesignerEditorCommands.h"
#include "Rendering/DrawElements.h"
#include "VirtualControlSetup.h"
#include "Engine/UserInterfaceSettings.h"
#include "Viewport/SVirtualControlEditor.h"
#include "Widgets/Layout/SConstraintCanvas.h"

void SVirtualControlPreviewerTab::Construct(const FArguments& InArgs, TSharedPtr<FVirtualControlDesignerEditor> InTouchDesignerEditor)
{
	Editor = InTouchDesignerEditor;
	State = Inactive;
	
	// just set some defaults
	CurrentOpacity = 1.0f;

	ThumbOffset = FVector2D::ZeroVector;

	
	ChildSlot
	[
		SNew(SOverlay)
		.Visibility(EVisibility::SelfHitTestInvisible)
		
		+SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(FMargin(5.0f))
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				ConstructToolbar()
			]
		]

		+SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.Padding(FMargin(20.0f))
		[
			SAssignNew(Canvas, SConstraintCanvas)
		]
	];

	GenerateVirtualControl();
}

TSharedRef<SWidget> SVirtualControlPreviewerTab::ConstructToolbar()
{
	FToolBarBuilder ToolBarBuilder(Editor.Pin()->GetCommands(), FMultiBoxCustomization::None);
	
	const FName ToolBarStyle = "VirtualControlPreviewerToolBar";
	
	//ToolBarBuilder.SetStyle(&FAppStyle::Get(), ToolBarStyle);
	ToolBarBuilder.SetLabelVisibility(EVisibility::Collapsed);

	ToolBarBuilder.BeginSection("View");
	ToolBarBuilder.BeginBlockGroup();
	ToolBarBuilder.AddToolBarButton(FVirtualControlDesignerCommands::Get().TogglePreviewOpacityStateCommand, NAME_None, TAttribute<FText>(), TAttribute<FText>(), TAttribute<FSlateIcon>(), "PreviewOpacityState");
	ToolBarBuilder.EndBlockGroup();
	ToolBarBuilder.EndSection();

	return ToolBarBuilder.MakeWidget();
}

TSharedRef<SWidget> SVirtualControlPreviewerTab::GenerateVirtualControl()
{
	SConstraintCanvas::FSlot* ConstructedSlot = nullptr;
	
	Canvas->AddSlot()
	.Alignment(FVector2D(0.5f, 0.5f))
	.Anchors(FAnchors(0))
	.AutoSize(true)
	.ZOrder(100)
	.Expose(ConstructedSlot)
	[
		SAssignNew(PreviewWidget, SVirtualControlEditor, Editor.Pin())
		.Slot(ConstructedSlot)
		.VirtualControlData(VirtualControlData)
		.StartPosition(Canvas->GetTickSpaceGeometry().GetLocalSize() / 2.0f)
		.CanvasSize(this, &SVirtualControlPreviewerTab::GetCanvasSize)
		.ScaleFactor(this, &SVirtualControlPreviewerTab::GetScaleFactor)
		.Opacity(this, &SVirtualControlPreviewerTab::GetOpacity)
		.ShowPressedState(this, &SVirtualControlPreviewerTab::GetPressedState)
		.ThumbOffset(this, &SVirtualControlPreviewerTab::GetThumbOffset)
		.ShowInteractiveZone(true)
		.ShowDashedOutline(false)
		.HoverEffect(false)
		.Visibility(EVisibility::Hidden)
	];

	PreviewWidget->SetCanvasSlot(ConstructedSlot);
	PreviewWidget->SetPositionInCanvas(FVector2D(0.5f), true, false);
	return PreviewWidget.ToSharedRef();
}

FReply SVirtualControlPreviewerTab::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	bIsPressed = true;
	
	if (VirtualControlData.Type == EControlType::Joystick && bEnablePreview)
	{
		if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			const FVector2D LocalCoord = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
			
			if (PreviewWidget->PositionIsInside(MouseEvent.GetScreenSpacePosition()))
			{
				State = Active;
				bCaptureMouse = true;
				ThumbOffset = (LocalCoord - MyGeometry.AbsoluteToLocal(PreviewWidget->GetTickSpaceGeometry().GetAbsolutePosition())) - PreviewWidget->GetTickSpaceGeometry().GetLocalSize() * 0.5f;				
				return FReply::Handled().CaptureMouse(SharedThis(this));
			}
		}
	}

	if (VirtualControlData.Type == EControlType::Button && bEnablePreview)
	{
		if (PreviewWidget->PositionIsInside(MouseEvent.GetScreenSpacePosition()))
		{
			State = Active;
			GEditor->PlayEditorSound(VirtualControlData.PressedSound);
			//GEditor->PlayPreviewSound(VirtualControlData.PressedSound);
			return FReply::Handled();
		}
		
	}
	
	return FReply::Unhandled();
}

FReply SVirtualControlPreviewerTab::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (bCaptureMouse)
	{
		const FVector2D LocalCoord = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
		ThumbOffset = (LocalCoord - MyGeometry.AbsoluteToLocal(PreviewWidget->GetTickSpaceGeometry().GetAbsolutePosition())) - PreviewWidget->GetTickSpaceGeometry().GetLocalSize() * 0.5f;
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply SVirtualControlPreviewerTab::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	bIsPressed = false;
	
	if (bCaptureMouse)
	{
		bCaptureMouse = false;
		ThumbOffset = FVector2D(0.0f);
		State = CountingDownToInactive;
		return FReply::Handled().ReleaseMouseCapture();
	}

	State = CountingDownToInactive;

	return FReply::Unhandled();
}

void SVirtualControlPreviewerTab::Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime )
{
	const FVector2D GeometrySize = AllottedGeometry.GetLocalSize();

	if (GeometrySize != PreviousGeometrySize)
	{
		PreviewWidget->Refresh();
	}

	const UVirtualControlSetup* Setup = Editor.Pin()->GetVirtualControlSetup();
	
	//Set opacity
	//Todo: fade and preview curve
	switch (State)
	{
	case Active:
		CurrentOpacity = Setup->ActiveOpacity;
		Countdown = Setup->TimeUntilDeactivated;
		break;

	case CountingDownToInactive:
		Countdown -= InDeltaTime;
		if (Countdown <= 0.0f)
		{
			State = Inactive;
		}
		break;

	case Inactive:
		CurrentOpacity = Setup->InactiveOpacity;
		break;

	default:
		State = Inactive;
		break;
	}

	PreviousGeometrySize = GeometrySize;
}

void SVirtualControlPreviewerTab::DrawLayer(const FVisualLayer& InLayer, const FVector2D InSize, const FVector2D InBrushSize, const FVector2D InOffset, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const
{
	const FSlateBrush* Brush = &InLayer.Brush;
	FLinearColor BackgroundTint = Brush->GetTint(InWidgetStyle);
	BackgroundTint.A *= CurrentOpacity;

	const FVector2D BrushSize = InLayer.bUseBrushSize ? Brush->ImageSize : InBrushSize;
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

FVector2D SVirtualControlPreviewerTab::GetCanvasSize() const
{
	return Canvas->GetTickSpaceGeometry().GetLocalSize();
}

FVector2D SVirtualControlPreviewerTab::GetThumbOffset() const
{
	return ThumbOffset;
}

float SVirtualControlPreviewerTab::GetScaleFactor() const
{
	const float AvailableSpace = Canvas->GetTickSpaceGeometry().GetLocalSize().GetMin(); /*FMath::Clamp(Canvas->GetTickSpaceGeometry().GetLocalSize().GetMin() - 20, 0.0f, Canvas->GetTickSpaceGeometry().GetLocalSize().GetMin());*/
	
	if (bEnablePreview)
	{		
		const float MinRequiredSpace = VirtualControlData.Type == EControlType::Joystick ? VirtualControlData.VisualSize.GetMax() + VirtualControlData.ThumbSize.GetMax() : VirtualControlData.VisualSize.GetMax();
		return AvailableSpace/MinRequiredSpace;
	}

	return 1.0f;
}

float SVirtualControlPreviewerTab::GetOpacity() const
{
	return CurrentOpacity;
}

bool SVirtualControlPreviewerTab::GetPressedState() const
{
	return bIsPressed;
}

void SVirtualControlPreviewerTab::SetPreviewData(FVirtualControl InControlData)
{
	VirtualControlData = InControlData;
	PreviewWidget->Update(VirtualControlData, false);
	PreviewWidget->SetPositionInCanvas(FVector2D(0.5f), true, false);
	PreviewWidget->SetVisibility(EVisibility::HitTestInvisible);
	bEnablePreview = true;
}

void SVirtualControlPreviewerTab::ClearPreview()
{
	PreviewWidget->SetVisibility(EVisibility::Hidden);
	bEnablePreview = false;
}
