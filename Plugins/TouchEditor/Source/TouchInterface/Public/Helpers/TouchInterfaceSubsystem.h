// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

#include "Subsystems/LocalPlayerSubsystem.h"
#include "Classes/VirtualControlSetup.h"
//Begin Needed for mobile
#include "Runtime/Launch/Resources/Version.h"
//End Needed for mobile
#include "TouchInterfaceSubsystem.generated.h"

class UTouchInterfaceListener;
class STouchInterface;
class UWorld;
class ULocalPlayer;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTouch, int32, FingerIndex, FVector2D, AbsolutePosition, FVector2D, LocalPosition);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStateChangedSignature, bool, Value);

USTRUCT(BlueprintType)
struct FTouchInterfaceInit
{
	GENERATED_BODY()

	UPROPERTY(Category="Touch Interface", EditAnywhere, BlueprintReadWrite)
	bool IsVisible;

	UPROPERTY(Category="Touch Interface", EditAnywhere, BlueprintReadWrite)
	bool bBlockInput;

	UPROPERTY(Category="Touch Interface", EditAnywhere, BlueprintReadWrite)
	bool bBlockRecognizers;

	UPROPERTY(Category="Touch Interface", EditAnywhere, BlueprintReadWrite)
	bool bLoadConfiguration;

	UPROPERTY(Category="Touch Interface", EditAnywhere, BlueprintReadWrite)
	int32 UserIndex;
	
	UPROPERTY(Category="Touch Interface", EditAnywhere, BlueprintReadWrite)
	int32 ConfigIndex;

	FTouchInterfaceInit() :
	IsVisible(true),
	bBlockInput(false),
	bBlockRecognizers(false),
	bLoadConfiguration(false),
	UserIndex(0),
	ConfigIndex(0)
	{
		
	}
};

USTRUCT(BlueprintType)
struct FBaseVirtualControl
{
	GENERATED_BODY()

	// Name of control (used both in editor and runtime)
	UPROPERTY(Category="Virtual Control", EditAnywhere, BlueprintReadWrite)
	FName ControlName;

	// If enabled, the control will be hidden when the interface is added to screen
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

	// The center point of the control in landscape mode. It's relative to screen size (0-1)
	UPROPERTY(Category="Virtual Control", EditAnywhere, BlueprintReadWrite)
	FVector2D LandscapeCenter;

	// The center point of the control in portrait mode. It's relative to screen size (0-1)
	UPROPERTY(Category="Virtual Control", EditAnywhere, BlueprintReadWrite)
	FVector2D PortraitCenter;

	// Visual for control. Configure visual with bitflag
	UPROPERTY(Category="Virtual Control", EditAnywhere, BlueprintReadWrite)
	TArray<FVisualLayer> VisualLayers;

	// The size of the control. It's absolute
	UPROPERTY(Category="Virtual Control", EditAnywhere, BlueprintReadWrite)
	FVector2D VisualSize;

	// The interactive size of the control. It's absolute
	UPROPERTY(Category="Virtual Control", EditAnywhere, BlueprintReadWrite)
	FVector2D InteractionSize;

	FBaseVirtualControl()
	: bStartHidden(false)
	, bRecenterOnTouch(false)
	, bBlockTouchRegion(false)
	, LandscapeCenter(0.5f,0.5f)
	, PortraitCenter(0.5f,0.5f)
	, VisualSize(60.0f,60.0f)
	, InteractionSize(60.0f,60.0f)
	{

	}
};

USTRUCT(BlueprintType)
struct FVirtualButtonData : public FBaseVirtualControl
{
	GENERATED_BODY()

	// The Input Action to use for button. Should be Digital (bool)
	UPROPERTY(Category="Virtual Control|Button", EditAnywhere, BlueprintReadWrite)
	UInputAction* ButtonAction;

	/** The main input to send from this button
	 * Also used for press and release of joystick if bSendPressAndReleaseEvent is equal to true
	 */
	UPROPERTY(Category="Virtual Control|Button", EditAnywhere, BlueprintReadWrite)
	FKey ButtonInputKey;

	UPROPERTY(Category="Virtual Control|Button", EditAnywhere, BlueprintReadWrite)
	USoundBase* PressedSound;

