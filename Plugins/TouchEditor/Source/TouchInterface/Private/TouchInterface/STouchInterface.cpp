// Copyright Lost in Game Studio. All Rights Reserved.

#include "TouchInterface/STouchInterface.h"

#include "Rendering/DrawElements.h"
#include "Misc/ConfigCacheIni.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateUser.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputSubsystems.h"
#include "ShapeManager.h"
#include "SVirtualShapeDrawer.h"
#include "TouchInterfaceStyle.h"
#include "TouchInterfaceSettings.h"
#include "TouchInterface/STouchRegion.h"
#include "TouchInterface/SVirtualButton.h"
#include "TouchInterface/SVirtualControlJoystick.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "VirtualControlSetup.h"
#include "Components/TouchInterfaceListener.h"
#include "SaveSystem/VirtualControlSave.h"

DEFINE_LOG_CATEGORY_STATIC(LogTouchInterface, All, All);

const float OPACITY_LERP_RATE = 3.f;

void STouchInterface::Construct(const FArguments& InArgs, ULocalPlayer* InLocalPlayer, UVirtualControlSetup* InVirtualControlSetup)
{
	LocalPlayer = InLocalPlayer;

	check(LocalPlayer);
	
	EnhancedInputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	WorldContext = LocalPlayer->GetWorld();
	check(WorldContext);

	ControllerIdAssociated = LocalPlayer->GetControllerId();
	
	//ControllerIdAssociated = 0; //Get Player state then player id

	VirtualControlSetup = InVirtualControlSetup;
	
	State = State_Inactive;
	bVisible = true;
	bActiveEventSend = false;
	
	Settings = GetDefault<UTouchInterfaceSettings>();

	bDrawDebug = Settings->bDrawDebug;
	
	ActiveOpacity = VirtualControlSetup->ActiveOpacity;
	InactiveOpacity = VirtualControlSetup->InactiveOpacity;
	TimeUntilDeactivated = VirtualControlSetup->TimeUntilDeactivated;
	TimeUntilReset = VirtualControlSetup->TimeUntilReset;
	ActivationDelay = VirtualControlSetup->ActivationDelay;
	StartupDelay = VirtualControlSetup->StartupDelay;

	bCalculatePositionAuto = VirtualControlSetup->bCalculatePortraitPositionAtRuntime;
	
	CurrentOpacity = InactiveOpacity;
	
	ScalingMode = Settings->ScalingMode;
	ScaleMultiplier = Settings->ScaleMultiplier;
	bGestureRecognizerEnabled = Settings->bEnableGestureRecognizer;
	bShapeRecognizerEnabled = Settings->bEnableShapeRecognizer;
	bDrawUserShape = Settings->bDrawUserShape;
	
	bBlockInput = false;
	bBlockRecognizers = false;
	bIsInLandscapeMode = true;

	OnTouchBeganEvent = InArgs._OnTouchBegan;
	OnTouchMovedEvent = InArgs._OnTouchMoved;
	OnTouchEndedEvent = InArgs._OnTouchEnded;

	OnActiveStateChanged = InArgs._OnActiveStateChanged;
	OnVisibilityStateChanged = InArgs._OnVisibilityStateChanged;

	if (StartupDelay > 0.f)
	{
		State = State_WaitForStart;
	}
	
	ChildSlot
	[
		SAssignNew(VirtualControlCanvas, SConstraintCanvas)
		.Visibility(EVisibility::HitTestInvisible)
	];

	if (InArgs._LoadConfig)
	{
		if (!LoadConfiguration(InArgs._UserIndex, InArgs._ConfigIndex))
		{
			GenerateVirtualControls(VirtualControlSetup->VirtualControls);
		}
	}
	else
	{
		GenerateVirtualControls(VirtualControlSetup->VirtualControls);
	}

	if (ActivationDelay != 0.0f)
	{
		SetEnabled(false);
		RegisterActiveTimer(ActivationDelay, FWidgetActiveTimerDelegate::CreateSP(this, &STouchInterface::ActivateTouchInterface));
	}

	if (bDrawUserShape)
	{		
		/** Todo: Create UWidget factory to allow user to create specialized widget that is responsible for appearance of user drawing.
		 * Make event like OnPointAdded, OnShapePointAdded (when runtime detection will be available) 
		 * Then user select class in touch interface settings
		 */
		VirtualControlCanvas->AddSlot().ZOrder(Settings->DrawerZOrder)
		[
			SAssignNew(ShapeDrawer, SVirtualShapeDrawer)
			.Visibility(EVisibility::HitTestInvisible)
		];
	}
	
	// listen for displaymetrics (screen resolution, monitor re-arranged) changes to recalculate virtual controls size and position
	//FSlateApplication::Get().GetPlatformApplication()->OnDisplayMetricsChanged().AddSP(this, &STouchInterface::HandleDisplayMetricsChanged);

	//Listen for screen orientation changes to reposition virtual controls
	FCoreDelegates::ApplicationReceivedScreenOrientationChangedNotificationDelegate.AddSP(this, &STouchInterface::HandleOnOrientationChanged);
}

void STouchInterface::GenerateVirtualControls(TArray<FVirtualControl> InVirtualControls)
{	
	for (const FVirtualControl& VirtualControl : InVirtualControls)
	{
		if (!VirtualControl.bIsChild)
		{
			TSharedPtr<SVirtualControl> ConstructedControl = nullptr;
		
			SConstraintCanvas::FSlot* ConstructedSlot = nullptr;
			VirtualControlCanvas->AddSlot()
			//.Alignment(FVector2D(0.5f,0.5f))
			//.Anchors(FAnchors(0,0,0,0))
			.AutoSize(true)
			.Expose(ConstructedSlot)
			[
				ConstructVirtualControlWidget(VirtualControl, ConstructedSlot, ConstructedControl)
			];

			ConstructedControl->SetCanvasSlot(ConstructedSlot);

			if (VirtualControl.IsParent())
			{
				GenerateChildControls(VirtualControl.Children, ConstructedControl);
			}
		}
	}
}

