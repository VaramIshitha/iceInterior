// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

//#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Runtime/Launch/Resources/Version.h"
#include "TouchInterfaceSettings.generated.h"

class UVirtualControlSetup;
class UVirtualShape;

UENUM(meta=(Bitflags, UseEnumValuesAsMaskValuesInEditor="true"))
enum class EGestureEvents
{
	None			= 0x0 UMETA(Hidden), //0

	//Called when finger hit screen. See TapTime in GestureConfig.
	Tap				= 0x01,				//1

	//Called when finger hit screen twice. See DoubleTapTime in GestureConfig.
	DoubleTap		= 0x02,				//2

	//Called after the finger remains motionless on the screen for x second on first contact. See LongPressDuration in GestureConfig.
	LongPress		= 0x04,				//4

	//Called after long press when finger move or when in Zoom or Rotate gesture if enabled.
	Drag			= 0x08,				//8

	//Called when user's finger move rapidly in one direction.
	Swipe			= 0x10,				//16

	//Called when two close fingers move rapidly in one direction.
	TwoFingerSwipe	= 0x20,				//32

	//Called when user spread or bring one's fingers together (zoom in/out).
	Zoom			= 0x40,				//64

	//Called when user performs a rotational movement with his two fingers.
	Rotate			= 0x80				//128
};
ENUM_CLASS_FLAGS(EGestureEvents);

USTRUCT(BlueprintType)
struct FGesturesConfig
{
	GENERATED_BODY()
	
	UPROPERTY(Category="Gesture Events", EditDefaultsOnly, meta=(Bitmask, BitmaskEnum="/Script/TouchInterface.EGestureEvents"))
	int32 Gestures;

	/** To detect Tap, the finger must maintain contact with the touchscreen for at least this amount of time. */
	UPROPERTY(Category="Double Tap", EditDefaultsOnly)
	float TapTime;
	
	/** If a Tap detected twice in this amount of time, It's considered as a Double Tap. */
	UPROPERTY(Category="Double Tap", EditDefaultsOnly)
	float DoubleTapTime;
	
	/** The amount of time in seconds you hold a finger down before it's detected as a long press. */
	UPROPERTY(Category="Long Press", EditDefaultsOnly)
	float LongPressDuration;

	/** The amount of movement allowed before the finger is no longer considered valid for a Tap, DoubleTap or Long press, until it's removed and re-pressed.
	 * If with a mouse, a value of 1 seems correct, with a finger, the value must be higher due to the contact surface with the screen. A value of 10 seems sufficient.
	 */
	UPROPERTY(Category="Long Press", EditDefaultsOnly)
	float LongPressAllowedMovement;

	/** The amount of length in pixel from start a finger should make before it's detected as a swipe. */
	UPROPERTY(Category="Swipe", EditDefaultsOnly)
	float SwipeThreshold;

	/** The amount of Velocity a finger should make before it's detected as a swipe.
	 * (Offset from start in pixel / touch duration)
	 * 0 disable this feature.
	 */
	UPROPERTY(Category="Swipe", EditDefaultsOnly)
	float SwipeVelocityThreshold;

	/** The maximum distance in pixel allowed between the two fingers to detect a two-finger swipe.
	 * 0 disable this feature due to the contact surface with the screen for finger.
	 */
	UPROPERTY(Category="Two Finger Swipe", EditDefaultsOnly)
	float TwoFingerSwipeClosenessThreshold;

	/** Used to avoid stray movements (micro movement) caused by finger contact surface with the screen.
	 * A value of 1 seems correct */
	UPROPERTY(Category="Zoom", EditDefaultsOnly)
	float MinimumDeltaToSendEvent;

	/** Allow system to send drag event when zoom or rotate gesture detected.
	 * Useful when you want to move something in combination with zoom and rotate */
	UPROPERTY(Category="Zoom", EditDefaultsOnly)
	uint8 bSendDragEventWhenZoomOrRotateGestureDetected:1;

