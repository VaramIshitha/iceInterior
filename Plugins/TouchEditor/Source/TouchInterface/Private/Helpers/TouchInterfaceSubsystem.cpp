// Copyright Lost in Game Studio. All Rights Reserved.

#include "Helpers/TouchInterfaceSubsystem.h"

#include "STouchInterface.h"
#include "GestureManager.h"

#include "TouchInterfaceSettings.h"
#include "Misc/CoreDelegates.h"

//Needed for mobile
#include "Components/InputComponent.h"
#include "Framework/Commands/InputChord.h"
#include "Engine/GameViewportClient.h"
//Needed for mobile

DEFINE_LOG_CATEGORY_STATIC(LogTouchInterfaceSubsystem, All, All)

void UTouchInterfaceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTouchInterfaceSubsystem, Log, TEXT("Initialize subsystem with ID : %d"), GetLocalPlayer()->GetControllerId());
	WorldContext = GetLocalPlayer()->GetWorld();
	LocalPlayerInstance = GetLocalPlayer<ULocalPlayer>();

	check(WorldContext);
	check(LocalPlayerInstance);
}

void UTouchInterfaceSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UTouchInterfaceSubsystem::RegisterTouchManagerComponent(UTouchInterfaceListener* Manager)
{
	if (!RegisteredTouchManager.Contains(Manager))
	{
		RegisteredTouchManager.AddUnique(Manager);

		if (RegisteredTouchManager.Num() > 1)
		{
			RegisteredTouchManager.Sort([](const UTouchInterfaceListener& A, const UTouchInterfaceListener& B)
			{
				return A.Priority > B.Priority;
			});
		}
		
		if (TouchInterface.IsValid())
		{
			TouchInterface->SetTouchInputComponentList(RegisteredTouchManager);
			return true;
		}
	}
	
	return false;
}

bool UTouchInterfaceSubsystem::UnregisterTouchManagerComponent(UTouchInterfaceListener* Manager)
{
	if (RegisteredTouchManager.Contains(Manager))
	{
		RegisteredTouchManager.Remove(Manager);
		if (TouchInterface.IsValid()) return TouchInterface->UnregisterTouchInputComponent(Manager);
	}
	
	return false;
}

void UTouchInterfaceSubsystem::AddToPlayerScreen(APlayerController* PlayerController, UVirtualControlSetup* NewTouchDesignerInterface, const FTouchInterfaceInit& State)
{
	if (!PlayerController)
	{
#if WITH_EDITOR
		GEngine->AddOnScreenDebugMessage(-1, 10,FColor::Red,TEXT("Add To Player Screen fail because PlayerController is null! Virtual Control Manager need a Player Controller to work properly"));
#endif
		return;
	}	

	if (NewTouchDesignerInterface)
	{
		CurrentVirtualControlSetup = NewTouchDesignerInterface;
		
		CreateTouchInterface(State.bLoadConfiguration, State.UserIndex, State.ConfigIndex);
	}
	else
	{
		// Get path for Default Touch Designer Interface in Settings
		const FSoftObjectPath DefaultVirtualControlSetupName = GetDefault<UTouchInterfaceSettings>()->DefaultVirtualControlSetup;
		if (DefaultVirtualControlSetupName.IsValid())
		{
#if WITH_EDITOR
			GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Orange, TEXT("Warning! Virtual Control Setup is null so the controller load Default Virtual Control Setup instead"));
#endif

			// Load Default Touch Designer Interface
			CurrentVirtualControlSetup = LoadObject<UVirtualControlSetup>(nullptr, *DefaultVirtualControlSetupName.ToString());
			if (CurrentVirtualControlSetup)
			{
				CreateTouchInterface(State.bLoadConfiguration, State.UserIndex, State.ConfigIndex);
			}
			else
			{
#if WITH_EDITOR
				GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Orange, TEXT("Warning! The loading of default Virtual Control Setup has failed! No Touch interface has been added to the screen"));
#endif
				return;
			}
		}
	}
	
	if (State.IsVisible)
	{
		ShowWidget(!State.bBlockInput, !State.bBlockRecognizers);
	}
	else
	{
		HideWidget(State.bBlockRecognizers);
	}

	if (GetDefault<UTouchInterfaceSettings>()->bHideTouchInterfaceWhenGamepadIsConnected)
	{
		//Todo: check if gamepad is already connected
		//FSlateApplication::Get().IsGamepadAttached()
		
#if ENGINE_MAJOR_VERSION > 4 && ENGINE_MINOR_VERSION > 0
		IPlatformInputDeviceMapper::Get().GetOnInputDeviceConnectionChange().AddUObject(this, &UTouchInterfaceSubsystem::HandleOnControllerConnexionChange);
#else
		FCoreDelegates::OnControllerConnectionChange.AddUObject(this, &UTouchInterfaceSubsystem::HandleOnControllerConnexionChange);
#endif
		
#if ENGINE_MAJOR_VERSION < 5
		FCoreDelegates::OnControllerConnectionChange.AddUObject(this, &UTouchInterfaceSubsystem::HandleOnControllerConnexionChange);
#endif

		LocalPlayerInstance->GetPlayerController(GetWorld())->InputComponent->BindTouch(IE_Pressed, this, &UTouchInterfaceSubsystem::HandleOnTouch);
		FInputKeyBinding& Binding = PlayerController->InputComponent->BindKey(FInputChord(EKeys::AnyKey), IE_Pressed, this, &UTouchInterfaceSubsystem::HandleOnAnyKeyPressed);
		Binding.bConsumeInput = false;
	}
}

