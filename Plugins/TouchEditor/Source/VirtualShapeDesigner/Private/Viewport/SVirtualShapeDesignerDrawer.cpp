// Copyright Lost in Game Studio. All Rights Reserved.

#include "SVirtualShapeDesignerDrawer.h"

#include "VirtualShapeDesignerEditorStyle.h"
#include "Settings/TouchInterfaceSettings.h"
#include "TouchInterfaceDesignerSettings.h"
#include "VirtualShapeDesignerEditor.h"
#include "Classes/ShapeRecognizerCore.h"
#include "Classes/VirtualShape.h"

DEFINE_LOG_CATEGORY_STATIC(LogVirtualShapeDrawer, All, All);

void SVirtualShapeDesignerDrawer::Construct(const FArguments& InArgs, TSharedPtr<FVirtualShapeDesignerEditor> InVirtualShapeDesignerEditor)
{
	EditorPtr = InVirtualShapeDesignerEditor;
	ShapeDotDistance = GetDefault<UTouchInterfaceSettings>()->ShapeDotDistance;
	bIsDrawing = false;
	EditorSettings = GetDefault<UTouchInterfaceDesignerSettings>();
	AllowedDelayBetweenSection = 2.0f;
	bIsDrawing = false;
	bIsComputing = false;
	bLaunchTimer = false;

#if WITH_EDITOR
	GetMutableDefault<UTouchInterfaceSettings>()->OnSettingChanged().AddSP(this, &SVirtualShapeDesignerDrawer::HandleOnSettingsChanged);
#endif

	//Todo: Option show shape line and point
}

FReply SVirtualShapeDesignerDrawer::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const FVector2D LocalCoord = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (bIsComputing)
		{
			return FReply::Unhandled();
		}

		//Todo: Use Timer, right click or button to define when computation start. Option in settings (Quick settings in toolbar)
		if (!bLaunchTimer)
		{
			UserDrawing.Empty();
		}
		else
		{
			bLaunchTimer = false;
		}

		UserDrawing.Add(FDotData(LocalCoord, true));
		bIsDrawing = true;

		return FReply::Handled();
	}
	
	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		bIsComputing = false;
		UserDrawing.Empty();
		DotData.Empty();
		ClearDrawDots();
		ClearDrawLines();
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply SVirtualShapeDesignerDrawer::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const FVector2D LocalCoord = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	
	if (bIsDrawing)
	{
		if (FVector2D::Distance(UserDrawing.Last().Location, LocalCoord) > ShapeDotDistance)
		{			
			UserDrawing.Add(FDotData(LocalCoord));

			return FReply::Handled();
		}
	}

	return FReply::Unhandled();
}

FReply SVirtualShapeDesignerDrawer::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (bIsDrawing)
		{
			bIsDrawing = false;
		
			const FVector2D LocalCoord = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());

			// Test if localCoord is near to first point location
			if (FVector2D::Distance(UserDrawing[0].Location, LocalCoord) <= ShapeDotDistance)
			{
				// If so, set the location of last point to location of first point
				UserDrawing.Last().Location = UserDrawing[0].Location;
			}

			UserDrawing.Last().bIsEndPoint = true;
			
			bLaunchTimer = true;
		}
	}

	return FReply::Unhandled();
}

void SVirtualShapeDesignerDrawer::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	//Todo: If user's pointer leave window, clear drawing or clamp position to viewport size
	SLeafWidget::OnMouseLeave(MouseEvent);
}

void SVirtualShapeDesignerDrawer::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if (bLaunchTimer)
	{
		LastTouchTimer += InDeltaTime;
		if (LastTouchTimer > AllowedDelayBetweenSection)
		{
			bLaunchTimer = false;
			LastTouchTimer = 0.0f;
			ComputeRecognition();
		}
	}
}