	FGesturesConfig() :
	Gestures((uint8)EGestureEvents::None),
	TapTime(0.15f),
	DoubleTapTime(0.2f),
	LongPressDuration(0.8f),
	LongPressAllowedMovement(10.0f),
	SwipeThreshold(20.0f),
	SwipeVelocityThreshold(200.0f),
	TwoFingerSwipeClosenessThreshold(20.0f),
	MinimumDeltaToSendEvent(1.0f),
	bSendDragEventWhenZoomOrRotateGestureDetected(false)
	{
		
	}
};

UENUM()
enum class EScalingMode
{
	// No scaling
	NONE UMETA(DisplayName="Disabled"),

	// Use desktop DPI Curve based on geometry size
	DPI UMETA(DisplayName="Desktop DPI Curve"),

	// Scaling the touch interface based on the reference design size (DesiredWidth)
	DesignSize,

	// Use your own scaling algorithm
	Custom
	
	//Mobile DPI Curve based on viewport size
};

/*UENUM()
enum class EScaleSide
{
	// Use the shortest side for DPI Curve or Design Size based scaling
	ShortestSide,

	// Use the longest side for DPI Curve or Design Size based scaling
	LargestSide,

	// Use the X-axis of geometry (viewport) for DPI Curve or Design Size based scaling
	Horizontal,

	// Use the Y-axis of geometry (viewport) for DPI Curve or Design Size based scaling
	Vertical
};*/

/**
 * Settings for Touch Interface created by Touch Interface Designer
 */