	FVirtualControl GetVirtualControl() const
	{
		FVirtualControl Control = FVirtualControl();
		Control.ControlName = ControlName;
		Control.bStartHidden = bStartHidden;
		Control.bRecenterOnTouch = bRecenterOnTouch;
		Control.bBlockTouchRegion = bBlockTouchRegion;
		Control.LandscapeCenter = LandscapeCenter;
		Control.PortraitCenter = PortraitCenter;
		Control.VisualLayers = VisualLayers;
		Control.VisualSize = VisualSize;
		Control.InteractionSize = InteractionSize;
		Control.Type = EControlType::Button;
		Control.ButtonAction = ButtonAction;
		Control.ButtonInputKey = ButtonInputKey;
		Control.PressedSound = PressedSound;
		return Control;
	}

	FVirtualButtonData() :
	ButtonAction(nullptr),
	PressedSound(nullptr)
	{
		bStartHidden = false;
		bRecenterOnTouch = false;
		bBlockTouchRegion = false;
		LandscapeCenter = FVector2D(0.5f);
		PortraitCenter = FVector2D(0.5f);
		VisualSize = FVector2D(60.0f);
		InteractionSize = FVector2D(60.0f);
	}

	explicit FVirtualButtonData(const FVirtualControl VirtualControl)
	{
		ControlName = VirtualControl.ControlName;
		bStartHidden = VirtualControl.bStartHidden;
		bRecenterOnTouch = VirtualControl.bRecenterOnTouch;
		bBlockTouchRegion = VirtualControl.bBlockTouchRegion;
		LandscapeCenter = VirtualControl.LandscapeCenter;
		PortraitCenter = VirtualControl.PortraitCenter;
		VisualLayers = VirtualControl.VisualLayers;
		VisualSize = VirtualControl.VisualSize;
		InteractionSize = VirtualControl.InteractionSize;
		ButtonAction = VirtualControl.ButtonAction;
		ButtonInputKey = VirtualControl.ButtonInputKey;
		PressedSound = VirtualControl.PressedSound;
	}
};

USTRUCT(BlueprintType)
struct FVirtualJoystickData : public FBaseVirtualControl
{
	GENERATED_BODY()

	/** The main input to send from this button
	 * Also used for press and release of joystick if bSendPressAndReleaseEvent is equal to true
	 */
	UPROPERTY(Category="Virtual Control|Button", EditAnywhere, BlueprintReadWrite)
	FKey ButtonInputKey;


	// For sticks, the size of the thumb. It's absolute
	UPROPERTY(Category="Virtual Control|Joystick", EditAnywhere, BlueprintReadWrite)
	FVector2D ThumbSize;


	// The main input to send from this control (for sticks, this is the horizontal axis)
	UPROPERTY(Category="Virtual Control|Joystick", EditAnywhere, BlueprintReadWrite)
	FKey HorizontalInputKey;

	// The alternate input to send from this control (for sticks, this is the vertical axis, not used in button)
	UPROPERTY(Category="Virtual Control|Joystick", EditAnywhere, BlueprintReadWrite)
	FKey VerticalInputKey;

	// The Input Action to use for joystick. Should be Axis2D
	UPROPERTY(Category="Virtual Control|Joystick", EditAnywhere, BlueprintReadWrite)
	UInputAction* JoystickAction;


	// Whether or not to send a press and release event via ActionInputKey when the user starts/finishes touching the joystick
	UPROPERTY(Category="Virtual Control|Joystick", EditAnywhere, BlueprintReadWrite)
	bool bSendPressAndReleaseEvent;


	/** The scale for joystick input value (ThumbValueCurve * InputScale)
	 * Used as sensibility for Touch Region offset value
	 */
	UPROPERTY(Category="Virtual Control|Joystick", EditAnywhere, BlueprintReadWrite)
	float InputScale;

	// The value of Thumbstick in Joystick. Should be >= 0
	UPROPERTY(Category="Virtual Control|Joystick", EditAnywhere, BlueprintReadWrite)
	FRuntimeFloatCurve ThumbValueCurve;

	// Primary used for camera movement. If true, ThumbValueCurve * TurnRate * DeltaTime
	UPROPERTY(Category="Virtual Control|Joystick", EditAnywhere, BlueprintReadWrite)
	bool bUseTurnRate;

	// Speed rate of rotation
	UPROPERTY(Category="Virtual Control|Joystick", EditAnywhere, BlueprintReadWrite)
	float TurnRate;