int32 SVirtualShapeDesignerDrawer::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	if (UserDrawing.Num() > 0)
	{
		TArray<TArray<FVector2D>> LinePositions;
		LinePositions.Add({});
		int32 CurrentIndex = 0;
		
		// Draw dot
		for (const FDotData& Dot : UserDrawing)
		{
			LinePositions[CurrentIndex].Add(Dot.Location);

			if (Dot.bIsEndPoint)
			{
				LinePositions.Add({});
				CurrentIndex++;
			}
		
			/*FSlateDrawElement::MakeBox
			(
				OutDrawElements,
				LayerId+1,
				AllottedGeometry.ToPaintGeometry(Dot.Location - FVector2D(EditorSettings->ShapeDotSize * 0.5f), FVector2D(EditorSettings->ShapeDotSize)),
				FVirtualShapeDesignerEditorStyle::Get().GetBrush("WhiteCircle"),
				ESlateDrawEffect::None,
				EditorSettings->ShapeDotColor
			);*/
		}

		for (const TArray<FVector2D>& Line : LinePositions)
		{
			//Draw line between dot
			FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), Line, ESlateDrawEffect::None, EditorSettings->ShapeLineColor, false, EditorSettings->ShapeLineSize);
		}
	}

	if (bIsComputing)
	{
		if (DotData.Num() > 0)
		{
			for (const FDotData& Dot : DotData)
			{
				FSlateDrawElement::MakeBox
				(
					OutDrawElements,
					LayerId+2,
					AllottedGeometry.ToPaintGeometry(Dot.Location - FVector2D(EditorSettings->ShapeDotSize * 0.5f), FVector2D(EditorSettings->ShapeDotSize)),
					FVirtualShapeDesignerEditorStyle::Get().GetBrush("WhiteCircle"),
					ESlateDrawEffect::None,
					FLinearColor::Blue
				);
			}
		}
	}

	if (DrawDots.Num() > 0)
	{
		for (const FVector2D& Location : DrawDots)
		{
			FSlateDrawElement::MakeBox
			(
				OutDrawElements,
				LayerId+4,
				AllottedGeometry.ToPaintGeometry(Location - FVector2D(EditorSettings->ShapeDotSize * 0.5f), FVector2D(EditorSettings->ShapeDotSize)),
				FVirtualShapeDesignerEditorStyle::Get().GetBrush("WhiteCircle"),
				ESlateDrawEffect::None,
				FLinearColor::Red
			);
			
		}
	}

	if (DrawLines.Num() > 0)
	{
		for (const TArray<FVector2D>& Line : DrawLines)
		{
			//Draw line between dot
			FSlateDrawElement::MakeLines(OutDrawElements, LayerId+3, AllottedGeometry.ToPaintGeometry(), Line, ESlateDrawEffect::None, FLinearColor::Red, false, EditorSettings->ShapeLineSize);
		}
	}

	//Todo: If enabled, show dot and line generated for recognition

	//Todo: If enabled, show current virtual shape

	return LayerId;
}

void SVirtualShapeDesignerDrawer::ComputeRecognition()
{
	bIsComputing = true;
	LastTouchTimer = 0.0f;

	UE_LOG(LogVirtualShapeDrawer, Log, TEXT("Start Computing"));

	const float CornerDetectionThreshold = GetDefault<UTouchInterfaceSettings>()->CornerDetectionThreshold;

	//Todo: make fast algorithm, calculate lines and angle directly from UserDrawing (In RealTime ?)

	//TArray<FShapeLine> ShapeLines;
	//TArray<FShapeAngle> ShapeAngles;
	//FShapeRecognizerCore::ProcessUserDrawing(UserDrawing, DotData, ShapeLines, ShapeAngles);
	
	DotData.Empty();
	int32 StartPointIndex = 0;
	FVector2D CurrentDirection = FVector2D::ZeroVector;
	//int32 Itr = 0;

	for (int32 Itr = 0; Itr < UserDrawing.Num(); ++Itr)
	{
		if (UserDrawing[Itr].bIsStartPoint)
		{
			StartPointIndex = DotData.Add(FDotData(UserDrawing[Itr].Location, true));
			AddNewDrawDot(UserDrawing[Itr].Location);
		}
		else if (UserDrawing[Itr].bIsEndPoint)
		{
			//Todo: Check if there is more than two points
			//Todo: Check if point is same direction. If not, do not take into account this point and go to the next
			FDotData NewPoint = FDotData(UserDrawing[Itr].Location);
			NewPoint.bIsEndPoint = true;
			DotData.Add(NewPoint);
			CurrentDirection = FVector2D::ZeroVector;
			AddNewDrawDot(NewPoint.Location);
		}
		else
		{
			if (CurrentDirection != FVector2D::ZeroVector)
			{
				const FVector2D Direction = (UserDrawing[Itr].Location - UserDrawing[Itr-1].Location).GetSafeNormal();
				UE_LOG(LogVirtualShapeDrawer, Log, TEXT("[%d] Direction = %f x %f"), Itr, Direction.X, Direction.Y);
				
				UE_LOG(LogVirtualShapeDrawer, Log, TEXT("[%d] Dot Result = %f"), Itr, CurrentDirection | Direction);
				if ((CurrentDirection | Direction) <= CornerDetectionThreshold)
				{
					StartPointIndex = DotData.Add(FDotData(UserDrawing[Itr-1].Location));
					CurrentDirection = FVector2D::ZeroVector;
					AddNewDrawDot(UserDrawing[Itr-1].Location);
				}
			}
			else
			{
				CurrentDirection = (UserDrawing[Itr].Location - DotData[StartPointIndex].Location).GetSafeNormal();
				UE_LOG(LogVirtualShapeDrawer, Log, TEXT("[%d] Direction Ref = %f x %f"), Itr, CurrentDirection.X, CurrentDirection.Y);
			}
		}
	}

	UE_LOG(LogVirtualShapeDrawer, Log, TEXT("Num = %d"), DotData.Num());
	
	constexpr float MaxValue =  TNumericLimits<float>::Max();
	
	FVector2D TopLeftBound = FVector2D(MaxValue);
	FVector2D BottomRightBound = FVector2D(0.0f);
	
	for (const FDotData& Dot : DotData)
	{
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
		if (Dot.Location.ComponentwiseAllLessThan(TopLeftBound))
		{
			TopLeftBound = Dot.Location;
		}

		if (Dot.Location.ComponentwiseAllGreaterThan(BottomRightBound))
		{
			BottomRightBound = Dot.Location;
		}
#else
		if (Dot.Location < TopLeftBound)
		{
			TopLeftBound = Dot.Location;
		}

		if (Dot.Location > BottomRightBound)
		{
			BottomRightBound = Dot.Location;
		}
#endif
	}
	
	const FVector2D BoundCenter = TopLeftBound + (BottomRightBound - TopLeftBound) * 0.5f;
	UE_LOG(LogVirtualShapeDrawer, Log, TEXT("TopLeft = %f x %f | BottomRight = %f x %f | Center = %f x %f"),
		TopLeftBound.X, TopLeftBound.Y, BottomRightBound.X, BottomRightBound.Y, BoundCenter.X, BoundCenter.Y);
	
	CalculateDataBasedOnBound(BoundCenter);
	
	TArray<FShapeLine> ShapeLines;
	TArray<FShapeAngle> ShapeAngles;
	FVector2D SumOfLineDirections = FVector2D::ZeroVector;
	
	for (int32 Itr = 0; Itr < DotData.Num()-1; ++Itr)
	{
		const FDotData& Dot = DotData[Itr];
		const FDotData& NextDot = DotData[Itr+1];
	
		if (!Dot.bIsEndPoint)
		{
			const FVector2D Direction = (NextDot.Location - Dot.Location).GetSafeNormal();
			SumOfLineDirections += Direction;
			const float Lenght = FVector2D::Distance(Dot.Location, NextDot.Location);
			ShapeLines.Add(FShapeLine(Dot.Location, NextDot.Location, Direction, Lenght));			
		}
		else
		{
			ShapeLines.Last().bHasEndPoint = true;
		}
	}

	for (const FShapeLine& Line : ShapeLines)
	{
		AddNewDrawLine(FVector2D(200) + Line.StartPosition, FVector2D(200) + Line.EndPosition);
	}
	
	constexpr float DegreeConversion = 180.0f/PI;
	float TotalAngleValue = 0.0f;

	if (ShapeLines.Num() > 1)
	{
		for (int32 Itr = 0; Itr < ShapeLines.Num(); ++Itr)
		{
			FVector2D Dir = ShapeLines[Itr].Direction;
			const int32 NextItr = Itr+1 > ShapeLines.Num()-1 ? 0 : Itr+1;
			FVector2D NextDir = ShapeLines[NextItr].Direction;
		
			if (!ShapeLines[Itr].bHasEndPoint)
			{
				const float DotProductResult = Dir | NextDir;
				const float AngleDegree = 180.0f - DegreeConversion * FMath::Acos(DotProductResult);
				TotalAngleValue += AngleDegree;
	
				ShapeAngles.Add(FShapeAngle(ShapeLines[Itr].EndPosition, AngleDegree, FMath::IsWithin(AngleDegree, 88.0f, 92.0f)));
			}
		}
	}
	
	const float Score = EditorPtr.Pin()->GetVirtualShape()->Evaluate(ShapeLines, ShapeAngles, DotData);
	
	UE_LOG(LogVirtualShapeDrawer, Log, TEXT("Matching Score = %f"), Score);
	
	//Todo: Show Score in shape info tab
}

