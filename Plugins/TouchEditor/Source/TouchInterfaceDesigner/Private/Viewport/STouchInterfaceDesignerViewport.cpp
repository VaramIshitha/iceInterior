// Copyright Lost in Game Studio. All Rights Reserved

#include "STouchInterfaceDesignerViewport.h"

#include "SDeviceProfileInfo.h"
#include "Editor/VirtualControlDesignerEditor.h"
#include "Editor/VirtualControlDesignerEditorCommands.h"
#include "VirtualControlSetup.h"

#include "TouchInterfaceDesignerSettings.h"
#include "Settings/TouchInterfaceSettings.h"

#include "Slate/Public/Widgets/Layout/SConstraintCanvas.h"

#if	ENGINE_MAJOR_VERSION > 4
#include "Styling/ToolBarStyle.h"
#endif

#include "STouchInterfaceDesignerRuler.h"
#include "Palette/DragAndDrop/PaletteItemDragDropOp.h"
#include "Palette/DragAndDrop/SPaletteItemDraggedWidget.h"

#include "SVirtualControlEditor.h"
#include "SViewportZoomPan.h"

#include "Settings/TouchInterfaceDesignerDeviceProfile.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Layout/SGridPanel.h"

#define LOCTEXT_NAMESPACE "STouchDesignerEditorViewportWidget"

FDesignerDeviceProfile CurrentProfile;

void STouchInterfaceDesignerViewport::Construct(const FArguments& InArgs, TSharedPtr<FVirtualControlDesignerEditor> InTouchDesignerEditor)
{
	PanningOffset = FVector2D::ZeroVector;
	
	DesignerSize = FIntPoint(1920,1080);
	MouseRelativeToViewportPosition = FVector2D::ZeroVector;
	bIsInLandscapeMode = true;
	TouchDesignerEditorPtr = InTouchDesignerEditor;
	VirtualControlSetupEdited = InTouchDesignerEditor->GetVirtualControlSetup();
	TouchInterfaceSettings = GetDefault<UTouchInterfaceSettings>();
	TouchInterfaceDesignerSettings = GetDefault<UTouchInterfaceDesignerSettings>();
	bEnableDashedOutline = TouchInterfaceDesignerSettings->bShowDashedOutlineByDefault;
	DesignerScale = 1;
	PreviousDesignerScale = 1;
	ActiveOpacity = VirtualControlSetupEdited->ActiveOpacity;
	InactiveOpacity = VirtualControlSetupEdited->InactiveOpacity;
	bOpacityInActiveState = true;
	CurrentOpacity = bOpacityInActiveState ? ActiveOpacity : InactiveOpacity;

	bEnableOutline = true;

	CustomWidth = 1920;
	CustomHeight = 1080;
	
	FTouchInterfaceDesignerDeviceProfile::GetProfile("Generic 16/9", CurrentProfile);
	
	CurrentScaleFactor = TouchInterfaceSettings->GetScaleFactor(FVector2D(DesignerSize));

#if ENGINE_MAJOR_VERSION > 4
	const FToolBarStyle& ToolBarStyle = FAppStyle::Get().GetWidgetStyle<FToolBarStyle>("EditorViewportToolBar");
#endif
	
	
	ChildSlot
	[
		SNew(SGridPanel)
		.FillColumn(1, 1.0f)
		.FillRow(1, 1.0f)

		// Corner
		+ SGridPanel::Slot(0, 0)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
			.BorderBackgroundColor(FLinearColor(FColor(48, 48, 48)))
		]

		// Top Ruler
		+ SGridPanel::Slot(1, 0)
		[
			SAssignNew(TopRuler, STouchInterfaceDesignerRuler)
			.Orientation(Orient_Horizontal)
			.Visibility(EVisibility::Visible)
		]

		// Side Ruler
		+ SGridPanel::Slot(0, 1)
		[
			SAssignNew(SideRuler, STouchInterfaceDesignerRuler)
			.Orientation(Orient_Vertical)
			.Visibility(EVisibility::Visible)
		]

		// Designer content area
		+ SGridPanel::Slot(1, 1)
		[
			SNew(SOverlay)
			//.Visibility(this, &STouchDesignerEditorViewportWidget::GetDesignerVisibility)

			+SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				// Arrange Children Zoom and offset
				SNew(SViewportZoomPan)
				.Visibility(EVisibility::SelfHitTestInvisible)
				.ZoomAmount(this, &STouchInterfaceDesignerViewport::GetDesignerScale)
				.ViewOffset(this, &STouchInterfaceDesignerViewport::GetDesignerOffset)
				[
					SNew(SBox)
					.Visibility(EVisibility::SelfHitTestInvisible)
					.WidthOverride(this, &STouchInterfaceDesignerViewport::GetDesignerWidth)
					.HeightOverride(this, &STouchInterfaceDesignerViewport::GetDesignerHeight)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						// Canvas that contain virtual control editor
						SAssignNew(DesignerSurface, SConstraintCanvas)
						.Visibility(EVisibility::Visible)
					]
				]
			]
			
			+SOverlay::Slot()
			.Padding(2.0f,2.0f,2.0f,0)
			[
				SNew(SHorizontalBox)

				// Zoom
				+SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				[
					SNew(STextBlock)
					.Visibility(EVisibility::HitTestInvisible)
					.TextStyle(GetSlateStyle(), "Graph.ZoomText")
					.ColorAndOpacity(FSlateColor(FLinearColor(1,1,1,0.25f)))
					.Text(this, &STouchInterfaceDesignerViewport::GetZoomText)
				]
				
				+SHorizontalBox::Slot()
				.FillWidth(1)
				[
					SNew(SSpacer)
				]

				// Designer Surface Toolbar
				+SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Top)
				[
					SNew(SHorizontalBox)

					+SHorizontalBox::Slot()
					.AutoWidth()
					[
						MakeViewportToolbar()
					]
					
					+SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(5.0f, 0.0f)
					[
						SNew(SComboButton)
		#if ENGINE_MAJOR_VERSION > 4
						.ButtonStyle(&ToolBarStyle.ButtonStyle)
						.ContentPadding(ToolBarStyle.ButtonPadding)
						.MenuPlacement(MenuPlacement_BelowAnchor)
						.OnGetMenuContent(FOnGetContent::CreateSP(this, &STouchInterfaceDesignerViewport::OnGetVisualizationMenuContent))
						.ButtonContent()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("VisualizationKey", "Visualization"))
							.TextStyle(&ToolBarStyle.LabelStyle)
						]
		#else
						.ButtonStyle(FEditorStyle::Get(), "ViewportMenu.Button")
						.ContentPadding(FEditorStyle::Get().GetMargin("ViewportMenu.SToolBarButtonBlock.Button.Padding"))
						.MenuPlacement(MenuPlacement_BelowAnchor)
						.OnGetMenuContent(FOnGetContent::CreateSP(this, &STouchInterfaceDesignerViewport::OnGetVisualizationMenuContent))
						.ButtonContent()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("VisualizationKey", "Visualization"))
							.TextStyle(FEditorStyle::Get(), "ViewportMenu.Label")
						]
		#endif

					]
					
					+SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SComboButton)
		#if ENGINE_MAJOR_VERSION > 4
						.ButtonStyle(&ToolBarStyle.ButtonStyle)
						.ContentPadding(ToolBarStyle.ButtonPadding)
						.MenuPlacement(MenuPlacement_BelowAnchor)
						.OnGetMenuContent(FOnGetContent::CreateSP(this, &STouchInterfaceDesignerViewport::OnGetScreenSizeMenuContent))
						.ButtonContent()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("ScreenSizeKey", "Screen Size"))
							.TextStyle(&ToolBarStyle.LabelStyle)
						]
		#else
						.ButtonStyle(FEditorStyle::Get(), "ViewportMenu.Button")
						.ContentPadding(FEditorStyle::Get().GetMargin("ViewportMenu.SToolBarButtonBlock.Button.Padding"))
						.MenuPlacement(MenuPlacement_BelowAnchor)
						.OnGetMenuContent(FOnGetContent::CreateSP(this, &STouchInterfaceDesignerViewport::OnGetScreenSizeMenuContent))
						.ButtonContent()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("ScreenSizeKey", "Screen Size"))
							.TextStyle(FEditorStyle::Get(), "ViewportMenu.Label")
						]
		#endif

					]

					+SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(FMargin(10.0f, 0.0f))
					[
						CreateCustomScreenSizeButtons()
					]
				]
			]

			// Device Profile Info
			+SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Bottom)
			.Padding(FMargin(5.0f, 0.0f, 0.0f, 5.0f))
			[
				SAssignNew(DeviceProfileInfo, SDeviceProfileInfo)
				.DefaultProfile(TEXT("Generic 16/9"))
			]
			
			+SOverlay::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.HAlign(HAlign_Right)
				.Padding(0,0,6,2)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Bottom)
					[
						SNew(STextBlock)
						.TextStyle(GetSlateStyle(), "Graph.ZoomText")
						.ColorAndOpacity(FSlateColor(FLinearColor(1,1,1,0.25f)))
						.Text(this, &STouchInterfaceDesignerViewport::GetScaleFactorAsText)
					]
					
					+SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(6,0,0,0)
					.VAlign(VAlign_Bottom)
					[
						//Todo: DPI Settings in TouchInterfaceDesigner Settings if scale mode == DPI
						SNew(SButton)
						.ButtonStyle(GetSlateStyle(), "HoverHintOnly")
						.ContentPadding(FMargin(3,1))
						.OnClicked(this, &STouchInterfaceDesignerViewport::HandleOnScaleSettingClicked)
						.ToolTipText(LOCTEXT("DPISettingsTooltip", "Configure the UI Scale Curve to Control how the UI is scaled on different resolution"))
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(SImage)
							.Image(GetSlateStyle().GetBrush("UMGEditor.DPISettings"))
						]
					]
				]
			]

			// Outline and text for important state.
			+ SOverlay::Slot()
			.Padding(0)
			.VAlign(VAlign_Fill)
			.HAlign(HAlign_Fill)
			[
				SNew(SOverlay)
				.Visibility(this, &STouchInterfaceDesignerViewport::GetDesignerOutlineVisibility)

				// Top-right corner text indicating PIE is active
				+ SOverlay::Slot()
				.Padding(0)
				.VAlign(VAlign_Fill)
				.HAlign(HAlign_Fill)
				[
					SNew(SImage)
					.ColorAndOpacity(FLinearColor(0.863f, 0.407, 0.0f))
					.Image(GetSlateStyle().GetBrush(TEXT("UMGEditor.DesignerMessageBorder")))
				]

				// Top-right corner text indicating PIE is active
				+ SOverlay::Slot()
				.Padding(20)
				.VAlign(VAlign_Top)
				.HAlign(HAlign_Right)
				[
					SNew(STextBlock)
					.TextStyle(GetSlateStyle(), "Graph.SimulatingText")
					.ColorAndOpacity(FLinearColor(0.863f, 0.407, 0.0f))
					.Text(LOCTEXT("SIMULATING", "SIMULATING"))
				]
			]
		]
	];

	BindCommands();
	
	RecenterHandle = RegisterActiveTimer(0.0f, FWidgetActiveTimerDelegate::CreateSP(this, &STouchInterfaceDesignerViewport::AutoRecenter));
	
	GenerateVirtualControlWidgets(VirtualControlSetupEdited->VirtualControls);
}

