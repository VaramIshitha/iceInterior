// Copyright Lost in Game Studio. All Rights Reserved.

#include "SVirtualShapeDesignerViewport.h"

#include "STouchInterfaceDesignerRuler.h"
#include "SVirtualShapeDesignerToolBar.h"
#include "TouchInterfaceDesignerSettings.h"
#include "VirtualShapeDesignerCommands.h"
#include "VirtualShapeDesignerEditor.h"
#include "VirtualShapeDesignerEditorStyle.h"
#include "Classes/VirtualShape.h"
#include "Settings/TouchInterfaceSettings.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SGridPanel.h"

#if	ENGINE_MAJOR_VERSION > 4
#include "Styling/ToolBarStyle.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(LogVirtualShapeViewport, All, All);

FShapeLine CurrentShapeLine = FShapeLine();

void FBoxSelection::UpdatePointerLocation(const FVector2D InPointerLocation)
{
	PointerLocation = InPointerLocation;
	
	Extend = (PointerLocation - Origin) * 0.5f;

	if (Extend.X < 0 || Extend.Y < 0)
	{
		Center = GetOrigin() + Extend.GetAbs();
		return;
	}
	
	Center = GetOrigin() + Extend;
}

FVector2D FBoxSelection::GetOrigin() const
{
	if (Extend.X < 0 && Extend.Y < 0)
	{
		return PointerLocation;
	}
	
	if (Extend.X < 0)
	{
		return FVector2D(PointerLocation.X , Origin.Y);
	}

	if (Extend.Y < 0)
	{
		return FVector2D(Origin.X , PointerLocation.Y);
	}
	
	return Origin;
}

/*bool FDotData::IsUnderCursor(const FVector2D AbsolutePosition) const
{
	return FVector2D::Distance(Location, AbsolutePosition) <= 20.0f;
}*/

/*bool FDotData::IsUnderBoxSelection(FBoxSelection BoxSelection) const
{
	return Location.X > BoxSelection.GetBoxCenter().X - BoxSelection.GetBoxExtend().X
	&& Location.X < BoxSelection.GetBoxCenter().X + BoxSelection.GetBoxExtend().X
	&& Location.Y > BoxSelection.GetBoxCenter().Y - BoxSelection.GetBoxExtend().Y
	&& Location.Y < BoxSelection.GetBoxCenter().Y + BoxSelection.GetBoxExtend().Y;
}*/

void SVirtualShapeDesignerViewport::Construct(const FArguments& InArgs, TSharedPtr<FVirtualShapeDesignerEditor> InVirtualShapeDesignerEditor)
{
	CommandList = InArgs._CommandList.Get();
	
	ShapeDotDistance = GetDefault<UTouchInterfaceSettings>()->ShapeDotDistance;
	bIsDrawing = false;
	LastCornerAngle = 0.0f;
	
	EditorSettings = GetDefault<UTouchInterfaceDesignerSettings>();
	RuntimeSettings = GetDefault<UTouchInterfaceSettings>();

	EditorPtr = InVirtualShapeDesignerEditor;
	VirtualShapeEdited = InVirtualShapeDesignerEditor->GetVirtualShape();
	ShapeInfoWidget = EditorPtr.Pin()->GetInfoWidget();

	GridSpacingValue = 10;
	bGridSnappingEnabled = false;

	SelectedTool = EDrawTools::SelectPoint;

	bBoxSelection = false;

#if WITH_EDITOR
	GetMutableDefault<UTouchInterfaceSettings>()->OnSettingChanged().AddSP(this, &SVirtualShapeDesignerViewport::HandleOnSettingsChanged);
#endif

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
			.Visibility(this, &SVirtualShapeDesignerViewport::GetRulerVisibility)
		]
		
		// Side Ruler
		+ SGridPanel::Slot(0, 1)
		[
			SAssignNew(SideRuler, STouchInterfaceDesignerRuler)
			.Orientation(Orient_Vertical)
			.Visibility(this, &SVirtualShapeDesignerViewport::GetRulerVisibility)
		]

		// Designer content area
		+ SGridPanel::Slot(1, 1)
		[
			SNew(SOverlay)
		
			+SOverlay::Slot()
			.Padding(2.0f,2.0f,2.0f,0)
			[
				SNew(SHorizontalBox)

				+SHorizontalBox::Slot()
				.FillWidth(1)
				[
					SNew(SSpacer)
				]

				// Designer Toolbar
				+SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Top)
				[
					SNew(SHorizontalBox)

					+SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SVirtualShapeDesignerToolBar)
						.GridSpacing(GridSpacingValue)
						.EnableGridSnapping(bGridSnappingEnabled)
						.OnGridSnappingChanged(this, &SVirtualShapeDesignerViewport::HandleOnGridSnappingChanged)
						.OnGridSpacingChanged(this, &SVirtualShapeDesignerViewport::HandleOnGridSpacingChanged)						
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
						.OnGetMenuContent(FOnGetContent::CreateSP(this, &SVirtualShapeDesignerViewport::OnGetVisualizationMenuContent))
						.ButtonContent()
						[
							SNew(STextBlock)
							.Text(INVTEXT("Visualization"))
							.TextStyle(&ToolBarStyle.LabelStyle)
						]
		#else
						.ButtonStyle(FEditorStyle::Get(), "ViewportMenu.Button")
						.ContentPadding(FEditorStyle::Get().GetMargin("ViewportMenu.SToolBarButtonBlock.Button.Padding"))
						.MenuPlacement(MenuPlacement_BelowAnchor)
						.OnGetMenuContent(FOnGetContent::CreateSP(this, &SVirtualShapeDesignerViewport::OnGetVisualizationMenuContent))
						.ButtonContent()
						[
							SNew(STextBlock)
							.Text(INVTEXT("Visualization"))
							.TextStyle(FEditorStyle::Get(), "ViewportMenu.Label")
						]
		#endif

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
				.Visibility(this, &SVirtualShapeDesignerViewport::GetDesignerOutlineVisibility)

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
					.Text(INVTEXT("SIMULATING"))
				]
			]

			+SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SAssignNew(Canvas, SConstraintCanvas)
			]
		]
	];

	TopRuler->SetRuling(GetTickSpaceGeometry().GetAbsolutePosition(), 1.0f);
	SideRuler->SetRuling(GetTickSpaceGeometry().GetAbsolutePosition(), 1.0f);

	BindCommand();
	GenerateDataFromVirtualShapeAsset();
}