	/** The joystick can be held in one direction to trigger automatic movement.
	 * See AutoMoveThreshold and AutoMoveHoldDuration in settings.
	 */
	UPROPERTY(Category="Virtual Control|Joystick", EditAnywhere, BlueprintReadWrite)
	bool bAutoMove;

	// Image for lock icon when auto move or drag to sprint is enabled
	UPROPERTY(Category="Virtual Control|Joystick", EditAnywhere, BlueprintReadWrite)
	FSlateBrush LockIcon;


	/** if the user's finger exceeds the detection zone of the joystick then the system activates the drag to sprint function.
	 * This feature needs space above the joystick to work properly.
	 * Pay attention to the placement of the joystick which must be at the bottom of the screen
	 */
	UPROPERTY(Category="Virtual Control|Joystick", EditAnywhere, BlueprintReadWrite)
	bool bDragToSprint;

	/** Visual layer for Sprint button.
	 * Use unhovered and hovered bitflag.
	 */
	UPROPERTY(Category="Virtual Control|Joystick", EditAnywhere, BlueprintReadWrite)
	TArray<FVisualLayer> SprintButton;

	// The size of the sprint button
	UPROPERTY(Category="Virtual Control|Joystick", EditAnywhere, BlueprintReadWrite)
	FVector2D SprintButtonVisualSize;	

	// The sprint input to send from this control (should be button key, press/release)
	UPROPERTY(Category="Virtual Control|Joystick", EditAnywhere, BlueprintReadWrite)
	FKey SprintInputKey;

	// The Drag to Sprint input action to use. Should be Digital (bool)
	UPROPERTY(Category="Virtual Control|Joystick", EditAnywhere, BlueprintReadWrite)
	UInputAction* SprintAction;

	FVirtualControl GetVirtualControl() const
	{
		FVirtualControl Control = FVirtualControl();
		Control.ControlName = ControlName;
		Control.bStartHidden = bStartHidden;
		Control.bRecenterOnTouch = bRecenterOnTouch;
		Control.bBlockTouchRegion = bBlockTouchRegion;
		Control.LandscapeCenter = LandscapeCenter;
		Control.PortraitCenter = PortraitCenter;
		Control.VisualLayers = VisualLayers;
		Control.VisualSize = VisualSize;
		Control.InteractionSize = InteractionSize;
		Control.Type = EControlType::Button;
		Control.ButtonInputKey = ButtonInputKey;
		Control.ThumbSize = ThumbSize;
		Control.HorizontalInputKey = HorizontalInputKey;
		Control.VerticalInputKey = VerticalInputKey;
		Control.JoystickAction = JoystickAction;
		Control.bSendPressAndReleaseEvent = bSendPressAndReleaseEvent;
		Control.InputScale = InputScale;
		Control.ThumbValueCurve = ThumbValueCurve;
		Control.bUseTurnRate = bUseTurnRate;
		Control.TurnRate = TurnRate;
		Control.bAutoMove = bAutoMove;
		Control.LockIcon = LockIcon;
		Control.bDragToSprint = bDragToSprint;
		Control.SprintButton = SprintButton;
		Control.SprintButtonVisualSize = SprintButtonVisualSize;
		Control.SprintInputKey = SprintInputKey;
		Control.SprintAction = SprintAction;
		return Control;
	}

	FVirtualJoystickData() :	
	ThumbSize(60.0f),
	JoystickAction(nullptr),
	bSendPressAndReleaseEvent(false),
	InputScale(1.0f),
	bUseTurnRate(false),
	TurnRate(30.0f),
	bAutoMove(false),
	bDragToSprint(false),
	SprintButtonVisualSize(80.0f),
	SprintAction(nullptr)
	{
		bStartHidden = false;
		bRecenterOnTouch = false;
		bBlockTouchRegion = false;
		LandscapeCenter = FVector2D(0.5f);
		PortraitCenter = FVector2D(0.5f);
		VisualSize = FVector2D(100.0f);
		InteractionSize = FVector2D(60.0f);
	}

