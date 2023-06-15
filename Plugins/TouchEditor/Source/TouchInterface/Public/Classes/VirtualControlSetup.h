// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "SlateFwd.h"
#include "UObject/Object.h"
#include "InputCoreTypes.h"
#include "Styling/SlateBrush.h"
#include "Sound/SoundBase.h"
#include "Curves/CurveFloat.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "VirtualControlSetup.generated.h"

class STouchInterface;
class UVirtualControlEvent;

UENUM(BlueprintType)
enum class EControlType : uint8
{
	Button,
	Joystick,
	TouchRegion UMETA(DisplayName="Touch Region")
};

UENUM(BlueprintType, meta=(Bitflags, UseEnumValuesAsMaskValuesInEditor="true"))
enum class ELayerType : uint8
{
	None		= 0x0,	//0
	Background	= 0x01,	//1
	Thumb		= 0x02,	//2
	Unpressed	= 0x04,	//4
	Pressed		= 0x08,	//8
	UnHovered	= 0x10, //16
	Hovered		= 0x20	//32
};
ENUM_CLASS_FLAGS(ELayerType)

UENUM(BlueprintType)
enum class EHitTestType : uint8
{
	Square,
	Circle
	//Alpha,
	//Custom Shape
};

USTRUCT(BlueprintType)
struct FVisualLayer
{
	GENERATED_BODY()

	UPROPERTY(Category="Layer", EditAnywhere, BlueprintReadWrite)
	FSlateBrush Brush;

	UPROPERTY(Category="Layer", EditAnywhere, BlueprintReadWrite)
	uint8 bUseBrushSize:1;

	//Todo: Add anchor position

	UPROPERTY(Category="Layer", EditAnywhere, BlueprintReadWrite)
	FVector2D Offset;

	UPROPERTY(Category="Layer", EditAnywhere, BlueprintReadWrite, meta=(Bitmask, BitmaskEnum="/Script/TouchInterface.ELayerType"))
	int32 LayerType;

	UPROPERTY(Category="Layer", EditAnywhere, BlueprintReadWrite)
	FName ExposedLayerName;

	static FORCEINLINE bool Accept(int32 LayerType, TArray<ELayerType> TestLayer)
	{
		if (LayerType != (int32)ELayerType::None)
		{
			for (ELayerType Type : TestLayer)
			{
				if (LayerType & (int32)Type)
				{
				
				}
				else
				{
					return false;
				}
			}
			return true;
		}
		return false;
	}

	FVisualLayer() :
	bUseBrushSize(false),
	Offset(0.0f),
	LayerType(0)
	{
		
	}
	
	FVisualLayer(const FSlateBrush Default, const int32 InLayerType, const bool InUseBrushSize = false, const float BrushSize = 80.0f) :
	Brush(Default),
	bUseBrushSize(InUseBrushSize),
	Offset(0.0f),
	LayerType(InLayerType)
	{
		if (InUseBrushSize)
		{
			Brush.ImageSize = FVector2D(BrushSize);
		}
	}

	FVisualLayer(UTexture2D* DefaultResource, const int32 InLayerType, const bool InUseBrushSize = false, const float BrushSize = 80.0f) :
	bUseBrushSize(InUseBrushSize),
	Offset(0.0f),
	LayerType(InLayerType)
	{
		Brush.SetResourceObject(DefaultResource);
		if (InUseBrushSize)
		{
			Brush.ImageSize = FVector2D(BrushSize);
		}
	}

	FVisualLayer(UMaterialInterface* DefaultResource, const int32 InLayerType, const bool InUseBrushSize = false, const float BrushSize = 80.0f) :
	bUseBrushSize(InUseBrushSize),
	Offset(0.0f),
	LayerType(InLayerType)
	{
		Brush.SetResourceObject(DefaultResource);
		if (InUseBrushSize)
		{
			Brush.ImageSize = FVector2D(BrushSize);
		}
	}
};

//Todo: Make class instead of struct ?
USTRUCT(BlueprintType)
struct FVirtualControl
{
	GENERATED_BODY()

	//COMMON

	//Todo: Add anchor position ?
	
	/** Name of control (used both in editor and runtime) */
	UPROPERTY(Category="Virtual Control", EditAnywhere, BlueprintReadWrite)
	FName ControlName;

