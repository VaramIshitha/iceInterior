// Copyright Lost in Game Studio. All Rights Reserved.

#include "SVirtualShapeDesignerPalette.h"

#include "SVirtualShapeDesignerViewport.h"
#include "VirtualShapeDesignerEditor.h"
#include "VirtualShapeDesignerEditorStyle.h"

DEFINE_LOG_CATEGORY_STATIC(LogVirtualShapeDesignerPalette, All, All);

void SVirtualShapeDesignerPalette::Construct(const FArguments& InArgs, TSharedPtr<FVirtualShapeDesignerEditor> InVirtualShapeDesignerEditor)
{
	DesignerEditor = InVirtualShapeDesignerEditor;
	
	//Todo: Use SMultibox (see landscape toolbar)
	
	UnselectedStyle = FVirtualShapeDesignerEditorStyle::Get().GetWidgetStyle<FButtonStyle>("UnselectedButton");
	SelectedStyle = FVirtualShapeDesignerEditorStyle::Get().GetWidgetStyle<FButtonStyle>("SelectedButton");

	PreviousModeSelected = EDrawTools::SelectPoint;
	
	ChildSlot
	[
		SAssignNew(ToolContainer, SVerticalBox)
	];

	MakeToolWidgets();
}

void SVirtualShapeDesignerPalette::MakeToolWidgets()
{
	AddTool(FName("Select"), EDrawTools::SelectPoint, FName("Palette.SelectPoint.Icon"));
	AddTool(FName("Draw"), EDrawTools::FreeDraw, FName("Palette.FreeDraw.Icon"));
	AddTool(FName("Add"), EDrawTools::AddPoint, FName("Palette.AddPoint.Icon"));
	AddTool(FName("Delete"), EDrawTools::RemovePoint, FName("Palette.RemovePoint.Icon"));
	AddTool(FName("Clear"), EDrawTools::ClearPoint, FName("Palette.ClearDraw.Icon"));
	AddTool(FName("Flatten"), EDrawTools::FlattenHorizontally, FName("Palette.FlattenHorizontally.Icon"));
	AddTool(FName("Flatten"), EDrawTools::FlattenVertically, FName("Palette.FlattenVertically.Icon"));
	
	//AddTool(FName("BezierCurve"));
	//AddTool(FName("EditBezierCurve"));
	//AddTool(FName("BendPoint"));
}

void SVirtualShapeDesignerPalette::AddTool(FName FriendlyName, EDrawTools Type, FName BrushName)
{	
	TSharedPtr<SButton> ConstructedButton = nullptr;
	
	ToolContainer->AddSlot()
	.AutoHeight()
	.Padding(1.0f)
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		SAssignNew(ConstructedButton, SButton)
		.OnClicked(FOnClicked::CreateSP(this, &SVirtualShapeDesignerPalette::HandleOnToolSelected, Type))
		.ButtonStyle(Type == PreviousModeSelected ? &SelectedStyle : &UnselectedStyle)
		.ForegroundColor(FSlateColor(FLinearColor(1,1,1,1)))
		.ToolTipText(FText::FromName(FriendlyName))
		[
			SNew(SVerticalBox)
			
			+SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SImage)
				.Image(FVirtualShapeDesignerEditorStyle::Get().GetBrush(BrushName))
			]
			
			+SVerticalBox::Slot()
			.AutoHeight()
			//.Padding(GetSlateStyle().GetMargin(ISlateStyle::Join(FName("ToolBar"),".Label.Padding")))
			.Padding(FMargin(0.0f, 2.0f))
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Visibility(EVisibility::HitTestInvisible)
				.Text(FText::FromName(FriendlyName))
				//.TextStyle(GetSlateStyle(), "ToolBar.Label")	// Smaller font for tool tip labels
				.ShadowOffset(FVector2D::UnitVector)
			]
		]
	];

	Tools.Add(Type, ConstructedButton);
}

int32 SVirtualShapeDesignerPalette::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const FSlateBrush* BackgroundImage = GetSlateStyle().GetBrush(TEXT("Graph.Panel.SolidBackground")); //"Graph.DelegatePin.Connected"
	// Draw Background
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), BackgroundImage);
	
	return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

const ISlateStyle& SVirtualShapeDesignerPalette::GetSlateStyle() const
{
	
#if ENGINE_MAJOR_VERSION > 4
	return FAppStyle::Get();
#else
	return FEditorStyle::Get();
#endif
	
}

void SVirtualShapeDesignerPalette::SelectTool(EDrawTools Type, bool bPropagate)
{
	Tools.FindRef(PreviousModeSelected)->SetButtonStyle(&UnselectedStyle);
	if (bPropagate)
	{
		DesignerEditor.Pin()->GetViewport()->ChangeToolMode(Type);
	}
	Tools.FindRef(Type)->SetButtonStyle(&SelectedStyle);
	PreviousModeSelected = Type;
}

FReply SVirtualShapeDesignerPalette::HandleOnToolSelected(EDrawTools ToolType)
{
	//Todo: Make Action function in viewport to avoid to manually call function here. Use FUICommandList or info or indicate if tool mode is an action instead of toggle
	if (ToolType == EDrawTools::ClearPoint)
	{
		DesignerEditor.Pin()->GetViewport()->ClearDraw();
		return FReply::Handled();
	}

	if (ToolType == EDrawTools::FlattenHorizontally)
	{
		DesignerEditor.Pin()->GetViewport()->Flatten();
		return FReply::Handled();
	}

	if (ToolType == EDrawTools::FlattenVertically)
	{
		DesignerEditor.Pin()->GetViewport()->Flatten(false);
		return FReply::Handled();
	}
	
	Tools.FindRef(PreviousModeSelected)->SetButtonStyle(&UnselectedStyle);
	DesignerEditor.Pin()->GetViewport()->ChangeToolMode(ToolType);
	Tools.FindRef(ToolType)->SetButtonStyle(&SelectedStyle);
	PreviousModeSelected = ToolType;

	return FReply::Handled();
}