void SVirtualShapeDesignerViewport::BindCommand()
{
	const FVirtualShapeDesignerCommands& Commands = FVirtualShapeDesignerCommands::Get();
	
	CommandList->MapAction(Commands.SimplifyCommand, FExecuteAction::CreateSP(this, &SVirtualShapeDesignerViewport::SimplifyShape) /*Todo: Add FCanExecuteAction */);
}

void SVirtualShapeDesignerViewport::GenerateDataFromVirtualShapeAsset()
{
	DotData = VirtualShapeEdited->GetDotDataByRef();
	
	
	// TArray<FShapeLine> Lines = VirtualShapeEdited->GetShapeLines();
	//
	// for (int32 Itr = 0; Itr < Lines.Num(); ++Itr)
	// {
	// 	FDotData Dot = FDotData(Lines[Itr].StartPosition);
	// 	//Dot.bIsEndPoint = Lines[Itr].bHasEndPoint;
	// 	Dot.bIsStartPoint = Itr == 0 || Lines[Itr-1 < 0 ? 0 : Itr-1].bHasEndPoint;
	//
	// 	DotData.Add(Dot);
	// 	
	// 	if (Lines[Itr].bHasEndPoint)
	// 	{
	// 		FDotData EndDot = FDotData(Lines[Itr].EndPosition);
	// 		Dot.bIsEndPoint = true;
	// 		DotData.Add(EndDot);
	// 	}		
	// }

	//RegenerateShape();
}

void SVirtualShapeDesignerViewport::ChangeToolMode(EDrawTools Mode)
{
	//Clear the already selected points
	ClearDotSelection();

	//Change tool mode
	SelectedTool = Mode;
}

void SVirtualShapeDesignerViewport::ClearDraw()
{
	DotData.Empty();
}

void SVirtualShapeDesignerViewport::Flatten(bool bHorizontally)
{
	//Todo: Take into account Snap
	float AveragePosition = 0;
	int32 Divisor = 0;

	for (const FDotData& Dot : DotData)
	{
		if (Dot.bIsSelected)
		{
			AveragePosition += bHorizontally ? Dot.Location.Y : Dot.Location.X;
			++Divisor;
		}
	}

	AveragePosition = AveragePosition / Divisor;

	for (FDotData& Dot : DotData)
	{
		if (Dot.bIsSelected)
		{
			if (bHorizontally)
			{
				Dot.Location.Y = AveragePosition;
			}
			else
			{
				Dot.Location.X = AveragePosition;
			}
		}
	}

	VirtualShapeEdited->Modify();
}

void SVirtualShapeDesignerViewport::ClearDotSelection()
{
	for (FDotData& Dot : DotData)
	{
		Dot.SetIsSelected(false);
	}
}

FReply SVirtualShapeDesignerViewport::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{	
	switch (SelectedTool)
	{
	case EDrawTools::FreeDraw:
		return OnFreeDrawStarted(MyGeometry, MouseEvent);
	case EDrawTools::SelectPoint:
		return OnSelectStarted(MyGeometry, MouseEvent);
	case EDrawTools::AddPoint:
		return FReply::Handled();
	case EDrawTools::RemovePoint:
		RemovePoint(MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()));
		return FReply::Unhandled();
	default:
		return FReply::Unhandled();
	}	
}

FReply SVirtualShapeDesignerViewport::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	UE_LOG(LogVirtualShapeViewport, Log, TEXT("On Drag Detected!"));

	if (SelectedTool == EDrawTools::SelectPoint)
	{
		if (bDotSelected)
		{
			OldMouseLocation = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
			//PanningStartLocation = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
			bIsPanning = true;
		}
		/*else
		{
			//Todo: Remove this, because, OnDragDetected is used on to know if point is dragged
			bBoxSelection = true;
			UE_LOG(LogVirtualShapeViewport, Log, TEXT("Origin = %f x %f"), MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()).X, MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()).Y);
			BoxSelectionData = FBoxSelection(MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()));
		}*/
		
		return FReply::Handled().CaptureMouse(SharedThis(this));
		//return FReply::Handled().UseHighPrecisionMouseMovement(SharedThis(this));
	}
	
	return SCompoundWidget::OnDragDetected(MyGeometry, MouseEvent);
}