void UTouchInterfaceSubsystem::CreateTouchInterface(const bool bLoadConfiguration, const int32 UserIndex, const int32 ConfigIndex)
{
	if (TouchInterface.IsValid())
	{
		TouchInterface->RebuildTouchInterface(bLoadConfiguration, UserIndex, ConfigIndex);
	}
	else
	{
		SAssignNew(TouchInterface, STouchInterface, GetLocalPlayer(), CurrentVirtualControlSetup)
		.OnTouchBegan(FOnTouchEvent::CreateUObject(this, &UTouchInterfaceSubsystem::HandleBeganTouchEvent))
		.OnTouchMoved(FOnTouchEvent::CreateUObject(this, &UTouchInterfaceSubsystem::HandleMovedTouchEvent))
		.OnTouchEnded(FOnTouchEvent::CreateUObject(this, &UTouchInterfaceSubsystem::HandleEndedTouchEvent))
		.OnActiveStateChanged(FOnStateChangedEvent::CreateUObject(this, &UTouchInterfaceSubsystem::HandleOnActiveStateChangedEvent))
		.OnVisibilityStateChanged(FOnStateChangedEvent::CreateUObject(this, &UTouchInterfaceSubsystem::HandleOnVisibilityStateChangedEvent))
		.LoadConfig(bLoadConfiguration)
		.UserIndex(UserIndex)
		.ConfigIndex(ConfigIndex);
		
		ULocalPlayer* LocalPlayer = GetLocalPlayer();
		if (LocalPlayer && LocalPlayer->ViewportClient && STouchInterface::ShouldDisplayTouchInterface())
		{
			LocalPlayer->ViewportClient->AddViewportWidgetForPlayer(LocalPlayer, TouchInterface.ToSharedRef(), 0);
		}

		TouchInterface->SetTouchInputComponentList(RegisteredTouchManager);
	}
}

void UTouchInterfaceSubsystem::RemoveFromPlayerScreen(APlayerController* PlayerController)
{
	if (TouchInterface.IsValid())
	{
		ULocalPlayer* LocalPlayer = GetLocalPlayer();
		if (LocalPlayer && LocalPlayer->ViewportClient)
		{
			LocalPlayer->ViewportClient->RemoveViewportWidgetForPlayer(LocalPlayer, TouchInterface.ToSharedRef());
		}
		GetLocalPlayer()->GetPlayerController(GetLocalPlayer()->GetWorld())->FlushPressedKeys();
		//Todo: Something to do with EnhancedInput ?
		TouchInterface = nullptr;
	}
}

void UTouchInterfaceSubsystem::EnableInput()
{
	if (TouchInterface.IsValid()) TouchInterface->BlockInput(false);
}

void UTouchInterfaceSubsystem::DisableInput()
{
	if (TouchInterface.IsValid()) TouchInterface->BlockInput(true);
}