	/** If enabled, the control will be hidden when the interface is added to screen */
	UPROPERTY(Category="Virtual Control", EditAnywhere, BlueprintReadWrite)
	uint8 bStartHidden:1;

	/** If true, the control is re-centered when the user touches it.
	 * The maximum offset from origin is based on the interaction size.
	 */
	UPROPERTY(Category="Virtual Control", EditAnywhere, BlueprintReadWrite)
	uint8 bRecenterOnTouch:1;

	/** If true, the control blocks the input from the touch region if it is placed in its interaction zone
	 * (no interaction on Touch Region)
	 */
	UPROPERTY(Category="Virtual Control", EditAnywhere, BlueprintReadWrite)
	uint8 bBlockTouchRegion:1;
	
	/** The center point of the control in landscape mode. It's relative to screen size (0-1) */
	UPROPERTY(Category="Virtual Control", EditAnywhere, BlueprintReadWrite)
	FVector2D LandscapeCenter;

	/** The center point of the control in portrait mode. It's relative to screen size (0-1) */
	UPROPERTY(Category="Virtual Control", EditAnywhere, BlueprintReadWrite)
	FVector2D PortraitCenter;

	/** True if this control is a child. Hidden in detail panel */
	UPROPERTY(Category="Virtual Control", EditAnywhere, BlueprintReadOnly)
	uint8 bIsChild:1;

	/** Parent control name. Hidden in detail panel */
	UPROPERTY(Category="Virtual Control", EditAnywhere, BlueprintReadOnly)
	FName ParentName;
	
	/** The offset from parent control. It's absolute value */
	UPROPERTY(Category="Virtual Control", EditAnywhere, BlueprintReadOnly)
	FVector2D ParentOffset;

	/** If true, child control keep same offset when parent recenter */
	UPROPERTY(Category="Virtual Control", EditAnywhere, BlueprintReadOnly)
	uint8 bMoveWhenParentRecenter:1;

	/** Child virtual control data. Hidden in detail panel */
	UPROPERTY(Category="Virtual Control", EditAnywhere, BlueprintReadOnly)
	TArray<FName> Children;

	/** Visual for control. Configure visual with bitflag */
	UPROPERTY(Category="Virtual Control", EditAnywhere, BlueprintReadWrite)
	TArray<FVisualLayer> VisualLayers;
	
	/** The size of the control. It's absolute */
	UPROPERTY(Category="Virtual Control", EditAnywhere, BlueprintReadWrite)
	FVector2D VisualSize;

	/** Select the interaction shape to detect hit */
	UPROPERTY(Category="Virtual Control", EditAnywhere, BlueprintReadWrite)
	EHitTestType InteractionShape;
	
	/** The interactive size of the control if square. It's absolute */
	UPROPERTY(Category="Virtual Control", EditAnywhere, BlueprintReadWrite, meta=(EditCondition="InteractionShape==EHitTestType::Square", EditConditionHides))
	FVector2D InteractionSize;

	/** The control's interaction size if it's a circle. It's absolute */
	UPROPERTY(Category="Virtual Control", EditAnywhere, BlueprintReadWrite, meta=(ClampMin=1.0f, EditCondition="InteractionShape==EHitTestType::Circle", EditConditionHides))
	float InteractionRadiusSize;

	/** Define the type of control. Used internally */
	UPROPERTY(Category="Virtual Control", EditAnywhere, BlueprintReadWrite)
	EControlType Type;

