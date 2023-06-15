// Copyright Lost in Game Studio. All Rights Reserved.

#include "VirtualShape.h"

DEFINE_LOG_CATEGORY_STATIC(LogVirtualShape, All, All);

/*FLinearColor FDotData::GetColor() const
{
	if (bIsSelected)
	{
		return FLinearColor::Green;
	}

	if (bIsHovered)
	{
		return FLinearColor(FColor::Cyan);
	}

	return FLinearColor::White;
}*/

UVirtualShape::UVirtualShape()
{
	
}

float UVirtualShape::Evaluate(const TArray<FShapeLine>& Lines, const TArray<FShapeAngle>& Angles, const TArray<FDotData>& Dots) const
{
	//Todo: Be careful of the size of the bound, apply scale to fit the bound of user draw

	float MatchingScore = 0.0f;
	int32 NumOfFilterUsed = 0;
	
	if (Filters != static_cast<int32>(EDetectionFilter::None))
	{
		//Todo: Make filter for dots that is too far from other and center (option)
		//Todo: Make filter for dots that is too near each other
		//Todo: Scale position based on bound
		
		if (Filters & (uint8)EDetectionFilter::AngleBased)
		{
			MatchingScore += EvaluateAngles(Angles);
			NumOfFilterUsed++;
		}

		if (Filters & static_cast<int32>(EDetectionFilter::LineBased))
		{
			MatchingScore += EvaluateLines(Lines);
			NumOfFilterUsed++;
		}

		if (Filters & static_cast<int32>(EDetectionFilter::DotBased))
		{
			MatchingScore += EvaluateDot(Dots);
			NumOfFilterUsed++;
		}		
	}

	return MatchingScore / NumOfFilterUsed;
}

#if WITH_EDITOR
void UVirtualShape::SetDotData(TArray<FDotData> InDotData)
{
	DotData = InDotData;

	constexpr float MaxValue =  TNumericLimits<float>::Max();

	TopLeftBound = FVector2D(MaxValue);
	BottomRightBound = FVector2D(0.0f);

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

	BoundCenter = TopLeftBound + (BottomRightBound - TopLeftBound) * 0.5f;

	UE_LOG(LogVirtualShape, Log, TEXT("TopLeftBound = %f x %f | BottomRightBound = %f x %f | Center of Bound = %f x %f"),
		TopLeftBound.X, TopLeftBound.Y, BottomRightBound.X, BottomRightBound.Y, BoundCenter.X, BoundCenter.Y);

	CalculateDataBasedOnBound(BoundCenter);
	
	ShapeLines.Empty();
	ShapeAngles.Empty();
	SumOfLineDirections = FVector2D::ZeroVector;
	TotalLenght = 0.0f;
	
	for (int32 Itr = 0; Itr < DotDataRelativeToBoundCenter.Num()-1; ++Itr)
	{
		const FDotData& Dot = DotDataRelativeToBoundCenter[Itr];
		const FDotData& NextDot = DotDataRelativeToBoundCenter[Itr+1];

		if (!Dot.bIsEndPoint)
		{
			const FVector2D Direction = (NextDot.Location - Dot.Location).GetSafeNormal();
			SumOfLineDirections += Direction;
			const float Lenght = FVector2D::Distance(Dot.Location, NextDot.Location);
			TotalLenght += Lenght;
			ShapeLines.Add(FShapeLine(Dot.Location, NextDot.Location, Direction, Lenght));			
		}
		else
		{
			ShapeLines.Last().bHasEndPoint = true;
			UE_LOG(LogVirtualShape, Log, TEXT("Has End Point"));
		}
		
		UE_LOG(LogVirtualShape, Log, TEXT("New Line : Start = %f x %f | End = %f x %f | Direction = %f x %f | Lenght = %f"),
			ShapeLines.Last().StartPosition.X, ShapeLines.Last().StartPosition.Y, ShapeLines.Last().EndPosition.X, ShapeLines.Last().EndPosition.Y,
			ShapeLines.Last().Direction.X, ShapeLines.Last().Direction.Y, ShapeLines.Last().Lenght);
	}

	constexpr float DegreeConversion = 180.0f/PI;
	TotalAngleValue = 0.0f;

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
				UE_LOG(LogVirtualShape, Log, TEXT("Add new angle : Position = %f x %f | Angle = %f"), ShapeLines[Itr].EndPosition.X, ShapeLines[Itr].EndPosition.Y, AngleDegree);
			}
		}
	}
}
#endif

void UVirtualShape::CalculateDataBasedOnBound(const FVector2D& InBoundCenter)
{
	DotDataRelativeToBoundCenter.Empty();

	for (const FDotData& Dot : DotData)
	{
		FDotData NewDot = FDotData();
		NewDot.Location = InBoundCenter - Dot.Location;
		NewDot.bIsStartPoint = Dot.bIsStartPoint;
		NewDot.bIsEndPoint = Dot.bIsEndPoint;
		DotDataRelativeToBoundCenter.Add(NewDot);
	}
}

void UVirtualShape::DotsPositionFilter()
{
	//Todo: Filter for dot that is too far from center of bound
}

float UVirtualShape::CalculateScale(const FVector2D DrawTopLeftBound, const FVector2D DrawBottomRightBound)
{
	return 1.0f;
}