void UTouchInterfaceSubsystem::EnableGesture()
{
	if (TouchInterface.IsValid()) TouchInterface->BlockGesture(false);
}

void UTouchInterfaceSubsystem::DisableGesture()
{
	if (TouchInterface.IsValid()) TouchInterface->BlockGesture(true);
}

void UTouchInterfaceSubsystem::TryRecognizeShape()
{
	if (TouchInterface.IsValid()) TouchInterface->TryRecognizeShapeOnAllManagers();
}

bool UTouchInterfaceSubsystem::SetInputKey(const FName ControlName, FKey ButtonInput, FKey HorizontalInputKey, FKey VerticalInputKey)
{
	if (TouchInterface.IsValid())
	{
		return TouchInterface->ChangeInputKey(ControlName, HorizontalInputKey, VerticalInputKey);
	}

	return false;
}

bool UTouchInterfaceSubsystem::SetInputAction(const FName ControlName, UInputAction* InputAction)
{
	if (TouchInterface)
	{
		return TouchInterface->ChangeInputAction(ControlName, InputAction);
	}

	return false;
}

bool UTouchInterfaceSubsystem::SetButtonAction(const FName ControlName, UInputAction* ButtonAction)
{
	if (TouchInterface)
	{
		return TouchInterface->ChangeInputAction(ControlName, ButtonAction);
	}

	return false;
}

bool UTouchInterfaceSubsystem::SetJoystickAction(const FName ControlName, UInputAction* JoystickAction)
{
	if (TouchInterface)
	{
		return TouchInterface->ChangeInputAction(ControlName, JoystickAction);
	}

	return false;
}

// VISIBILITY

void UTouchInterfaceSubsystem::ShowWidget(bool bEnableInput, bool bEnableGesture)
{
	if (TouchInterface.IsValid()) TouchInterface->SetWidgetVisibility(true, false, !bEnableInput, !bEnableGesture);
}

void UTouchInterfaceSubsystem::HideWidget(bool bDisableGesture)
{
	if (TouchInterface.IsValid()) TouchInterface->SetWidgetVisibility(false, false, true, bDisableGesture);
}

void UTouchInterfaceSubsystem::ShowVirtualControl(const FName ControlName, bool bIncludeChild)
{
	if (TouchInterface.IsValid()) TouchInterface->SetControlVisibility(ControlName, true, bIncludeChild);
}

void UTouchInterfaceSubsystem::HideVirtualControl(const FName ControlName, bool bIncludeChild)
{
	if (TouchInterface.IsValid()) TouchInterface->SetControlVisibility(ControlName, false, bIncludeChild);
}

void UTouchInterfaceSubsystem::HideAllButtons()
{
	if (TouchInterface.IsValid()) return TouchInterface->HideAllControls();
}

void UTouchInterfaceSubsystem::HideAllJoystick()
{
	if (TouchInterface.IsValid()) return TouchInterface->HideAllControls(false, true);
}

void UTouchInterfaceSubsystem::HideAllTouchRegion()
{
	if (TouchInterface.IsValid()) return TouchInterface->HideAllControls(false, false, true);
}

void UTouchInterfaceSubsystem::ShowAllButtons()
{
	if (TouchInterface.IsValid()) return TouchInterface->ShowAllControls();
}

void UTouchInterfaceSubsystem::ShowAllJoystick()
{
	if (TouchInterface.IsValid()) return TouchInterface->ShowAllControls(false, true);
}

void UTouchInterfaceSubsystem::ShowAllTouchRegion()
{
	if (TouchInterface.IsValid()) return TouchInterface->ShowAllControls(false, false, true);
}

// DATA

bool UTouchInterfaceSubsystem::GetVirtualControlDataByRef(const FName ControlName, FVirtualControl& Data)
{
	if (TouchInterface.IsValid())
	{
		return TouchInterface->GetVirtualControlData(ControlName, Data);
	}
	return false;
}

bool UTouchInterfaceSubsystem::GetVirtualControlData(const FName ControlName, FVirtualControl& Data)
{
	if (TouchInterface.IsValid())
	{
		FVirtualControl Copy;
		if (TouchInterface->GetVirtualControlData(ControlName, Copy))
		{
			Data = Copy;
			return true;
		}
	}
	return false;
}