TSharedRef<SVirtualControl> STouchInterface::ConstructVirtualControlWidget(const FVirtualControl& InVirtualControl, SConstraintCanvas::FSlot* OwnerSlot, TSharedPtr<SVirtualControl>&ConstructedControl)
{	
	TSharedPtr<SVirtualControl> VirtualControl = nullptr;

	const EVisibility ControlVisibility = InVirtualControl.bStartHidden ? EVisibility::Hidden : EVisibility::Visible;
	
	switch (InVirtualControl.Type)
	{
	case EControlType::Button:
		{
			VirtualControl = SNew(SVirtualButton)
			.TouchInterface(SharedThis(this))
			.VirtualControl(InVirtualControl)
			.Slot(OwnerSlot)
			.AutoPositioning(bCalculatePositionAuto)
			.Opacity(this, &STouchInterface::GetVirtualControlOpacity)
			.ScaleFactor(this, &STouchInterface::GetCurrentScaleFactor)
			.DrawDebug(this, &STouchInterface::GetDrawDebug)
			.UseInputAction(Settings->bUseEnhancedInput)
			.Visibility(ControlVisibility);
		}
		break;
	case EControlType::Joystick:
		{
			VirtualControl = SNew(SVirtualControlJoystick)
			.TouchInterface(SharedThis(this))
			.VirtualControl(InVirtualControl)
			.Slot(OwnerSlot)
			.AutoPositioning(bCalculatePositionAuto)
			.Opacity(this, &STouchInterface::GetVirtualControlOpacity)
			.ScaleFactor(this, &STouchInterface::GetCurrentScaleFactor)
			.DrawDebug(this, &STouchInterface::GetDrawDebug)
			.UseInputAction(Settings->bUseEnhancedInput)
			.Visibility(ControlVisibility);
		}
		break;
	case EControlType::TouchRegion:
		{
			VirtualControl = SNew(STouchRegion)
			.TouchInterface(SharedThis(this))
			.VirtualControl(InVirtualControl)
			.Slot(OwnerSlot)
			.AutoPositioning(bCalculatePositionAuto)
			.ScaleFactor(this, &STouchInterface::GetCurrentScaleFactor)
			.DrawDebug(this, &STouchInterface::GetDrawDebug)
			.UseInputAction(Settings->bUseEnhancedInput)
			.DeltaThresholdSetting(Settings->DeltaThreshold)
			.Visibility(ControlVisibility);
		}
		break;
	default:
		UE_LOG(LogTouchInterface, Warning, TEXT("STouchInterface::ConstructVirtualControlWidget INVALID_CONTROL_TYPE"));
		break;
	}

	ChildWidgets.Add(FVirtualControlWidget(InVirtualControl.ControlName, OwnerSlot, VirtualControl, InVirtualControl.bIsChild));
	ConstructedControl = VirtualControl;
	return VirtualControl.ToSharedRef();
}

void STouchInterface::GenerateChildControls(const TArray<FName> ChildrenName, TSharedPtr<SVirtualControl> Parent)
{
	TArray<FVirtualControl> VirtualControls = VirtualControlSetup->VirtualControls;

	for (const FName ChildName : ChildrenName)
	{
		for (const FVirtualControl& VirtualControl : VirtualControls)
		{
			if (VirtualControl.ControlName == ChildName)
			{
				if (VirtualControl.bIsChild)
				{
					UE_LOG(LogTouchInterface, Log, TEXT("Is Child"));
				}
				else
				{
					UE_LOG(LogTouchInterface, Error, TEXT("Try to create a virtual control that is not a child"));
				}
				
				TSharedPtr<SVirtualControl> ConstructedControl = nullptr;
		
				SConstraintCanvas::FSlot* ConstructedSlot = nullptr;
				VirtualControlCanvas->AddSlot()
				.AutoSize(true)
				.Expose(ConstructedSlot)
				[
					ConstructVirtualControlWidget(VirtualControl, ConstructedSlot, ConstructedControl)
				];

				ConstructedControl->SetCanvasSlot(ConstructedSlot);

				Parent->AddChild(ConstructedControl);
			}
		}
	}
}

EActiveTimerReturnType STouchInterface::ActivateTouchInterface(double InCurrentTime, float InDeltaTime)
{
	SetEnabled(true);
	return EActiveTimerReturnType::Stop;
}

FReply STouchInterface::OnTouchStarted(const FGeometry& MyGeometry, const FPointerEvent& Event)
{
	FReply Reply = FReply::Unhandled();

	if (bDrawDebug)
	{
		Fingers.Add(FFingersData(Event.GetPointerIndex(), MyGeometry.AbsoluteToLocal(Event.GetScreenSpacePosition())));
		Reply = FReply::Handled();
	}

	bool IsThereAnActiveControl = false;
	bool bBlockTouchRegionInput = false;
	TSharedPtr<SVirtualControl> ProcessLast = nullptr;

	FVector2D LocalCoord = MyGeometry.AbsoluteToLocal(Event.GetScreenSpacePosition());

	FSlateApplication::Get().GetUser(GetControllerId())->SetFocus(SharedThis(this));

	if (!bBlockInput)
	{
		if (OnTouchBeganEvent.IsBound()) OnTouchBeganEvent.Execute(Event.GetPointerIndex(), Event.GetScreenSpacePosition(), LocalCoord);		

		for (FVirtualControlWidget& ControlWidget : ChildWidgets)
		{
			if (ControlWidget.VirtualControl->IsInside(LocalCoord))
			{
				if (ControlWidget.VirtualControl->GetControlType() == EControlType::TouchRegion)
				{
					ProcessLast = ControlWidget.VirtualControl;
				}
				else
				{
					IsThereAnActiveControl = true;
					if (ControlWidget.VirtualControl->OnPress(MyGeometry, Event))
					{
						bBlockTouchRegionInput = true;
					}
					NumberOfActiveControl += 1;
				}
			}
		}

		if (ProcessLast.IsValid() && !bBlockTouchRegionInput)
		{
			IsThereAnActiveControl = true;
			ProcessLast->OnPress(MyGeometry, Event);
			NumberOfActiveControl += 1;
		}
	}

	if (IsThereAnActiveControl)
	{
		CurrentOpacity = ActiveOpacity;
		Reply = FReply::Handled().CaptureMouse(SharedThis(this));
	}
	else
	{
		if ((bGestureRecognizerEnabled || bShapeRecognizerEnabled) && !bBlockRecognizers)
		{
			bool bBlockLowerPriority = false;
			int32 Priority = TNumericLimits<int32>::Max();
			for (UTouchInterfaceListener* InputManager : TouchInputComps)
			{
				if (bBlockLowerPriority)
				{
					if (InputManager->Priority == Priority)
					{
						InputManager->OnTouchStarted(MyGeometry, Event);
					}
					else
					{
						break;
					}
				}
				else
				{
					if (InputManager->OnTouchStarted(MyGeometry, Event))
					{
						bBlockLowerPriority = true;
						Priority = InputManager->Priority;
					}
				}
			}
			
			Reply = FReply::Handled().CaptureMouse(SharedThis(this));
		}

		if (bDrawUserShape)
		{
			ShapeDrawer->DrawStarted(MyGeometry, Event);
		}
	}

	return Reply;
}