FReply SVirtualShapeDesignerViewport::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	MousePosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	
	switch (SelectedTool)
	{
	case EDrawTools::FreeDraw:
		return OnFreeDrawMoved(MyGeometry, MouseEvent);
	case EDrawTools::SelectPoint:
		return OnSelectMoved(MyGeometry, MouseEvent);
	case EDrawTools::AddPoint:
		return FReply::Handled();
	case EDrawTools::RemovePoint:
		//Todo: When user hover point to remove, show the modification. Same for adding ?
		return FReply::Unhandled();
	default:
		return FReply::Unhandled();
	}
}

FReply SVirtualShapeDesignerViewport::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{	
	switch (SelectedTool)
	{
	case EDrawTools::FreeDraw:
		return OnFreeDrawEnded(MyGeometry, MouseEvent);
	case EDrawTools::SelectPoint:
		return OnSelectEnded(MyGeometry, MouseEvent);
	case EDrawTools::AddPoint:
		return OnAddEnded(MyGeometry, MouseEvent);
	case EDrawTools::RemovePoint:
		return FReply::Unhandled();
	default:
		return FReply::Unhandled();
	}
}

void SVirtualShapeDesignerViewport::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	//UE_LOG(LogVirtualShapeViewport, Log, TEXT("Cursor leave viewport!"));
	SCompoundWidget::OnMouseLeave(MouseEvent);
}

FCursorReply SVirtualShapeDesignerViewport::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const
{	
	if (SelectedTool != EDrawTools::SelectPoint)
	{
		return SCompoundWidget::OnCursorQuery(MyGeometry, CursorEvent);
	}

	if (bIsHoverDot)
	{
		return FCursorReply::Cursor(EMouseCursor::GrabHand);
	}
	
	if (bIsPanning)
	{
		return FCursorReply::Cursor(EMouseCursor::CardinalCross);
	}
	
	return SCompoundWidget::OnCursorQuery(MyGeometry, CursorEvent);
}

FReply SVirtualShapeDesignerViewport::OnFreeDrawStarted(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const FVector2D LocalCoord = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());

	//Clear already selected point
	ClearDotSelection();
	
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		//Todo: User should be able to draw multiple section
		
		DotPositions.Empty();
		Deprecated_Lines.Empty();
		ShapeLines.Empty();
	
		CurrentShapeLine = FShapeLine(LocalCoord);
		Deprecated_Lines.Add(LocalCoord);

		ShapeLines.Add(FShapeLine(LocalCoord));
	
		DotPositions.Add(LocalCoord);
		bIsDrawing = true;

		return FReply::Handled();
	}

	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		//Todo: Right Mouse Button to clear or valid draw ? Then generate data
		EditorPtr.Pin()->SetIsDirty();
	}

	return FReply::Unhandled();
}

FReply SVirtualShapeDesignerViewport::OnFreeDrawMoved(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const FVector2D LocalCoord = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	
	if (bIsDrawing)
	{
		if (FVector2D::Distance(DotPositions[DotPositions.Num()-1], LocalCoord) > ShapeDotDistance)
		{
			if (CurrentShapeLine.bHasDirection)
			{
				const FVector2D Direction = (LocalCoord - DotPositions[DotPositions.Num()-1]).GetSafeNormal();
				
				if ((CurrentShapeLine.Direction | Direction) > RuntimeSettings->CornerDetectionThreshold)
				{
					CurrentShapeLine.EndPosition = LocalCoord;
					Deprecated_Lines[Deprecated_Lines.Num()-1] = LocalCoord;
					LastCornerAngle = ComputeCornerAngle(CurrentShapeLine.Direction, (LocalCoord - CurrentShapeLine.StartPosition).GetSafeNormal());
				}
				else
				{
					if (Deprecated_Lines.Num() > 2)
					{
						//const float CornerAngle = ComputeCornerAngle(CurrentShapeLine.Direction, Direction);
						const bool RightAngle = FMath::IsWithin(LastCornerAngle, 88.0f, 92.0f);
					
						Angles.Add(FShapeAngle(DotPositions[DotPositions.Num()-1], LastCornerAngle, RightAngle));
						UE_LOG(LogVirtualShapeViewport, Log, TEXT("Corner Angle : %f x %f | %f | %d"), DotPositions[DotPositions.Num()-1].X, DotPositions[DotPositions.Num()-1].Y, LastCornerAngle, RightAngle ? 1 : 0);
					}
					
					const FVector2D EndPoint = CurrentShapeLine.EndPosition;
					CurrentShapeLine = FShapeLine(EndPoint);
					Deprecated_Lines.Add(EndPoint);
				}
			}
			else
			{
				CurrentShapeLine.EndPosition = LocalCoord;
				CurrentShapeLine.Direction = (LocalCoord - CurrentShapeLine.StartPosition).GetSafeNormal();
				CurrentShapeLine.bHasDirection = true;
				Deprecated_Lines.Add(LocalCoord);
			}
			
			DotPositions.Add(LocalCoord);

			return FReply::Handled();
		}
	}

	return FReply::Unhandled();
}