EActiveTimerReturnType STouchInterfaceDesignerViewport::AutoRecenter(double InCurrentTime, float InDeltaTime)
{
	const FVector2D ViewportSize = GetTickSpaceGeometry().GetLocalSize();

	if (ViewportSize == FVector2D::ZeroVector)
	{
		return EActiveTimerReturnType::Continue;
	}

	RecenterDesigner();
	return EActiveTimerReturnType::Stop;
}

void STouchInterfaceDesignerViewport::RecenterDesigner()
{
	// Check viewport size and determine the best scale for control canvas and place it in center of viewport
	const float MinSize = GetTickSpaceGeometry().GetLocalSize().GetMin()-100;
	const float PercentReduction = MinSize / DesignerSize.GetMax();
	const float NewScale = FMath::FloorToInt(PercentReduction * 10) / 10.0f;

	DesignerScale = NewScale;

	const FVector2D ViewportSize = GetTickSpaceGeometry().GetLocalSize();
	const FVector2D ScaledDesignerSize = FVector2D(DesignerSize) * NewScale;

	//FVector2D StartPanningOffset = FVector2D((ViewportSize.X - ScaledDesignerSize.X) /2, (ViewportSize.Y - ScaledDesignerSize.Y) /2 );
	const FVector2D StartPanningOffset = (ViewportSize - ScaledDesignerSize) /2;
	
	PanningOffset = -StartPanningOffset;
}

void STouchInterfaceDesignerViewport::GenerateVirtualControlWidgets(TArray<FVirtualControl> VirtualControls)
{
	for (const FVirtualControl& VirtualControl : VirtualControls)
	{
		const FVector2D CanvasPosition = NormalizedToCanvasSpace(VirtualControl.LandscapeCenter);
		TSharedPtr<SVirtualControlEditor> ConstructedControl = nullptr;
		// Construct virtual controls that are not child because they are constructed later
		if (!VirtualControl.bIsChild)
		{
			// Construct parent and normal control
			ConstructedControl = AddVirtualControlInCanvasSpace(VirtualControl, CanvasPosition);
		}

		// If he has children, build them
		if (VirtualControl.IsParent())
		{
			for (const FName ChildName : VirtualControl.Children)
			{
				FVirtualControl& ChildVirtualControl = VirtualControlSetupEdited->GetVirtualControlRef(ChildName);
				
				//const TSharedPtr<SVirtualControlEditor> ChildControl = AddVirtualControlInCanvasSpace(ChildVirtualControl, CanvasPosition + ChildVirtualControl.ParentOffset);
				const TSharedPtr<SVirtualControlEditor> ChildControl = AddVirtualControlInCanvasSpace(ChildVirtualControl, CanvasPosition + ChildVirtualControl.ParentOffset * CurrentScaleFactor);
				ChildControl->SetIsChild(true);
				ConstructedControl->AddChild(ChildName, ChildControl);
			}
		}
	}
}

TSharedPtr<SVirtualControlEditor> STouchInterfaceDesignerViewport::AddVirtualControlInCanvasSpace(const FVirtualControl& ControlAdded)
{
	const FVector2D CanvasPosition = NormalizedToCanvasSpace(IsInLandscapeOrientation() ? ControlAdded.LandscapeCenter : ControlAdded.PortraitCenter);
	return AddVirtualControlInCanvasSpace(ControlAdded, CanvasPosition);
}

TSharedPtr<SVirtualControlEditor> STouchInterfaceDesignerViewport::AddVirtualControlInCanvasSpace(const FVirtualControl& ControlAdded, const FVector2D Position)
{
	//check(Position != FVector2D::ZeroVector);
	
	TSharedPtr<SVirtualControlEditor> NewControl = nullptr;
	
	SConstraintCanvas::FSlot* ConstructedSlot = nullptr;
	DesignerSurface->AddSlot()
	.ZOrder(ControlAdded.Type == EControlType::TouchRegion ? 99 : 100)
	//Todo: Alignment and anchors can be removed because values == default values in FSlot
	.Alignment(FVector2D(0.5f,0.5f))
	.Anchors(FAnchors(0,0,0,0))
	.Expose(ConstructedSlot)
	.AutoSize(true)
	[
		SAssignNew(NewControl, SVirtualControlEditor,TouchDesignerEditorPtr.Pin())
		.VirtualControlData(ControlAdded)
		//.Slot(ConstructedSlot)
		.CanvasSize(this, &STouchInterfaceDesignerViewport::GetDesignerSize)
		.ScaleFactor(this, &STouchInterfaceDesignerViewport::GetScaleFactor)
		.LandscapeOrientation(this, &STouchInterfaceDesignerViewport::IsInLandscapeOrientation)
		.Opacity(this, &STouchInterfaceDesignerViewport::GetOpacityStateValue)
		.ShowText(this, &STouchInterfaceDesignerViewport::IsShowingControlName)
		.ShowDashedOutline(this, &STouchInterfaceDesignerViewport::IsShowingDashedOutline)
		.ShowInteractiveZone(this, &STouchInterfaceDesignerViewport::IsShowInteractiveZone)
		.ShowPressedState(this, &STouchInterfaceDesignerViewport::IsShowPressedState)
		.ShowParentLine(this, &STouchInterfaceDesignerViewport::IsShowParentingLine)
	];

	NewControl->SetCanvasSlot(ConstructedSlot);
	NewControl->SetPositionInCanvas(Position, false, false);
	ControlWidgets.Add(ControlAdded.ControlName, FControlWidget(NewControl, ConstructedSlot));
	return NewControl;
}

void STouchInterfaceDesignerViewport::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	//Recalculate at each frame the scaling factor to handle the changes made in the project settings
	CurrentScaleFactor = TouchInterfaceSettings->GetScaleFactor(FVector2D(DesignerSize));

	//Todo: If user link control with DesignSize at 1920 and after, change DesignSize to 1280, what's going on ? The OffsetFromParent should be recalculated ?
}

int32 STouchInterfaceDesignerViewport::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	//int32 MaxLayerId = LayerId;
	const FSlateBrush* BackgroundImage = GetSlateStyle().GetBrush(TEXT("Graph.Panel.SolidBackground")); //"Graph.DelegatePin.Connected"
	PaintViewportBackground(BackgroundImage, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId);

	//const FGeometry DesignerGeometry = DesignerSurface->GetTickSpaceGeometry();
	
	// Create Designer Surface Geometry to allow you to draw control and background easily
	const FGeometry DesignerSurfaceGeometry = AllottedGeometry.MakeChild
	(
		FVector2D(DesignerSize),
		FSlateLayoutTransform(DesignerScale, -PanningOffset + FVector2D(18.0f)/** Offset from rules */)
	);

	// Create Designer Surface Rect for same reason above
	const FSlateRect DesignerSurfaceRect = FSlateRect(FVector2D::ZeroVector, FVector2D(DesignerSize));

	PaintDeviceMockup(AllottedGeometry, MyCullingRect, OutDrawElements, LayerId);
	
	PaintDesignerBackground(DesignerSurfaceGeometry, DesignerSurfaceRect, OutDrawElements, LayerId, InWidgetStyle);

	//PaintInterfaceControls(DesignerSurfaceGeometry, DesignerSurfaceRect, OutDrawElements, LayerId);
	
	DrawSelectionAndHoverOutline(DesignerSurfaceGeometry, DesignerSurfaceRect, OutDrawElements, LayerId);

	PaintLinkedVirtualControl(DesignerSurfaceGeometry, DesignerSurfaceRect, OutDrawElements, LayerId);
	
	return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