	explicit FVirtualJoystickData(const FVirtualControl VirtualControl)
	{
		ControlName = VirtualControl.ControlName;
		bStartHidden = VirtualControl.bStartHidden;
		bRecenterOnTouch = VirtualControl.bRecenterOnTouch;
		bBlockTouchRegion = VirtualControl.bBlockTouchRegion;
		LandscapeCenter = VirtualControl.LandscapeCenter;
		PortraitCenter = VirtualControl.PortraitCenter;
		VisualLayers = VirtualControl.VisualLayers;
		VisualSize = VirtualControl.VisualSize;
		InteractionSize = VirtualControl.InteractionSize;
		ButtonInputKey = VirtualControl.ButtonInputKey;
		ThumbSize = VirtualControl.ThumbSize;
		HorizontalInputKey = VirtualControl.HorizontalInputKey;
		VerticalInputKey = VirtualControl.VerticalInputKey;
		JoystickAction = VirtualControl.JoystickAction;
		bSendPressAndReleaseEvent = VirtualControl.bSendPressAndReleaseEvent;
		InputScale = VirtualControl.InputScale;
		ThumbValueCurve = VirtualControl.ThumbValueCurve;
		bUseTurnRate = VirtualControl.bUseTurnRate;
		TurnRate = VirtualControl.TurnRate;
		bAutoMove = VirtualControl.bAutoMove;
		LockIcon = VirtualControl.LockIcon;
		bDragToSprint = VirtualControl.bDragToSprint;
		SprintButton = VirtualControl.SprintButton;
		SprintButtonVisualSize = VirtualControl.SprintButtonVisualSize;
		SprintInputKey = VirtualControl.SprintInputKey;
		SprintAction = VirtualControl.SprintAction;
	}
};

USTRUCT(BlueprintType)
struct FTouchRegionData : public FBaseVirtualControl
{
	GENERATED_BODY()

	// The main input to send from this control (for sticks, this is the horizontal axis)
	UPROPERTY(Category="Virtual Control|Touch Region", EditAnywhere, BlueprintReadWrite)
	FKey HorizontalInputKey;

	// The alternate input to send from this control (for sticks, this is the vertical axis, not used in button)
	UPROPERTY(Category="Virtual Control|Touch Region", EditAnywhere, BlueprintReadWrite)
	FKey VerticalInputKey;

	// The Input Action to use for joystick. Should be Axis2D
	UPROPERTY(Category="Virtual Control|Touch Region", EditAnywhere, BlueprintReadWrite)
	UInputAction* InputAction;

	// The sensibility for or Touch Region offset value
	UPROPERTY(Category="Virtual Control|Touch Region", EditAnywhere, BlueprintReadWrite)
	float InputSensibility;

	FVirtualControl GetVirtualControl() const
	{
		FVirtualControl Control = FVirtualControl();
		Control.ControlName = ControlName;
		Control.LandscapeCenter = LandscapeCenter;
		Control.PortraitCenter = PortraitCenter;
		Control.InteractionSize = InteractionSize;
		Control.Type = EControlType::TouchRegion;
		Control.HorizontalInputKey = HorizontalInputKey;
		Control.VerticalInputKey = VerticalInputKey;
		Control.JoystickAction = InputAction;
		Control.InputScale = InputSensibility;
		return Control;
	}

	FTouchRegionData() :
	InputAction(nullptr),
	InputSensibility(30.0f)
	{
		LandscapeCenter = FVector2D(0.5f);
		PortraitCenter = FVector2D(0.5f);
		InteractionSize = FVector2D(60.0f);
	}

	explicit FTouchRegionData(const FVirtualControl VirtualControl)
	{
		ControlName = VirtualControl.ControlName;
		LandscapeCenter = VirtualControl.LandscapeCenter;
		PortraitCenter = VirtualControl.PortraitCenter;
		InteractionSize = VirtualControl.InteractionSize;
		HorizontalInputKey = VirtualControl.HorizontalInputKey;
		VerticalInputKey = VirtualControl.VerticalInputKey;
		InputAction = VirtualControl.JoystickAction;
		InputSensibility = VirtualControl.InputScale;
	}
};

/**
 * This subsystem allows you to add and modify the touch interface.
 * It also allows you to manage other components related to the touch interface.
 */