FReply SVirtualShapeDesignerViewport::OnFreeDrawEnded(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const FVector2D LocalCoord = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (bIsDrawing)
		{
			bIsDrawing = false;
	
			if (FVector2D::Distance(DotPositions[DotPositions.Num()-1], LocalCoord) > ShapeDotDistance)
			{
				DotPositions.Add(LocalCoord);
			}

			EditorPtr.Pin()->SetIsDirty();
			
			return FReply::Handled().ReleaseMouseCapture();
		}
	}

	return FReply::Unhandled().ReleaseMouseCapture();
}

FReply SVirtualShapeDesignerViewport::OnSelectStarted(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const FVector2D LocalCoord = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (DotData.Num() > 0)
		{
			PanningStartLocation = LocalCoord;
			
			// Find if any point is hovered
			int32 LastDotHoveredIndex = -1;
			for (int32 Itr = 0; Itr < DotData.Num(); ++Itr)
			{
				/*if (DotData[Itr].DotIsUnderCursor(LocalCoord))
				{
					LastDotHoveredIndex = Itr;
				}*/
				
				if (DotIsUnderLocation(DotData[Itr].Location, LocalCoord))
				{
					LastDotHoveredIndex = Itr;
				}
			}

			if (LastDotHoveredIndex != -1)
			{
				// Add to selection
				if (MouseEvent.IsLeftShiftDown())
				{
					DotData[LastDotHoveredIndex].SetIsSelected(true);
					return FReply::Handled();
				}

				// Subtract from selection
				if (MouseEvent.IsLeftAltDown())
				{
					DotData[LastDotHoveredIndex].SetIsSelected(false);
					return FReply::Handled();
				}

				// Already selected, detect drag to move selected points
				if (DotData[LastDotHoveredIndex].bIsSelected)
				{
					bDotSelected = true;
					return FReply::Handled().DetectDrag(SharedThis(this), EKeys::LeftMouseButton);
				}

				// Clear already selected point and select last hovered point. Then detect drag to move selected point
				ClearDotSelection();
				DotData[LastDotHoveredIndex].SetIsSelected(true);
				bDotSelected = true;
				
				return FReply::Handled().DetectDrag(SharedThis(this), EKeys::LeftMouseButton);
			}
		}

		bBoxSelection = true;
		UE_LOG(LogVirtualShapeViewport, Log, TEXT("Origin = %f x %f"), MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()).X, MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()).Y);
		BoxSelectionData = FBoxSelection(MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()));
		
		return FReply::Handled().DetectDrag(SharedThis(this), EKeys::LeftMouseButton);
	}

	return FReply::Unhandled();
}

FReply SVirtualShapeDesignerViewport::OnSelectMoved(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const FVector2D LocalCoord = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());

	if (bIsPanning)
	{
		const FVector2D DeltaMove = OldMouseLocation - LocalCoord;
		OldMouseLocation = LocalCoord;

		if (bGridSnappingEnabled)
		{
			const FVector2D SnapDelta = PanningStartLocation - LocalCoord;
			if (FMath::Abs(SnapDelta.X) >= GridSpacingValue || FMath::Abs(SnapDelta.Y) >= GridSpacingValue)
			{
				for (FDotData& Dot : DotData)
				{
					if (Dot.bIsSelected)
					{
						Dot.Location = GetSnappingPosition(Dot.Location - SnapDelta, GridSpacingValue);
					}
				}
				
				//Save snapped position
				PanningStartLocation = LocalCoord;
			}
		}
		else
		{
			for (FDotData& Dot : DotData)
			{
				if (Dot.bIsSelected)
				{
					Dot.Location = Dot.Location = Dot.Location - DeltaMove;
				}
			}
		}
		
		//Check if bound out of viewport
		return FReply::Handled();
	}
		
	if (bBoxSelection)
	{
		BoxSelectionData.UpdatePointerLocation(LocalCoord);

		// Add to selection
		if (MouseEvent.IsLeftShiftDown())
		{
			for (int32 Itr = 0; Itr < DotData.Num(); ++Itr)
			{
				if (DotInUnderBoxSelection(DotData[Itr].Location, BoxSelectionData))
				{
					DotData[Itr].SetIsSelected(true);
				}
			}
		}
		else if (MouseEvent.IsLeftAltDown())
		{
			for (int32 Itr = 0; Itr < DotData.Num(); ++Itr)
			{
				if (DotInUnderBoxSelection(DotData[Itr].Location, BoxSelectionData))
				{
					DotData[Itr].SetIsSelected(false);
				}
			}
		}
		else
		{
			TArray<int32> SelectedDots;
			for (int32 Itr = 0; Itr < DotData.Num(); ++Itr)
			{
				if (DotInUnderBoxSelection(DotData[Itr].Location, BoxSelectionData))
				{
					SelectedDots.Add(Itr);
					DotData[Itr].SetIsSelected(true);
				}
				else
				{
					DotData[Itr].SetIsSelected(false);
				}
			}
		}
		return FReply::Handled();
	}

	//Hover effect
	if (DotData.Num() > 0)
	{
		int32 LastDotHoveredIndex = -1;
		for (int32 Itr = 0; Itr < DotData.Num(); ++Itr)
		{
			DotData[Itr].SetIsHover(false);
			if (DotIsUnderLocation(DotData[Itr].Location, LocalCoord))
			{
				LastDotHoveredIndex = Itr;
			}
		}

		if (LastDotHoveredIndex != -1)
		{
			DotData[LastDotHoveredIndex].SetIsHover(true);
		}
	}

	return FReply::Handled();
}