void UTouchInterfaceSubsystem::AddVirtualButton(const FVirtualButtonData ButtonData)
{
	//Todo: Name in function parameter (Name should not be invalid)
	//Todo: Check if name is valid (a control has not same name)
	if (TouchInterface.IsValid())
	{
		TouchInterface->AddControl(ButtonData.GetVirtualControl());
	}

	//Todo: bool SaveInConfiguration
}

void UTouchInterfaceSubsystem::AddVirtualJoystick(const FVirtualJoystickData JoystickData)
{
	if (TouchInterface.IsValid())
	{
		TouchInterface->AddControl(JoystickData.GetVirtualControl());
	}

	//Todo: bool SaveInConfiguration
}

void UTouchInterfaceSubsystem::AddVirtualTouchRegion(const FTouchRegionData TouchRegionData)
{
	if (TouchInterface.IsValid())
	{
		TouchInterface->AddControl(TouchRegionData.GetVirtualControl());
	}

	//Todo: bool SaveInConfiguration
}

void UTouchInterfaceSubsystem::RemoveVirtualControl(const FName ControlName, bool bIncludeChildren)
{
	if(TouchInterface.IsValid()) TouchInterface->RemoveControl(ControlName, bIncludeChildren);
}

void UTouchInterfaceSubsystem::ResetToDefault()
{
	if(TouchInterface.IsValid()) TouchInterface->ResetToDefault();
}

// VISUAL

void UTouchInterfaceSubsystem::SetVirtualControlAppearance(const FName ControlName, TArray<FVisualLayer> Layers)
{
	if (TouchInterface.IsValid())
	{
		TouchInterface->SetVirtualControlVisualLayers(ControlName, Layers);
	}
}

void UTouchInterfaceSubsystem::AddNewLayer(const FName ControlName, FVisualLayer Layer)
{
	//Todo: Make function
#if WITH_EDITOR
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, TEXT("Not yet implemented"));
#endif
}

void UTouchInterfaceSubsystem::RemoveLayer(const FName ControlName, const FName LayerName)
{
	//Todo: Make function
#if WITH_EDITOR
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, TEXT("Not yet implemented"));
#endif
}

bool UTouchInterfaceSubsystem::GetDynamicMaterialInstanceFromLayer(const FName ControlName, const FName LayerName, UMaterialInstanceDynamic* &DMI)
{
	if (TouchInterface.IsValid())
	{
		DMI = TouchInterface->GetLayerDynamicMaterialInstance(ControlName, LayerName, this);
		
		const FString ConvertedNameToString = ControlName.ToString() + LayerName.ToString();

		//Save material instance dynamic in TMap to avoid issues with GC.
		CachedMaterialInstance.Add(FName(*ConvertedNameToString), DMI);

		//Todo: Use it as a real cache system?
	}

	return IsValid(DMI);
}

void UTouchInterfaceSubsystem::ChangeButtonSize(const FName ControlName, const FVector2D NewVisualSize, const FVector2D NewInteractionSize)
{
	if (TouchInterface.IsValid()) TouchInterface->SetControlSize(ControlName, NewVisualSize, FVector2D::ZeroVector, NewInteractionSize);
}

void UTouchInterfaceSubsystem::ChangeJoystickSize(const FName ControlName, const FVector2D NewVisualSize, const FVector2D NewThumbSize, const FVector2D NewInteractionSize)
{
	if (TouchInterface.IsValid()) TouchInterface->SetControlSize(ControlName, NewVisualSize, NewThumbSize, NewInteractionSize);
}

void UTouchInterfaceSubsystem::ChangeTouchRegionSize(const FName ControlName, const FVector2D NewInteractionSize)
{
	if (TouchInterface.IsValid()) TouchInterface->SetControlSize(ControlName, FVector2D::ZeroVector, FVector2D::ZeroVector, NewInteractionSize);
}

bool UTouchInterfaceSubsystem::ChangePosition(const FName ControlName, const FVector2D NewCenter, const bool LandscapePosition)
{	
	if (TouchInterface) return TouchInterface->SetControlPosition(ControlName, NewCenter, LandscapePosition);
	return false;
}

// UTILITIES