FReply STouchInterface::OnTouchMoved(const FGeometry& MyGeometry, const FPointerEvent& Event)
{
	FReply Reply = FReply::Unhandled();

	TOptional<EFocusCause> FocusCause = FSlateApplication::Get().GetUser(GetControllerId())->HasFocus(SharedThis(this));
	if (FocusCause.IsSet())
	{
		UE_LOG(LogTouchInterface, Log, TEXT("Touch Interface has focus"));
	}
	

	FVector2D LocalCoord = MyGeometry.AbsoluteToLocal(Event.GetScreenSpacePosition());
	
	if (bDrawDebug)
	{
		for (FFingersData& Data : Fingers)
		{
			if (Data.FingerIndex == Event.GetPointerIndex())
			{
				Data.Position = LocalCoord;
			}
		}
		Reply = FReply::Handled();
	}
	
	if (OnTouchMovedEvent.IsBound()) OnTouchMovedEvent.Execute(Event.GetPointerIndex(), Event.GetScreenSpacePosition(), LocalCoord);
	
	if ((bGestureRecognizerEnabled || bShapeRecognizerEnabled) && !bBlockRecognizers)
	{
		for (UTouchInterfaceListener* GestureManager : TouchInputComps)
		{
			if (GestureManager->HasStarted())
			{
				GestureManager->OnTouchMoved(MyGeometry, Event);
			}
		}
	}

	if (bDrawUserShape)
	{
		ShapeDrawer->DrawUpdated(MyGeometry, Event);
	}

	bool IsThereAnActiveControl = false;

	for (const FVirtualControlWidget& ControlWidget : ChildWidgets)
	{
		if (ControlWidget.VirtualControl->GetPointerIndex() == Event.GetPointerIndex())
		{
			IsThereAnActiveControl = true;
			ControlWidget.VirtualControl->OnMove(MyGeometry, Event);
		}
	}

	if (IsThereAnActiveControl)
	{
		Reply = FReply::Handled();
	}

	return Reply;
}

FReply STouchInterface::OnTouchEnded(const FGeometry& MyGeometry, const FPointerEvent& Event)
{
	FReply Reply = FReply::Unhandled();

	FVector2D LocalCoord = MyGeometry.AbsoluteToLocal(Event.GetScreenSpacePosition());

	FSlateApplication::Get().GetUser(GetControllerId())->SetFocus(SharedThis(this));
	
	if (bDrawDebug)
	{
		int32 IndexToRemove = -1;
		for (int32 Itr = 0; Itr < Fingers.Num(); ++Itr)
		{
			if (Fingers[Itr].FingerIndex == Event.GetPointerIndex())
			{
				IndexToRemove = Itr;
			}
		}

		if (IndexToRemove != -1)
		{
			Fingers.RemoveAt(IndexToRemove);
			Reply = FReply::Handled();
		}
	}	
	
	if (bGestureRecognizerEnabled && !bBlockRecognizers)
	{
		for (UTouchInterfaceListener* GestureManager : TouchInputComps)
		{
			if (GestureManager->HasStarted())
			{
				GestureManager->OnTouchEnded(MyGeometry, Event);
			}
		}
		
		Reply = FReply::Handled().ReleaseMouseCapture();
	}

	if (bDrawUserShape)
	{
		ShapeDrawer->DrawEnded(MyGeometry, Event);
	}

	for (FVirtualControlWidget& ControlWidget : ChildWidgets)
	{
		if (ControlWidget.VirtualControl->GetPointerIndex() == Event.GetPointerIndex())
		{
			ControlWidget.VirtualControl->OnRelease(MyGeometry, Event);
			NumberOfActiveControl -= 1;
		}
	}

	if (NumberOfActiveControl <= 0)
	{
		Reply = FReply::Handled().ReleaseMouseCapture();
	}

	if (OnTouchEndedEvent.IsBound()) OnTouchEndedEvent.Execute(Event.GetPointerIndex(), Event.GetScreenSpacePosition(), LocalCoord);
	
	return Reply;
}

void STouchInterface::OnDrawStarted(const FGeometry& MyGeometry, const FPointerEvent& Event)
{
	
}

void STouchInterface::OnDrawUpdated(const FGeometry& MyGeometry, const FPointerEvent& Event)
{
	
}

void STouchInterface::OnDrawEnded(const FGeometry& MyGeometry, const FPointerEvent& Event)
{
	
}

void STouchInterface::ProcessUserDrawing()
{
	
}

void STouchInterface::PredictVirtualShape()
{
	
}

void STouchInterface::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	const float ScaleFactor = GetScaleFactor(AllottedGeometry);
	const FVector2D GeometrySize = AllottedGeometry.GetAbsoluteSize();

	const bool bForceUpdate = ScaleFactor != PreviousScalingFactor || GeometrySize != PreviousGeometrySize;
	
	for (const FVirtualControlWidget& ControlWidget : ChildWidgets)
	{
		if (!ControlWidget.bIsChild)
		{
			ControlWidget.VirtualControl->OnTick(AllottedGeometry, ScaleFactor, InCurrentTime, InDeltaTime, bForceUpdate, bIsInLandscapeMode);
		}
	}
	
	SetWidgetOpacity(NumberOfActiveControl, InDeltaTime);
	PreviousScalingFactor = ScaleFactor;
	PreviousGeometrySize = GeometrySize;
}