FReply SVirtualShapeDesignerViewport::OnSelectEnded(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (bIsPanning)
	{
		bIsPanning = false;
		EditorPtr.Pin()->SetIsDirty();
	}
	
	if (bDotSelected)
	{
		bDotSelected = false;
	}
		
	if (bBoxSelection)
	{
		bBoxSelection = false;
	}
	
	return FReply::Handled().ReleaseMouseCapture();
}

FReply SVirtualShapeDesignerViewport::OnAddEnded(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const FVector2D LocalCoord = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	
	/*{
		SConstraintCanvas::FSlot* ConstructedSlot = nullptr;
		TSharedPtr<SVirtualShapeDesignerDot> ConstructedWidget = nullptr;

		Canvas->AddSlot()
		.Expose(ConstructedSlot)
		.Offset(FMargin(LocalCoord.X - 18.0f, LocalCoord.Y - 18.0f, 10.0f, 10.0f))
		[
			SAssignNew(ConstructedWidget, SVirtualShapeDesignerDot)
			.Id(FMath::Clamp(DotWidgetData.Num(), 0, RuntimeSettings->MaxPoint))
		];
	
		DotWidgetData.Add(FDotWidgetData(LocalCoord, FMath::Clamp(DotWidgetData.Num(), 0, RuntimeSettings->MaxPoint), ConstructedSlot, ConstructedWidget));
		//Todo: Fix location of widget and add point in struct. Recalculate shape
	}*/	
	
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (DotData.Num() <= 0 || DotData.Last().bIsEndPoint)
		{
			if (bGridSnappingEnabled)
			{
				DotData.Add(FDotData(GetSnappingPosition(LocalCoord, GridSpacingValue), true));
			}
			else
			{
				DotData.Add(FDotData(LocalCoord, true));
				UE_LOG(LogVirtualShapeViewport, Log, TEXT("Add Position = %f x %f"), LocalCoord.X, LocalCoord.Y);
			}			
		}
		else
		{
			if (bGridSnappingEnabled)
			{
				DotData.Add(FDotData(GetSnappingPosition(LocalCoord, GridSpacingValue)));
			}
			else
			{
				DotData.Add(FDotData(LocalCoord));
				UE_LOG(LogVirtualShapeViewport, Log, TEXT("Add Position = %f x %f"), LocalCoord.X, LocalCoord.Y);
			}
		}

		bShowNextLine = true;

		EditorPtr.Pin()->SetIsDirty();
		
		return FReply::Handled();
	}
		
	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		//Todo: Stop Line, next point added will be starting point for line
		//Todo: Set last point to end point
		//Todo: Test if num point > 0 or if this is the first point

		if (DotData.Num() > 0 && !DotData.Last().bIsStartPoint)
		{
			DotData.Last().bIsEndPoint = true;
			bShowNextLine = false;
			EditorPtr.Pin()->SetIsDirty();
			return FReply::Handled();
		}
	}

	return FReply::Unhandled();
}

int32 SVirtualShapeDesignerViewport::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const FSlateBrush* BackgroundImage = GetSlateStyle().GetBrush(TEXT("Graph.Panel.SolidBackground")); //"Graph.DelegatePin.Connected"
	PaintViewportGrid(BackgroundImage, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId);

	PaintImageReference(AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle);
	
	PaintShape(AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle);

	PaintSelectionBox(AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle);
	
	return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

void SVirtualShapeDesignerViewport::PaintViewportGrid(const FSlateBrush* BackgroundImage, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32& DrawLayerId) const
{
	const int32 LineSpacing = bGridSnappingEnabled ? GridSpacingValue : 10;
	check(LineSpacing > 0);

	const FLinearColor RegularColor(GetSlateStyle().GetColor("Graph.Panel.GridLineColor"));

	// Draw Background
	FSlateDrawElement::MakeBox(OutDrawElements, DrawLayerId, AllottedGeometry.ToPaintGeometry(), BackgroundImage);

	TArray<FVector2D> LinePoints;
	new (LinePoints) FVector2D(0,0);
	new (LinePoints) FVector2D(0,0);

	float ImageOffsetY = 0.0f;
	float ImageOffsetX = 0.0f;

	// Vertical Lines
	while (ImageOffsetY < AllottedGeometry.GetLocalSize().Y)
	{
		ImageOffsetY += LineSpacing;
		
		if (ImageOffsetY >= 0.0f)
		{
			// Start & end location of current line
			LinePoints[0] = FVector2D(0.0f, ImageOffsetY);
			LinePoints[1] = FVector2D(AllottedGeometry.GetLocalSize().X, ImageOffsetY);

			FSlateDrawElement::MakeLines(OutDrawElements, DrawLayerId, AllottedGeometry.ToPaintGeometry(), LinePoints, ESlateDrawEffect::None, RegularColor, false);
		}
	}

	// Horizontal Lines
	while (ImageOffsetX < AllottedGeometry.GetLocalSize().X)
	{
		ImageOffsetX += LineSpacing;
		
		if (ImageOffsetX >= 0.0f)
		{			
			LinePoints[0] = FVector2D(ImageOffsetX, 0);
			LinePoints[1] = FVector2D(ImageOffsetX, AllottedGeometry.GetLocalSize().Y);
	
			FSlateDrawElement::MakeLines(OutDrawElements, DrawLayerId, AllottedGeometry.ToPaintGeometry(), LinePoints, ESlateDrawEffect::None, RegularColor, false);
		}
	}
}