UCLASS()
class TOUCHINTERFACE_API UTouchInterfaceSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Register this component to receive input */
	UFUNCTION(Category="Touch Interface", BlueprintCallable)
	bool RegisterTouchManagerComponent(UTouchInterfaceListener* Manager);

	/** Unregister this component.
	 * This component will no longer receive input. */
	UFUNCTION(Category="Touch Interface", BlueprintCallable)
	bool UnregisterTouchManagerComponent(UTouchInterfaceListener* Manager);

	/** Add new touch interface to player screen */
	UFUNCTION(Category="Touch Interface", BlueprintCallable/*, meta=(DefaultToSelf)*/)
	virtual void AddToPlayerScreen(APlayerController* PlayerController, UVirtualControlSetup* NewTouchDesignerInterface, const FTouchInterfaceInit& State);

private:
	void CreateTouchInterface(const bool bLoadConfiguration, const int32 UserIndex, const int32 ConfigIndex);

public:
	/** Remove touch interface from player screen */
	UFUNCTION(Category="Touch Interface", BlueprintCallable)
	virtual void RemoveFromPlayerScreen(APlayerController* PlayerController);

	/** Enable input in touch interface */
	UFUNCTION(Category="Touch Interface|Input", BlueprintCallable)
	virtual void EnableInput();

	/** Disable input in touch interface (no interaction) */
	UFUNCTION(Category="Touch Interface|Input", BlueprintCallable)
	virtual void DisableInput();

	/** Enable gesture in touch interface */
	UFUNCTION(Category="Touch Interface|Input", BlueprintCallable)
	virtual void EnableGesture();

	/** Disable gesture in touch interface (no interaction) */
	UFUNCTION(Category="Touch Interface|Input", BlueprintCallable)
	virtual void DisableGesture();

	/** Launch Shape Recognition on all Shape Manager.
	 * This is a global call, you can call TryRecognizeShape on each Shape Manager.
	 * For now, DO NOT CALL this function until the user has finished drawing
	 */
	UFUNCTION(Category="Touch Interface|Input", BlueprintCallable)
	virtual void TryRecognizeShape();

	/** Changes the key of the virtual control.
	 * Primary use case is in Option Menu.
	 * Warning! It call Release before change
	 * @param ControlName Name of control
	 * @param ButtonInput For Button or Joystick (press and release)
	 * @param HorizontalInputKey For Joystick or Touch Region (Horizontal Axis)
	 * @param VerticalInputKey For Joystick or Touch Region (Vertical Axis)
	 * @return Return true if rebinding success
	 */
	UFUNCTION(Category="Touch Interface|Input", BlueprintCallable)
	bool SetInputKey(const FName ControlName, FKey ButtonInput, FKey HorizontalInputKey, FKey VerticalInputKey);

	/** Changes the Button Input Action of the virtual control button.
	 * Primary use case is in Option Menu.
	 * Warning! It call Release before change
	 * @param ControlName Name of control
	 * @param InputAction the desired input action for Button
	 * @return Return true if rebinding success
	 */
	UFUNCTION(Category="Touch Interface|Input", BlueprintCallable)
	bool SetInputAction(const FName ControlName, UInputAction* InputAction);
	
	/** Changes the Button Input Action of the virtual control button.
	 * Primary use case is in Option Menu.
	 * Warning! It call Release before change
	 * @param ControlName Name of control
	 * @param ButtonAction the desired input action for Button
	 * @return Return true if rebinding success
	 */
	UFUNCTION(Category="Touch Interface|Input", BlueprintCallable, meta=(DeprecatedFunction, DeprecatedMessage="Use SetInputAction instead"))
	bool SetButtonAction(const FName ControlName, UInputAction* ButtonAction);

	/** Changes the Joystick or Touch Region Input Action of the virtual control.
	 * Primary use case is in Option Menu.
	 * Warning! It call Release before change
	 * @param ControlName Name of control
	 * @param JoystickAction the desired input action for joystick or Touch Region
	 * @return Return true if rebinding success
	 */
	UFUNCTION(Category="Touch Interface|Input", BlueprintCallable, meta=(DeprecatedFunction, DeprecatedMessage="Use SetInputAction instead"))
	bool SetJoystickAction(const FName ControlName, UInputAction* JoystickAction);

	
	//VISIBILITY
	
	/** Display touch interface */
	UFUNCTION(Category="Touch Interface|Visibility", BlueprintCallable)
	virtual void ShowWidget(bool bEnableInput, bool bEnableGesture);

	//Todo: Replace by bKeepGestureEnabled ?
	/** Hide touch interface */
	UFUNCTION(Category="Touch Interface|Visibility", BlueprintCallable)
	virtual void HideWidget(bool bDisableGesture);

	UFUNCTION(Category="Touch Interface|Visibility", BlueprintCallable)
	virtual void ShowVirtualControl(const FName ControlName, bool bIncludeChild);

	UFUNCTION(Category="Touch Interface|Visibility", BlueprintCallable)
	virtual void HideVirtualControl(const FName ControlName, bool bIncludeChild);

	//Deprecate function below ? It's useless.
	UFUNCTION(Category="Touch Interface|Visibility", BlueprintCallable)
	virtual void HideAllButtons();
	
	UFUNCTION(Category="Touch Interface|Visibility", BlueprintCallable)
	virtual void HideAllJoystick();

	UFUNCTION(Category="Touch Interface|Visibility", BlueprintCallable)
	virtual void HideAllTouchRegion();
	
	UFUNCTION(Category="Touch Interface|Visibility", BlueprintCallable)
	virtual void ShowAllButtons();

	UFUNCTION(Category="Touch Interface|Visibility", BlueprintCallable)
	virtual void ShowAllJoystick();

	UFUNCTION(Category="Touch Interface|Visibility", BlueprintCallable)
	virtual void ShowAllTouchRegion();

	
	//DATA

	/** Get virtual control data by ref */
	UFUNCTION(Category="Touch Interface|Data", BlueprintCallable)
	virtual bool GetVirtualControlDataByRef(const FName ControlName, FVirtualControl& Data);

	/** Get a copy of virtual control data */
	UFUNCTION(Category="Touch Interface|Data", BlueprintCallable)
	virtual bool GetVirtualControlData(const FName ControlName, FVirtualControl& Data);

	/** Add new virtual control as Button */
	UFUNCTION(Category="Touch Interface|Data", BlueprintCallable)
	virtual void AddVirtualButton(const FVirtualButtonData ButtonData);

	/** Add new virtual control as Joystick */
	UFUNCTION(Category="Touch Interface|Data", BlueprintCallable)
	virtual void AddVirtualJoystick(const FVirtualJoystickData JoystickData);

	/** Add new virtual control as Touch Region */
	UFUNCTION(Category="Touch Interface|Data", BlueprintCallable)
	virtual void AddVirtualTouchRegion(const FTouchRegionData TouchRegionData);

	/** Remove virtual control from Touch Interface
	 * @param ControlName Name of virtual control that you want to remove
	 * @param bIncludeChildren Whether or not to remove children too from Touch Interface
	 */
	UFUNCTION(Category="Touch Interface|Data", BlueprintCallable)
	virtual void RemoveVirtualControl(const FName ControlName, bool bIncludeChildren);

	UFUNCTION(Category="Touch Interface|Data", BlueprintCallable)
	virtual void ResetToDefault();

	
	// VISUAL

	UFUNCTION(Category="Touch Interface|Visual", BlueprintCallable)
	virtual void SetVirtualControlAppearance(const FName ControlName, TArray<FVisualLayer> Layers);

	UFUNCTION(Category="Touch Interface|Visual", BlueprintCallable)
	virtual void AddNewLayer(const FName ControlName, FVisualLayer Layer);

	UFUNCTION(Category="Touch Interface|Visual", BlueprintCallable)
	virtual void RemoveLayer(const FName ControlName, const FName LayerName);
	
	/** Create and/or Get dynamic material instance associated with layer
	 * * DO NOT CALL THIS EVERY FRAME!
	 * @param ControlName Name of virtual control
	 * @param LayerName Name of layer
	 * @param DMI Dynamic Material instance that is created or recovered
	 * @return Return true if material can be recovered
	 */
	UFUNCTION(Category="Touch Interface|Visual", BlueprintCallable)
	virtual bool GetDynamicMaterialInstanceFromLayer(const FName ControlName, const FName LayerName, UMaterialInstanceDynamic* &DMI);

	/** Set size of virtual control button. (VisualSize and interaction size)
	 * Leave to zero has no change.
	 * Be careful when changing size of control when in use.
	 */
	UFUNCTION(Category="Touch Interface|Size and Position", BlueprintCallable)
	virtual void ChangeButtonSize(const FName ControlName, const FVector2D NewVisualSize, const FVector2D NewInteractionSize);

	/** Set size of virtual control joystick. (VisualSize, thumb size and interaction size).
	 * Leave to zero has no change.
	 * Be careful when changing size of control when in use.
	 */
	UFUNCTION(Category="Touch Interface|Size and Position", BlueprintCallable)
	virtual void ChangeJoystickSize(const FName ControlName, const FVector2D NewVisualSize, const FVector2D NewThumbSize, const FVector2D NewInteractionSize);

	/** Set interaction size of virtual Touch Region.
	 * Leave to zero has no change.
	 */
	UFUNCTION(Category="Touch Interface|Size and Position", BlueprintCallable)
	virtual void ChangeTouchRegionSize(const FName ControlName, const FVector2D NewInteractionSize);
	
	/** Set position of control. Normalized to [0 to 1]
	 * negative value has no change
	 * Be careful when changing position of control when in use.
	 */
	UFUNCTION(Category="Touch Interface|Size and Position", BlueprintCallable)
	virtual bool ChangePosition(const FName ControlName, const FVector2D NewCenter, const bool LandscapePosition = true);
	

	// UTILITIES

	UFUNCTION(Category="Touch Interface|Utility", BlueprintCallable)
	EControlType GetTypeOfVirtualControl(const FName ControlName) const;

	UFUNCTION(Category="Touch Interface|Utility", BlueprintCallable)
	virtual bool ContainName(const FName ControlName);
	
	UFUNCTION(Category="Touch Interface|Utility", BlueprintCallable)
	virtual bool GetAllControlsName(TArray<FName>& Names);
	
	UFUNCTION(Category="Touch Interface|Utility", BlueprintCallable)
	virtual bool GetLayerNames(const FName ControlName, TArray<FName>& Names);

	/** Return true if interface is in active state (a control is currently hit) */
	UFUNCTION(Category="Touch Interface|Utility", BlueprintCallable)
	bool IsInterfaceActive() const;
	
	/** Return true if interface is visible */
	UFUNCTION(Category="Touch Interface|Utility", BlueprintCallable)
	bool IsVisible() const;
	
	UFUNCTION(Category="Touch Interface|Utility", BlueprintCallable)
	void SetScaleMultiplier(const float NewScaleMultiplier);

	/** Transform Local position (Touch Interface Geometry) to normalized (0 to 1)
	* @param LocalPosition LocalPosition in Touch Interface
	* @param NormalizedPosition return normalized position (0 to 1)
	* @return return true if touch interface is valid and normalized position is calculated
	 */
	UFUNCTION(Category="Touch Interface|Utility", BlueprintCallable)
	bool LocalToNormalized(const FVector2D LocalPosition, FVector2D& NormalizedPosition);

	/** Transform normalized (0 to 1) to Local position (Touch Interface Geometry)
	* @param NormalizedPosition Normalized value (0 to 1)
	* @param LocalPosition return local position
	* @return return true if touch interface is valid and local position is calculated
	 */
	UFUNCTION(Category="Touch Interface|Utility", BlueprintCallable)
	bool NormalizedToLocal(const FVector2D NormalizedPosition, FVector2D& LocalPosition);
	

	// SAVE

	/** Check if virtual control configuration save game object exist in disk
	 *@param UserIndex User Index used with configuration
	 *@return true if a file was found
	 */
	UFUNCTION(Category="Touch Interface|Save and Load", BlueprintCallable)
	virtual bool VirtualControlConfigurationExist(const int32 UserIndex);
	
	/** Check if a configuration with specified config index exist in virtual control configuration save game object
	 *@param UserIndex User Index used with configuration
	 *@param ConfigIndex Index used to identify configuration
	 *@return true if specified configuration exist
	 */
	UFUNCTION(Category="Touch Interface|Save and Load", BlueprintCallable)
	virtual bool ConfigurationExist(const int32 UserIndex, const int32 ConfigIndex);
	
	/** Save current setup in configuration SaveGame Slot
	 *@param UserIndex User Index used with configuration
	 *@param ConfigIndex Index used to identify configuration
	 *@remark if configuration SaveGame Slot does not exist, a new one is created
	 */
	UFUNCTION(Category="Touch Interface|Save and Load", BlueprintCallable)
	virtual bool SaveConfiguration(const int32 UserIndex, const int32 ConfigIndex);

	/** Load configuration saved in SaveGame slot.
	 *@param UserIndex User Index used with configuration
	 *@param ConfigIndex Index used for identify configuration
	 *@return true if configuration with User Index and Config Index was found
	 */
	UFUNCTION(Category="Touch Interface|Save and Load", BlueprintCallable)
	virtual bool LoadConfiguration(const int32 UserIndex, const int32 ConfigIndex);

	/** Delete configuration saved in save game object */
	UFUNCTION(Category="Touch Interface|Save and Load", BlueprintCallable)
	virtual bool DeleteConfiguration(const int32 UserIndex, const int32 ConfigIndex);

	/** Delete virtual control configuration (save game object) */
	UFUNCTION(Category="Touch Interface|Save and Load", BlueprintCallable)
	virtual bool DeleteConfigurationSlot(const int32 UserIndex);

	//EVENTS
	
	/** Called when interface active state was changed
	 * Return true if interface is active (a control is currently hit)
	 */
	UPROPERTY(Category="Touch Interface|State", BlueprintAssignable)
	FOnStateChangedSignature OnActiveStateChanged;

	/** Called when visibility state of interface was changed
	 * Return true if currently visible
	 */
	UPROPERTY(Category="Touch Interface|State", BlueprintAssignable)
	FOnStateChangedSignature OnVisibilityStateChanged;
	
	/** Called when any user finger start touching touch screen */
	UPROPERTY(Category="Touch Interface|Gestures", BlueprintAssignable)
	FOnTouch OnTouchBegan;

	/** Called when any user finger move in touch screen */
	UPROPERTY(Category="Touch Interface|Gestures", BlueprintAssignable)
	FOnTouch OnTouchMoved;

	/** Called when any user finger end touching touch screen */
	UPROPERTY(Category="Touch Interface|Gestures", BlueprintAssignable)
	FOnTouch OnTouchEnded;
	