	/** Define the Virtual Control Event that extends the functionality of this control */
	UPROPERTY(Category="Virtual Control", EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UVirtualControlEvent> VirtualControlEvent;
	

	//BUTTON
	
	/** The Input Action to use for button. Should be Digital (bool) */
	UPROPERTY(Category="Virtual Control|Button", EditAnywhere, BlueprintReadWrite)
	UInputAction* ButtonAction;
	
	/** The main input to send from this button
	 * Also used for press and release of joystick if bSendPressAndReleaseEvent is equal to true
	 */
	UPROPERTY(Category="Virtual Control|Button", EditAnywhere, BlueprintReadWrite)
	FKey ButtonInputKey;

	/** Sound that be played when user press button (one time) */
	UPROPERTY(Category="Virtual Control|Button", EditAnywhere, BlueprintReadWrite)
	USoundBase* PressedSound;

	//JOYSTICK

	/** For sticks, the size of the thumb. It's absolute */
	UPROPERTY(Category="Virtual Control|Joystick", EditAnywhere, BlueprintReadWrite)
	FVector2D ThumbSize;

	
	/** The main input to send from this control (for sticks, this is the horizontal axis) */
	UPROPERTY(Category="Virtual Control|Joystick", EditAnywhere, BlueprintReadWrite)
	FKey HorizontalInputKey;

	/** The alternate input to send from this control (for sticks, this is the vertical axis, not used in button) */
	UPROPERTY(Category="Virtual Control|Joystick", EditAnywhere, BlueprintReadWrite)
	FKey VerticalInputKey;

	/** The Input Action to use for joystick. Should be Axis2D */
	UPROPERTY(Category="Virtual Control|Joystick", EditAnywhere, BlueprintReadWrite)
	UInputAction* JoystickAction;

	
	/** Whether or not to send a press and release event via ActionInputKey when the user starts/finishes touching the joystick */
	UPROPERTY(Category="Virtual Control|Joystick", EditAnywhere, BlueprintReadWrite)
	uint8 bSendPressAndReleaseEvent:1;
	

	/** The scale for joystick input value (ThumbValueCurve * InputScale)
	 * Used as sensibility for Touch Region offset value
	 */
	UPROPERTY(Category="Virtual Control|Joystick", EditAnywhere, BlueprintReadWrite)
	float InputScale;

	/** The value of Thumbstick in Joystick. Should be >= 0 */
	UPROPERTY(Category="Virtual Control|Joystick", EditAnywhere, BlueprintReadWrite)
	FRuntimeFloatCurve ThumbValueCurve;

	/** Primary used for camera movement. If true, ThumbValueCurve * TurnRate * DeltaTime */
	UPROPERTY(Category="Virtual Control|Joystick", EditAnywhere, BlueprintReadWrite)
	uint8 bUseTurnRate:1;
	
	/** Speed rate of rotation */
	UPROPERTY(Category="Virtual Control|Joystick", EditAnywhere, BlueprintReadWrite)
	float TurnRate; //Todo: Clamp min value (no negative value)
	
	
	/** The joystick can be held in one direction to trigger automatic movement.
	 * See AutoMoveThreshold and AutoMoveHoldDuration in settings. */
	UPROPERTY(Category="Virtual Control|Joystick", EditAnywhere, BlueprintReadWrite)
	uint8 bAutoMove:1;

	/** Image for lock icon when auto move or drag to sprint is enabled */
	UPROPERTY(Category="Virtual Control|Joystick", EditAnywhere, BlueprintReadWrite)
	FSlateBrush LockIcon;
	

	/** if the user's finger exceeds the detection zone of the joystick then the system activates the drag to sprint function.
	 * This feature needs space above the joystick to work properly.
	 * Pay attention to the placement of the joystick which must be at the bottom of the screen */
	UPROPERTY(Category="Virtual Control|Joystick", EditAnywhere, BlueprintReadWrite)
	uint8 bDragToSprint:1;

	/** Visual layer for Sprint button.
	 * Use unhovered and hovered bitflag. */
	UPROPERTY(Category="Virtual Control|Joystick", EditAnywhere, BlueprintReadWrite)
	TArray<FVisualLayer> SprintButton;

	/** The size of the sprint button */
	UPROPERTY(Category="Virtual Control|Joystick", EditAnywhere, BlueprintReadWrite)
	FVector2D SprintButtonVisualSize;
	
	/** The sprint input to send from this control (should be button key, press/release) */
	UPROPERTY(Category="Virtual Control|Joystick", EditAnywhere, BlueprintReadWrite)
	FKey SprintInputKey;

	/** The Drag to Sprint input action to use. Should be Digital (bool) */
	UPROPERTY(Category="Virtual Control|Joystick", EditAnywhere, BlueprintReadWrite)
	UInputAction* SprintAction;

	// TOUCH REGION

	/** If enabled, Touch Region act like joystick (send input continuously) */
	UPROPERTY(Category="Virtual Control|Touch Region", EditAnywhere, BlueprintReadWrite)
	uint8 bJoystickMode:1;
	

	bool IsParent() const
	{
		return Children.Num() > 0;
	}

	void ClearLinkData()
	{
		bIsChild = false;
		ParentName = NAME_None;
		ParentOffset = FVector2D::ZeroVector;
		bMoveWhenParentRecenter = false;
		Children.Empty();
	}

	/** Default constructor */
	FVirtualControl()
	: bStartHidden(false)
	, bRecenterOnTouch(false)
	, bBlockTouchRegion(false)
	, LandscapeCenter(0.5f,0.5f)
	, PortraitCenter(0.5f,0.5f)
	, bIsChild(false)
	, ParentOffset(ForceInitToZero)
	, bMoveWhenParentRecenter(false)
	, VisualSize(60.0f,60.0f)
	, InteractionShape(EHitTestType::Square)
	, InteractionSize(60.0f,60.0f)
	, InteractionRadiusSize(10.0f)
	, Type(EControlType::Button)
	, VirtualControlEvent(nullptr)

	//Button
	, ButtonAction(nullptr)
	, PressedSound(nullptr)
	
	//Joystick
	, ThumbSize(60.0f)
	, JoystickAction(nullptr)
	, bSendPressAndReleaseEvent(false)
	, InputScale(1.0f)
	, bUseTurnRate(false)
	, TurnRate(30.0f)
	, bAutoMove(false)
	, bDragToSprint(false)
	, SprintButtonVisualSize(80.0f)
	, SprintAction(nullptr)
	, bJoystickMode(false)	
	{
		ControlName = FName(TEXT("NewControl"));
	}

	//Construct new virtual control based on type
	explicit FVirtualControl(EControlType InType)
	: bStartHidden(false)
	, bRecenterOnTouch(false)
	, bBlockTouchRegion(false)
	, LandscapeCenter(0.5f)
	, PortraitCenter(0.5f)
	, bIsChild(false)
	, ParentOffset(ForceInitToZero)
	, bMoveWhenParentRecenter(false)
	, InteractionShape(EHitTestType::Square)
	, Type(InType)
	, VirtualControlEvent(nullptr)

	//Button
	, ButtonAction(nullptr)
	, PressedSound(nullptr)

	//Joystick
	, ThumbSize(60.0f)
	, JoystickAction(nullptr)
	, bSendPressAndReleaseEvent(false)
	, InputScale(1.0f)
	, bUseTurnRate(false)
	, TurnRate(30.0f)
	, bAutoMove(false)
	, bDragToSprint(false)
	, SprintButtonVisualSize(80.0f)
	, SprintAction(nullptr)
	, bJoystickMode(false)
	{
		switch (InType)
		{
		case EControlType::Button:
			ControlName = FName(TEXT("NewButton"));
			VisualSize = FVector2D(60.0f);
			InteractionSize = FVector2D(60.0f);
			InteractionRadiusSize = 30.0f;
			InputScale = 1.0f;
			break;
		case EControlType::Joystick:
			ControlName = FName(TEXT("NewJoystick"));
			VisualSize = FVector2D(100.0f);
			InteractionSize = FVector2D(60.0f);
			InteractionRadiusSize = 50.0f;
			ThumbValueCurve.EditorCurveData.AddKey(0.0f,0.0f);
			ThumbValueCurve.EditorCurveData.AddKey(1.0f,1.0f);
			InputScale = 1.0f;
			break;
		case EControlType::TouchRegion:
			ControlName = FName(TEXT("NewTouchRegion"));
			InteractionSize = FVector2D(300.0f, 300.0f);
			InteractionRadiusSize = 150.0f;
			InputScale = 30.0f;
			break;
		}
	}
};

//Todo: Remove Background Settings from this class. Make UObject for background and associate detail panel and tab in editor
USTRUCT()
struct FDesignerBackgroundSettings
{
	GENERATED_BODY()