UCLASS(Config=Engine, defaultConfig, meta=(DisplayName="Touch Interface"))
class TOUCHINTERFACE_API UTouchInterfaceSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UTouchInterfaceSettings();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	float GetScaleFactor(const FVector2D Size, const float LayoutScale = 1.0f, const bool InEditorMode = true) const;

	bool CustomScalingClassIsValid() const { return CustomTouchInterfaceScalingInstance != nullptr; }

	// Get the resolution size associated with a key whose value is equal to scale
	bool GetResolutionSizeFromDpiCurveScale(const float Scale, float& ResolutionSize) const;
	

	//Todo: Use SoftObjectPtr with no empty feature and make a basic interface by default
	/** In case you forget to fill in an interface in the TouchDesignerController, it will use this one by default  */
	UPROPERTY(Category="General", Config, EditAnywhere, meta=(AllowedClasses="/Script/TouchInterface.VirtualControlSetup"))
	FSoftObjectPath DefaultVirtualControlSetup;

	/** Should touch interface be shown in desktop platform */
	UPROPERTY(Config, EditAnywhere, Category="General")
	bool bShowInDesktopPlatform;

	
	/** Scaling Mode */
	UPROPERTY(Category="Scaling", Config, EditAnywhere)
	EScalingMode ScalingMode;
	
	/** The design width of Touch interface used as reference for scaling calculation in DesignSize mode */
	UPROPERTY(Category="Scaling", Config, EditAnywhere, meta=(EditCondition="ScalingMode == EScalingMode::DesignSize", EditConditionHides))
	float DesignWidth;

	/** Choose the rule for calculation of touch interface scaling */
	//UPROPERTY(Config, EditAnywhere, Category="Scaling", meta=(EditCondition="ScalingMode == EScalingMode::DesignSize", EditConditionHides))
	//EScaleSide ScaleSideRule;
	
	//UPROPERTY(Config, EditAnywhere, Category="Scaling", meta=(EditCondition="ScalingMode == EScalingMode::DPI", EditConditionHides))
	//FRuntimeFloatCurve TouchInterfaceScaleCurve;

	//Todo: Use TSoftClassPtr with NoClear meta
	/**
	 * This class will be used for touch interface scaling.
	 */
	UPROPERTY(Category="Scaling", config, EditAnywhere, meta=(MetaClass="/Script/TouchInterface.CustomTouchInterfaceScaling", EditCondition="ScalingMode == EScalingMode::Custom", EditConditionHides))
	FSoftClassPath CustomTouchInterfaceScalingClass;

	/** If you use ApplicationScale (see UserInterface settings) it scale all widget in interface.
	 * If you want to modify only the scale of virtual control, you can use this multiplier.
	 * Be careful with this parameter, too high or too low value can broke virtual controls!
	 * Can be modified in runtime.*/
	UPROPERTY(Category="Scaling", Config, EditAnywhere)
	float ScaleMultiplier;

	
	/** Enable the recognition of gestures */
	UPROPERTY(Category="Gestures Recognizer", Config, EditAnywhere)
	bool bEnableGestureRecognizer;

	/** This is the default gestures config used in Gesture Manager Component */
	UPROPERTY(Category="Gestures Recognizer", Config, EditAnywhere, meta=(EditCondition="bEnableGestureRecognizer", EditConditionHides))
	FGesturesConfig DefaultGesturesConfig;

	
	// EXPERIMENTAL! Enable detection of user-drawn shapes
	UPROPERTY(Category="Shape Recognizer", Config, EditAnywhere)
	bool bEnableShapeRecognizer;

	UPROPERTY(Category="Shape Recognizer", Config, EditAnywhere, meta=(EditCondition="bEnableShapeRecognizer", EditConditionHides, ClampMin=0.1f, ClampMax=1.0f))
	float CornerDetectionThreshold;

	UPROPERTY(Category="Shape Recognizer", Config, EditAnywhere, meta=(EditCondition="bEnableShapeRecognizer", EditConditionHides, ClampMin=0.1f, ClampMax=1.0f))
	int32 MaxPoint;

	// Default shape recognized by all shape manager components
	UPROPERTY(Category="Shape Recognizer", Config, EditAnywhere, meta=(AllowedClasses="/Script/TouchInterface.VirtualShape", EditCondition="bEnableShapeRecognizer", EditConditionHides))
	TArray<FSoftObjectPath> DefaultVirtualShapes;
	
	/** If enabled, use a timer after the user's finger leaves the screen before starting recognition.
	 * This will allow the user to draw several sections.
	 * If Timer > DelayBetweenEndDrawAndComputation, recognition starts.
	 * If this option is disabled, you need to call TryToRecognizeShape() on the corresponding Shape Manager
	 * or use Touch Interface subsystem to make a global call
	 */
	UPROPERTY(Category="Shape Recognizer", Config, EditAnywhere)
	uint8 bUseTimer:1;
	
	UPROPERTY(Category="Shape Recognizer", Config, EditAnywhere, meta=(AllowedClasses="/Script/TouchInterface.VirtualShape", EditCondition="bEnableShapeRecognizer", EditConditionHides))
	float DelayBetweenEndDrawAndComputation;

	UPROPERTY(Category="Shape Recognizer", Config, EditAnywhere)
	float MinMatchingScoreToTriggerEvent;

	/** Whether or not touch event draw lines and points (used for Shape Recognizer)
	 * If not, the points and lines is not drawn but Shape Recognizer is able to recognize shape
	 */
	UPROPERTY(Category="Shape Recognizer|Drawer", Config, EditAnywhere)
	uint8 bDrawUserShape:1;
		
	// Texture used for point
	UPROPERTY(Category="Shape Recognizer|Drawer", Config, EditAnywhere, meta=(AllowedClasses="/Script/Engine.Texture2D, /Script/Engine.Material"))
	FSoftObjectPath PointBrush;

	// Line size
	UPROPERTY(Category="Shape Recognizer|Drawer", Config, EditAnywhere, meta=(ClampMin=1.0f, ClampMax=10.0f))
	float DrawLineSize;

	// Brush size
	UPROPERTY(Category="Shape Recognizer|Drawer", Config, EditAnywhere, meta=(ClampMin=4.0f, ClampMax=10.0f))
	float DrawBrushSize;
	
	// Line color
	UPROPERTY(Category="Shape Recognizer|Drawer", Config, EditAnywhere)
	FLinearColor DrawLineColor;

	// Brush color
	UPROPERTY(Category="Shape Recognizer|Drawer", Config, EditAnywhere)
	FLinearColor DrawBrushColor;

	// Line should be draw
	UPROPERTY(Category="Shape Recognizer|Drawer", Config, EditAnywhere)
	uint8 bDrawLines:1;

	// Point/Brush should be draw
	UPROPERTY(Category="Shape Recognizer|Drawer", Config, EditAnywhere)
	uint8 bDrawPoint:1;

	/** Define the ZOrder of virtual shape drawer
	 * Larger values will cause shape drawer widget to appear on top of widget with lower values.
	 */
	UPROPERTY(Category="Shape Recognizer|Drawer", Config, EditAnywhere)
	int32 DrawerZOrder;
	

	// EXPERIMENTAL! Enable detection of device movements based on gyroscope data
	UPROPERTY(Category="Motion Recognizer", Config, EditAnywhere)
	uint8 bEnableMotionRecognizer:1;

	//Todo: Motion Recognizer config

	
	/** The minimum value of joystick (vector length) to trigger auto move. */
	UPROPERTY(Category="Inputs|Auto Move", Config, EditAnywhere, meta=(ClampMin=0.0f, ClampMax=1.0f, UIMin=0.0f, UIMax=1.0f))
	float AutoMoveThreshold;

	/** The max value tolerated for auto move detection (AngleDelta When Hold).
	 * A value of 0.98 seems correct with Fingers.
	 * Use a value of 1 for mouse.
	 */
	UPROPERTY(Category="Inputs|Auto Move", Config, EditAnywhere, meta=(ClampMin=0.9f, UIMin=0.9f, ClampMax=1.0f, UIMax=1.0f))
	float AutoMoveDirectionThreshold;

	/** The amount of time in seconds you hold a joystick to one direction before it trigger auto move.
	 * A value of 0 disable this feature */
	UPROPERTY(Category="Inputs|Auto Move", Config, EditAnywhere, meta=(ClampMin=0.0f, UIMin=0.0f))
	float AutoMoveHoldDuration;

	/** The minimum drag lenght in pixel (Y offset) before lock input to DragToSprint when finger's user end touch */
	UPROPERTY(Category="Inputs|Drag To Sprint", Config, EditAnywhere, meta=(ClampMin=0.0f, UIMin=0.0f))
	float DragToSprintTrigger;

	/** The max value tolerated for DragToSprint detection (Lenght). A value of 40 seems correct */
	UPROPERTY( Category="Inputs|Drag To Sprint", Config, EditAnywhere, meta=(ClampMin=0.0f, UIMin=0.0f))
	float DragToSprintThreshold;

	/** The minimum value of the delta (offset between the position of the finger between each frame) to send an event (input).
	 * Leave at 0 to disable this feature */
	UPROPERTY(Category="Inputs|Touch Region", Config, EditAnywhere, meta=(ClampMin=0.0f, UIMin=0.0f))
	float DeltaThreshold;

	
	/** Use Enhanced Input plugin to manage input. Use Input Action instead of Fkey.
	 * Affect Editor and runtime.
	 * Note for 4.27 Users : Enhanced Input is disabled because it does not have the necessary features.
	 */
	UPROPERTY(Category="Inputs|Enhanced Input", Config, EditAnywhere)
	bool bUseEnhancedInput;

	
	/** If true, Touch interface is automatically hidden when a gamepad is detected
	 * or when pressing a button on the connected gamepad.
	 * Touch interface appears again if the user touches the screen.
	 * Warning! On some device, the detection can be slow
	 */
	UPROPERTY( Category="Gamepad", Config, EditAnywhere)
	bool bHideTouchInterfaceWhenGamepadIsConnected;
	
	
	/** Slot name for saved configuration of Touch Interface at runtime */
	UPROPERTY(Category="Build-in Save System", Config, EditAnywhere, BlueprintReadOnly)
	FString SaveSlotName;
	
	
	/** Show some information for debugging like Fingers position, virtual control name, interaction box, etc */
	UPROPERTY( Category="Debugging", Config, EditAnywhere)
	bool bDrawDebug;

	/** Opacity of debug information */
	UPROPERTY(Category="Debugging", Config, EditAnywhere, meta=(ClampMin=0.0f, UIMin=0.0f, ClampMax=1.0f, UIMax=1.0f), meta=(EditCondition="bDrawDebug", EditConditionHides))
	float DebugOpacity;

	float ShapeDotDistance;

private:
#if ENGINE_MAJOR_VERSION > 4
	TObjectPtr<class UCustomTouchInterfaceScaling> CustomTouchInterfaceScalingInstance;
#else
	class UCustomTouchInterfaceScaling* CustomTouchInterfaceScalingInstance;
#endif
};