void SVirtualShapeDesignerViewport::PaintImageReference(const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const
{
	
}

void SVirtualShapeDesignerViewport::PaintShape(const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const
{
	if (SelectedTool == EDrawTools::FreeDraw)
	{
		if (DotPositions.Num() > 0)
		{
			// Draw dot
			for (int32 Itr = 0; Itr < DotPositions.Num(); ++Itr)
			{
				FVector2D PointPosition = DotPositions[Itr];
				
				//Todo: This is deprecated!
				FSlateDrawElement::MakeBox
				(
					OutDrawElements,
					LayerId+1,
					//AllottedGeometry.ToPaintGeometry(PointPosition - FVector2D(EditorSettings->ShapeDotSize * 0.5f), FVector2D(EditorSettings->ShapeDotSize)),
					AllottedGeometry.ToPaintGeometry(FVector2D(EditorSettings->ShapeDotSize), FSlateLayoutTransform(PointPosition - FVector2D(EditorSettings->ShapeDotSize * 0.5f))),
					FVirtualShapeDesignerEditorStyle::Get().GetBrush("WhiteCircle"),
					ESlateDrawEffect::None,
					EditorSettings->ShapeDotColor
				);
			}

			//Draw line between dot
			FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), DotPositions, ESlateDrawEffect::None, EditorSettings->ShapeLineColor, false, EditorSettings->ShapeLineSize);
		}

		if (Deprecated_Lines.Num() > 1)
		{
			//Draw simplified shape line
			FSlateDrawElement::MakeLines(OutDrawElements, LayerId+1, AllottedGeometry.ToPaintGeometry(), Deprecated_Lines, ESlateDrawEffect::None, FLinearColor::Blue, false, 2.0f);
		}

		return;
	}

	// If DotData array is empty, do nothing
	if (DotData.Num() > 0)
	{
		const FVector2D DotSize = FVector2D(EditorSettings->ShapeDotSize);

		TArray<TArray<FVector2D>> LinesArray;
		LinesArray.Add({});
		int32 LineArrayIndex = 0;

		for (int32 Itr = 0; Itr < DotData.Num(); ++Itr)
		{
			const FDotData& Dot = DotData[Itr];

			//const int32 PreviousIndex = Itr-1 < 0 ? 0 : Itr-1;
			//const FVector2D LocalDotSize = (DotData[PreviousIndex].bIsEndPoint || Dot.bIsEndPoint) ? DotSize * 1.25f : DotSize;
			const FVector2D LocalDotSize = Dot.IsPrimaryPoint() ? DotSize * 1.25f : DotSize;
			
			FSlateDrawElement::MakeBox
			(
				OutDrawElements,
				LayerId+1,
				AllottedGeometry.ToPaintGeometry(LocalDotSize, FSlateLayoutTransform(1.0f, Dot.Location - LocalDotSize * 0.5f)),
				FVirtualShapeDesignerEditorStyle::Get().GetBrush(GetDotStyleName(Dot)),
				ESlateDrawEffect::None,
				/*FLinearColor::White*/
				Dot.GetColor()
			);

			if (Dot.bIsEndPoint)
			{
				LinesArray.Add({});
				LinesArray[LineArrayIndex].Add(Dot.Location);
				++LineArrayIndex;
			}
			else
			{
				LinesArray[LineArrayIndex].Add(Dot.Location);
			}
		}

		if (LinesArray.Num() > 0)
		{
			for (int32 Itr = 0; Itr < LinesArray.Num(); ++Itr)
			{
				FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), LinesArray[Itr], ESlateDrawEffect::None, FLinearColor::Blue, false, 2.0f);
			}
		}
	}
	
	if (SelectedTool == EDrawTools::AddPoint)
	{
		const FVector2D SnappedMousePosition = bGridSnappingEnabled ? GetSnappingPosition(MousePosition, GridSpacingValue) : MousePosition;
		
		FSlateDrawElement::MakeBox
		(
			OutDrawElements,
			LayerId+1,
			//AllottedGeometry.ToPaintGeometry(SnappedMousePosition - FVector2D(EditorSettings->ShapeDotSize * 0.5f), FVector2D(EditorSettings->ShapeDotSize)),
			AllottedGeometry.ToPaintGeometry(FVector2D(EditorSettings->ShapeDotSize), FSlateLayoutTransform(SnappedMousePosition - FVector2D(EditorSettings->ShapeDotSize * 0.5f))),
			FVirtualShapeDesignerEditorStyle::Get().GetBrush("WhiteCircle"),
			ESlateDrawEffect::None,
			FLinearColor::Gray
		);
		//Todo: Show influence on shape

		if (bShowNextLine)
		{
			TArray<FVector2D> NextLinePoints;
			NextLinePoints.Add(DotData.Last().Location);
			NextLinePoints.Add(SnappedMousePosition);
			FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), NextLinePoints, ESlateDrawEffect::None, FLinearColor::Gray, false, 2.0f);
		}
	}

	if (SelectedTool == EDrawTools::RemovePoint)
	{
		//Todo: Show influence on shape if hovered point is removed
	}
}

