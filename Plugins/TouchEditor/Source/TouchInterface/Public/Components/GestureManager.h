// Copyright Lost in Game Studio. All Rights Reserved.


#pragma once

//#include "CoreMinimal.h"
#include "TouchInterfaceListener.h"
#include "Settings/TouchInterfaceSettings.h"
#include "Components/ActorComponent.h"
#include "Layout/Geometry.h"
#include "Input/Events.h"
#include "GestureManager.generated.h"


UENUM(BlueprintType)
enum class ESwipeDirection : uint8
{
	Up,
	Right,
	Down,
	Left
};

USTRUCT(BlueprintType)
struct FSwipeData
{
	GENERATED_BODY()

	UPROPERTY(Category="Swipe Data", BlueprintReadOnly)
	ESwipeDirection Direction;

	UPROPERTY(Category="Swipe Data", BlueprintReadOnly)
	FVector2D BeginPosition;
	
	UPROPERTY(Category="Swipe Data", BlueprintReadOnly)
	FVector2D EndPosition;

	UPROPERTY(Category="Swipe Data", BlueprintReadOnly)
	FVector2D Offset;

	UPROPERTY(Category="Swipe Data", BlueprintReadOnly)
	float Velocity;

	UPROPERTY(Category="Swipe Data", BlueprintReadOnly)
	float Duration;

	FSwipeData() :
	Direction(ESwipeDirection::Down),
	BeginPosition(ForceInitToZero),
	EndPosition(ForceInitToZero),
	Offset(ForceInitToZero),
	Velocity(0.0f),
	Duration(0.0f)
	{
		
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSimpleGestureSignature, int32, FingerIndex, FVector2D, AbsolutePosition, FVector2D, LocalPosition);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComplexGesture, int32, FingerIndex, float, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSwipeGestureSignature, int32, FingerIndex, FSwipeData, SwipeData);

enum class ETouchMode : uint8
{
	SingleTouch,
	DoubleTouch,
	MultiTouch
};

struct FFingerData
{
	uint32 PointerIndex;
	
	/** Time when tap was detected */
	double LastTapTime;
	float PressDuration;
	double TouchStartTime;
	
	FVector2D BeginPosition;
	FVector2D LastPosition;
	FVector2D OffsetFromBegin;

	bool AcceptPointerIndex(const uint32 InPointerIndex) const
	{
		return PointerIndex == InPointerIndex;
	}

	FFingerData() :
	PointerIndex(-1),
	LastTapTime(0.0f),
	PressDuration(0.0f),
	TouchStartTime(0.0f),
	BeginPosition(ForceInitToZero),
	LastPosition(ForceInitToZero),
	OffsetFromBegin(ForceInitToZero),
	bDetected(false),
	bFirstTouchDetected(false),
	bDoubleTapDetected(false),
	bLongPressDetected(false),
	bDragDetected(false)
	{
		FMemory::Memzero(this, sizeof(*this));
	}

	FFingerData(const uint32 InPointerIndex, const FVector2D InStartPosition, const double InStartTime) :
	PointerIndex(InPointerIndex),	
	LastTapTime(0.0f),
	PressDuration(0.0f),
	TouchStartTime(InStartTime),
	BeginPosition(InStartPosition),
	LastPosition(InStartPosition),
	OffsetFromBegin(ForceInitToZero),
	bDetected(false),
	bFirstTouchDetected(false),
	bDoubleTapDetected(false),
	bLongPressDetected(false),
	bDragDetected(false)
	{
		
	}
	
private:
	friend class UGestureManager;

	uint8 bDetected:1;
	uint8 bFirstTouchDetected:1;
	uint8 bDoubleTapDetected:1;
	uint8 bLongPressDetected:1;
	uint8 bDragDetected:1;
	
	/** Reset finger data */
	void Reset()
	{
		PointerIndex = -1;
		LastTapTime = 0.0f;
		PressDuration = 0.0f;
		TouchStartTime = 0.0f;
		BeginPosition = FVector2D::ZeroVector;
		LastPosition = FVector2D::ZeroVector;
		OffsetFromBegin = FVector2D::ZeroVector;
		bDetected = false;
		bFirstTouchDetected = false;
		bDoubleTapDetected = false;
		bLongPressDetected = false;
		bDragDetected = false;
	}
};

/**
 * A Component that work with Touch Interface to handle Gestures
 */