float UVirtualShape::EvaluateAngles(const TArray<FShapeAngle>& Angles) const
{
	// Begin at 100% and down each time criteria is too far from ref
	float AngleScore = 100.0f;

	// If there is no angle data, return 0 because it mean the shape drawn by user does
	if ((ShapeAngles.Num() > 0 && Angles.Num() <= 0) || (ShapeAngles.Num() <= 0 && Angles.Num() > 0)) return 0.0f;

	// If Virtual Shape and Shape drawn by user doesn't contain any angle, return 100.0f because no test can be executed
	if (ShapeAngles.Num() <= 0 && Angles.Num() <= 0) return 100.0f;

	//Todo: If there is one angle and angle fail, return 0
	
	// Test Angle Quantity
	int32 Fail = FMath::Abs(ShapeAngles.Num() - Angles.Num());
	AngleScore -= 10.0f * Fail; //Todo: Add multiplier when there is more en more angle

	UE_LOG(LogVirtualShape, Log, TEXT("Angle Score : %f"), AngleScore);

	TArray<float> AngleValues;
	for (const FShapeAngle& Angle : ShapeAngles)
	{
		AngleValues.Add(Angle.Angle);
	}

	int32 TestAngleFail = AngleValues.Num();
	
	// Test each angle value
	for (const FShapeAngle& Angle : Angles)
	{		
		for (int32 Itr = 0; Itr < AngleValues.Num(); ++Itr)
		{
			if (FMath::IsNearlyEqual(Angle.Angle, AngleValues[Itr], 5.0f))
			{
				--TestAngleFail;
				AngleValues.RemoveAt(Itr);
				break;
			}
		}
	}

	// If there is too many angle that not corresponding, return 0
	if (TestAngleFail > 2)
	{
		UE_LOG(LogVirtualShape, Log, TEXT("Too many angle fail, return 0"));
		return 0.0f;
	}

	AngleScore -= 15.0f * TestAngleFail;

	UE_LOG(LogVirtualShape, Log, TEXT("Angle Fail = %d | Angle Score : %f"), TestAngleFail, AngleScore);
	
	// Test sum of angle values

	float Sum = 0.0f;
	for (const FShapeAngle& Angle : Angles)
	{
		Sum += Angle.Angle;
	}

	UE_LOG(LogVirtualShape, Log, TEXT("Ref Sum = %f | Sum = %f"), TotalAngleValue, Sum);
	
	float SumAngleValueMatching = FMath::Abs(TotalAngleValue - Sum);
	AngleScore -= SumAngleValueMatching /**Add multiplier*/;

	UE_LOG(LogVirtualShape, Log, TEXT("Angle Score : %f"), AngleScore);
	
	//Todo: Angle position -> should be normalized based on bound and scaled
	
	return FMath::Clamp(AngleScore, 0.0f, 100.0f);
}

float UVirtualShape::EvaluateLines(const TArray<FShapeLine>& Lines) const
{
	float LineScore = 100.0f;

	// Test Line Quantity
	int32 Fail = FMath::Abs(ShapeLines.Num() - Lines.Num());
	LineScore -= 20.0f * Fail;

	UE_LOG(LogVirtualShape, Log, TEXT("Line Score : %f"), LineScore);

	TArray<FVector2D> LineDirections;
	for (const FShapeLine& Line : ShapeLines)
	{
		LineDirections.Add(Line.Direction);
	}

	//Todo: User can draw square in different direction (start almost anything) so keep this in mind for comparison

	int32 TestDirectionFail = LineDirections.Num();
	
	// Test each line direction
	for (const FShapeLine& Line : Lines)
	{		
		for (int32 Itr = 0; Itr < LineDirections.Num(); ++Itr)
		{
			//Todo: Add a property to define if a line must have the same direction. If it is not the case, the direction is also tested in reverse.
			//Todo: Line lenght -> should be normalized based on bound
			UE_LOG(LogVirtualShape, Log, TEXT(""))
			if ((Line.Direction | LineDirections[Itr]) >= 0.85f)
			{
				--TestDirectionFail;
				LineDirections.RemoveAt(Itr);
				break;
			}
		}
	}

	// If there is too many direction that not corresponding, return 0
	if (TestDirectionFail > 2)
	{
		UE_LOG(LogVirtualShape, Log, TEXT("Too many line direction fail, return 0"));
		return 0.0f;
	}

	LineScore -= 15.0f * TestDirectionFail;

	UE_LOG(LogVirtualShape, Log, TEXT("Line Direction Fail = %d | Line Score : %f"), TestDirectionFail, LineScore);
	
	// Test sum of direction is similar to sum of angle
	// FVector2D Sum = FVector2D::ZeroVector;
	// for (const FShapeLine& Line : Lines)
	// {
	// 	Sum += Line.Direction;
	// }
	//
	// const FVector2D SumLineDirectionMatching = SumOfLineDirections - Sum;
	// LineScore -= SumLineDirectionMatching.X * 20.0f;
	// LineScore -= SumLineDirectionMatching.Y * 20.0f;

	UE_LOG(LogVirtualShape, Log, TEXT("Line Score : %f"), LineScore);

	return FMath::Clamp(LineScore, 0.0f, 100.0f);
}

float UVirtualShape::EvaluateDot(const TArray<FDotData>& Dots) const
{
	float DotScore = 100.0f;

	// Test point Quantity
	int32 Fail = FMath::Abs(DotData.Num() - Dots.Num());
	DotScore -= 10.0f * Fail;

	UE_LOG(LogVirtualShape, Log, TEXT("Dot Score : %f"), DotScore);

	//Todo: Check amount of start and end point
	
	//Todo: Dot location should be normalized, use bound. This algorithm is much more restrictive

	return FMath::Clamp(DotScore, 0.0f, 100.0f);
}