void STouchInterfaceDesignerViewport::PaintViewportBackground(const FSlateBrush* BackgroundImage, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32& DrawLayerId) const
{
	const int32 RulePeriod = (int32)GetSlateStyle().GetFloat("Graph.Panel.GridRulePeriod");
	check(RulePeriod > 0);

	const FLinearColor RegularColor(GetSlateStyle().GetColor("Graph.Panel.GridLineColor"));
	const FLinearColor RuleColor(GetSlateStyle().GetColor("Graph.Panel.GridRuleColor"));
	//const FLinearColor CenterColor(FEditorStyle::GetColor("Graph.Panel.GridCenterColor"));
	
	const float GraphSmallestGridSize = 8.0f;
	const float RawZoomFactor = DesignerScale;
	const float NominalGridSize = 4.0f;

	float ZoomFactor = RawZoomFactor;
	float Inflation = 1.0f;
	while (ZoomFactor*Inflation*NominalGridSize <= GraphSmallestGridSize)
	{
		Inflation *= 2.0f;
	}

	const float GridCellSize = NominalGridSize * ZoomFactor * Inflation;

	FVector2D LocalGridOrigin = -PanningOffset;

	float ImageOffsetX = LocalGridOrigin.X - ((GridCellSize * RulePeriod) * FMath::Max(FMath::CeilToInt(LocalGridOrigin.X / (GridCellSize*RulePeriod)), 0));
	float ImageOffsetY = LocalGridOrigin.Y - ((GridCellSize * RulePeriod) * FMath::Max(FMath::CeilToInt(LocalGridOrigin.Y / (GridCellSize * RulePeriod)), 0));

	// Draw Background
	FSlateDrawElement::MakeBox(OutDrawElements, DrawLayerId, AllottedGeometry.ToPaintGeometry(), BackgroundImage);

	TArray<FVector2D> LinePoints;
	new (LinePoints) FVector2D(0,0);
	new (LinePoints) FVector2D(0,0);

	// Horizontal Lines	
	for (int32 GridIndex = 0; ImageOffsetY < AllottedGeometry.GetLocalSize().Y; ImageOffsetY += GridCellSize, ++GridIndex)
	{
		if (ImageOffsetY >= 0.0f)
		{
			// Check with modulo if line is rule line (true if equal to 0)
			const bool bIsRuleLine = GridIndex % RulePeriod == 0;

			// Make sure rule lines are drawn above regular lines
			const int32 Layer = bIsRuleLine ? DrawLayerId + 1 : DrawLayerId;

			// Color (use pointer for optimization)
			const FLinearColor* Color = bIsRuleLine ? &RuleColor : &RegularColor;

			// Center line color (disable for now)
			/*if (FMath::IsNearlyEqual(LocalGridOrigin.Y, ImageOffsetY, 1.0f))
			{
				Color = &CenterColor;
			}*/

			// Start & end location of current line
			LinePoints[0] = FVector2D(0.0f, ImageOffsetY);
			LinePoints[1] = FVector2D(AllottedGeometry.GetLocalSize().X, ImageOffsetY);

			FSlateDrawElement::MakeLines(OutDrawElements, Layer, AllottedGeometry.ToPaintGeometry(), LinePoints, ESlateDrawEffect::None, *Color, false);
		}
	}

	// Vertical Lines
	for (int32 GridIndex = 0; ImageOffsetX < AllottedGeometry.GetLocalSize().X; ImageOffsetX += GridCellSize, ++GridIndex)
	{
		if (ImageOffsetX >= 0.0f)
		{
			const bool bIsRuleLine = GridIndex % RulePeriod == 0;
			const FLinearColor* Color = bIsRuleLine ? &RuleColor : &RegularColor;

			/*if (FMath::IsNearlyEqual(LocalGridOrigin.Y, ImageOffsetY, 1.0f))
			{
				Color = &CenterColor;
			}*/
			
			LinePoints[0] = FVector2D(ImageOffsetX, 0);
			LinePoints[1] = FVector2D(ImageOffsetX, AllottedGeometry.GetLocalSize().Y);

			FSlateDrawElement::MakeLines(OutDrawElements, DrawLayerId, AllottedGeometry.ToPaintGeometry(), LinePoints, ESlateDrawEffect::None, *Color, false);
		}
	}
}

void STouchInterfaceDesignerViewport::PaintDeviceMockup(const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId) const
{
	
	// Draw smartphone mockup
	if (VirtualControlSetupEdited->BackgroundSettings.bEnableDeviceMockup && !bDesignerHasCustomSize)
	{		
		FString BrushName = CurrentProfile.StyleName;
		const float BrushScale = DesignerScale;
		//const FVector2D MockupOffset = bIsInLandscapeMode ? FVector2D(300.0f, 2716) : FVector2D(300.0f);
		const FVector2D MockupOffset = bIsInLandscapeMode ? FVector2D(300.0f, -1380.0f) : FVector2D(300.0f);
		const FVector2D Translation = (-PanningOffset + FVector2D(18.0f) /** Offset from rules */) - (MockupOffset * BrushScale);

		const float MockupRotation = bIsInLandscapeMode ? FMath::DegreesToRadians(-90.0f) : 0.0f;
		
		FSlateDrawElement::MakeBox
		(
			OutDrawElements,
			LayerId+3, //2
			AllottedGeometry.ToPaintGeometry(FVector2D(1536, 4096), FSlateLayoutTransform(BrushScale, Translation), FSlateRenderTransform(FQuat2D(MockupRotation)), FVector2D(0.0f)),
			FDesignerDeviceStyle::Get().GetBrush(FName(BrushName))
		);
	}
}

void STouchInterfaceDesignerViewport::PaintDesignerBackground(const FGeometry& DesignerGeometry, const FSlateRect& DesignerCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const
{
	const FSlateBrush* FillColorBrush = GetSlateStyle().GetBrush("WhiteTexture");
	const FLinearColor BackgroundColor = VirtualControlSetupEdited->BackgroundSettings.FillColor;
	
	FSlateDrawElement::MakeBox
	(
		OutDrawElements,
		LayerId+2, //1
		DesignerGeometry.ToPaintGeometry(),
		FillColorBrush,
		ESlateDrawEffect::None,
		BackgroundColor
	);

	/*FSlateDrawElement::MakeBox
	(
		OutDrawElements,
		LayerId+2, //1
		DesignerGeometry.ToPaintGeometry(),
		FVirtualControlDesignerEditorStyle::Get().GetBrush("BlankBrush"),
		ESlateDrawEffect::None,
		BackgroundColor
	);*/
	
	const FSlateBrush* BackgroundImage = &VirtualControlSetupEdited->BackgroundSettings.Image;
	const FVector2D BackgroundImageSize = BackgroundImage->ImageSize;
	const FLinearColor BackgroundImageColor = BackgroundImage->GetTint(InWidgetStyle);
	
	float BackgroundImageScale = 1.0;
	
	if (VirtualControlSetupEdited->BackgroundSettings.bFill)
	{
		if (BackgroundImageSize.X > BackgroundImageSize.Y)
		{
			BackgroundImageScale = DesignerGeometry.GetLocalSize().X / BackgroundImageSize.X;
		}
		else
		{
			BackgroundImageScale = DesignerGeometry.GetLocalSize().Y / BackgroundImageSize.Y;
		}
	}

	const FVector2D BackgroundImageOffset = DesignerGeometry.GetLocalSize() * 0.5 - (BackgroundImageSize * 0.5 * BackgroundImageScale);

	FSlateDrawElement::MakeBox
	(
		OutDrawElements,
		LayerId+2, //1
		DesignerGeometry.ToPaintGeometry(BackgroundImageSize, FSlateLayoutTransform(BackgroundImageScale, BackgroundImageOffset)),
		BackgroundImage,
		ESlateDrawEffect::None,
		BackgroundImageColor
	);
}

void STouchInterfaceDesignerViewport::DrawSelectionAndHoverOutline(const FGeometry& DesignerSurfaceGeometry, const FSlateRect& DesignerSurfaceRect, FSlateWindowElementList& OutDrawElements, int32 LayerId) const
{
	if (bEnableOutline)
	{
		TArray<FVector2D> CanvasBorderPoints;
		CanvasBorderPoints.Add(DesignerSurfaceRect.GetTopLeft());
		CanvasBorderPoints.Add(DesignerSurfaceRect.GetTopRight());
		CanvasBorderPoints.Add(DesignerSurfaceRect.GetBottomRight());
		CanvasBorderPoints.Add(DesignerSurfaceRect.GetBottomLeft());
		CanvasBorderPoints.Add(DesignerSurfaceRect.GetTopLeft());
	
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId+4, DesignerSurfaceGeometry.ToPaintGeometry(), CanvasBorderPoints, ESlateDrawEffect::None, FLinearColor::Gray, false, 2.0f /*/ DesignerScale*/);
	}
	
	if (bEnableDashedOutline)
	{
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId+5, DesignerSurfaceGeometry.ToPaintGeometry(), GetSlateStyle().GetBrush("MarqueeSelection"));
	}
}

void STouchInterfaceDesignerViewport::PaintLinkedVirtualControl(const FGeometry& DesignerSurfaceGeometry, const FSlateRect& DesignerSurfaceRect, FSlateWindowElementList& OutDrawElements, int32 LayerId) const
{
	if (bWaitForParentSelection)
	{
		TArray<FVector2D> LinePoints;
		LinePoints.Add(WidgetSelected->GetPositionInCanvas());
		LinePoints.Add(AbsoluteToCanvasSpace(FSlateApplication::Get().GetCursorPos()));
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId, DesignerSurfaceGeometry.ToPaintGeometry(), LinePoints, ESlateDrawEffect::None, FLinearColor::Green, true, 2.0f);
	}

	if (bEnableParentingLine)
	{
		for (auto Itr = ControlWidgets.CreateConstIterator(); Itr; ++Itr)
		{
			if (Itr->Value.Widget->IsParent())
			{
				for (const FName ChildName : Itr->Value.Widget->GetChildName())
				{
					TArray<FVector2D> LinePoints;
					LinePoints.Add(Itr->Value.Widget->GetPositionInCanvas());
					LinePoints.Add(ControlWidgets.FindRef(ChildName).Widget->GetPositionInCanvas());
					FSlateDrawElement::MakeLines(OutDrawElements, LayerId, DesignerSurfaceGeometry.ToPaintGeometry(), LinePoints, ESlateDrawEffect::None, FLinearColor::Gray, true, 2.0f);
				}
			}
		}
	}
}