int32 STouchInterface::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{	
	if (bDrawDebug)
	{
		FLinearColor CircleColor = FLinearColor::White;
		CircleColor.A = GetDefault<UTouchInterfaceSettings>()->DebugOpacity;

		FLinearColor LineColor = FLinearColor::Blue;
		LineColor.A = GetDefault<UTouchInterfaceSettings>()->DebugOpacity;
		
		for (const FFingersData& Data : Fingers)
		{
			FSlateDrawElement::MakeBox
			(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(FVector2D(32.0f), FSlateLayoutTransform(Data.Position - FVector2D(16.0f))),
				FTouchInterfaceStyle::Get().GetBrush("WhiteCircle"),
				ESlateDrawEffect::None,
				CircleColor
			);

			TArray<FVector2D> HorizontalLinePoints;
			HorizontalLinePoints.Add(FVector2D(0.0f, Data.Position.Y));
			HorizontalLinePoints.Add(FVector2D(AllottedGeometry.GetLocalSize().X, Data.Position.Y));
			FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), HorizontalLinePoints, ESlateDrawEffect::None, LineColor);

			TArray<FVector2D> VerticalLinePoints;
			VerticalLinePoints.Add(FVector2D(Data.Position.X, 0.0f));
			VerticalLinePoints.Add(FVector2D(Data.Position.X, AllottedGeometry.GetLocalSize().Y));
			FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), VerticalLinePoints, ESlateDrawEffect::None, LineColor);
		}
	}
	
	return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

void STouchInterface::RebuildTouchInterface(const bool bLoadConfig, const int32 UserIndex, const int32 ConfigIndex)
{
	if (bLoadConfig)
	{
		if (LoadConfiguration(UserIndex, ConfigIndex))
		{
			return;
		}		
	}
	
	//Todo: Flush all input/gesture/event before clear

	//Set Widget visibility to HitTestInvisible

	//Call Flush input on all virtual control
	
	//Clear all widget
	VirtualControlCanvas->ClearChildren();
	VirtualControlWidgets.Empty();

	//and rebuild
	GenerateVirtualControls(VirtualControlSetup->VirtualControls);
}

bool STouchInterface::ShouldDisplayTouchInterface()
{
	const bool bShowInDesktop = GetDefault<UTouchInterfaceSettings>()->bShowInDesktopPlatform;
	
	// Check if we want to show Touch Interface
	return FPlatformMisc::GetUseVirtualJoysticks() || bShowInDesktop || (FSlateApplication::Get().IsFakingTouchEvents() && FPlatformMisc::ShouldDisplayTouchInterfaceOnFakingTouchEvents());
}

UWorld* STouchInterface::GetWorldContext() const
{
	return WorldContext;
}

// INPUT

bool STouchInterface::RegisterTouchInputComponent(UTouchInterfaceListener* ManagerComp)
{
	if (!TouchInputComps.Contains(ManagerComp))
	{
		TouchInputComps.AddUnique(ManagerComp);
		return true;
	}
	return false;
}

bool STouchInterface::UnregisterTouchInputComponent(UTouchInterfaceListener* ManagerComp)
{
	//Todo: Need to call OnTouchEnd before unregister ?
	if (TouchInputComps.Contains(ManagerComp))
	{
		TouchInputComps.Remove(ManagerComp);
		return true;
	}
	return false;
}

bool STouchInterface::RegisterShapeManagerComponent(UShapeManager* ManagerComp)
{
	if (!ShapeManagerComps.Contains(ManagerComp))
	{
		ShapeManagerComps.AddUnique(ManagerComp);
		return true;
	}
	return false;
}

bool STouchInterface::UnregisterShapeManagerComponent(UShapeManager* ManagerComp)
{
	if (ShapeManagerComps.Contains(ManagerComp))
	{
		ShapeManagerComps.Remove(ManagerComp);
		return true;
	}
	return false;
}

void STouchInterface::SetTouchInputComponentList(TArray<UTouchInterfaceListener*>& Comps)
{
	//Todo: Sorting here instead of in TouchInterface subsystem ?
	TouchInputComps = Comps;
}

void STouchInterface::ShowDrawGuide(const FName VirtualShapeName)
{
	//Todo: Need Implementation
}

void STouchInterface::HideDrawGuide()
{
	//Todo: Need Implementation
}

void STouchInterface::TryRecognizeShapeOnAllManagers()
{
	for (UTouchInterfaceListener* Manager : TouchInputComps)
	{
		if (UShapeManager* ShapeManager = Cast<UShapeManager>(Manager))
		{
			ShapeManager->TryRecognizeShape();
		}
	}
}

bool STouchInterface::ChangeInputKey(const FName ControlName, FKey& Main, FKey& Alt)
{
	//Todo: Change name of FKey, Use ButtonInputKey for press and release of Joystick
	for (FVirtualControlWidget& ControlWidget : ChildWidgets)
	{
		if (ControlWidget.Name.IsEqual(ControlName))
		{
			ControlWidget.VirtualControl->FlushPressedKey();
			FVirtualControl& Data = ControlWidget.VirtualControl->GetDataByRef();

			switch (Data.Type)
			{
			case EControlType::Button:
				Data.ButtonInputKey = Main;
				break;
			case EControlType::Joystick:
				Data.HorizontalInputKey = Main;
				Data.VerticalInputKey = Alt;
				break;
			case EControlType::TouchRegion:
				Data.HorizontalInputKey = Main;
				Data.VerticalInputKey = Alt;
				break;
			}
		}
	}
	return false;
}

