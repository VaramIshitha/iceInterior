// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "VirtualShape.generated.h"

UENUM(meta=(Bitflags, UseEnumValuesAsMaskValuesInEditor="true"))
enum class EDetectionFilter
{
	None = 0x0 UMETA(Hidden),

	// Test the correspondence of the shapes according to the angles (number of angles, value of the angles, etc.)
	AngleBased = 0x01,

	// Test the correspondence of the shapes according to the lines (number of lines, direction, etc.)
	LineBased = 0x02,

	/** Test the correspondence of the Shape according to the points (number of points, position, etc)
	 * This algorithm is very restrictive
	 */
	DotBased = 0x04
};
ENUM_CLASS_FLAGS(EDetectionFilter)

USTRUCT(BlueprintType)
struct FDotData
{
	GENERATED_BODY()

	UPROPERTY()
	FVector2D Location;

	UPROPERTY()
	uint8 bIsHovered:1;

	UPROPERTY()
	uint8 bIsSelected:1;

	UPROPERTY()
	uint8 bIsStartPoint:1;

	UPROPERTY()
	uint8 bIsEndPoint:1;
	
	bool IsPrimaryPoint() const { return bIsStartPoint || bIsEndPoint; }

	FLinearColor GetColor() const
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
	}

	void SetIsHover(const bool bIsHover) { bIsHovered = bIsHover; }
	void SetIsSelected(const bool bSelect) { bIsSelected = bSelect; }

	FDotData()
	: Location(ForceInitToZero)
	, bIsHovered(false)
	, bIsSelected(false)
	, bIsStartPoint(false)
	, bIsEndPoint(false)
	{
		
	}

	explicit FDotData(const FVector2D InLocation, const bool bIsFirstPoint = false, const bool bSelect = false)
	: Location(InLocation)
	, bIsHovered(false)
	, bIsSelected(bSelect)
	, bIsStartPoint(bIsFirstPoint)
	, bIsEndPoint(false)
	{
		
	}

	/*explicit FDotData(const FVector2D InLocation, const bool bIsFirstPoint = false, const bool bSelect = false, const bool bInIsEndPoint = false)
	: Location(InLocation)
	, bIsHovered(false)
	, bIsSelected(bSelect)
	, bIsStartPoint(bIsFirstPoint)
	, bIsEndPoint(bInIsEndPoint)
	{
		
	}*/
};

USTRUCT(BlueprintType)
struct FShapeLine
{
	GENERATED_BODY()

	UPROPERTY()
	FVector2D StartPosition;

	UPROPERTY()
	FVector2D EndPosition;

	UPROPERTY()
	FVector2D Direction;

	UPROPERTY()
	uint8 bHasEndPoint:1;

	UPROPERTY()
	float Lenght;

	FShapeLine()
	: StartPosition(ForceInitToZero)
	, EndPosition(ForceInitToZero)
	, Direction(ForceInitToZero)
	, bHasEndPoint(false)
	, Lenght(0.0f)
	, bHasDirection(false)
	{
		
	}

	explicit FShapeLine(FVector2D InStartPosition)
	: StartPosition(InStartPosition)
	, EndPosition(ForceInitToZero)
	, Direction(ForceInitToZero)
	, bHasEndPoint(false)
	, Lenght(0.0f)
	, bHasDirection(false)
	{
		
	}

	FShapeLine(const FVector2D InStartPosition, const FVector2D InEndPosition, const FVector2D InDirection, float InLenght)
	: StartPosition(InStartPosition)
	, EndPosition(InEndPosition)
	, Direction(InDirection)
	, bHasEndPoint(false)
	, Lenght(InLenght)
	, bHasDirection(false)
	{
		
	}

private:
	friend class SVirtualShapeDesignerViewport;

	uint8 bHasDirection:1;
};

USTRUCT(BlueprintType)
struct FShapeAngle
{
	GENERATED_BODY()

	UPROPERTY()
	FVector2D CornerPosition;
	
	UPROPERTY()
	float Angle;

	UPROPERTY()
	uint8 bIsRightCornerAngle:1;

	FShapeAngle()
	: CornerPosition(ForceInitToZero)
	, Angle(0.0f)
	, bIsRightCornerAngle(false)
	{
		
	}

	FShapeAngle(FVector2D InPosition, float InAngle, bool InIsRightCornerAngle)
	: CornerPosition(InPosition)
	, Angle(InAngle)
	, bIsRightCornerAngle(InIsRightCornerAngle)
	{
		
	}
};

