// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

#include "VirtualShape.h"
#include "Components/TouchInterfaceListener.h"
#include "ShapeManager.generated.h"

class UVirtualShape;
struct FDotData;

struct FDrawing
{
	FVector2D StartPosition;
	FVector2D EndPosition;
	FVector2D Direction;
	
	float Lenght;
	
	uint8 bHasDirection:1;
	
	FDrawing()
		: StartPosition(ForceInitToZero)
		, EndPosition(ForceInitToZero)
		, Direction(ForceInitToZero)
		, Lenght(0.0f)
		, bHasDirection(false)
	{
		
	}
	
	explicit FDrawing(const FVector2D InPosition)
	: StartPosition(InPosition)
	, EndPosition(ForceInitToZero)
	, Direction(ForceInitToZero)
	, Lenght(0.0f)
	, bHasDirection(false)
	{
		
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShapeDetectionSuccessSignature, FName, VirtualShapeName, const UVirtualShape*, Asset);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShapeDetectionFailedSignature); //Todo: Add reason

/**
 * A Component that work with Touch Interface to handle Shape Recognition
 */
UCLASS(ClassGroup=(TouchInterfaceDesigner), meta=(BlueprintSpawnableComponent))
class TOUCHINTERFACE_API UShapeManager : public UTouchInterfaceListener
{
	GENERATED_BODY()

public:
	UShapeManager();

protected:
	virtual void BeginPlay() override;

public:
	virtual bool OnTouchStarted(const FGeometry& Geometry, const FPointerEvent& Events) override;
	virtual void OnTouchMoved(const FGeometry& Geometry, const FPointerEvent& Events) override;
	virtual void OnTouchEnded(const FGeometry& Geometry, const FPointerEvent& Events) override;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void HandleOnOrientationChanged() override;

	//Todo: Add/Remove shape

	/** Only recognize one shape
	 * Find if this shape exist in VirtualShapes array. If not return false
	 */
	bool RestrictRecognizerToShapeByName(const FName VirtualShapeName);

	/** Only recognize one shape
	 * Find if this shape exist in VirtualShapes array. If not, add this shape to array
	 */
	void RestrictRecognizerToShapeByAsset(const UVirtualShape* InVirtualShape);

	void ClearRecognizerRestriction() { RestrictedVirtualShapeName = NAME_None; }

	//Todo: Show/Hide guide

	/** Call this function to launch shape Recognition
	 * For now, DO NOT CALL this function until the user has finished drawing
	 */
	virtual void TryRecognizeShape();

protected:
	// Called at the end of user drawing to recognize shape
	virtual void Compute();

	// Try to recognize the shape while the user is drawing (partial data)
	virtual void PredictiveCompute();

	virtual void ClearData();
	
private:
	void CalculateDataBasedOnBound(const FVector2D& BoundCenter);
	
	//Shape you want that this manager recognize
	UPROPERTY(Category="Shape", EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TArray<UVirtualShape*> VirtualShapes;
	
	UPROPERTY(Category="Shape", EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	uint8 bCloseShape:1;

	UPROPERTY(Category="Shape", EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true", EditCondition="bCloseShape", EditConditionHides))
	float bMaxCloseDistance;

	UPROPERTY(Category="Shape", BlueprintAssignable)
	FOnShapeDetectionSuccessSignature OnShapeDetectionSuccess;

	UPROPERTY(Category="Shape", BlueprintAssignable)
	FOnShapeDetectionFailedSignature OnShapeDetectionFailed;

	uint8 bRestricted:1;

	//UVirtualShape* RestrictedVirtualShape;
	FName RestrictedVirtualShapeName;
	
	TArray<FDotData> UserDrawing;
	TArray<FDotData> DotData;

	float ShapeDotDistance;

	uint8 bIsComputing:1;
	uint8 bIsDrawing:1;
	uint8 bLaunchTimer:1;
	FTimerHandle ComputeTimer;
	
	FVector2D PreviousDotPosition;
};