FReply STouchInterfaceDesignerViewport::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	PreviousDesignerScale = DesignerScale;

	//Todo: Reduce WheelDelta amount
	DesignerScale = FMath::Clamp(DesignerScale + MouseEvent.GetWheelDelta() * 0.1f, 0.1f, 2.0f);

	FVector2D CurrentCanvasPosition = MyGeometry.AbsoluteToLocal(DesignerSurface->GetTickSpaceGeometry().GetAbsolutePosition());

	FGeometry GeometryBeforeScale = MyGeometry.MakeChild(FVector2D(DesignerSize),FSlateLayoutTransform(PreviousDesignerScale, CurrentCanvasPosition));
	FVector2D MouseRelativePosition = GeometryBeforeScale.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());

	FGeometry GeometryAfterScale = MyGeometry.MakeChild(FVector2D(DesignerSize), FSlateLayoutTransform(DesignerScale, CurrentCanvasPosition));
	FVector2D MouseRelativePositionScaled = GeometryAfterScale.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());

	FVector2D Distance = MouseRelativePositionScaled - MouseRelativePosition;

	PanningOffset -= Distance * DesignerScale;
	
// 	if(GetDefault<UVirtualControlDesignerEditorSettings>()->bZoomToPointerPosition)
// 	{
// 		FVector2D CurrentCanvasPosition = MyGeometry.AbsoluteToLocal(DesignerSurface->GetTickSpaceGeometry().GetAbsolutePosition());
//
// 		FGeometry GeometryBeforeScale = MyGeometry.MakeChild(FVector2D(DesignerSize),FSlateLayoutTransform(PreviousDesignerScale, CurrentCanvasPosition));
// 		FVector2D MouseRelativePosition = GeometryBeforeScale.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
//
// 		FGeometry GeometryAfterScale = MyGeometry.MakeChild(FVector2D(DesignerSize), FSlateLayoutTransform(DesignerScale, CurrentCanvasPosition));
// 		FVector2D MouseRelativePositionScaled = GeometryAfterScale.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
//
// 		FVector2D Distance = MouseRelativePositionScaled - MouseRelativePosition;
//
// 		PanningOffset -= Distance * DesignerScale;
// 	}
// 	else
// 	{
// 		//Todo: Other Zoom Algorithm. Keep the same fraction offset into the panel (see SDesignSurface line 304)
// 		//FVector2D NewCanvasSize = FVector2D(DesignerSize) * DesignerScale;
// 		//FVector2D CurrentPanning = PanningOffset;
// 		
// 		//FVector2D CanvasPosition = MyGeometry.AbsoluteToLocal(ControlCanvas->GetTickSpaceGeometry().GetAbsolutePosition());
// 		//FVector2D MousePosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
// 			
// 		/*FVector2D MousePositionInsideCanvas = ControlCanvas->GetTickSpaceGeometry().AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
// 		//GLog->Log("Position Inside Canvas = " + MousePositionInsideCanvas.ToString());
//
// 		FVector2D MousePositionPercentage = FVector2D(100 - ((MousePositionInsideCanvas.X * 100) / DesignerSize.X), 100 - ((MousePositionInsideCanvas.Y * 100) / DesignerSize.Y));
// 		//GLog->Log("Percent = " + MousePositionPercentage.ToString());
//
// 		FVector2D MousePositionInsideInflatedCanvas = FVector2D(100 - ((MousePositionInsideCanvas.X * 100) / NewCanvasSize.X), 100 - ((MousePositionInsideCanvas.Y * 100) / NewCanvasSize.Y));
//
// 		FVector2D PercentToAddToPanning = MousePositionInsideInflatedCanvas - MousePositionPercentage;
// 		//GLog->Log("Add Percent = " + PercentToAddToPanning.ToString());
//
// 		PanningOffset.X += NewCanvasSize.X * (PercentToAddToPanning.X / 100);
// 		PanningOffset.Y += NewCanvasSize.Y * (PercentToAddToPanning.Y / 100);*/
// 	}
	
	return FReply::Handled();
}

FReply STouchInterfaceDesignerViewport::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (bWaitForParentSelection)
	{
		if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			TSharedPtr<SVirtualControlEditor> FoundWidget = GetControlWidgetHovered();
			if (FoundWidget)
			{
				if (!FoundWidget->IsChild())
				{
					HandleApplyLinkControl(FoundWidget.Get()->GetControlName());
					return FReply::Handled();
				}
			}
			
			HandleCancelLinkControl();
			return FReply::Unhandled();
		}
	}
	else
	{
		if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			// if any widget is selected, so unselect
			if (WidgetSelected.IsValid())
			{
				WidgetSelected->SetIsSelected(false);
				TouchDesignerEditorPtr.Pin()->UnselectControl();
				WidgetSelected = nullptr;
				OldWidgetSelected = nullptr;
			}
			
			if (GetControlWidgetHovered(WidgetSelected))
			{
				if (TouchInterfaceDesignerSettings->bKeepCursorOffset)
				{
					const FVector2D ControlCenterPosition = WidgetSelected->GetTickSpaceGeometry().GetAbsolutePosition() + WidgetSelected->GetTickSpaceGeometry().GetLocalSize() * 0.5f * DesignerScale;
					CursorOffset = MouseEvent.GetScreenSpacePosition() - ControlCenterPosition;
				}
				else
				{
					CursorOffset = FVector2D(0.0f, 0.0f);
				}
			
				TouchDesignerEditorPtr.Pin()->SelectControl(WidgetSelected->GetControlName());

				//Todo: move this in SelectControl() from TouchDesignerEditor ?
				WidgetSelected->SetIsSelected(true);
				OldWidgetSelected = WidgetSelected;
			
				// Use detect drag to make sure that user want to move widget and not only select
				return FReply::Handled().DetectDrag(SharedThis(this), EKeys::LeftMouseButton);
			}
		}
	
		if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
		{
			return FReply::Handled().DetectDrag(SharedThis(this),EKeys::RightMouseButton);
		}
	}
	
	return FReply::Unhandled();
}

FReply STouchInterfaceDesignerViewport::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		bDragControl = true;
		WidgetSelected->SetIsMoving(true);
		
		//Use CaptureMouse to avoid issue when pointer hover other widget when it drag
		return FReply::Handled().CaptureMouse(SharedThis(this));
	}

	if (MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
	{
		bIsPanning = true;
		PanningOffsetStart = PanningOffset;
		AbsoluteMousePositionStart = MyGeometry.AbsoluteToLocal(MouseEvent.GetLastScreenSpacePosition());
		
		//Use CaptureMouse to avoid issue when pointer hover other widget when it drag
		return FReply::Handled().CaptureMouse(AsShared());
	}

	return FReply::Handled();
}

FReply STouchInterfaceDesignerViewport::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FReply Reply = FReply::Unhandled();

	if (bIsPanning)
	{
		//PanningOffset -= MouseEvent.GetCursorDelta();

		PanningOffset = PanningOffsetStart + (AbsoluteMousePositionStart - MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()));
		
		Reply = FReply::Handled();
	}
	if (bDragControl)
	{
		FVector2D CanvasPosition = DesignerSurface->GetTickSpaceGeometry().AbsoluteToLocal(MouseEvent.GetScreenSpacePosition() - CursorOffset);
		WidgetSelected->SetPositionInCanvas(CanvasPosition, false, true);
		//TouchDesignerEditorPtr.Pin()->ForceRefresh(); // Bad Performance
		Reply = FReply::Handled();
	}

	return Reply;
}

