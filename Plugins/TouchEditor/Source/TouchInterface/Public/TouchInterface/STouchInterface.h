// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

#include "VirtualControlSetup.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SConstraintCanvas.h"

class UVirtualShape;
class UShapeManager;
class SVirtualShapeDrawer;
enum class EScalingMode;

class UTouchInterfaceSettings;
class SVirtualControl;
class UTouchInterfaceListener;
class UWorld;
class UGameInstance;
class ULocalPlayer;
class UEnhancedInputLocalPlayerSubsystem;

DECLARE_DELEGATE_ThreeParams(FOnTouchEvent, int32, FVector2D, FVector2D);
DECLARE_DELEGATE_OneParam(FOnStateChangedEvent, bool);
DECLARE_DELEGATE_OneParam(FOnAutoMoveSignature, FVector2D);

struct FVirtualControlWidget
{
	FName Name;
	SConstraintCanvas::FSlot* Slot;
	TSharedPtr<SVirtualControl> VirtualControl;
	uint8 bIsChild:1;

	FVirtualControlWidget(const FName InName, SConstraintCanvas::FSlot* InSlot, TSharedPtr<SVirtualControl> VirtualControl, const bool IsChild) :
	Name(InName),
	Slot(InSlot),
	VirtualControl(VirtualControl),
	bIsChild(IsChild)
	{
		
	}
};

struct FFingersData
{
	uint32 FingerIndex;
	FVector2D Position;

	FFingersData(const uint32 InFingerIndex, const FVector2D InPosition) :
	FingerIndex(InFingerIndex),
	Position(InPosition)
	{
		
	}
};

class TOUCHINTERFACE_API STouchInterface : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(STouchInterface)
		: _LoadConfig(false)
		, _UserIndex(0)
		, _ConfigIndex(0)
		{}

		SLATE_ARGUMENT(bool, LoadConfig)
		SLATE_ARGUMENT(int32, UserIndex)
		SLATE_ARGUMENT(int32, ConfigIndex)
		
		SLATE_EVENT(FOnTouchEvent, OnTouchBegan)
		SLATE_EVENT(FOnTouchEvent, OnTouchMoved)
		SLATE_EVENT(FOnTouchEvent, OnTouchEnded)
		SLATE_EVENT(FOnStateChangedEvent, OnActiveStateChanged)
		SLATE_EVENT(FOnStateChangedEvent, OnVisibilityStateChanged)
	SLATE_END_ARGS()
		
	void Construct(const FArguments& InArgs, ULocalPlayer* InLocalPlayer, UVirtualControlSetup* InVirtualControlSetup);

private:
	/** Generate virtual controls and add to touch interface as child */
	void GenerateVirtualControls(TArray<FVirtualControl> InVirtualControls);
	
	TSharedRef<SVirtualControl> ConstructVirtualControlWidget(const FVirtualControl& InVirtualControl, SConstraintCanvas::FSlot* OwnerSlot, TSharedPtr<SVirtualControl>&);

	/** Generate child virtual control and add to touch interface as child. Only called by parent control */
	void GenerateChildControls(const TArray<FName> ChildrenName, TSharedPtr<SVirtualControl> Parent);

	EActiveTimerReturnType ActivateTouchInterface(double InCurrentTime, float InDeltaTime);

public:
	//Begin SWidget implementation
	virtual FReply OnTouchStarted(const FGeometry& MyGeometry, const FPointerEvent& Event) override;
	virtual FReply OnTouchMoved(const FGeometry& MyGeometry, const FPointerEvent& Event) override;
	virtual FReply OnTouchEnded(const FGeometry& MyGeometry, const FPointerEvent& Event) override;
	//End SWidget implementation

private:
	virtual void OnDrawStarted(const FGeometry& MyGeometry, const FPointerEvent& Event);
	virtual void OnDrawUpdated(const FGeometry& MyGeometry, const FPointerEvent& Event);
	virtual void OnDrawEnded(const FGeometry& MyGeometry, const FPointerEvent& Event);

	/** Try to determine vitual shape drawn by user */
	virtual void ProcessUserDrawing();

	/** Try to determine virtual shape drawn by user with incomplete data */
	virtual void PredictVirtualShape();

