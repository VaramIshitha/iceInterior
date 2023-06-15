// Copyright Lost in Game Studio. All Rights Reserved.

#include "MotionManager.h"
#include "Kismet/GameplayStatics.h"

//Needed for mobile
#include "TimerManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
//Needed for mobile

DEFINE_LOG_CATEGORY_STATIC(LogMotionManager, All, All);

UMotionManager::UMotionManager()
{
	PrimaryComponentTick.bCanEverTick = true;

	MinDeltaToDetectTap = 2.0f;
	bCanDetectTap = true;
	bLeftTapDetected = false;
	bRightTapDetected = false;
	DelayBetweenTapDetection = 0.25f;
}

void UMotionManager::BeginPlay()
{
	Super::BeginPlay();

	PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), AutoReceiveInput);

	if (!PlayerController)
	{
		UE_LOG(LogMotionManager, Error, TEXT("PlayerController not valid!"));
	}
}

void UMotionManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (PlayerController)
	{
		PlayerController->GetInputMotionState(Tilt, RotationRate, Gravity, Acceleration);
	}

	OnIMUInputState.Broadcast(AutoReceiveInput, Acceleration, Gravity, RotationRate, Tilt);

	TapDetector();
	ShakeDetector();

	//Todo: Camera Movement : 3D Effect, FPS view
}

void UMotionManager::Calibrate()
{
	GEngine->Exec(nullptr, TEXT("CALIBRATEMOTION"));
}

void UMotionManager::StabilizeGyroscope()
{
	//Todo: Tries to counteract the parasitic rotation of the sensor
	//Todo: Message warning : Keep the device on a stable and immovable surface
	//When detect nothing (acceleration) during 2 seconds, launch the stabilization
	//During 5 seconds, get gyroscope data and try to determine drift and correct
}

void UMotionManager::TapDetector()
{
	//Todo: Detect tap on each coin of device

	if (bRightTapDetected)
	{
		if (Acceleration.Y >= 0.0f)
		{
			bRightTapDetected = false;
			return;
		}
	}

	if (bLeftTapDetected)
	{
		if (Acceleration.Y <= 0.0f)
		{
			bLeftTapDetected = false;
			return;
		}
	}

	if (!bCanDetectTap)
	{
		return;
	}
	
	if (Acceleration.Y >= MinDeltaToDetectTap)
	{
		bLeftTapDetected = true;
		bCanDetectTap = false;
		OnTapOnDevice.Broadcast(AutoReceiveInput, ETapType::Left);
		GetWorld()->GetTimerManager().SetTimer(TapDetectorTimerHandle, FTimerDelegate::CreateLambda([this](){ bCanDetectTap = true; }), DelayBetweenTapDetection, false);
		return;
	}

	if (Acceleration.Y <= -MinDeltaToDetectTap)
	{
		bRightTapDetected = true;
		bCanDetectTap = false;
		OnTapOnDevice.Broadcast(AutoReceiveInput, ETapType::Right);
		GetWorld()->GetTimerManager().SetTimer(TapDetectorTimerHandle, FTimerDelegate::CreateLambda([this](){ bCanDetectTap = true; }), DelayBetweenTapDetection, false);
		return;
	}
}

void UMotionManager::ShakeDetector()
{
	
}