bool STouchInterface::ChangeInputAction(const FName ControlName, UInputAction* Action)
{
	for (FVirtualControlWidget& ControlWidget : ChildWidgets)
	{
		if (ControlWidget.Name.IsEqual(ControlName))
		{
			ControlWidget.VirtualControl->FlushPressedKey();
			FVirtualControl& Data = ControlWidget.VirtualControl->GetDataByRef();

			switch (Data.Type)
			{
			case EControlType::Button:
				Data.ButtonAction = Action;
				break;
			case EControlType::Joystick:
				Data.JoystickAction = Action;
				break;
			case EControlType::TouchRegion:
				Data.JoystickAction = Action;
				break;
			}
		}
	}
	return false;
}

// TOUCH INTERFACE

void STouchInterface::SetWidgetVisibility(const bool bInVisible, const bool bInFade, const bool bInBlockInput, const bool bInBlockGesture)
{
	// if we aren't fading, then just set the current opacity to desired
	if (bInFade)
	{
		if (bInVisible)
		{
			CurrentOpacity = GetBaseOpacity();
		}
		else
		{
			CurrentOpacity = 0.0f;
		}
	}
	else
	{
		if (bInVisible)
		{
			SetVisibility(EVisibility::Visible);
		}
		else
		{
			SetVisibility(EVisibility::Hidden);
		}
		
	}

	//Todo: GestureRecognizerEnabled defined by setting then BlockGesture if user want to temporally disable gesture

	bVisible = bInVisible;
	bBlockInput = bInBlockInput;
	bBlockRecognizers = bInBlockGesture;
	if (OnVisibilityStateChanged.IsBound())
	{
		OnVisibilityStateChanged.Execute(bInVisible);
	}
}

//VIRTUAL CONTROL

void STouchInterface::SetControlVisibility(const FName Name, const bool bInVisible, const bool bIncludeChildren)
{
	for (const FVirtualControlWidget& ControlWidget : ChildWidgets)
	{
		if (ControlWidget.Name.IsEqual(Name))
		{
			ControlWidget.VirtualControl->SetVisibility(bInVisible ? EVisibility::HitTestInvisible : EVisibility::Hidden);
			if (ControlWidget.VirtualControl->IsParent() && bIncludeChildren)
			{
				for (const TSharedPtr<SVirtualControl>& VirtualControl : ControlWidget.VirtualControl->GetLinkedVirtualControls())
				{
					VirtualControl->SetVisibility(bInVisible ? EVisibility::HitTestInvisible : EVisibility::Hidden);
				}
			}
			break;
		}
	}
}

void STouchInterface::ShowAllControls(const bool IncludeButton, const bool IncludeJoystick, const bool IncludeTouchRegion)
{
	for (const FVirtualControlWidget& ControlWidget : ChildWidgets)
	{
		switch (ControlWidget.VirtualControl->GetControlType())
		{
		case EControlType::Button:
			if (IncludeButton)
			{
				ControlWidget.VirtualControl->SetVisibility(EVisibility::HitTestInvisible);
			}
			break;
		case EControlType::Joystick:
			if (IncludeJoystick)
			{
				ControlWidget.VirtualControl->SetVisibility(EVisibility::HitTestInvisible);
			}
			break;
		case EControlType::TouchRegion:
			if (IncludeTouchRegion)
			{
				ControlWidget.VirtualControl->SetVisibility(EVisibility::HitTestInvisible);
			}
			break;
		}
	}
}

void STouchInterface::HideAllControls(const bool IncludeButton, const bool IncludeJoystick, const bool IncludeTouchRegion)
{
	for (const FVirtualControlWidget& ControlWidget : ChildWidgets)
	{
		switch (ControlWidget.VirtualControl->GetControlType())
		{
		case EControlType::Button:
			if (IncludeButton)
			{
				ControlWidget.VirtualControl->SetVisibility(EVisibility::Hidden);
			}
			break;
		case EControlType::Joystick:
			if (IncludeJoystick)
			{
				ControlWidget.VirtualControl->SetVisibility(EVisibility::Hidden);
			}
			break;
		case EControlType::TouchRegion:
			if (IncludeTouchRegion)
			{
				ControlWidget.VirtualControl->SetVisibility(EVisibility::Hidden);
			}
			break;
		}
	}
}

bool STouchInterface::GetVirtualControlData(const FName ControlName, FVirtualControl& VirtualControl)
{
	for (FVirtualControlWidget& ControlWidget : ChildWidgets)
	{
		if (ControlWidget.Name.IsEqual(ControlName))
		{
			VirtualControl = ControlWidget.VirtualControl->GetData();
			return true;
		}
	}
	return false;
}

TArray<FVirtualControl> STouchInterface::GetAllControls(const bool IncludeButton, const bool IncludeJoystick, const bool IncludeTouchRegion)
{
	TArray<FVirtualControl> OutControls;

	for (const FVirtualControlWidget& ControlWidget : ChildWidgets)
	{
		switch (ControlWidget.VirtualControl->GetControlType())
		{
		case EControlType::Button:
			if (IncludeButton)
			{
				OutControls.Add(ControlWidget.VirtualControl->GetDataByRef());
			}
			break;
		case EControlType::Joystick:
			if (IncludeJoystick)
			{
				OutControls.Add(ControlWidget.VirtualControl->GetDataByRef());
			}
			break;
		case EControlType::TouchRegion:
			if (IncludeTouchRegion)
			{
				OutControls.Add(ControlWidget.VirtualControl->GetDataByRef());
			}
			break;
		}
	}

	return OutControls;
}

void STouchInterface::AddControl(const FVirtualControl& NewControl)
{
	TSharedPtr<SVirtualControl> ConstructedControl = nullptr;
	
	SConstraintCanvas::FSlot* Slot = nullptr;
	VirtualControlCanvas->AddSlot()
	.Expose(Slot)
	[
		ConstructVirtualControlWidget(NewControl, Slot, ConstructedControl)
	];

	ConstructedControl->SetCanvasSlot(Slot);
}