public:
	//Begin SWidget implementation
	virtual void Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime ) override;
	virtual int32 OnPaint( const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled ) const override;
	virtual bool SupportsKeyboardFocus() const override { return false; }
	virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(100, 100); }
	//End SWidget implementation
	
	/** Force the reconstruction of all virtual controls
	 * Do not call this every frame! */
	virtual void RebuildTouchInterface(const bool bLoadConfig, const int32 UserIndex, const int32 ConfigIndex);
	
	/**
	 * Static function to return if external users should create/activate/etc a touch interface
	 * Note that this function is also used internally, so even if this returns false but an STouchInterface
	 * is created, it won't actually show any controls
	 */
	static bool ShouldDisplayTouchInterface();

	/** Get current controller id from player controller */
	int32 GetControllerId() const { return ControllerIdAssociated; }

	/** Get world context object */
	UWorld* GetWorldContext() const;
	

	// INPUT

	bool RegisterTouchInputComponent(UTouchInterfaceListener* ManagerComp);
	bool UnregisterTouchInputComponent(UTouchInterfaceListener* ManagerComp);

	bool RegisterShapeManagerComponent(UShapeManager* ManagerComp);
	bool UnregisterShapeManagerComponent(UShapeManager* ManagerComp);
	
	/** Update gesture component list */
	void SetTouchInputComponentList(TArray<UTouchInterfaceListener*>& Comps);
	
	/** Whether or not input is enabled */
	void BlockInput(const bool bBlock) { bBlockInput = bBlock; }

	/** Whether or not gesture is enabled */
	void BlockGesture(const bool bBlock) { bBlockRecognizers = bBlock; }
	
	void ShowDrawGuide(const FName VirtualShapeName);
	void HideDrawGuide();
	
	/** Call TryRecognizeShape() on all managers */
	void TryRecognizeShapeOnAllManagers();
	
	/** Change FKey on virtual control */
	bool ChangeInputKey(const FName ControlName, FKey& Main, FKey& Alt);

	/** Change InputAction on virtual control */
	bool ChangeInputAction(const FName ControlName, UInputAction* Action);
	
	
	// VISIBILITY

	/** Shows or hides the controls (for instance during cinematics) */
	void SetWidgetVisibility(const bool bInVisible, const bool bInFade, const bool bInBlockInput, const bool bInBlockGesture);
	
	/** Show or hide virtual control by name */
	void SetControlVisibility(const FName Name, const bool bInVisible, const bool bIncludeChildren);

	/** Show all virtual controls by type */
	void ShowAllControls(const bool IncludeButton = true, const bool IncludeJoystick = false, const bool IncludeTouchRegion = false);

	/** Hide all virtual controls by type */
	void HideAllControls(const bool IncludeButton = true, const bool IncludeJoystick = false, const bool IncludeTouchRegion = false);


	//DATA
	
	/** Get data of virtual control */
	bool GetVirtualControlData(const FName ControlName, FVirtualControl& VirtualControl);

	/** Get all virtual controls data by type */
	TArray<FVirtualControl> GetAllControls(const bool IncludeButton = true, const bool IncludeJoystick = false, const bool IncludeTouchRegion = false);

	/** Add new virtual control */
	void AddControl(const FVirtualControl& NewControl);

	/** Remove virtual control from struct by name */
	void RemoveControl(const FName Name, const bool bRemoveChildren);

	/** Remove all virtual controls added */
	void ResetToDefault();

	
	// VISUAL
	
	/** Change visual layer of virtual control */
	void SetVirtualControlVisualLayers(const FName ControlName, TArray<FVisualLayer> Layers);

	//Todo: Add function to allow dev to Add/Remove layer

	/** Create or get dynamic material instance from layer
	 * DO NOT CALL THIS EVERY FRAME! */
	UMaterialInstanceDynamic* GetLayerDynamicMaterialInstance(const FName ControlName, const FName LayerName, UObject* InOuter);

	void SetControlSize(const FName Name, const FVector2D NewVisualSize, const FVector2D NewThumbSize, const FVector2D NewInteractionSize);

	bool SetControlPosition(const FName Name, const FVector2D NewPosition, const bool InLandscape);

	
	// UTILITIES
	
	/***/
	bool GetTypeOfControl(const FName ControlName, EControlType& Type);

	bool ContainName(const FName ControlName);

	TArray<FName> GetAllControlNames();
	
	TArray<FName> GetLayerNames(const FName ControlName);

	UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem() const { return EnhancedInputSubsystem; }

	bool IsActive() const { return State == State_Active; }

	bool IsVisible() const { return bVisible; }

	void SetScaleMultiplier(const float NewScale) { ScaleMultiplier = NewScale; }

	/** Transform Local position (Touch Interface Geometry) to normalized (0 to 1) */
	FVector2D LocalToNormalized(const FVector2D LocalPosition) const;

	/** Transform normalized (0 to 1) to Local position (Touch Interface Geometry) */
	FVector2D NormalizedToLocal(const FVector2D NormalizedPosition) const;

	static float GetDebugOpacity();

	//SAVE

	bool ConfigurationFileExist(const uint32 UserIndex) const;
	bool ConfigurationExist(const uint32 UserIndex, const uint32 ConfigIndex) const;
	bool SaveConfiguration(const uint32 UserIndex, const uint32 ConfigIndex);
	bool LoadConfiguration(const uint32 UserIndex, const uint32 ConfigIndex);
	bool DeleteConfiguration(const uint32 UserIndex, const uint32 ConfigIndex) const;
	bool DeleteConfigurationSlot(const uint32 UserIndex) const;
	
	// EVENTS
	
	FOnTouchEvent OnTouchBeganEvent;
	FOnTouchEvent OnTouchMovedEvent;
	FOnTouchEvent OnTouchEndedEvent;

	FOnStateChangedEvent OnActiveStateChanged;
	FOnStateChangedEvent OnVisibilityStateChanged;

	FOnAutoMoveSignature OnAutoMoveBeganEvent;
	FOnAutoMoveSignature OnAutoMoveEndedEvent;

	FOnStateChangedEvent OnDragToSprintChanged;