void SVirtualShapeDesignerViewport::PaintSelectionBox(const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const
{
	if (bBoxSelection)
	{
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(BoxSelectionData.GetBoxSize(),FSlateLayoutTransform(1.0f, BoxSelectionData.GetOrigin())), GetSlateStyle().GetBrush("FocusRectangle") , ESlateDrawEffect::None, FLinearColor::Gray);

		FSlateDrawElement::MakeBox
		(
			OutDrawElements,
			LayerId+1,
			AllottedGeometry.ToPaintGeometry(FVector2D(4.0f), FSlateLayoutTransform(1.0f, BoxSelectionData.GetBoxCenter())),
			FVirtualShapeDesignerEditorStyle::Get().GetBrush("WhiteCircle"),
			ESlateDrawEffect::None,
			FLinearColor::Gray
		);
	}
}

TSharedRef<SWidget> SVirtualShapeDesignerViewport::OnGetVisualizationMenuContent()
{
	FMenuBuilder MenuBuilder(false, nullptr);
	
	FUIAction OutlineAction(FExecuteAction::CreateRaw(this, &SVirtualShapeDesignerViewport::ToggleDirectionDebug), FCanExecuteAction(), FIsActionChecked::CreateLambda([this](){ return bShowDrawDirection; }));
	MenuBuilder.AddMenuEntry(INVTEXT("Draw Direction"),FText::GetEmpty(), FSlateIcon(), OutlineAction, NAME_None, EUserInterfaceActionType::Check);
	
	FUIAction ControlNameAction(FExecuteAction::CreateRaw(this, &SVirtualShapeDesignerViewport::ToggleLenghtDebug), FCanExecuteAction(), FIsActionChecked::CreateLambda([this](){ return bShowDrawLenght; }));
	MenuBuilder.AddMenuEntry(INVTEXT("Lenght"),FText::GetEmpty(), FSlateIcon(), ControlNameAction, NAME_None, EUserInterfaceActionType::Check);
	
	FUIAction InteractionSizeAction(FExecuteAction::CreateRaw(this, &SVirtualShapeDesignerViewport::ToggleAngleDebug), FCanExecuteAction(), FIsActionChecked::CreateLambda([this](){ return bShowDrawAngle; }));
	MenuBuilder.AddMenuEntry(INVTEXT("Angle"),FText::GetEmpty(), FSlateIcon(), InteractionSizeAction, NAME_None, EUserInterfaceActionType::Check);
	
	FUIAction ClippingAction(FExecuteAction::CreateRaw(this, &SVirtualShapeDesignerViewport::ToggleBoundDebug), FCanExecuteAction(), FIsActionChecked::CreateLambda([this](){ return bShowDrawBound; }));
	MenuBuilder.AddMenuEntry(INVTEXT("Bound"),FText::GetEmpty(), FSlateIcon(), ClippingAction, NAME_None, EUserInterfaceActionType::Check);

	FUIAction LinkAction(FExecuteAction::CreateRaw(this, &SVirtualShapeDesignerViewport::ToggleOrderDebug), FCanExecuteAction(), FIsActionChecked::CreateLambda([this](){ return bShowOrder; }));
	MenuBuilder.AddMenuEntry(INVTEXT("Order"),FText::GetEmpty(), FSlateIcon(), LinkAction, NAME_None, EUserInterfaceActionType::Check);

	return MenuBuilder.MakeWidget();
}

const ISlateStyle& SVirtualShapeDesignerViewport::GetSlateStyle() const
{
	
#if ENGINE_MAJOR_VERSION > 4
	return FAppStyle::Get();
#else
	return FEditorStyle::Get();
#endif
	
}

EVisibility SVirtualShapeDesignerViewport::GetDesignerOutlineVisibility() const
{
	if (GEditor->bIsSimulatingInEditor || GEditor->PlayWorld != nullptr)
	{
		return EVisibility::HitTestInvisible;
	}

	return EVisibility::Hidden;
}

EVisibility SVirtualShapeDesignerViewport::GetRulerVisibility() const
{
	return EVisibility::Visible;
}

void SVirtualShapeDesignerViewport::HandleOnGridSnappingChanged(bool bIsEnabled)
{
	bGridSnappingEnabled = bIsEnabled;
}

void SVirtualShapeDesignerViewport::HandleOnGridSpacingChanged(int32 Spacing)
{
	GridSpacingValue = Spacing;
}

#if WITH_EDITOR
void SVirtualShapeDesignerViewport::HandleOnSettingsChanged(UObject* Object, FPropertyChangedEvent& Property)
{
	//Useless for now because ShapeDotDistance isn't exposed to user
	ShapeDotDistance = GetDefault<UTouchInterfaceSettings>()->ShapeDotDistance;
}
#endif