private:
	UFUNCTION()
	UWorld* GetWorldContext() const { return WorldContext; }

	UFUNCTION()
	ULocalPlayer* GetLocalPlayerInstance() const { return LocalPlayerInstance; }
	
	void HandleBeganTouchEvent(const int32 FingerIndex, const FVector2D AbsolutePosition, const FVector2D LocalPosition);
	void HandleMovedTouchEvent(const int32 FingerIndex, const FVector2D AbsolutePosition, const FVector2D LocalPosition);
	void HandleEndedTouchEvent(const int32 FingerIndex, const FVector2D AbsolutePosition, const FVector2D LocalPosition);

	void HandleOnActiveStateChangedEvent(const bool IsActive);
	void HandleOnVisibilityStateChangedEvent(const bool IsVisible);


#if ENGINE_MAJOR_VERSION > 4
	#if ENGINE_MINOR_VERSION > 0
		void HandleOnControllerConnexionChange(EInputDeviceConnectionState ConnectionState, FPlatformUserId UserId, FInputDeviceId DeviceId);
	#else
		void HandleOnControllerConnexionChange(bool bIsConnected, FPlatformUserId UserId, int Other);
	#endif
#else
		void HandleOnControllerConnexionChange(bool bIsConnected, int UserId, int Other);
#endif
	
	
	void HandleOnAnyKeyPressed();
	void HandleOnTouch(ETouchIndex::Type Index, FVector Coordinate);
	
	/*~ MyOtherFunction shows flags named after the values from EColorBits. */
	/*UFUNCTION(BlueprintCallable)
	void MyOtherFunction(UPARAM(meta=(Bitmask, BitmaskEnum = "EColorBits")) int32 ColorFlagsParam)*/
	
	
	UPROPERTY()
	UVirtualControlSetup* CurrentVirtualControlSetup;

	TSharedPtr<STouchInterface> TouchInterface;

	UPROPERTY()
	TArray<UTouchInterfaceListener*> RegisteredTouchManager;

	UPROPERTY()
	UWorld* WorldContext;

	UPROPERTY()
	ULocalPlayer* LocalPlayerInstance;

	/** Used for gamepad detection */
	uint8 bIsTouchInput:1;

	UPROPERTY()
	TMap<FName, UMaterialInstanceDynamic*> CachedMaterialInstance;

	UPROPERTY()
	UTexture2D* DebugTexture;
};