FReply STouchInterfaceDesignerViewport::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (bDragControl)
		{
			bDragControl = false;
			WidgetSelected->SetIsMoving(false);
			
			FVector2D CanvasPosition = DesignerSurface->GetTickSpaceGeometry().AbsoluteToLocal(MouseEvent.GetScreenSpacePosition() - CursorOffset);
			TouchDesignerEditorPtr.Pin()->NotifyControlPositionChanged(WidgetSelected->GetControlName(), CanvasPosition);

			return FReply::Handled().ReleaseMouseCapture();
		}
	}

	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		FReply Reply = FReply::Handled();
		
		if (bIsPanning)
		{
			bIsPanning = false;
			return FReply::Handled().ReleaseMouseCapture();
		}
		
		if (GetControlWidgetHovered(WidgetSelected))
		{
			if (OldWidgetSelected.IsValid() && WidgetSelected != OldWidgetSelected)
			{
				OldWidgetSelected->SetIsSelected(false);
				TouchDesignerEditorPtr.Pin()->UnselectControl();
			}
			
			//Todo: Check if WidgetSelected == OldSelectedWidget in TouchDesignerEditor because it refresh detail panel each time
			TouchDesignerEditorPtr.Pin()->SelectControl(WidgetSelected->GetControlName());
			WidgetSelected->SetIsSelected(true);
			OldWidgetSelected = WidgetSelected;

			//Todo: Get CommandList in TouchDesignerEditor ?
			FMenuBuilder MenuBuilder(true,nullptr);

			FUIAction CutControlAction(FExecuteAction::CreateSP(TouchDesignerEditorPtr.Pin().ToSharedRef(), &FVirtualControlDesignerEditor::CutSelected));
			FUIAction CopyControlAction(FExecuteAction::CreateSP(TouchDesignerEditorPtr.Pin().ToSharedRef(), &FVirtualControlDesignerEditor::CopySelected));
			FUIAction DuplicateControlAction(FExecuteAction::CreateSP(TouchDesignerEditorPtr.Pin().ToSharedRef(), &FVirtualControlDesignerEditor::DuplicateSelected));
			FUIAction DeleteAction(FExecuteAction::CreateSP(TouchDesignerEditorPtr.Pin().ToSharedRef(), &FVirtualControlDesignerEditor::RemoveSelected));

			FUIAction LinkAction(FExecuteAction::CreateSP(this, &STouchInterfaceDesignerViewport::HandleLinkControl));
			FUIAction UnLinkAction(FExecuteAction::CreateSP(this, &STouchInterfaceDesignerViewport::HandleUnLinkControl));
			FUIAction UnLinkAllAction(FExecuteAction::CreateSP(this, &STouchInterfaceDesignerViewport::HandleUnLinkAllControl));

			//Todo: Snap to nearest round position

			MenuBuilder.BeginSection(NAME_None, LOCTEXT("ControlMenuBuilder", "Control"));
			MenuBuilder.AddMenuEntry(LOCTEXT("Cut", "Cut"),LOCTEXT("Cut control", "Cut Control"), FSlateIcon(), CutControlAction);
			MenuBuilder.AddMenuEntry(LOCTEXT("Copy", "Copy"),LOCTEXT("Copy control", "Copy Control"), FSlateIcon(), CopyControlAction);
			MenuBuilder.AddMenuEntry(LOCTEXT("Duplicate", "Duplicate"),LOCTEXT("Duplicate control", "Duplicate Control"), FSlateIcon(), DuplicateControlAction);
			MenuBuilder.AddMenuEntry(LOCTEXT("Delete", "Delete"),LOCTEXT("Delete control", "Delete Control"), FSlateIcon(), DeleteAction);

			if (WidgetSelected.Get()->IsParent())
			{
				MenuBuilder.AddMenuEntry(INVTEXT("Unlink All"),INVTEXT("Unlink all child from parent"), FSlateIcon(), UnLinkAllAction);
			}
			else if (WidgetSelected.Get()->IsChild())
			{
				MenuBuilder.AddMenuEntry(INVTEXT("Unlink"),INVTEXT("Unlink child from parent"), FSlateIcon(), UnLinkAction);
			}
			else
			{
				MenuBuilder.AddMenuEntry(INVTEXT("Link"),INVTEXT("Link this control to other"), FSlateIcon(), LinkAction);
			}
			
			MenuBuilder.EndSection();

			FWidgetPath WidgetPath = MouseEvent.GetEventPath() != nullptr ? *MouseEvent.GetEventPath() : FWidgetPath();
			FSlateApplication::Get().PushMenu(AsShared(), WidgetPath, MenuBuilder.MakeWidget(), MouseEvent.GetScreenSpacePosition(), FPopupTransitionEffect::ContextMenu);
			
			return FReply::Handled();
		}

		if (DesignerSurface->IsHovered())
		{
			FMenuBuilder MenuBuilder(true, nullptr);

			const FVector2D CanvasPosition = AbsoluteToCanvasSpace(MouseEvent.GetScreenSpacePosition());
			
			FUIAction AddNewJoystickAction(FExecuteAction::CreateSP(TouchDesignerEditorPtr.Pin().ToSharedRef(), &FVirtualControlDesignerEditor::AddNewControl, EControlType::Joystick, CanvasPosition));
			FUIAction AddNewButtonAction(FExecuteAction::CreateSP(TouchDesignerEditorPtr.Pin().ToSharedRef(), &FVirtualControlDesignerEditor::AddNewControl, EControlType::Button, CanvasPosition));
			FUIAction AddNewTouchRegionAction(FExecuteAction::CreateSP(TouchDesignerEditorPtr.Pin().ToSharedRef(), &FVirtualControlDesignerEditor::AddNewControl, EControlType::TouchRegion, CanvasPosition));
			FUIAction PasteControlAction(FExecuteAction::CreateSP(TouchDesignerEditorPtr.Pin().ToSharedRef(), &FVirtualControlDesignerEditor::PasteCopiedControl), FCanExecuteAction::CreateSP(TouchDesignerEditorPtr.Pin().ToSharedRef(), &FVirtualControlDesignerEditor::IsCopyAvailable));

			MenuBuilder.BeginSection(NAME_None, LOCTEXT("CanvasMenuBuilder", "Add control"));
			MenuBuilder.AddMenuEntry(LOCTEXT("AddJoystickLabel", "Add Joystick"), LOCTEXT("AddJoystickToolTip", "Add new joystick control"), FSlateIcon(), AddNewJoystickAction);
			MenuBuilder.AddMenuEntry(LOCTEXT("AddButtonLabel", "Add Button"), LOCTEXT("AddButtonToolTip", "Add new Button control"), FSlateIcon(), AddNewButtonAction);
			MenuBuilder.AddMenuEntry(INVTEXT("Add Touch Region"), INVTEXT("Add new Touch Region"), FSlateIcon(), AddNewTouchRegionAction);
			MenuBuilder.AddMenuEntry(LOCTEXT("Paste", "Paste"),LOCTEXT("Paste control", "Paste Control"), FSlateIcon(), PasteControlAction);
			MenuBuilder.EndSection();

			FWidgetPath WidgetPath = MouseEvent.GetEventPath() != nullptr ? *MouseEvent.GetEventPath() : FWidgetPath();
			FSlateApplication::Get().PushMenu(AsShared(), WidgetPath, MenuBuilder.MakeWidget(), MouseEvent.GetScreenSpacePosition(), FPopupTransitionEffect::ContextMenu);
			
			return FReply::Handled();
		}

		//Possible action for viewport here
		
		return FReply::Handled();
	}
	
	return FReply::Unhandled();
}

void STouchInterfaceDesignerViewport::OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	const TSharedPtr<FPaletteItemDragDropOp> DragDropOp = DragDropEvent.GetOperationAs<FPaletteItemDragDropOp>();

	if (DragDropOp.IsValid())
	{
		DesignerSurface->AddSlot()
		.Expose(DraggedWidgetSlot)
		.AutoSize(true)
		[
			SAssignNew(DraggedWidgetInstance, SPaletteItemDraggedWidget)
			.Type(DragDropOp->GetControlType())
			.ScaleFactor(this, &STouchInterfaceDesignerViewport::GetScaleFactor)
			.Visibility(EVisibility::Hidden)
		];
	}
	
	SCompoundWidget::OnDragEnter(MyGeometry, DragDropEvent);
}

void STouchInterfaceDesignerViewport::OnDragLeave(const FDragDropEvent& DragDropEvent)
{
	const TSharedPtr<FPaletteItemDragDropOp> DragDropOp = DragDropEvent.GetOperationAs<FPaletteItemDragDropOp>();

	if (DragDropOp.IsValid())
	{
		DragDropOp->SetCursor(EMouseCursor::GrabHandClosed);

		if (DraggedWidgetInstance.IsValid())
		{
			DesignerSurface->RemoveSlot(DraggedWidgetInstance.ToSharedRef());
			DraggedWidgetInstance = nullptr;
		}
	}
	
	SCompoundWidget::OnDragLeave(DragDropEvent);
}

FReply STouchInterfaceDesignerViewport::OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{	
	const TSharedPtr<FPaletteItemDragDropOp> DragDropOp = DragDropEvent.GetOperationAs<FPaletteItemDragDropOp>();

	if (DragDropOp.IsValid())
	{
		if (DesignerSurface->GetTickSpaceGeometry().IsUnderLocation(DragDropOp->GetDecoratorPosition()))
		{
			DragDropOp->SetCursor(EMouseCursor::GrabHandClosed);
			if (DraggedWidgetInstance.IsValid())
			{
				DraggedWidgetInstance->SetVisibility(EVisibility::HitTestInvisible);
#if ENGINE_MAJOR_VERSION > 4
				FMargin CurrentMargin = DraggedWidgetSlot->GetOffset();
#else
				FMargin CurrentMargin = DraggedWidgetSlot->OffsetAttr.Get();
#endif
				
				const FVector2D CurrentPosition = AbsoluteToCanvasSpace(DragDropOp->GetDecoratorPosition());
				CurrentMargin.Left = CurrentPosition.X;
				CurrentMargin.Top = CurrentPosition.Y;
#if ENGINE_MAJOR_VERSION > 4
				DraggedWidgetSlot->SetOffset(CurrentMargin);
#else
				DraggedWidgetSlot->Offset(CurrentMargin);
#endif
				
			}
		}
		else
		{
			DragDropOp->SetCursor(EMouseCursor::SlashedCircle);
			DraggedWidgetInstance->SetVisibility(EVisibility::Hidden);
		}
	}
	
	return SCompoundWidget::OnDragOver(MyGeometry, DragDropEvent);
}

FReply STouchInterfaceDesignerViewport::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	const TSharedPtr<FPaletteItemDragDropOp> DragDropOp = DragDropEvent.GetOperationAs<FPaletteItemDragDropOp>();

	if (DragDropOp.IsValid())
	{
		if (DraggedWidgetInstance.IsValid())
		{
			DesignerSurface->RemoveSlot(DraggedWidgetInstance.ToSharedRef());
			DraggedWidgetInstance = nullptr;
		}
	
		if (DesignerSurface->GetTickSpaceGeometry().IsUnderLocation(DragDropOp->GetDecoratorPosition()))
		{
			const FVector2D CanvasPosition = AbsoluteToCanvasSpace(DragDropOp->GetDecoratorPosition());
		
			switch (DragDropOp->GetControlType())
			{
			case EControlType::Button:
				TouchDesignerEditorPtr.Pin()->AddNewControl(EControlType::Button, CanvasPosition);
				break;
			case EControlType::Joystick:
				TouchDesignerEditorPtr.Pin()->AddNewControl(EControlType::Joystick, CanvasPosition);
				break;
			case EControlType::TouchRegion:
				TouchDesignerEditorPtr.Pin()->AddNewControl(EControlType::TouchRegion, CanvasPosition);
				break;
			default:
				break;
			}
			return FReply::Handled();
		}
	}
	
	return  FReply::Unhandled();	
}

FCursorReply STouchInterfaceDesignerViewport::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const
{	
	if (bIsPanning)
	{
		return FCursorReply::Cursor(EMouseCursor::GrabHand);
	}
	
	if (bDragControl)
	{
		return FCursorReply::Cursor(EMouseCursor::CardinalCross);
	}
	
	return FCursorReply::Cursor(EMouseCursor::Default);
}

void STouchInterfaceDesignerViewport::SelectWidget(const FName ControlName)
{
	if(WidgetSelected.IsValid())
	{
		WidgetSelected->SetIsSelected(false);
		WidgetSelected = nullptr;
	}
	
	if (ControlWidgets.Contains(ControlName))
	{
		WidgetSelected = ControlWidgets.FindRef(ControlName).Widget;
		WidgetSelected->SetIsSelected(true);
		OldWidgetSelected = WidgetSelected;
	}
}

