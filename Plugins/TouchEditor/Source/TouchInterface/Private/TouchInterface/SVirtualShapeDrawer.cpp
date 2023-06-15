// Copyright Lost in Game Studio. All Rights Reserved.

#include "SVirtualShapeDrawer.h"

#include "TouchInterfaceSettings.h"
#include "TouchInterfaceStyle.h"
#include "VirtualShape.h"

DEFINE_LOG_CATEGORY_STATIC(LogVirtualShapeDrawer, All, All);

void SVirtualShapeDrawer::Construct(const FArguments& InArgs)
{
	UE_LOG(LogVirtualShapeDrawer, Log, TEXT("Drawer Constructed!"));
	
	Settings = GetDefault<UTouchInterfaceSettings>();
	
	ShapeDotDistance = Settings->ShapeDotDistance;

	DelayBetweenEndDrawAndComputation = Settings->DelayBetweenEndDrawAndComputation;
	
	bTimerLaunched = false;

	LoadResources();
}

void SVirtualShapeDrawer::DrawStarted(const FGeometry& MyGeometry, const FPointerEvent& Event)
{
	//UE_LOG(LogVirtualShapeDrawer, Log, TEXT("Draw Started"));
	
	if (Event.GetPointerIndex() == 0)
	{
		const FVector2D LocalCoord = MyGeometry.AbsoluteToLocal(Event.GetScreenSpacePosition());

		if (bTimerLaunched)
		{
			bTimerLaunched = false;
			TimerCounter = 0.0f;
		}

		UserDrawing.Add(FDotData(LocalCoord, true));
		bIsDrawing = true;
	}
}

void SVirtualShapeDrawer::DrawUpdated(const FGeometry& MyGeometry, const FPointerEvent& Event)
{
	//UE_LOG(LogVirtualShapeDrawer, Log, TEXT("Draw Updated"));

	if (bIsDrawing)
	{
		const FVector2D LocalCoord = MyGeometry.AbsoluteToLocal(Event.GetScreenSpacePosition());
		if (FVector2D::Distance(UserDrawing.Last().Location, LocalCoord) > ShapeDotDistance)
		{			
			UserDrawing.Add(FDotData(LocalCoord));
		}
	}
}

void SVirtualShapeDrawer::DrawEnded(const FGeometry& MyGeometry, const FPointerEvent& Event)
{
	//UE_LOG(LogVirtualShapeDrawer, Log, TEXT("Draw Ended"));

	if (bIsDrawing)
	{
		bIsDrawing = false;
		
		const FVector2D LocalCoord = MyGeometry.AbsoluteToLocal(Event.GetScreenSpacePosition());

		// Test if localCoord is near to first point location
		if (FVector2D::Distance(UserDrawing[0].Location, LocalCoord) <= ShapeDotDistance)
		{
			// If so, set the location of last point to location of first point
			UserDrawing.Last().Location = UserDrawing[0].Location;
		}

		UserDrawing.Last().bIsEndPoint = true;
		
		bTimerLaunched = true;
	}
}

int32 SVirtualShapeDrawer::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
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

			if (Settings->bDrawPoint)
			{
				FSlateDrawElement::MakeBox
				(
					OutDrawElements,
					LayerId+1,
					AllottedGeometry.ToPaintGeometry(Dot.Location - FVector2D(Settings->DrawBrushSize * 0.5f), FVector2D(Settings->DrawBrushSize)),
					&PointBrush,
					ESlateDrawEffect::None,
					Settings->DrawBrushColor
				);	
			}
		}

		if (Settings->bDrawLines)
		{
			for (const TArray<FVector2D>& Line : LinePositions)
			{
				//Draw line between dot
				FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), Line, ESlateDrawEffect::None, Settings->DrawLineColor, false, Settings->DrawLineSize);
			}
		}

		//Todo: Draw strait lines (Option in settings)
	}

	return LayerId;
}

void SVirtualShapeDrawer::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SLeafWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (bTimerLaunched)
	{
		TimerCounter += InDeltaTime;
		if (TimerCounter > DelayBetweenEndDrawAndComputation)
		{
			ClearDrawData();
		}
	}
}

SVirtualShapeDrawer::~SVirtualShapeDrawer()
{
	if (BrushResource.IsValid())
	{
		BrushResource->RemoveFromRoot();
	}
}

void SVirtualShapeDrawer::ClearDrawData()
{
	bTimerLaunched = false;
	TimerCounter = 0.0f;
	UserDrawing.Empty();
	UE_LOG(LogVirtualShapeDrawer, Log, TEXT("User Drawing cleared!"));
}

void SVirtualShapeDrawer::LoadResources()
{
	const FSoftObjectPath TextureObjectPath = Settings->PointBrush;
	if (TextureObjectPath.IsValid())
	{
		//UTexture2D* TextureObject = LoadObject<UTexture2D>(nullptr, *TextureObjectPath.ToString());
		UObject* TextureObject = LoadObject<UObject>(nullptr, *TextureObjectPath.ToString());
		if (TextureObject)
		{
			//UObjectBaseUtility::AddToRoot()
			BrushResource = TextureObject;
			BrushResource->AddToRoot();
			PointBrush.SetResourceObject(BrushResource.Get());
			return;
		}
	}
	
	PointBrush = *FTouchInterfaceStyle::Get().GetBrush("WhiteCircle");
}