	UPROPERTY(Category="Background", EditAnywhere)
	FLinearColor FillColor;

	UPROPERTY(Category="Background", EditAnywhere)
	FSlateBrush Image;

	UPROPERTY(Category="Background", EditAnywhere)
	uint8 bFill:1;

	/** Draw Device mockup in background, this feature is not totally accurate, especially with curved screen device */
	UPROPERTY(Category="Background", EditAnywhere)
	uint8 bEnableDeviceMockup:1;

	/** Default Constructor */
	FDesignerBackgroundSettings()
	: FillColor(1.0f,1.0f,1.0f,0.1f)
	, bFill(false)
	, bEnableDeviceMockup(true)
	{
		Image.TintColor = FSlateColor(FLinearColor(1.0f, 1.0f, 1.0f, 0.0f));
	}
};

/** Todo: Make class instead of struct
 * Allow to make Virtual Control UObject for quick addition in runtime
 * Also, simplify also allows to simplify the management of children
 */

/**
 * 
 */
UCLASS(BlueprintType)
class TOUCHINTERFACE_API UVirtualControlSetup : public UObject
{
	GENERATED_BODY()

	UVirtualControlSetup();

public:
	/** Contain all data for virtual controls */
	UPROPERTY(Category="Virtual Control Setup|Virtual Control", EditAnywhere)
	TArray<FVirtualControl> VirtualControls;

	
	/** Opacity (0.0 - 1.0) of all virtual controls while touch interface is active */
	UPROPERTY(Category="Virtual Control Setup|General Settings", EditAnywhere, meta=(ClampMin=0.0f, ClampMax=1.0f, UIMin=0.0f, UIMax=1.0f))
	float ActiveOpacity;