void STouchInterface::RemoveControl(const FName Name, const bool bRemoveChildren)
{
	int32 IndexToRemove = -1;
	
	for (int32 ControlIndex = 0; ControlIndex < ChildWidgets.Num(); ++ControlIndex)
	{
		if (ChildWidgets[ControlIndex].Name.IsEqual(Name))
		{
			IndexToRemove = ControlIndex;
			break;
		}
	}

	if (IndexToRemove >= 0)
	{
		if (ChildWidgets[IndexToRemove].VirtualControl->IsParent() && bRemoveChildren)
		{
			const TArray<TSharedPtr<SVirtualControl>>& Children = ChildWidgets[IndexToRemove].VirtualControl->GetLinkedVirtualControls();
			for (const TSharedPtr<SVirtualControl>& Child : Children)
			{
				for (int32 ControlIndex = 0; ControlIndex < ChildWidgets.Num(); ++ControlIndex)
				{
					if (Child->GetControlName().IsEqual(ChildWidgets[ControlIndex].Name))
					{
						VirtualControlCanvas->RemoveSlot(ChildWidgets[ControlIndex].VirtualControl.ToSharedRef());
						ChildWidgets.RemoveAt(ControlIndex);
						break;
					}
				}
			}
		}
		
		VirtualControlCanvas->RemoveSlot(ChildWidgets[IndexToRemove].VirtualControl.ToSharedRef());
		ChildWidgets.RemoveAt(IndexToRemove);
	}
}

void STouchInterface::ResetToDefault()
{
	VirtualControlCanvas->ClearChildren();
	ChildWidgets.Empty();
	GenerateVirtualControls(VirtualControlSetup->VirtualControls);
}

// VISUAL

void STouchInterface::SetVirtualControlVisualLayers(const FName ControlName, TArray<FVisualLayer> Layers)
{
	for (FVirtualControlWidget& ControlWidget : ChildWidgets)
	{
		if (ControlWidget.Name.IsEqual(ControlName))
		{
			ControlWidget.VirtualControl->GetDataByRef().VisualLayers = Layers;
		}
	}
}

UMaterialInstanceDynamic* STouchInterface::GetLayerDynamicMaterialInstance(const FName ControlName, const FName LayerName, UObject* InOuter)
{
	for (FVirtualControlWidget& ControlWidget : ChildWidgets)
	{
		if (ControlWidget.Name.IsEqual(ControlName))
		{
			return ControlWidget.VirtualControl->GetLayerDynamicMaterialInstance(LayerName, InOuter);
		}
	}

	return nullptr;
}

void STouchInterface::SetControlSize(const FName Name, const FVector2D NewVisualSize, const FVector2D NewThumbSize, const FVector2D NewInteractionSize)
{	
	for (FVirtualControlWidget& ControlWidget : ChildWidgets)
	{
		if (ControlWidget.Name.IsEqual(Name))
		{
			FVirtualControl& Data = ControlWidget.VirtualControl->GetDataByRef();
			Data.VisualSize = NewVisualSize;
			Data.ThumbSize = NewThumbSize;
			Data.InteractionSize = NewInteractionSize;

			ControlWidget.VirtualControl->RefreshPosition();
			break;
		}
	}
}

bool STouchInterface::SetControlPosition(const FName Name, const FVector2D NewPosition, const bool InLandscape)
{	
	for (FVirtualControlWidget& ControlWidget : ChildWidgets)
	{
		if (ControlWidget.Name.IsEqual(Name))
		{
			if (ControlWidget.VirtualControl->IsParent())
			{
				// Clamp new position to 0-1
				const FVector2D DesiredPosition = NewPosition.ClampAxes(0.0f, 1.0f);
				if (bIsInLandscapeMode)
				{
					ControlWidget.VirtualControl->GetDataByRef().LandscapeCenter = DesiredPosition;
				}
				else
				{
					ControlWidget.VirtualControl->GetDataByRef().PortraitCenter = DesiredPosition;
				}

				ControlWidget.VirtualControl->RefreshPosition();
				return true;
			}
		}
	}

	return false;
}

// UTILITIES

bool STouchInterface::GetTypeOfControl(const FName ControlName, EControlType& Type)
{
	for (const FVirtualControlWidget& ControlWidget : ChildWidgets)
	{
		if (ControlWidget.Name.IsEqual(ControlName))
		{
			Type = ControlWidget.VirtualControl->GetControlType();
			return true;
		}
	}

	return false;
}

bool STouchInterface::ContainName(const FName ControlName)
{
	for (FVirtualControlWidget& ControlWidget : ChildWidgets)
	{
		if (ControlWidget.Name.IsEqual(ControlName))
		{
			return true;
		}
	}

	return false;
}

TArray<FName> STouchInterface::GetAllControlNames()
{
	TArray<FName> Names = {};

	for (const FVirtualControlWidget& ControlWidget : ChildWidgets)
	{
		Names.Add(ControlWidget.Name);
	}
	return Names;
}

TArray<FName> STouchInterface::GetLayerNames(const FName ControlName)
{
	TArray<FName> OutName;
	
	for (const FVirtualControlWidget& ControlWidget : ChildWidgets)
	{
		if (ControlWidget.Name.IsEqual(ControlName))
		{
			FVirtualControl& Data = ControlWidget.VirtualControl->GetDataByRef();

			for (FVisualLayer& Layer : Data.VisualLayers)
			{
				OutName.Add(Layer.ExposedLayerName);
			}
			break;
		}
		
	}
	return  OutName;
}

FVector2D STouchInterface::LocalToNormalized(const FVector2D LocalPosition) const
{
	const FVector2D LocalSize = GetTickSpaceGeometry().GetLocalSize();
	FVector2D NormalizedPosition;

	NormalizedPosition.X = LocalPosition.X / LocalSize.X;
	NormalizedPosition.Y = LocalPosition.Y / LocalSize.Y;
	
	return NormalizedPosition;
}

FVector2D STouchInterface::NormalizedToLocal(const FVector2D NormalizedPosition) const
{
	const FVector2D LocalSize = GetTickSpaceGeometry().GetLocalSize();
	FVector2D LocalPosition;
	
	LocalPosition.X = NormalizedPosition.X * LocalSize.X;
	LocalPosition.Y = NormalizedPosition.Y * LocalSize.Y;
	
	return LocalPosition;
}

