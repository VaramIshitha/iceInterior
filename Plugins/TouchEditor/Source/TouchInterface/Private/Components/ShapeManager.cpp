// Copyright Lost in Game Studio. All Rights Reserved.

#include "Components/ShapeManager.h"
#include "TouchInterfaceSettings.h"
#include "VirtualShape.h"

//Needed for mobile
#include "TimerManager.h"
#include "Engine/World.h"
//Needed for mobile

DEFINE_LOG_CATEGORY_STATIC(LogShapeManager, All, All);

// Sets default values for this component's properties
UShapeManager::UShapeManager()
{
	PrimaryComponentTick.bCanEverTick = false;
	bCloseShape = false;
	bMaxCloseDistance = 10.0f;
	PreviousDotPosition = FVector2D::ZeroVector;
	RestrictedVirtualShapeName = NAME_None;
}

// Called when the game starts
void UShapeManager::BeginPlay()
{
	Super::BeginPlay();

	ShapeDotDistance = GetDefault<UTouchInterfaceSettings>()->ShapeDotDistance;
	//CornerDetectionThreshold = GetDefault<UTouchInterfaceSettings>()->CornerDetectionThreshold;
}

bool UShapeManager::OnTouchStarted(const FGeometry& Geometry, const FPointerEvent& Events)
{
	bool Return = Super::OnTouchStarted(Geometry, Events);
	
	const FVector2D LocalCoord = Geometry.AbsoluteToLocal(Events.GetScreenSpacePosition());
	
	if (Events.GetPointerIndex() == 0)
	{
		if (bIsComputing)
		{
			return Return;
		}

		//Todo: Use Timer, right click or button to define when computation start. Option in settings (Quick settings in toolbar)
		if (!bLaunchTimer)
		{
			UserDrawing.Empty();
		}
		else
		{
			bLaunchTimer = false;
			GetWorld()->GetTimerManager().ClearTimer(ComputeTimer);
		}

		UserDrawing.Add(FDotData(LocalCoord, true));
		bIsDrawing = true;
	}

	return Return;
}

void UShapeManager::OnTouchMoved(const FGeometry& Geometry, const FPointerEvent& Events)
{
	Super::OnTouchMoved(Geometry, Events);
	
	if (Events.GetPointerIndex() == 0)
	{
		const FVector2D LocalCoord = Geometry.AbsoluteToLocal(Events.GetScreenSpacePosition());
	
		if (bIsDrawing)
		{
			if (FVector2D::Distance(UserDrawing.Last().Location, LocalCoord) > ShapeDotDistance)
			{			
				UserDrawing.Add(FDotData(LocalCoord));
			}
		}
	}
}

void UShapeManager::OnTouchEnded(const FGeometry& Geometry, const FPointerEvent& Events)
{
	Super::OnTouchEnded(Geometry, Events);

	if (Events.GetPointerIndex() == 0)
	{
		if (bIsDrawing)
		{
			bIsDrawing = false;
	
			const FVector2D LocalCoord = Geometry.AbsoluteToLocal(Events.GetScreenSpacePosition());

			// Test if localCoord is near to first point location
			if (FVector2D::Distance(UserDrawing[0].Location, LocalCoord) <= ShapeDotDistance)
			{
				// If so, set the location of last point to location of first point
				UserDrawing.Last().Location = UserDrawing[0].Location;
			}

			UserDrawing.Last().bIsEndPoint = true;
		
			bLaunchTimer = true;
			FTimerDelegate FunctionToCall = FTimerDelegate::CreateUObject(this, &UShapeManager::TryRecognizeShape);
			GetWorld()->GetTimerManager().SetTimer(ComputeTimer, FunctionToCall, GetDefault<UTouchInterfaceSettings>()->DelayBetweenEndDrawAndComputation, false);
		}
	}
}

void UShapeManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UShapeManager::HandleOnOrientationChanged()
{
	Super::HandleOnOrientationChanged();
}

bool UShapeManager::RestrictRecognizerToShapeByName(const FName VirtualShapeName)
{
	for (UVirtualShape* VirtualShape : VirtualShapes)
	{
		if (VirtualShape->Name.IsEqual(VirtualShapeName))
		{
			RestrictedVirtualShapeName = VirtualShapeName;
			return true;
		}
	}

	return false;
}

void UShapeManager::RestrictRecognizerToShapeByAsset(const UVirtualShape* InVirtualShape)
{
	for (UVirtualShape* VirtualShape : VirtualShapes)
	{
		if (InVirtualShape == VirtualShape)
		{
			RestrictedVirtualShapeName = VirtualShape->Name;
		}
	}
}