void STouchInterfaceDesignerViewport::HoverWidget(const FName ControlName, bool Enable)
{
	if (WidgetHovered.IsValid())
	{
		WidgetHovered->SetIsHovered(false);
	}
	
	if (ControlWidgets.Contains(ControlName))
	{
		WidgetHovered = ControlWidgets.FindRef(ControlName).Widget;
		WidgetHovered->SetIsHovered(Enable);
	}
}

void STouchInterfaceDesignerViewport::ClearOutlineEffect() const
{
	if (WidgetSelected.IsValid())
	{
		WidgetSelected->SetIsSelected(false);
	}
	if (WidgetHovered.IsValid())
	{
		WidgetHovered->SetIsHovered(false);
	}
}

void STouchInterfaceDesignerViewport::Link(const FName ParentControl, const FName ChildControl, const FVector2D OffsetFromParent) const
{	
	const TSharedPtr<SVirtualControlEditor> ChildWidget = ControlWidgets.FindRef(ChildControl).Widget;
	ChildWidget->SetIsChild(true);
	ChildWidget->SetOffsetFromParent(OffsetFromParent, false);
	
	ControlWidgets.FindRef(ParentControl).Widget->AddChild(ChildControl, ChildWidget);
}

void STouchInterfaceDesignerViewport::Unlink(const FName ParentControl, const FName ChildControl) const
{
	ControlWidgets.FindRef(ParentControl).Widget->RemoveChild(ChildControl);
	ControlWidgets.FindRef(ChildControl).Widget->SetIsChild(false);
}

void STouchInterfaceDesignerViewport::UpdateSelectedControl(const FVirtualControl ControlData) const
{
	const FControlWidget MyControlWidget = ControlWidgets.FindRef(ControlData.ControlName);
	MyControlWidget.Widget->Update(ControlData, true);
}

void STouchInterfaceDesignerViewport::ChangeOpacity(const float Active, const float Inactive)
{
	ActiveOpacity = Active;
	InactiveOpacity = Inactive;
	CurrentOpacity = bOpacityInActiveState ? Active : Inactive;
}

void STouchInterfaceDesignerViewport::ChangeControlName(const FName ControlName, const FName NewName, const FVirtualControl& UpdatedData)
{
	if (ControlWidgets.Contains(ControlName))
	{
		const FControlWidget ControlStruct = ControlWidgets.FindAndRemoveChecked(ControlName);
		ControlWidgets.Add(NewName, ControlStruct);
		ControlStruct.Widget->Update(UpdatedData, true);

		if (UpdatedData.bIsChild)
		{
			const FControlWidget ParentControlStruct = ControlWidgets.FindRef(UpdatedData.ParentName);
			ParentControlStruct.Widget->RenameChild(ControlName, NewName);
		}
	}
}

FVector2D STouchInterfaceDesignerViewport::GetVirtualControlPosition(const FName ControlName, const bool InNormalized) const
{
	check(ControlWidgets.Contains(ControlName))

	const FVector2D CurrentPosition = ControlWidgets.FindRef(ControlName).Widget->GetPositionInCanvas();

	return InNormalized ? CanvasSpaceToNormalized(CurrentPosition) : CurrentPosition;
}

FVector2D STouchInterfaceDesignerViewport::CanvasSpaceToNormalized(const FVector2D CanvasSpacePosition) const
{
	const FVector2D CurrentDesignerSize(DesignerSize);

	const float X = FMath::Clamp(CanvasSpacePosition.X / CurrentDesignerSize.X, 0.0f, CurrentDesignerSize.X);
	const float Y = FMath::Clamp(CanvasSpacePosition.Y / CurrentDesignerSize.Y, 0.0f, CurrentDesignerSize.Y);

	return FVector2D(X,Y);
}

FVector2D STouchInterfaceDesignerViewport::NormalizedToCanvasSpace(const FVector2D NormalizedPosition) const
{
	const FVector2D CurrentDesignerSize(DesignerSize);

	const float X = FMath::Clamp(NormalizedPosition.X, 0.0f, 1.0f) * CurrentDesignerSize.X;
	const float Y = FMath::Clamp(NormalizedPosition.Y, 0.0f,1.0f) * CurrentDesignerSize.Y;
	
	return FVector2D(X, Y);
}

FVector2D STouchInterfaceDesignerViewport::AbsoluteToCanvasSpace(const FVector2D ScreenSpacePosition) const
{
	return DesignerSurface->GetTickSpaceGeometry().AbsoluteToLocal(ScreenSpacePosition);
}

FVector2D STouchInterfaceDesignerViewport::AbsoluteToNormalize(const FVector2D ScreenSpacePosition) const
{
	return CanvasSpaceToNormalized(AbsoluteToCanvasSpace(ScreenSpacePosition));
}

FVector2D STouchInterfaceDesignerViewport::CalculateOffsetBetweenControl(const FName Parent, const FName Child) const
{
	FVector2D ParentScaledAbsolutePosition = FVector2D::ZeroVector;
	FVector2D ChildScaledAbsolutePosition = FVector2D::ZeroVector;

	for (const FVirtualControl& VirtualControl : VirtualControlSetupEdited->VirtualControls)
	{
		if (VirtualControl.ControlName == Parent)
		{
			ParentScaledAbsolutePosition = VirtualControl.LandscapeCenter * FVector2D(DesignerSize);
		}

		if (VirtualControl.ControlName == Child)
		{
			ChildScaledAbsolutePosition = VirtualControl.LandscapeCenter * FVector2D(DesignerSize);
		}
	}
	
	const FVector2D ScaledOffsetFromParent = ChildScaledAbsolutePosition - ParentScaledAbsolutePosition;
	UE_LOG(LogTemp, Verbose, TEXT("Scaled Offset from Parent = %f x %f"), ScaledOffsetFromParent.X, ScaledOffsetFromParent.Y);
	UE_LOG(LogTemp, Verbose, TEXT("UnScaled Offset = %f x %f"), ScaledOffsetFromParent.X / CurrentScaleFactor, ScaledOffsetFromParent.Y / CurrentScaleFactor);
	
	return ScaledOffsetFromParent / CurrentScaleFactor;
}

const FSlateBrush* STouchInterfaceDesignerViewport::GetOrientationModeBrush() const
{
	if (bIsInLandscapeMode)
	{
		return GetSlateStyle().GetBrush("UMGEditor.OrientLandscape");
	}

	return GetSlateStyle().GetBrush("UMGEditor.OrientPortrait");
}

TSharedRef<SWidget> STouchInterfaceDesignerViewport::MakeViewportToolbar() const
{
	FToolBarBuilder ToolBarBuilder(TouchDesignerEditorPtr.Pin()->GetCommands(), FMultiBoxCustomization::None);

#if ENGINE_MAJOR_VERSION > 4
	const FName ToolBarStyle = "EditorViewportToolBar";
	ToolBarBuilder.SetStyle(&FAppStyle::Get(), ToolBarStyle);
#else
	const FName ToolBarStyle = "ViewportMenu";
	ToolBarBuilder.SetStyle(&FEditorStyle::Get(), ToolBarStyle);
#endif
	
	ToolBarBuilder.SetLabelVisibility(EVisibility::Collapsed);

	ToolBarBuilder.BeginSection("View");
	ToolBarBuilder.BeginBlockGroup();
	ToolBarBuilder.AddToolBarButton(FVirtualControlDesignerCommands::Get().ToggleOutlineCommand, NAME_None, TAttribute<FText>(), TAttribute<FText>(), TAttribute<FSlateIcon>(), "ToggleDashedOutline");
	ToolBarBuilder.AddToolBarButton(FVirtualControlDesignerCommands::Get().ToggleOpacityCommand, NAME_None, TAttribute<FText>(), TAttribute<FText>(), TAttribute<FSlateIcon>(), "ToggleOpacity");
	ToolBarBuilder.AddToolBarButton(FVirtualControlDesignerCommands::Get().TogglePressedPreviewCommand, NAME_None, TAttribute<FText>(), TAttribute<FText>(), TAttribute<FSlateIcon>(), "TogglePressedPreview");
	ToolBarBuilder.AddToolBarButton(FVirtualControlDesignerCommands::Get().ToggleConstraintCommand, NAME_None, TAttribute<FText>(), TAttribute<FText>(), TAttribute<FSlateIcon>(), "ToggleConstraint");
	ToolBarBuilder.EndBlockGroup();
	ToolBarBuilder.EndSection();

	ToolBarBuilder.BeginSection("Orientation");
	// ToolBarBuilder.AddWidget(SNew(SButton)
	// 	.ButtonStyle(FEditorStyle::Get(), "ViewportMenu.Button")
	// 	.OnClicked(FOnClicked::CreateSP(this, &STouchDesignerEditorViewportWidget::ToggleOrientation))
	// 	[
	// 		SNew(SImage)
	// 		.Image(TAttribute<const FSlateBrush*>::Create(TAttribute<const FSlateBrush*>::FGetter::CreateSP(this, &STouchDesignerEditorViewportWidget::GetOrientationModeBrush)))
	// 	]
	// );
	ToolBarBuilder.AddToolBarButton(FVirtualControlDesignerCommands::Get().ToggleOrientationCommand, NAME_None, TAttribute<FText>(), TAttribute<FText>(), TAttribute<FSlateIcon>(), "ToggleOrientation");
	ToolBarBuilder.EndSection();

	return ToolBarBuilder.MakeWidget();
}

void STouchInterfaceDesignerViewport::ToggleOpacityState()
{
	bOpacityInActiveState = !bOpacityInActiveState;
	CurrentOpacity = bOpacityInActiveState ? ActiveOpacity : InactiveOpacity;
}

void STouchInterfaceDesignerViewport::ToggleDashedOutline()
{
	bEnableDashedOutline = !bEnableDashedOutline;
}

void STouchInterfaceDesignerViewport::ToggleOrientation()
{
	int32 OldX = DesignerSize.X;
	
	DesignerSize.X = DesignerSize.Y;
	DesignerSize.Y = OldX;

	bIsInLandscapeMode = !bIsInLandscapeMode;

	TouchDesignerEditorPtr.Pin()->NotifyOrientationChanged(bIsInLandscapeMode);
	
	TArray<FControlWidget> OutControlWidgets;
	ControlWidgets.GenerateValueArray(OutControlWidgets);
	
	// Update All control (position, visual, etc)
	for (const FControlWidget& Widget : OutControlWidgets)
	{
		Widget.Widget->Refresh();
	}
}