private:
	void SetWidgetOpacity(const int32 ActiveControls, const float DeltaTime);

	// Return the target opacity to lerp to given the current state
	FORCEINLINE float GetBaseOpacity();
	
	float GetVirtualControlOpacity() const { return CurrentOpacity; }

	float GetCurrentScaleFactor() const { return CurrentScaleFactor; }

	bool GetDrawDebug() const { return bDrawDebug; }
	
	float GetScaleFactor(const FGeometry& Geometry);

	// Callback for handling device orientation changes
	virtual void HandleOnOrientationChanged(const int32 Mode);

	void CalculateDataBasedOnBound(const FVector2D& BoundCenter);

	void ClearData();
	
	// Callback for handling display metrics changes
	//virtual void HandleDisplayMetricsChanged(const FDisplayMetrics& NewDisplayMetric);
	
	TSharedPtr<SConstraintCanvas> VirtualControlCanvas;
	
	TArray<TSharedRef<SVirtualControl>> VirtualControlWidgets;

	TSharedPtr<SVirtualShapeDrawer> ShapeDrawer;
	
	TArray<UTouchInterfaceListener*> TouchInputComps;

	TArray<UShapeManager*> ShapeManagerComps;

	TArray<UVirtualShape> VirtualShapes;

	TArray<FFingersData> Fingers;

	UVirtualControlSetup* VirtualControlSetup;
	
	UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem;

	const UTouchInterfaceSettings* Settings;

	class UVirtualControlSave* ConfigurationSaveSlot;

	UGameInstance* PersistantOuter;
	
	EScalingMode ScalingMode;

	TArray<FVirtualControlWidget> ChildWidgets;

	//TAttribute<UWorld*> WorldContext;
	//TAttribute<ULocalPlayer*> LocalPlayer;

	UWorld* WorldContext;
	ULocalPlayer* LocalPlayer;
	
	float CurrentScaleFactor;
	float ScaleMultiplier;

	/** True if the touch interface should be visible */
	uint8 bVisible:1;

	/** True if the interaction with touch interface is allowed
	 * Used mainly with Activation Delay setting */
	uint8 bIsActivated:1;

	
	
	uint8 bActiveEventSend:1;
	
	/** True if the widget should block input (no interaction) */
	uint8 bBlockInput:1;


	//GESTURE RECOGNIZER
	
	/** True if the widget should send input to Gesture Manager */
	uint8 bGestureRecognizerEnabled:1;
	
	/** True if the widget should block touch input manager (no interaction) */
	uint8 bBlockRecognizers:1;


	//SHAPE RECOGNIZER
	
	// True if the widget should send input to the Shape Recognition Manager
	uint8 bShapeRecognizerEnabled:1;

	// True if the touch interface should draw the user's drawing
	uint8 bDrawUserShape:1;
	
	/** True if the widget should block shape recognition (no interaction) */
	uint8 bBlockShapeRecognition:1;

	
	uint8 bDrawDebug:1;

	int32 NumberOfActiveControl;
	
	int32 ControllerIdAssociated;

	FSlateBrush DebugCircle;

	/** Global settings from the UTouchDesignerInterface */
	float ActiveOpacity;
	float InactiveOpacity;
	float TimeUntilDeactivated;
	float TimeUntilReset;
	float ActivationDelay;
	float StartupDelay;

	uint8 bCalculatePositionAuto:1;

	enum ETouchInterfaceState
	{
		State_Active,
		State_CountingDownToInactive,
		State_CountingDownToReset,
		State_Inactive,
		State_WaitForStart,
		State_CountingDownToStart,
	};

	/** The current state of all virtual controls */
	ETouchInterfaceState State;

	/** If true, use landscape center */
	uint32 bIsInLandscapeMode:1;

	uint8 bUseEnhancedInput:1;

	/** Target opacity */
	float CurrentOpacity;

	/* Countdown until next state change */
	float Countdown;

	/** Last used scaling value for  */
	float PreviousScalingFactor = 0.0f;

	FVector2D PreviousGeometrySize;
};