void UShapeManager::TryRecognizeShape()
{
	Compute();
}

void UShapeManager::Compute()
{	
	bIsComputing = true;

	UE_LOG(LogShapeManager, Log, TEXT("Start Computing"));

	const float CornerDetectionThreshold = GetDefault<UTouchInterfaceSettings>()->CornerDetectionThreshold;

	//Todo: Use ShapeRecognizerCore
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
		}
		else if (UserDrawing[Itr].bIsEndPoint)
		{
			//Todo: Check if there is more than two points
			//Todo: Check if point is same direction. If not, do not take into account this point and go to the next
			FDotData NewPoint = FDotData(UserDrawing[Itr].Location);
			NewPoint.bIsEndPoint = true;
			DotData.Add(NewPoint);
			CurrentDirection = FVector2D::ZeroVector;
		}
		else
		{
			if (CurrentDirection != FVector2D::ZeroVector)
			{
				const FVector2D Direction = (UserDrawing[Itr].Location - UserDrawing[Itr-1].Location).GetSafeNormal();
				UE_LOG(LogShapeManager, Log, TEXT("[%d] Direction = %f x %f"), Itr, Direction.X, Direction.Y);
				
				UE_LOG(LogShapeManager, Log, TEXT("[%d] Dot Result = %f"), Itr, CurrentDirection | Direction);
				if ((CurrentDirection | Direction) <= CornerDetectionThreshold)
				{
					StartPointIndex = DotData.Add(FDotData(UserDrawing[Itr-1].Location));
					CurrentDirection = FVector2D::ZeroVector;
				}
			}
			else
			{
				CurrentDirection = (UserDrawing[Itr].Location - DotData[StartPointIndex].Location).GetSafeNormal();
				UE_LOG(LogShapeManager, Log, TEXT("[%d] Direction Ref = %f x %f"), Itr, CurrentDirection.X, CurrentDirection.Y);
			}
		}
	}

	UE_LOG(LogShapeManager, Log, TEXT("Num = %d"), DotData.Num());
	
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
	UE_LOG(LogShapeManager, Log, TEXT("TopLeft = %f x %f | BottomRight = %f x %f | Center = %f x %f"),
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
	
	if (!RestrictedVirtualShapeName.IsNone())
	{
		for (const UVirtualShape* VirtualShape : VirtualShapes)
		{
			if (VirtualShape->Name.IsEqual(RestrictedVirtualShapeName))
			{
				const float Score = VirtualShape->Evaluate(ShapeLines, ShapeAngles, DotData);
				if (Score > 60.0f)
				{
					OnShapeDetectionSuccess.Broadcast(VirtualShape->Name, VirtualShape);
					return;
				}
			}
		}
		
		OnShapeDetectionFailed.Broadcast();
		return;
	}

	float MatchingScore = 0.0f;
	UVirtualShape* DetectedShape = nullptr;
	
	for (UVirtualShape* Shape : VirtualShapes)
	{
		//Todo: use struct in UVirtualShape and pass data in evaluate function
		const float Score = Shape->Evaluate(ShapeLines, ShapeAngles, DotData);
		if (Score > MatchingScore)
		{
			MatchingScore = Score;
			DetectedShape = Shape;

			//Todo: Break loop if matching score is high like 0.98f ?
		}
	}

	UE_LOG(LogShapeManager, Log, TEXT("Best Matching Score = %f"), MatchingScore);

	if (MatchingScore > GetSettings()->MinMatchingScoreToTriggerEvent)
	{
		//Todo: Add matching score in delegate ?
		OnShapeDetectionSuccess.Broadcast(DetectedShape->Name, DetectedShape);
	}
	else
	{
		OnShapeDetectionFailed.Broadcast();
	}

	bIsComputing = false;
	UserDrawing.Empty();
	DotData.Empty();
}

void UShapeManager::PredictiveCompute()
{
	//Todo: Each time a new point is added, launch comparison. Async ?
}

void UShapeManager::ClearData()
{
	UserDrawing.Empty();
}

void UShapeManager::CalculateDataBasedOnBound(const FVector2D& BoundCenter)
{
	TArray<FDotData> ScaledDotData;

	for (const FDotData& Dot : DotData)
	{
		FDotData NewDot = FDotData();
		NewDot.Location = BoundCenter - Dot.Location;
		NewDot.bIsStartPoint = Dot.bIsStartPoint;
		NewDot.bIsEndPoint = Dot.bIsEndPoint;
		ScaledDotData.Add(NewDot);
	}

	DotData = ScaledDotData;
}
