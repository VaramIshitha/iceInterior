// Copyright Lost in Game Studio. All Rights Reserved.


#include "Components/TouchInterfaceListener.h"

#include "TouchInterfaceSubsystem.h"
#include "Kismet/GameplayStatics.h"
//Begin Needed for mobile
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
//End Needed for mobile

UTouchInterfaceListener::UTouchInterfaceListener()
{
	PrimaryComponentTick.bCanEverTick = true;
	bHasStarted = false;
}

void UTouchInterfaceListener::BeginPlay()
{
	Super::BeginPlay();

	Settings = GetDefault<UTouchInterfaceSettings>();

	if (bRegisterOnStart && AutoReceiveInput != EAutoReceiveInput::Disabled)
	{
		const int32 PlayerIndex = int32(AutoReceiveInput.GetValue()) - 1;

		const APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), PlayerIndex);
		if (PC)
		{
			check(PC->GetLocalPlayer());
			
			UTouchInterfaceSubsystem* Subsystem = PC->GetLocalPlayer()->GetSubsystem<UTouchInterfaceSubsystem>();
			if (Subsystem)
			{
				if (!Subsystem->RegisterTouchManagerComponent(this))
				{
					//Todo: return Fail reason
					OnRegisterFail.Broadcast();
				}				
			}
		}
	}
}

bool UTouchInterfaceListener::OnTouchStarted(const FGeometry& Geometry, const FPointerEvent& Events)
{
	bHasStarted = true;
	return bBlockLowerPriority;
}

void UTouchInterfaceListener::OnTouchMoved(const FGeometry& Geometry, const FPointerEvent& Events)
{
	
}

void UTouchInterfaceListener::OnTouchEnded(const FGeometry& Geometry, const FPointerEvent& Events)
{
	bHasStarted = false;
}

void UTouchInterfaceListener::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UTouchInterfaceListener::HandleOnOrientationChanged()
{
	bHasStarted = false;
}

void UTouchInterfaceListener::Reset()
{
	bHasStarted = false;
}

ULocalPlayer* UTouchInterfaceListener::GetLocalPlayer()
{
	if (AutoReceiveInput != EAutoReceiveInput::Disabled)
	{
		const int32 PlayerIndex = int32(AutoReceiveInput.GetValue()) - 1;

		const APlayerController* PC = UGameplayStatics::GetPlayerController(this, PlayerIndex);
		if (PC)
		{
			return PC->GetLocalPlayer();
		}
	}

	return nullptr;
}

UTouchInterfaceSubsystem* UTouchInterfaceListener::GetSubsystem()
{
	if (bRegisterOnStart && AutoReceiveInput != EAutoReceiveInput::Disabled)
	{
		const int32 PlayerIndex = int32(AutoReceiveInput.GetValue()) - 1;

		const APlayerController* PC = UGameplayStatics::GetPlayerController(this, PlayerIndex);
		if (PC)
		{
			return PC->GetLocalPlayer()->GetSubsystem<UTouchInterfaceSubsystem>();
		}
	}
	
	return nullptr;
}