/**
 * 
 */
UCLASS()
class TOUCHINTERFACE_API UVirtualShape : public UObject
{
	GENERATED_BODY()

public:
	UVirtualShape();
		
	/** Full detection
	 * @return Get matching score for this shape
	 */
	float Evaluate(const TArray<FShapeLine>& Lines, const TArray<FShapeAngle>& Angles, const TArray<FDotData>& Dots) const;

	//Getters exposed to blueprint
	TArray<FShapeLine> GetShapeLines() const { return ShapeLines; }
	TArray<FShapeAngle> GetShapeAngles() const { return ShapeAngles; }
	
	TArray<FDotData> GetDotData() { return DotData; }
	TArray<FDotData>& GetDotDataByRef() { return DotData; }

#if WITH_EDITOR
 	void SetDotData(TArray<FDotData> InDotData);
#endif
	
	
	UPROPERTY(Category="Virtual Shape", EditAnywhere, BlueprintReadOnly)
	FName Name;

	UPROPERTY(Category="Virtual Shape", EditAnywhere, BlueprintReadOnly)
	FText FriendlyName;

	/** Choose the filter that is used to detect shape */
	UPROPERTY(Category="Virtual Shape", EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true", Bitmask, BitmaskEnum="/Script/TouchInterface.EDetectionFilter"))
	int32 Filters;
	
	//Todo: Choose algo type when is available
	
	UPROPERTY(Category="Virtual Shape", EditAnywhere, BlueprintReadOnly)
	uint8 bSameLenght:1;

	UPROPERTY(Category="Virtual Shape", EditAnywhere, BlueprintReadOnly)
	uint8 bSameOrientation:1;

	UPROPERTY(Category="Virtual Shape", EditAnywhere, BlueprintReadOnly)
	uint8 bClosedShape:1;

	//Draw order should be exactly the same as this shape
	UPROPERTY(Category="Virtual Shape", EditAnywhere, BlueprintReadOnly)
	uint8 bSameOrder:1;

	/**Draw direction should be the same as this shape with tolerance
	 * If disabled, the direction is not taken into account by algorithm
	 */
	UPROPERTY(Category="Virtual Shape", EditAnywhere, BlueprintReadOnly)
	uint8 bSameDirection:1;

	UPROPERTY(Category="Virtual Shape", EditAnywhere, BlueprintReadOnly)
	TArray<FDotData> DotData;

	UPROPERTY(Category="Virtual Shape", EditAnywhere, BlueprintReadOnly)
	TArray<FDotData> DotDataRelativeToBoundCenter;

	UPROPERTY(Category="Virtual Shape", EditAnywhere, BlueprintReadOnly)
	TArray<FShapeLine> ShapeLines;

	UPROPERTY(Category="Virtual Shape", EditAnywhere, BlueprintReadOnly)
	TArray<FShapeAngle> ShapeAngles;

	UPROPERTY(Category="Virtual Shape", EditAnywhere, BlueprintReadOnly)
	float TotalLenght;

	UPROPERTY(Category="Virtual Shape", EditAnywhere, BlueprintReadOnly)
	float TotalAngleValue;

	UPROPERTY(Category="Virtual Shape", EditAnywhere, BlueprintReadOnly)
	FVector2D SumOfLineDirections;

	UPROPERTY(Category="Virtual Shape", EditAnywhere, BlueprintReadOnly)
	FVector2D TopLeftBound;
	
	UPROPERTY(Category="Virtual Shape", EditAnywhere, BlueprintReadOnly)
	FVector2D BottomRightBound;

	UPROPERTY(Category="Virtual Shape", EditAnywhere, BlueprintReadOnly)
	FVector2D BoundCenter;
	
#if WITH_EDITORONLY_DATA
	UPROPERTY(Category="Virtual Shape", EditAnywhere, BlueprintReadOnly)
	FSlateBrush ReferenceImage;
#endif

private:
	void CalculateDataBasedOnBound(const FVector2D& InBoundCenter);
	void DotsPositionFilter();
	float CalculateScale(const FVector2D DrawTopLeftBound, const FVector2D DrawBottomRightBound);

	float EvaluateAngles(const TArray<FShapeAngle>& Angles) const;
	float EvaluateLines(const TArray<FShapeLine>& Lines) const;
	float EvaluateDot(const TArray<FDotData>& Dots) const;
};