UCLASS(ClassGroup=(TouchInterfaceDesigner), meta=(BlueprintSpawnableComponent))
class TOUCHINTERFACE_API UGestureManager : public UTouchInterfaceListener
{
	GENERATED_BODY()

public:
	UGestureManager();

protected:
	virtual void BeginPlay() override;

public:	
	virtual bool OnTouchStarted(const FGeometry& Geometry, const FPointerEvent& Events) override;
	virtual void OnTouchMoved(const FGeometry& Geometry, const FPointerEvent& Events) override;
	virtual void OnTouchEnded(const FGeometry& Geometry, const FPointerEvent& Events) override;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void HandleOnOrientationChanged() override;

private:
	/** Reset gesture data */
	void ResetGesture();

	/** Use the new Gesture Recognizer algorithm.
	 * Warning! This is currently experimental. Some feature are not working as expected or are missing. */
	//UPROPERTY(Category="Experimental", EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	bool bUseNewAlgorithm;
	
	/** Enable solo mode which detects gestures for each finger separately.
	 * Disables two-finger gestures like zooming or rotating.
	 * bUseNewAlgorithm should be enabled */
	//UPROPERTY(Category="Experimental", EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess="true", EditCondition="bUseNewAlgorithm"))
	bool bSoloMode;

	UPROPERTY(Category="Gesture Config", EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	bool bOverrideGestureConfig;

	UPROPERTY(Category="Gesture Config", EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess="true", EditCondition="bOverrideGestureConfig", EditConditionHides))
	FGesturesConfig GesturesConfig;

public:
	//EVENTS

	/** Called when user move its finger on screen
	 * Send Delta from start and delta between last movement
	 */
	UPROPERTY(Category="Gesture Manager|Gestures", BlueprintAssignable)
	FOnSimpleGestureSignature OnMoveDelta;
	
	/** Called when user Tap on touch screen.
	 * Note : GestureRecognizer should be activated and Tap Bitflag enabled
	 */
	UPROPERTY(Category="Gesture Manager|Gestures", BlueprintAssignable)
	FOnSimpleGestureSignature OnTap;
	
	/** Called when the user tap twice.
	 * see DoubleTapTime in settings
	 */
	UPROPERTY(Category="Gesture Manager|Gestures", BlueprintAssignable)
	FOnSimpleGestureSignature OnDoubleTap;
	
	/** Called when the user's finger remains in contact with the touchscreen without moving for a specified time.
	 * see LongPressDuration in settings
	 */
	UPROPERTY(Category="Gesture Manager|Gestures", BlueprintAssignable)
	FOnSimpleGestureSignature OnLongPress;
	
	/** Called when the user's finger move after LongPress gesture */
	UPROPERTY(Category="Gesture Manager|Gestures", BlueprintAssignable)
	FOnSimpleGestureSignature OnDrag;

	/** Called at the end of drag whe the user's finger leave the screen */
	UPROPERTY(Category="Gesture Manager|Gestures", BlueprintAssignable)
	FOnSimpleGestureSignature OnDragEnd;
	
	/** Called when user's finger move rapidly in direction */
	UPROPERTY(Category="Gesture Manager|Gestures", BlueprintAssignable)
	FOnSwipeGestureSignature OnSwipe;
	
	/** Called when two close fingers move rapidly in one direction. */
	UPROPERTY(Category="Gesture Manager|Gestures", BlueprintAssignable)
	FOnSwipeGestureSignature OnTwoFingerSwipe;
	
	/** Called when user spread or bring one's fingers together (zoom in/out) */
	UPROPERTY(Category="Gesture Manager|Gestures", BlueprintAssignable)
	FOnComplexGesture OnZoom;
	
	/** Called when user performs a rotational movement with his two fingers */
	UPROPERTY(Category="Gesture Manager|Gestures", BlueprintAssignable)
	FOnComplexGesture OnRotate;
	
private:
	FGesturesConfig Config;
	
	TArray<FFingerData> GestureData;

	ETouchMode CurrentTouchMode;

	/** Time when tap was detected */
	double LastTapTime;
	float PressDuration;
	double Finger0_TouchStartTime;
	double CurrentTime;
	
	FVector2D Finger0_BeginPosition;
	FVector2D Finger0_BeginAbsolutePosition;
	FVector2D Finger0_LastPosition;
	FVector2D Finger0_OffsetFromBegin;
	
	FVector2D Finger1_BeginPosition;
	FVector2D Finger1_LastPosition;
	FVector2D Finger1_OffsetFromBegin;

	FVector2D StartDirection;
	
	uint32 PointerIndex;

	uint8 bFinger0_Detected:1;
	uint8 bFinger1_Detected:1;

	uint8 bFingersCloseTogetherOnStart:1;

	uint8 bFirstTouchDetected:1;
	uint8 bDoubleTapDetected:1;
	uint8 bLongPressDetected:1;
	uint8 bDragDetected:1;

	float StartOffsetBetweenFinger;
	float LastZoomOffset;
	float LastRotationOffset;

	float TotalZoomOffset;
	float TotalRotationAngle;	
};