#if WITH_EDITOR
void SVirtualShapeDesignerDrawer::HandleOnSettingsChanged(UObject* Object, FPropertyChangedEvent& Property)
{
	//Useless for now because ShapeDotDistance isn't exposed to user
	ShapeDotDistance = GetDefault<UTouchInterfaceSettings>()->ShapeDotDistance;
}
#endif

void SVirtualShapeDesignerDrawer::CalculateDataBasedOnBound(const FVector2D& BoundCenter)
{
	TArray<FDotData> ScaledDotData;

	for (const FDotData& Dot : DotData)
	{
		FDotData NewDot = FDotData();
		NewDot.Location = BoundCenter - Dot.Location;
		NewDot.bIsStartPoint = Dot.bIsStartPoint;
		NewDot.bIsEndPoint = Dot.bIsEndPoint;
		ScaledDotData.Add(NewDot);
		AddNewDrawDot(FVector2D(200.0f) + NewDot.Location);
	}

	DotData = ScaledDotData;
}

void SVirtualShapeDesignerDrawer::AddNewDrawDot(FVector2D Location)
{
	DrawDots.Add(Location);
}

void SVirtualShapeDesignerDrawer::AddNewDrawDot(TArray<FVector2D> Locations)
{
	for (const FVector2D& NewLocation : Locations)
	{
		AddNewDrawDot(NewLocation);
	}
}

void SVirtualShapeDesignerDrawer::AddNewDrawLine(FVector2D StartLocation, FVector2D EndLocation)
{
	DrawLines.Add({StartLocation, EndLocation});
}

void SVirtualShapeDesignerDrawer::AddNewDrawLines(TArray<FVector2D> Locations)
{
	DrawLines.Add(Locations);
}

void SVirtualShapeDesignerDrawer::ClearDrawDots()
{
	DrawDots.Empty();
}

void SVirtualShapeDesignerDrawer::ClearDrawLines()
{
	DrawLines.Empty();
}