EControlType UTouchInterfaceSubsystem::GetTypeOfVirtualControl(const FName ControlName) const
{
	EControlType Type = EControlType::Button;
	if (TouchInterface.IsValid())
	{
		TouchInterface->GetTypeOfControl(ControlName, Type);
	}
	return Type;
}

bool UTouchInterfaceSubsystem::GetAllControlsName(TArray<FName>& Names)
{
	if(!TouchInterface.IsValid()) return false;
	Names = TouchInterface->GetAllControlNames();

#if ENGINE_MAJOR_VERSION > 4
	return !Names.IsEmpty();
#else
	return Names.Num() > 0;
#endif
}

bool UTouchInterfaceSubsystem::GetLayerNames(const FName ControlName, TArray<FName>& Names)
{
	if(!TouchInterface.IsValid()) return false;
	Names = TouchInterface->GetLayerNames(ControlName);
	
#if ENGINE_MAJOR_VERSION > 4
    return !Names.IsEmpty();
#else
    return Names.Num() > 0;
#endif
}

bool UTouchInterfaceSubsystem::IsInterfaceActive() const
{
	if(!TouchInterface.IsValid()) return false;
	return TouchInterface->IsActive();
}

bool UTouchInterfaceSubsystem::ContainName(const FName ControlName)
{
	if (TouchInterface.IsValid()) return TouchInterface->ContainName(ControlName);
	return false;
}

bool UTouchInterfaceSubsystem::IsVisible() const
{
	if (TouchInterface.IsValid()) return TouchInterface->IsVisible();
	return false;
}

void UTouchInterfaceSubsystem::SetScaleMultiplier(const float NewScaleMultiplier)
{
	if (TouchInterface.IsValid()) TouchInterface->SetScaleMultiplier(NewScaleMultiplier);
}

bool UTouchInterfaceSubsystem::LocalToNormalized(const FVector2D LocalPosition, FVector2D& NormalizedPosition)
{
	if (TouchInterface.IsValid())
	{
		NormalizedPosition = TouchInterface->LocalToNormalized(LocalPosition);
		return true;
	}
	
	NormalizedPosition = FVector2D::ZeroVector;
	return false;
}

bool UTouchInterfaceSubsystem::NormalizedToLocal(const FVector2D NormalizedPosition, FVector2D& LocalPosition)
{
	if (TouchInterface.IsValid())
	{
		LocalPosition = TouchInterface->NormalizedToLocal(NormalizedPosition);
		return true;
	}
	
	LocalPosition = FVector2D::ZeroVector;
	return false;
}

bool UTouchInterfaceSubsystem::VirtualControlConfigurationExist(const int32 UserIndex)
{
	if (!TouchInterface.IsValid()) return false;
	return TouchInterface->ConfigurationFileExist(UserIndex);
}

bool UTouchInterfaceSubsystem::ConfigurationExist(const int32 UserIndex, const int32 ConfigIndex)
{
	if (!TouchInterface.IsValid()) return false;
	return TouchInterface->ConfigurationExist(UserIndex, ConfigIndex);
}

bool UTouchInterfaceSubsystem::SaveConfiguration(const int32 UserIndex, const int32 ConfigIndex)
{
	if (!TouchInterface.IsValid()) return false;
	return TouchInterface->SaveConfiguration(UserIndex, ConfigIndex);
}

bool UTouchInterfaceSubsystem::LoadConfiguration(const int32 UserIndex, const int32 ConfigIndex)
{
	if (!TouchInterface.IsValid()) return false;
	return TouchInterface->LoadConfiguration(UserIndex, ConfigIndex);
}

bool UTouchInterfaceSubsystem::DeleteConfiguration(const int32 UserIndex, const int32 ConfigIndex)
{
	if (!TouchInterface.IsValid()) return false;
	return TouchInterface->DeleteConfiguration(UserIndex, ConfigIndex);
}

bool UTouchInterfaceSubsystem::DeleteConfigurationSlot(const int32 UserIndex)
{
	if (!TouchInterface.IsValid()) return false;
	return TouchInterface->DeleteConfigurationSlot(UserIndex);
}

// EVENTS

void UTouchInterfaceSubsystem::HandleBeganTouchEvent(const int32 FingerIndex, const FVector2D AbsolutePosition, const FVector2D LocalPosition)
{
	bIsTouchInput = true;
	OnTouchBegan.Broadcast(FingerIndex, AbsolutePosition, LocalPosition);
}