	/** Opacity (0.0 - 1.0) of all virtual controls while touch interface is inactive (no interaction) */
	UPROPERTY(Category="Virtual Control Setup|General Settings", EditAnywhere, meta=(ClampMin=0.0f, ClampMax=1.0f, UIMin=0.0f, UIMax=1.0f))
	float InactiveOpacity;

	/** How long after user interaction will all virtual controls fade out to Inactive Opacity */
	UPROPERTY(Category="Virtual Control Setup|General Settings", EditAnywhere, meta=(ClampMin=0.0f, UIMin=0.0f))
	float TimeUntilDeactivated;

	/** How long after going inactive will virtual controls reset/recenter themselves
	 * 0.0 will disable this feature and virtual controls are reset immediately when the touch interface is deactivated */
	UPROPERTY(Category="Virtual Control Setup|General Settings", EditAnywhere, meta=(ClampMin=0.0f, UIMin=0.0f))
	float TimeUntilReset;

	/** How long after the creation of the touch interface the inputs are activated
	 * 0.0 will disable this feature and the user can interact with the touch interface immediately */
	UPROPERTY(Category="Virtual Control Setup|General Settings", EditAnywhere, meta=(ClampMin=0.0f, UIMin=0.0f))
	float ActivationDelay;

	/** Delay on startup before virtual controls are drawn
	 * 0.0 will disable this feature */
	UPROPERTY(Category="Virtual Control Setup|General Settings", EditAnywhere, meta=(ClampMin=0.0f, UIMin=0.0f))
	float StartupDelay;

	/** Allow Touch Interface to calculate automatically virtual controls position based on device orientation
	 * For now, can produce bad result */
	UPROPERTY(Category="Virtual Control Setup|General Settings", EditAnywhere)
	uint8 bCalculatePortraitPositionAtRuntime:1;


#if WITH_EDITORONLY_DATA
	/** Background Settings of Touch Designer Surface in Touch Designer Editor */
	UPROPERTY(Category="Virtual Control Setup|Editor Background", EditAnywhere)
	FDesignerBackgroundSettings BackgroundSettings;
#endif

#if WITH_EDITOR
	/** Return type of current selected control */
	EControlType GetSelectedControlType() const;
	
	/** return index of current selected control */
	int32 GetSelectedControlIndex() const { return CurrentSelectedControlIndex; }

	FName GetUniqueName(const EControlType ControlType);

	void SetSelectedControlIndex(const int32 Index) { CurrentSelectedControlIndex = Index; }

	int32 SelectLastControl();

	FVirtualControl GetSelectedControl() const;

	FVirtualControl& GetVirtualControlRef(const FName Name);

private:
	bool DoesThisNameExistInControl(const FName OtherName);

	int32 GetUniqueId(const EControlType Type);
	int32 GetLastIndex() const;
	
	/** Return -1 if not found */
	int32 GetControlIndexByName(const FName Name);
	
	int32 CurrentSelectedControlIndex;
	EControlType CurrentSelectedControlType;
#endif
};