void STouchInterfaceDesignerViewport::ToggleConstraint()
{
	bEnableConstraintControl = !bEnableConstraintControl;
}

void STouchInterfaceDesignerViewport::TogglePressedState()
{
	bShowControlPressedState = !bShowControlPressedState;
}

TSharedRef<SWidget> STouchInterfaceDesignerViewport::OnGetVisualizationMenuContent()
{
	FMenuBuilder MenuBuilder(false, nullptr);

	// Outline Entry
	FUIAction OutlineAction(FExecuteAction::CreateRaw(this, &STouchInterfaceDesignerViewport::ToggleOutline), FCanExecuteAction(), FIsActionChecked::CreateLambda([this](){return bEnableOutline;}));
	MenuBuilder.AddMenuEntry(LOCTEXT("ShowOutlineKey", "Outline"),FText::GetEmpty(), FSlateIcon(), OutlineAction, NAME_None, EUserInterfaceActionType::Check);

	// Control Name Entry
	FUIAction ControlNameAction(FExecuteAction::CreateRaw(this, &STouchInterfaceDesignerViewport::ToggleText), FCanExecuteAction(), FIsActionChecked::CreateLambda([this](){return bEnableText;}));
	MenuBuilder.AddMenuEntry(LOCTEXT("ShowTextKey", "Control Name"),FText::GetEmpty(), FSlateIcon(), ControlNameAction, NAME_None, EUserInterfaceActionType::Check);

	// Interaction Size Entry
	FUIAction InteractionSizeAction(FExecuteAction::CreateRaw(this, &STouchInterfaceDesignerViewport::ToggleInteractionOutline), FCanExecuteAction(), FIsActionChecked::CreateLambda([this](){return bEnableInteractionOutline;}));
	MenuBuilder.AddMenuEntry(LOCTEXT("InteractionSizeKey", "Interaction Zone"),FText::GetEmpty(), FSlateIcon(), InteractionSizeAction, NAME_None, EUserInterfaceActionType::Check);

	// Clipping Entry
	FUIAction ClippingAction(FExecuteAction::CreateRaw(this, &STouchInterfaceDesignerViewport::ToggleClipping), FCanExecuteAction(), FIsActionChecked::CreateLambda([this](){return bEnableClipping;}));
	MenuBuilder.AddMenuEntry(LOCTEXT("Clipping","Enable Clipping"),FText::GetEmpty(), FSlateIcon(), ClippingAction, NAME_None, EUserInterfaceActionType::Check);

	FUIAction LinkAction(FExecuteAction::CreateRaw(this, &STouchInterfaceDesignerViewport::ToggleParentingLine), FCanExecuteAction(), FIsActionChecked::CreateLambda([this](){return bEnableParentingLine;}));
	MenuBuilder.AddMenuEntry(INVTEXT("Parenting"),FText::GetEmpty(), FSlateIcon(), LinkAction, NAME_None, EUserInterfaceActionType::Check);

	return MenuBuilder.MakeWidget();
}

void STouchInterfaceDesignerViewport::ToggleText()
{
	bEnableText = !bEnableText;
}

void STouchInterfaceDesignerViewport::ToggleOutline()
{
	bEnableOutline = !bEnableOutline;
}

void STouchInterfaceDesignerViewport::ToggleInteractionOutline()
{
	bEnableInteractionOutline = !bEnableInteractionOutline;
}

void STouchInterfaceDesignerViewport::ToggleClipping()
{
	bEnableClipping = !bEnableClipping;
	DesignerSurface->SetClipping(bEnableClipping ? EWidgetClipping::ClipToBounds : EWidgetClipping::Inherit);
}

void STouchInterfaceDesignerViewport::ToggleParentingLine()
{
	bEnableParentingLine = !bEnableParentingLine;
}

FReply STouchInterfaceDesignerViewport::HandleOnScaleSettingClicked() const
{
	const EScalingMode Mode = TouchInterfaceSettings->ScalingMode;

	if (Mode == EScalingMode::DPI)
	{
		return TouchDesignerEditorPtr.Pin()->HandleDPISettingsClicked();
	}

	return TouchDesignerEditorPtr.Pin()->HandleTouchInterfaceSettingsClicked();
}

void STouchInterfaceDesignerViewport::HandleLinkControl()
{
	bWaitForParentSelection = true;
	ChildControlSelected = WidgetSelected->GetControlName();
	
	//Todo: lock other panel and user interaction while another left click not occur
}

void STouchInterfaceDesignerViewport::HandleUnLinkControl() const
{
	TouchDesignerEditorPtr.Pin()->UnlinkControl(WidgetSelected.Get()->GetControlName(), true);
}

void STouchInterfaceDesignerViewport::HandleUnLinkAllControl() const
{
	TouchDesignerEditorPtr.Pin()->UnlinkAllControl(WidgetSelected.Get()->GetControlName());
}

void STouchInterfaceDesignerViewport::HandleCancelLinkControl()
{
	bWaitForParentSelection = false;
	//Todo: Unlock other panel and all user interaction
}

void STouchInterfaceDesignerViewport::HandleApplyLinkControl(const FName ParentControl)
{
	TouchDesignerEditorPtr.Pin()->LinkControl(ParentControl, ChildControlSelected, CalculateOffsetBetweenControl(ParentControl, ChildControlSelected), true);
	bWaitForParentSelection = false;
}

EVisibility STouchInterfaceDesignerViewport::GetDesignerOutlineVisibility() const
{
	if ( GEditor->bIsSimulatingInEditor || GEditor->PlayWorld != nullptr )
	{
		return EVisibility::HitTestInvisible;
	}

	return EVisibility::Hidden;
}

EVisibility STouchInterfaceDesignerViewport::GetDesignerVisibility() const
{
	if ( GEditor->bIsSimulatingInEditor || GEditor->PlayWorld != nullptr )
	{
		return EVisibility::HitTestInvisible;
	}

	return EVisibility::Visible;
}

void STouchInterfaceDesignerViewport::BindCommands()
{
	const TSharedPtr<FUICommandList> TouchDesignerCommands = TouchDesignerEditorPtr.Pin()->GetCommands();
	const FVirtualControlDesignerCommands& Commands = FVirtualControlDesignerCommands::Get();

	TouchDesignerCommands->MapAction
	(
		Commands.ToggleOutlineCommand,
		FExecuteAction::CreateSP(this, &STouchInterfaceDesignerViewport::ToggleDashedOutline),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &STouchInterfaceDesignerViewport::IsShowingDashedOutline)
	);

	TouchDesignerCommands->MapAction
	(
		Commands.ToggleOpacityCommand,
		FExecuteAction::CreateSP(this, &STouchInterfaceDesignerViewport::ToggleOpacityState),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &STouchInterfaceDesignerViewport::IsShowingActiveOpacity)
	);

	TouchDesignerCommands->MapAction
	(
		Commands.ToggleConstraintCommand,
		FExecuteAction::CreateSP(this, &STouchInterfaceDesignerViewport::ToggleConstraint),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &STouchInterfaceDesignerViewport::IsConstraintActive)
	);

	TouchDesignerCommands->MapAction
	(
		Commands.ToggleOrientationCommand,
		FExecuteAction::CreateSP(this, &STouchInterfaceDesignerViewport::ToggleOrientation),
		FCanExecuteAction()
	);

	TouchDesignerCommands->MapAction
	(
		Commands.TogglePressedPreviewCommand,
		FExecuteAction::CreateSP(this, &STouchInterfaceDesignerViewport::TogglePressedState),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &STouchInterfaceDesignerViewport::IsShowPressedState)
	);
}

TSharedRef<SWidget> STouchInterfaceDesignerViewport::OnGetScreenSizeMenuContent()
{
	FMenuBuilder MenuBuilder(true, nullptr);

	TArray<FName> Categories;
	if (FTouchInterfaceDesignerDeviceProfile::GetAllCategories(Categories))
	{
		for (const FName& Category : Categories)
		{
			//Construct all sub menu
			MenuBuilder.AddSubMenu(FText::FromName(Category), FText::GetEmpty(), FNewMenuDelegate::CreateRaw(this, &STouchInterfaceDesignerViewport::CreateScreenSizeSubMenu, Category));
		}
	}

	FUIAction CustomScreenSizeAction(FExecuteAction::CreateRaw(this, &STouchInterfaceDesignerViewport::OnCustomScreenSizeButtonClicked), FCanExecuteAction());
	MenuBuilder.AddMenuEntry(FText::FromString(TEXT("Custom")), FText::GetEmpty(), FSlateIcon(), CustomScreenSizeAction, NAME_None, EUserInterfaceActionType::Button);

	return MenuBuilder.MakeWidget();
}

void STouchInterfaceDesignerViewport::CreateScreenSizeSubMenu(FMenuBuilder& MenuBuilder, FName Category)
{
	TArray<FString> ProfileNames = FTouchInterfaceDesignerDeviceProfile::GetAllProfileNameByCategory(Category);

	for (const FString& ProfileName : ProfileNames)
	{
		FUIAction ScreenSizeAction(FExecuteAction::CreateRaw(this, &STouchInterfaceDesignerViewport::OnScreenSizeButtonClicked, ProfileName), FCanExecuteAction());
		MenuBuilder.AddMenuEntry(FText::FromString(ProfileName), FText::GetEmpty(), FSlateIcon(), ScreenSizeAction, NAME_None, EUserInterfaceActionType::Button);
	}
}