float STouchInterface::GetDebugOpacity()
{
	return GetDefault<UTouchInterfaceSettings>()->DebugOpacity;
}

bool STouchInterface::ConfigurationFileExist(const uint32 UserIndex) const
{
	const FString SaveName = GetDefault<UTouchInterfaceSettings>()->SaveSlotName;
	
	//Check if Configuration Save Game Slot exist in disk
	return UGameplayStatics::DoesSaveGameExist(SaveName, UserIndex);
}

bool STouchInterface::ConfigurationExist(const uint32 UserIndex, const uint32 ConfigIndex) const
{
	const FString SaveName = GetDefault<UTouchInterfaceSettings>()->SaveSlotName;
	
	//Check if Configuration Save Game Slot exist in disk
	if (UGameplayStatics::DoesSaveGameExist(SaveName, UserIndex))
	{
		//Get Configuration save game object
		if (const UVirtualControlSave* SaveFile = Cast<UVirtualControlSave>(UGameplayStatics::LoadGameFromSlot(SaveName, UserIndex)))
		{
			return SaveFile->Configurations.Contains(ConfigIndex);
		}
	}

	return false;
}

bool STouchInterface::SaveConfiguration(const uint32 UserIndex, const uint32 ConfigIndex)
{
	//Todo: Use asynchronous saving ? Not really needed cause of small amount of data
	
	const FString SaveName = GetDefault<UTouchInterfaceSettings>()->SaveSlotName;

	//Check if Configuration Save Game Slot exist in disk
	if (UGameplayStatics::DoesSaveGameExist(SaveName, UserIndex))
	{
		//Get Configuration save game object
		if (UVirtualControlSave* SaveFile = Cast<UVirtualControlSave>(UGameplayStatics::LoadGameFromSlot(SaveName, UserIndex)))
		{
			FTouchInterfaceConfiguration NewConfiguration;
			NewConfiguration.ActivationDelay = VirtualControlSetup->ActivationDelay;
			NewConfiguration.ActiveOpacity = VirtualControlSetup->ActiveOpacity;
			NewConfiguration.InactiveOpacity = VirtualControlSetup->InactiveOpacity;
			NewConfiguration.bCalculatePositionAuto = VirtualControlSetup->bCalculatePortraitPositionAtRuntime;
			NewConfiguration.StartupDelay = VirtualControlSetup->StartupDelay;
			NewConfiguration.TimeUntilDeactivated = VirtualControlSetup->TimeUntilDeactivated;
			NewConfiguration.TimeUntilReset = VirtualControlSetup->TimeUntilReset;

			TArray<FVirtualControl> VirtualControlToSave;
			for (const FVirtualControlWidget& ControlWidget : ChildWidgets)
			{
				//Get setup of each virtual control in Touch interface and add to array
				VirtualControlToSave.Add(ControlWidget.VirtualControl->GetData());
			}
			
			NewConfiguration.VirtualControls = VirtualControlToSave;

			//Add array to TMap with specified key (config index) in save game object. If key already exist, so configuration is replaced
			SaveFile->Configurations.Add(ConfigIndex, NewConfiguration);

			//Save data to disk with save name (specified in settings) and user index
			return UGameplayStatics::SaveGameToSlot(SaveFile, SaveName, UserIndex);
		}
	}

	//Create new configuration save game object
	if (UVirtualControlSave* NewSave = Cast<UVirtualControlSave>(UGameplayStatics::CreateSaveGameObject(UVirtualControlSave::StaticClass())))
	{
		FTouchInterfaceConfiguration NewConfiguration;
		NewConfiguration.ActivationDelay = VirtualControlSetup->ActivationDelay;
		NewConfiguration.ActiveOpacity = VirtualControlSetup->ActiveOpacity;
		NewConfiguration.InactiveOpacity = VirtualControlSetup->InactiveOpacity;
		NewConfiguration.bCalculatePositionAuto = VirtualControlSetup->bCalculatePortraitPositionAtRuntime;
		NewConfiguration.StartupDelay = VirtualControlSetup->StartupDelay;
		NewConfiguration.TimeUntilDeactivated = VirtualControlSetup->TimeUntilDeactivated;
		NewConfiguration.TimeUntilReset = VirtualControlSetup->TimeUntilReset;

		TArray<FVirtualControl> VirtualControlToSave;
		for (const FVirtualControlWidget& ControlWidget : ChildWidgets)
		{
			//Get setup of each virtual control in Touch interface and add to array
			VirtualControlToSave.Add(ControlWidget.VirtualControl->GetData());
		}
			
		NewConfiguration.VirtualControls = VirtualControlToSave;

		//Add array to TMap with specified key (config index) in save game object
		NewSave->Configurations.Add(ConfigIndex, NewConfiguration);

		NewSave->UserIndex = UserIndex;
		NewSave->SaveSlotName = SaveName;

		//Save data to disk with save name (specified in settings) and user index
		return UGameplayStatics::SaveGameToSlot(NewSave, SaveName, UserIndex);
	}

	return false;
}

bool STouchInterface::LoadConfiguration(const uint32 UserIndex, const uint32 ConfigIndex)
{
	//Todo: Use asynchronous loading ?

	//Todo: Strange warning : Failed to find object 'Object None.None'. This is caused by the initialization of FTouchInterfaceInitialization ?
	
	const FString SaveName = GetDefault<UTouchInterfaceSettings>()->SaveSlotName;

	//Check if Configuration Save Game Slot exist in disk
	if (UGameplayStatics::DoesSaveGameExist(SaveName, UserIndex))
	{
		if (USaveGame* SaveGame = UGameplayStatics::LoadGameFromSlot(SaveName, UserIndex))
		{
			//Get Configuration save game object
			if (const UVirtualControlSave* SaveFile = Cast<UVirtualControlSave>(SaveGame))
			{
				if (SaveFile->Configurations.Contains(ConfigIndex))
				{
					//Get configuration in TMap with specified key
					const FTouchInterfaceConfiguration ConfigurationLoaded = SaveFile->Configurations.FindRef(ConfigIndex);
				
					//Set global settings
					ActiveOpacity = ConfigurationLoaded.ActiveOpacity;
					InactiveOpacity = ConfigurationLoaded.InactiveOpacity;
					TimeUntilDeactivated = ConfigurationLoaded.TimeUntilDeactivated;
					TimeUntilReset = ConfigurationLoaded.TimeUntilReset;
					ActivationDelay = ConfigurationLoaded.ActivationDelay;
					StartupDelay = ConfigurationLoaded.StartupDelay;
					bCalculatePositionAuto = ConfigurationLoaded.bCalculatePositionAuto;
				
					//Remove widget from Touch interface
					VirtualControlCanvas->ClearChildren();
				
					//Remove instance saved in array
					ChildWidgets.Empty();
				
					//Generate virtual controls based on data saved in configuration
					GenerateVirtualControls(ConfigurationLoaded.VirtualControls);
					return true;
				}
			}
		}
	}
	
	//Return false if virtual control configuration save game object does not exist
	return false;
}