float SVirtualShapeDesignerViewport::ComputeCornerAngle(const FVector2D VectorA, const FVector2D VectorB, const float NormalizeTolerance)
{
	const float Result = FVector2D::DotProduct(VectorA, VectorB);
	UE_LOG(LogVirtualShapeViewport, Log, TEXT("Dot Product Result = %f"), Result);
	const float Angle = (180.0f)/PI * FMath::Acos(Result);

	return Angle;
	
	/*const FVector2D Result = VectorA.GetSafeNormal(NormalizeTolerance) - VectorB.GetSafeNormal(NormalizeTolerance);

	FVector2D Direction = FVector2D::ZeroVector;
	float Lenght = 0;
	
	Result.ToDirectionAndLength(Direction, Lenght);

	const FVector InVec = FVector(Direction.X, Direction.Y, 0);

	const FRotator Rotation = InVec.ToOrientationRotator();

	UE_LOG(LogVirtualShapeViewport, Log, TEXT("Angle = %f"), Rotation.Yaw);

	return 2 * (Rotation.Yaw < 0 ? (180 + (180 - FMath::Abs(Rotation.Yaw))) : Rotation.Yaw);*/
}

void SVirtualShapeDesignerViewport::RemovePoint(const FVector2D LocalCoord)
{
	//Todo: When remove point check if it is an start or end point
	
	// Select one point
	int32 LastDotHoveredIndex = -1;
	for (int32 Itr = 0; Itr < DotData.Num(); ++Itr)
	{
		if (DotIsUnderLocation(DotData[Itr].Location, LocalCoord))
		{
			LastDotHoveredIndex = Itr;
		}
	}

	if (LastDotHoveredIndex != -1)
	{
		DotData.RemoveAt(LastDotHoveredIndex);
		bDotSelected = false;
	}

	EditorPtr.Pin()->SetIsDirty();
}

void SVirtualShapeDesignerViewport::GenerateData(const bool SaveToAsset)
{
	//Todo: Calculate line lenght, corner angle, bound and save to virtual shape asset if needed

	/*ShapeLines.Empty();
	ShapeAngles.Empty();
	
	int32 CurrentLineIndex = 0;
	
	for (int32 Itr = 0; Itr < DotData.Num(); ++Itr)
	{
		const FDotData& Dot = DotData[Itr];
		
		if (!Dot.bIsEndPoint)
		{
			CurrentLineIndex = ShapeLines.Add(FShapeLine(Dot.Location));
			UE_LOG(LogVirtualShapeViewport, Log, TEXT("Start Line Position = %f x %f"), Dot.Location.X, Dot.Location.Y);
		}

		if (Itr > 0)
		{
			if (!DotData[Itr-1].bIsEndPoint)
			{
				ShapeLines[CurrentLineIndex-1].EndPosition = Dot.Location;
				UE_LOG(LogVirtualShapeViewport, Log, TEXT("End Line Position = %f x %f"), Dot.Location.X, Dot.Location.Y);
			}
			else
			{
				ShapeLines[CurrentLineIndex-1].bHasEndPoint = true;
			}
		}
	}*/

	if (SaveToAsset)
	{
		VirtualShapeEdited->SetDotData(DotData);
	}
}

void SVirtualShapeDesignerViewport::SimplifyShape()
{
	//Todo: Check DotVector between point and simplify if needed
	UE_LOG(LogVirtualShapeViewport, Log, TEXT("Simplify"));
}

FVector2D SVirtualShapeDesignerViewport::GetSnappingPosition(const FVector2D LocalCoord, int32 Multiple) const
{
	const int32 SmallerX = FMath::RoundToInt(LocalCoord.X / Multiple) * Multiple;
	const int32 LargerX = SmallerX + Multiple;
	
	const float NewSnapPositionX = (Multiple - SmallerX > LargerX - Multiple) ? LargerX : SmallerX;
	
	const int32 SmallerY = FMath::RoundToInt(LocalCoord.Y / Multiple) * Multiple;
	const int32 LargerY = SmallerX + Multiple;

	const float NewSnapPositionY = (Multiple - SmallerY > LargerY - Multiple) ? LargerY : SmallerY;

	return FVector2D(NewSnapPositionX, NewSnapPositionY);
}

FName SVirtualShapeDesignerViewport::GetDotStyleName(const FDotData& Dot) const
{
	if (Dot.bIsStartPoint)
	{
		return FName("StartPoint");
	}

	if (Dot.bIsEndPoint)
	{
		return FName("EndPoint");
	}

	return FName("WhiteCircle");
}

bool SVirtualShapeDesignerViewport::DotIsUnderCursor(const FVector2D DotLocation) const
{
	return FVector2D::Distance(DotLocation, MousePosition) <= 20.0f;
}

bool SVirtualShapeDesignerViewport::DotIsUnderLocation(const FVector2D DotLocation, const FVector2D Location) const
{
	return FVector2D::Distance(DotLocation, Location) <= 20.0f;
}

bool SVirtualShapeDesignerViewport::DotInUnderBoxSelection(const FVector2D DotLocation, const FBoxSelection& BoxSelection) const
{
	return DotLocation.X > BoxSelection.GetBoxCenter().X - BoxSelection.GetBoxExtend().X
	&& DotLocation.X < BoxSelection.GetBoxCenter().X + BoxSelection.GetBoxExtend().X
	&& DotLocation.Y > BoxSelection.GetBoxCenter().Y - BoxSelection.GetBoxExtend().Y
	&& DotLocation.Y < BoxSelection.GetBoxCenter().Y + BoxSelection.GetBoxExtend().Y;
}