void STouchInterfaceDesignerViewport::OnScreenSizeButtonClicked(const FString ProfileName)
{
	CustomSizeWidget->SetVisibility(EVisibility::Collapsed);
	
	DeviceProfileInfo->UpdateDeviceProfile(ProfileName);
	DeviceProfileInfo->SetVisibility(EVisibility::HitTestInvisible);
	bDesignerHasCustomSize = false;
	
	FDesignerDeviceProfile DesignerDeviceProfile;
	if (FTouchInterfaceDesignerDeviceProfile::GetProfile(ProfileName, DesignerDeviceProfile))
	{
		CurrentProfile = DesignerDeviceProfile;
		
		const int32 NewDesignerSizeX = FMath::Floor(1080.0f * (DesignerDeviceProfile.ScreenDefinition.X / DesignerDeviceProfile.ScreenDefinition.Y));
		DesignerSize = bIsInLandscapeMode ? FIntPoint(NewDesignerSizeX, 1080) : FIntPoint(1080, NewDesignerSizeX);
	}
	else
	{
		//Fallback to default size
		DesignerSize = bIsInLandscapeMode ? FIntPoint(1920, 1080) : FIntPoint(1080, 1920);
	}

	// Update scale factor because tick can be called after
	CurrentScaleFactor = TouchInterfaceSettings->GetScaleFactor(FVector2D(DesignerSize));
	
	/*for (auto Itr = ControlWidgets.CreateConstIterator(); Itr; ++Itr)
	{
		Itr->Value.Widget->Refresh();
	}*/
	
	for (FVirtualControl& Control : VirtualControlSetupEdited->VirtualControls)
	{
		if (!Control.bIsChild)
		{
			if (ControlWidgets.Contains(Control.ControlName))
			{
				ControlWidgets.FindRef(Control.ControlName).Widget->Refresh();
			}
		}
	}
}

TSharedRef<SWidget> STouchInterfaceDesignerViewport::CreateCustomScreenSizeButtons()
{
	return SNew(SBox)
	.WidthOverride(200.0f)
	.HAlign(HAlign_Fill)
	[
		SAssignNew(CustomSizeWidget, SHorizontalBox)
		.Visibility(EVisibility::Collapsed)

		+SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			SNew(STextBlock)
			.Text(FText::FromString("Width"))
		]

		+SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(FMargin(5.0f, 0.0f))
		[
			SAssignNew(WidthSpinBox, SSpinBox<int32>)
			.Value(CustomWidth)
			.MinValue(100)
			.MaxValue(4096)
			.MinSliderValue(100)
			.MaxSliderValue(4096)
			.OnValueChanged(this, &STouchInterfaceDesignerViewport::SetCustomWidth)
		]

		+SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.AutoWidth()
		.Padding(FMargin(5.0f, 0.0f))
		[
			SNew(STextBlock)
			.Text(FText::FromString("Height"))
		]

		+SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SAssignNew(HeightSpinBox, SSpinBox<int32>)
			.Value(CustomHeight)
			.MinValue(100)
			.MaxValue(4096)
			.MinSliderValue(100)
			.MaxSliderValue(4096)
			.OnValueChanged(this, &STouchInterfaceDesignerViewport::SetCustomHeight)
		]
	];
}

void STouchInterfaceDesignerViewport::OnCustomScreenSizeButtonClicked()
{
	CustomSizeWidget->SetVisibility(EVisibility::Visible);
	DeviceProfileInfo->SetVisibility(EVisibility::Collapsed);
	bDesignerHasCustomSize = true;
	
	DesignerSize = bIsInLandscapeMode ? FIntPoint(CustomWidth, CustomHeight) : FIntPoint(CustomHeight, CustomWidth);

	// Update scale factor because tick can be called after
	CurrentScaleFactor = TouchInterfaceSettings->GetScaleFactor(FVector2D(DesignerSize));
	
	/*for (auto Itr = ControlWidgets.CreateConstIterator(); Itr; ++Itr)
	{
		Itr->Value.Widget->Refresh();
	}*/
	
	for (FVirtualControl& Control : VirtualControlSetupEdited->VirtualControls)
	{
		if (!Control.bIsChild)
		{
			if (ControlWidgets.Contains(Control.ControlName))
			{
				ControlWidgets.FindRef(Control.ControlName).Widget->Refresh();
			}
		}
	}
}

void STouchInterfaceDesignerViewport::SetCustomWidth(const int32 Value)
{
	CustomWidth = Value;
	UpdateCustomScreenSize(CustomWidth, CustomHeight);
}

void STouchInterfaceDesignerViewport::SetCustomHeight(const int32 Value)
{
	CustomHeight = Value;
	UpdateCustomScreenSize(CustomWidth, CustomHeight);
}

void STouchInterfaceDesignerViewport::UpdateCustomScreenSize(const int32 Width, const int32 Height)
{	
	DesignerSize = bIsInLandscapeMode ? FIntPoint(CustomWidth, CustomHeight) : FIntPoint(CustomHeight, CustomWidth);

	// Update scale factor because tick can be called after
	CurrentScaleFactor = TouchInterfaceSettings->GetScaleFactor(FVector2D(DesignerSize));
	
	/*for (auto Itr = ControlWidgets.CreateConstIterator(); Itr; ++Itr)
	{
		Itr->Value.Widget->Refresh();
	}*/
	
	for (FVirtualControl& Control : VirtualControlSetupEdited->VirtualControls)
	{
		if (!Control.bIsChild)
		{
			if (ControlWidgets.Contains(Control.ControlName))
			{
				ControlWidgets.FindRef(Control.ControlName).Widget->Refresh();
			}
		}
	}
}

FText STouchInterfaceDesignerViewport::GetScaleFactorAsText() const
{
	const EScalingMode Mode = TouchInterfaceSettings->ScalingMode;

	FNumberFormattingOptions Options = FNumberFormattingOptions::DefaultNoGrouping();
	Options.MinimumIntegralDigits = 1;
	Options.MaximumFractionalDigits = 2;
	Options.MinimumFractionalDigits = 1;
	const FText ScaleText = FText::AsNumber(CurrentScaleFactor, &Options);
	
	switch (Mode)
	{
	case EScalingMode::NONE:
		return FText::FromString(TEXT("No Scaling : 1"));

	case EScalingMode::DPI: //Todo: when user choose DPI Scale Curve, the designer surface should use Render Resolution
		return FText::Format(INVTEXT("DPI Scale : {0}"), ScaleText);
		
	case EScalingMode::DesignSize:
		return FText::Format(INVTEXT("Design Size Scale : {0}"), ScaleText);
		
	case EScalingMode::Custom: //Todo: Warning if custom class not found
		//return INVTEXT("Warning: Using Custom DPI Rule with no rules class set. Set a class in Touch Interface Project Settings.");
		return FText::Format(INVTEXT("Custom Scale : {0}"), ScaleText);

	default:
		return FText::FromString(TEXT("Error! No Scaling Mode : 1"));
	}
}

EWidgetClipping STouchInterfaceDesignerViewport::GetClippingState() const
{
	return bEnableClipping ? EWidgetClipping::ClipToBounds : EWidgetClipping::Inherit;
}

void STouchInterfaceDesignerViewport::RemoveControl(const FName ControlName)
{
	if (ControlWidgets.Contains(ControlName))
	{
		const FControlWidget& ControlWidget = ControlWidgets.FindRef(ControlName);
		
		DesignerSurface->RemoveSlot(ControlWidget.Widget.ToSharedRef());
		ControlWidgets.FindAndRemoveChecked(ControlName);

		//Todo: If child, remove parent, set child to false, set offset to zero
		//Todo: If parent, remove child (name and widget ref)
	}
}

void STouchInterfaceDesignerViewport::Refresh()
{
	ControlWidgets.Empty();
	DesignerSurface->ClearChildren();
	GenerateVirtualControlWidgets(VirtualControlSetupEdited->VirtualControls);
}

TArray<TSharedPtr<SVirtualControlEditor>> STouchInterfaceDesignerViewport::GetAllWidgetInCanvas() const
{
	TArray<TSharedPtr<SVirtualControlEditor>> ChildWidgets = {};

	TArray<FControlWidget> MyControlWidgets = {};
	ControlWidgets.GenerateValueArray(MyControlWidgets);
	
	for (FControlWidget& MyControlWidget : MyControlWidgets)
	{
		ChildWidgets.Add(MyControlWidget.Widget);
	}
	return ChildWidgets;
}

bool STouchInterfaceDesignerViewport::GetControlWidgetHovered(TSharedPtr<SVirtualControlEditor>& OutWidget) const
{
	TArray<TSharedPtr<SVirtualControlEditor>> ChildWidgets = GetAllWidgetInCanvas();

	for (const TSharedPtr<SVirtualControlEditor> TouchDesignerEditor_Control : ChildWidgets)
	{
		if (TouchDesignerEditor_Control->IsHovered())
		{
			OutWidget = TouchDesignerEditor_Control;
			return true;
		}
	}
	
	return false;
}

TSharedPtr<SVirtualControlEditor> STouchInterfaceDesignerViewport::GetControlWidgetHovered()
{
	TArray<TSharedPtr<SVirtualControlEditor>> ChildWidgets = GetAllWidgetInCanvas();

	for (TSharedPtr<SVirtualControlEditor> VirtualControlEditor : ChildWidgets)
	{
		if (VirtualControlEditor->IsHovered())
		{
			return VirtualControlEditor;
		}
	}

	return nullptr;
}

const ISlateStyle& STouchInterfaceDesignerViewport::GetSlateStyle() const
{	
#if ENGINE_MAJOR_VERSION > 4
	return FAppStyle::Get();
#else
	return FEditorStyle::Get();
#endif
}

FText STouchInterfaceDesignerViewport::GetZoomText() const
{
	FFormatNamedArguments Args;
	Args.Add(TEXT("Scale"), DesignerScale);
	//DesignerScale == 1 ? TEXT("1:1") : FString::SanitizeFloat(DesignerScale)
	//FText Sign = DesignerScale>0 ? LOCTEXT("Positive","+") : LOCTEXT("Negative","-");*/
	return FText::Format(LOCTEXT("ZoomText","Zoom {Scale}"), Args);
}

#undef LOCTEXT_NAMESPACE