bool STouchInterface::DeleteConfiguration(const uint32 UserIndex, const uint32 ConfigIndex) const
{
	const FString SaveName = GetDefault<UTouchInterfaceSettings>()->SaveSlotName;
	
	if (ConfigurationFileExist(UserIndex))
	{
		if (UVirtualControlSave* SaveFile = Cast<UVirtualControlSave>(UGameplayStatics::LoadGameFromSlot(SaveName, UserIndex)))
		{
			if (SaveFile->UserIndex == UserIndex && SaveFile->Configurations.Contains(ConfigIndex))
			{
				SaveFile->Configurations.FindAndRemoveChecked(ConfigIndex);
				return true;
			}
		}
	}

	return false;
}

bool STouchInterface::DeleteConfigurationSlot(const uint32 UserIndex) const
{
	const FString SaveName = GetDefault<UTouchInterfaceSettings>()->SaveSlotName;
	
	if (ConfigurationFileExist(UserIndex))
	{
		return UGameplayStatics::DeleteGameInSlot(SaveName, UserIndex);
	}

	return false;
}

void STouchInterface::SetWidgetOpacity(const int32 ActiveControls, const float DeltaTime)
{
	if (State == State_WaitForStart || State == State_CountingDownToStart)
	{
		CurrentOpacity = 0.f;
	}
	else
	{
		// Lerp to the desired opacity based on whether the user is interacting with the joystick
		CurrentOpacity = FMath::Lerp(CurrentOpacity, GetBaseOpacity(), OPACITY_LERP_RATE * DeltaTime);
	}
	
	// STATE MACHINE!
	if (ActiveControls > 0)
	{
		// Any active control snaps the state to active immediately
		State = State_Active;

		if (!bActiveEventSend)
		{
			bActiveEventSend = true;
			if (OnActiveStateChanged.IsBound())
			{
				OnActiveStateChanged.Execute(true);
			}
		}
	}
	else
	{
		switch (State)
		{
		case State_WaitForStart:
			{
				State = State_CountingDownToStart;
				Countdown = StartupDelay;
			}
			break;
		case State_CountingDownToStart:
			// Update the countdown
			Countdown -= DeltaTime;
			if (Countdown <= 0.0f)
			{
				State = State_Inactive;
				bActiveEventSend = false;
				if (OnActiveStateChanged.IsBound())
				{
					OnActiveStateChanged.Execute(false);
				}
			}
			break;
		case State_Active:
			if (ActiveControls == 0)
			{
				// Start going to inactive
				State = State_CountingDownToInactive;
				Countdown = TimeUntilDeactivated;
			}
			break;

		case State_CountingDownToInactive:
			// Update the countdown
			Countdown -= DeltaTime;
			if (Countdown <= 0.0f)
			{
				// Should we start counting down to reset virtual controls ?
				if (TimeUntilReset > 0.0f)
				{
					State = State_CountingDownToReset;
					Countdown = TimeUntilReset;
				}
				else
				{
					// If not, then just go inactive
					State = State_Inactive;
					bActiveEventSend = false;
					if (OnActiveStateChanged.IsBound())
					{
						OnActiveStateChanged.Execute(false);
					}
				}
			}
			break;

		case State_CountingDownToReset:
			Countdown -= DeltaTime;
			if (Countdown <= 0.0f)
			{
				// Reset all the controls
				for (const FVirtualControlWidget& ControlWidget : ChildWidgets)
				{
					if (!ControlWidget.bIsChild)
					{
						ControlWidget.VirtualControl->Reset();
					}
				}

				// Finally, go inactive
				State = State_Inactive;
				bActiveEventSend = false;
				if (OnActiveStateChanged.IsBound())
				{
					OnActiveStateChanged.Execute(false);
				}
			}
			break;

		case State_Inactive:
			break;

		default:
			State = State_CountingDownToReset;
			break;
		}
	}
}

FORCEINLINE float STouchInterface::GetBaseOpacity()
{
	return (State == State_Active || State == State_CountingDownToInactive) ? ActiveOpacity : InactiveOpacity;
}

float STouchInterface::GetScaleFactor(const FGeometry& Geometry)
{
	return CurrentScaleFactor = Settings->GetScaleFactor(Geometry.GetAbsoluteSize(), Geometry.GetAccumulatedLayoutTransform().GetScale(), false);
}

void STouchInterface::HandleOnOrientationChanged(const int32 Mode)
{
	// Orientation doesn't change while a finger touch screen

	/**
	 *Unknown = 0 ?,
	 *Portrait = 1,
	 *PortraitUpsideDown = 2,
	 *LandscapeLeft = 3,
	 *LandscapeRight = 4,
	 *FaceUp = 5,
	 *FaceDown = 6,
	 */

	UE_LOG(LogTouchInterface, Log, TEXT("Handle Orientation Change"));
	
	if (Mode == 1 || Mode == 2)
	{
		UE_LOG(LogTouchInterface, Display, TEXT("On Portrait Orientation"));
		// Portrait
		bIsInLandscapeMode = false;
	}
	else
	{
		UE_LOG(LogTouchInterface, Display, TEXT("On Landscape Orientation"));
		// Landscape
		bIsInLandscapeMode = true;
	}
}