void UTouchInterfaceSubsystem::HandleMovedTouchEvent(const int32 FingerIndex, const FVector2D AbsolutePosition, const FVector2D LocalPosition)
{
	OnTouchMoved.Broadcast(FingerIndex, AbsolutePosition, LocalPosition);
}

void UTouchInterfaceSubsystem::HandleEndedTouchEvent(const int32 FingerIndex, const FVector2D AbsolutePosition, const FVector2D LocalPosition)
{
	if (FingerIndex == 0)
	{
		bIsTouchInput = false;
	}
	
	OnTouchEnded.Broadcast(FingerIndex, AbsolutePosition, LocalPosition);
}

void UTouchInterfaceSubsystem::HandleOnActiveStateChangedEvent(const bool IsActive)
{
	OnActiveStateChanged.Broadcast(IsActive);
}

void UTouchInterfaceSubsystem::HandleOnVisibilityStateChangedEvent(const bool IsVisible)
{
	OnVisibilityStateChanged.Broadcast(IsVisible);
}

#if ENGINE_MAJOR_VERSION > 4
#if ENGINE_MINOR_VERSION > 0
void UTouchInterfaceSubsystem::HandleOnControllerConnexionChange(EInputDeviceConnectionState ConnectionState, FPlatformUserId UserId, FInputDeviceId DeviceId)
{
	if (!TouchInterface.IsValid()) return;
	
	UE_LOG(LogTouchInterfaceSubsystem, Log, TEXT("Input Device Connection : UserID : %d | DeviceID : %d"), UserId.GetInternalId(), DeviceId.GetId());

	switch (ConnectionState)
	{
	case EInputDeviceConnectionState::Connected:
		UE_LOG(LogTouchInterfaceSubsystem, Log, TEXT("Input Device connected"));
		TouchInterface->SetWidgetVisibility(false, false, true, true);
		break;
	case EInputDeviceConnectionState::Disconnected:
		UE_LOG(LogTouchInterfaceSubsystem, Log, TEXT("Input Device disconnected"));
		TouchInterface->SetWidgetVisibility(true, false, false, false);
		break;
	case EInputDeviceConnectionState::Invalid:
		UE_LOG(LogTouchInterfaceSubsystem, Log, TEXT("Invalid Input Device"));
		break;
	case EInputDeviceConnectionState::Unknown:
		UE_LOG(LogTouchInterfaceSubsystem, Log, TEXT("Unknown Input Device"));
		break;
	}
}

#else
void UTouchInterfaceSubsystem::HandleOnControllerConnexionChange(bool bIsConnected, FPlatformUserId UserId, int Other)
{
	if (!TouchInterface.IsValid()) return;

	GLog->Log("Input Device Connection : UserID : " + FString::FromInt(UserId.GetInternalId()) + " DeviceID : " + FString::FromInt(Other));
	
	if (bIsConnected)
	{
		TouchInterface->SetWidgetVisibility(false, false, true, true);
	}
	else
	{
		TouchInterface->SetWidgetVisibility(true, false, false, false);
	}
}
#endif

#else
void UTouchInterfaceSubsystem::HandleOnControllerConnexionChange(bool bIsConnected, int UserId, int Other)
{
	if (!TouchInterface.IsValid()) return;

	GLog->Log("Input Device Connection : UserID : " + FString::FromInt(UserId) + " DeviceID : " + FString::FromInt(Other));
	
	if (bIsConnected)
	{
		TouchInterface->SetWidgetVisibility(false, false, true, true);
	}
	else
	{
		TouchInterface->SetWidgetVisibility(true, false, false, false);
	}
}
#endif

void UTouchInterfaceSubsystem::HandleOnAnyKeyPressed()
{
	if (!TouchInterface.IsValid()) return;
	
	if (!bIsTouchInput)
	{
		TouchInterface->SetWidgetVisibility(false, false, true, true);
	}
}

void UTouchInterfaceSubsystem::HandleOnTouch(ETouchIndex::Type Index, FVector Coordinate)
{
	if (!TouchInterface.IsValid()) return;
	
	TouchInterface->SetWidgetVisibility(true, false, false, false);
}