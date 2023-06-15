// Copyright Lost in Game Studio. All Rights Reserved.


#include "Components/GestureManager.h"


UGestureManager::UGestureManager()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UGestureManager::BeginPlay()
{
	Super::BeginPlay();
	
	if (bOverrideGestureConfig)
	{
		Config = GesturesConfig;
	}
	else
	{
		Config = GetDefault<UTouchInterfaceSettings>()->DefaultGesturesConfig;
	}
	
	//Todo: Check if lower priority should be blocked (priority is > 0)
}

//Todo: if multi touch detected, send a special event that provide average position of fingers
bool UGestureManager::OnTouchStarted(const FGeometry& Geometry, const FPointerEvent& Events)
{
	const FVector2D StartPosition = Geometry.AbsoluteToLocal(Events.GetScreenSpacePosition());

	if (bUseNewAlgorithm)
	{
		CurrentTouchMode = ETouchMode::SingleTouch;
	
		int32 NumFingerDetected = 0;

		//Todo: If two fingers are close to each other, switch to two fingers mode. Otherwise keep solo mode ?
		//Todo: Make array with 10 entry, check pointer index on start
		
		bool bAddNew = true;
		for (FFingerData& Data : GestureData)
		{
			if (Data.AcceptPointerIndex(Events.GetPointerIndex()))
			{
				Data.BeginPosition = StartPosition;
				Data.LastPosition = StartPosition;
				bAddNew = false;
				NumFingerDetected++;
			}
		}

		if (bAddNew)
		{
			GestureData.Add(FFingerData(Events.GetPointerIndex(), StartPosition, CurrentTime));
			NumFingerDetected++;
		}

		if (NumFingerDetected > 1)
		{
			CurrentTouchMode = ETouchMode::DoubleTouch;

			const float Closeness = FVector2D::Distance(GestureData[0].LastPosition, GestureData[1].BeginPosition);
			bFingersCloseTogetherOnStart = FMath::Abs(Closeness) <= Config.TwoFingerSwipeClosenessThreshold;

			StartDirection = (GestureData[0].BeginPosition - StartPosition).GetSafeNormal();
			StartOffsetBetweenFinger = (GestureData[0].LastPosition - StartPosition).Size();
			LastZoomOffset = StartOffsetBetweenFinger;
			LastRotationOffset = FMath::RadiansToDegrees(FMath::Atan2(StartDirection.Y, StartDirection.X));
		}
		else if (NumFingerDetected > 2)
		{
			CurrentTouchMode = ETouchMode::MultiTouch;
		}

		return UTouchInterfaceListener::OnTouchStarted(Geometry, Events);
	}

	
	if (Events.GetPointerIndex() == 0)
	{
		bFinger0_Detected = true;
		Finger0_BeginPosition = Geometry.AbsoluteToLocal(Events.GetScreenSpacePosition());
		Finger0_LastPosition = Geometry.AbsoluteToLocal(Events.GetScreenSpacePosition());
		Finger0_BeginAbsolutePosition = Events.GetScreenSpacePosition();

		//Todo: Check double tap on release!
		if ((Config.Gestures & (uint8)EGestureEvents::DoubleTap) != (uint8)EGestureEvents::None)
		{
			if (Finger0_OffsetFromBegin.Size() <= 1.0f && (CurrentTime - LastTapTime <= Config.DoubleTapTime) && bFirstTouchDetected)
			{
				bDoubleTapDetected = true;
				bFirstTouchDetected = false;
				LastTapTime = 0.0f;

				OnDoubleTap.Broadcast(Events.GetPointerIndex(), Events.GetScreenSpacePosition(), StartPosition);
			}
			else
			{
				bFirstTouchDetected = true;
				LastTapTime = CurrentTime;
			}
		}
	}
	else if (Events.GetPointerIndex() == 1)
	{
		bFinger1_Detected = true;
		bFinger0_Detected = false;
		Finger1_BeginPosition = Geometry.AbsoluteToLocal(Events.GetScreenSpacePosition());
		Finger1_LastPosition = Geometry.AbsoluteToLocal(Events.GetScreenSpacePosition());
		
		const float Closeness = FVector2D::Distance(Finger0_LastPosition, Finger1_BeginPosition);
		bFingersCloseTogetherOnStart = FMath::Abs(Closeness) <= Config.TwoFingerSwipeClosenessThreshold;
		
		StartDirection = (Finger0_BeginPosition - Geometry.AbsoluteToLocal(Events.GetScreenSpacePosition())).GetSafeNormal();
		StartOffsetBetweenFinger = (Finger0_LastPosition - Geometry.AbsoluteToLocal(Events.GetScreenSpacePosition())).Size();
		LastZoomOffset = StartOffsetBetweenFinger;
		LastRotationOffset = FMath::RadiansToDegrees(FMath::Atan2(StartDirection.Y, StartDirection.X));
	}

	return UTouchInterfaceListener::OnTouchStarted(Geometry, Events);
}

void UGestureManager::OnTouchMoved(const FGeometry& Geometry, const FPointerEvent& Events)
{
	UTouchInterfaceListener::OnTouchMoved(Geometry, Events);
	
	if (bUseNewAlgorithm)
	{
		return;
	}
	
	if (bFinger0_Detected)
	{
		if (Events.GetPointerIndex() == 0)
		{
			const FVector2D CurrentPosition = Geometry.AbsoluteToLocal(Events.GetScreenSpacePosition());
			const FVector2D Offset = CurrentPosition - Finger0_BeginPosition;
			Finger0_OffsetFromBegin = Offset;
			
			if (bLongPressDetected)
			{
				//Todo: enable drag only when offset is > LongPressAllowedMovement ?
				bDragDetected = true;

				if (Config.Gestures & (uint8)EGestureEvents::Drag)
				{
					//Todo: I don't like it, bad user experience. Make dedicated delegate signature ?
					OnDrag.Broadcast(Events.GetPointerIndex(), Events.GetCursorDelta(), Events.GetCursorDelta());
				}
			}
			else
			{
				// AbsolutePosition = Delta from start, LocalPosition = Delta from last movement
				OnMoveDelta.Broadcast(0, Finger0_OffsetFromBegin, Finger0_LastPosition - CurrentPosition);
			}
			
			Finger0_LastPosition = Geometry.AbsoluteToLocal(Events.GetScreenSpacePosition());
		}
	}
	else if(bFinger1_Detected)
	{			
		if (Events.GetPointerIndex() == 0)
		{
			Finger0_LastPosition = Geometry.AbsoluteToLocal(Events.GetScreenSpacePosition());
			const FVector2D Offset = Finger0_LastPosition - Finger0_BeginPosition;
			Finger0_OffsetFromBegin = Offset;
		}
		
		if (Events.GetPointerIndex() == 1)
		{
			Finger1_LastPosition = Geometry.AbsoluteToLocal(Events.GetScreenSpacePosition());
		}

		//Todo: If the fingers move closer or further away then disable the two-finger swipe
		if (!bFingersCloseTogetherOnStart)
		{
			if (Config.Gestures & (uint8)EGestureEvents::Zoom)
			{
				const FVector2D CurrentOffset = Finger1_LastPosition - Finger0_LastPosition;

				const float Delta = CurrentOffset.Size() - LastZoomOffset;

				TotalZoomOffset += Delta;
				LastZoomOffset = CurrentOffset.Size();

				const float OffsetFromStart = (CurrentOffset - StartOffsetBetweenFinger).Size();
			
				if (FMath::Abs(Delta) >= Config.MinimumDeltaToSendEvent)
				{
					//Todo: float = delta, use FVector2D -> X = current offset between finger (abs value) | Y = offset from start
					OnZoom.Broadcast(0, Delta);
					//OnGesture.Execute(EGestureEvents::Zoom, 0, Delta, FVector2D(CurrentOffset.Size(), OffsetFromStart));
				}
			}

			if (Config.Gestures & (uint16)EGestureEvents::Rotate)
			{
				const FVector2D CurrentDirection = (Finger0_LastPosition - Finger1_LastPosition).GetSafeNormal();
			
				const float CurrentRotation = FMath::RadiansToDegrees(FMath::Atan2(CurrentDirection.Y, CurrentDirection.X));
				const float Delta = CurrentRotation - LastRotationOffset;
				//Todo: Clamp to 360 degre ?
				TotalRotationAngle += Delta;
				LastRotationOffset = CurrentRotation;
			
				if (FMath::Abs(Delta) >= Config.MinimumDeltaToSendEvent)
				{
					OnRotate.Broadcast(0, Delta);
				}
			}

			if (Config.bSendDragEventWhenZoomOrRotateGestureDetected)
			{
				if (Config.Gestures & (uint8)EGestureEvents::Drag)
				{
					OnDrag.Broadcast(Events.GetPointerIndex(), Events.GetCursorDelta(), Events.GetCursorDelta());
				}
			}
		}
	}
}

void UGestureManager::OnTouchEnded(const FGeometry& Geometry, const FPointerEvent& Events)
{
	if (bUseNewAlgorithm)
	{
		return;
	}
	
	if (Events.GetPointerIndex() == 0)
	{
		Finger0_LastPosition = Geometry.AbsoluteToLocal(Events.GetScreenSpacePosition());
		if (Config.Gestures & (uint8)EGestureEvents::Tap)
		{
			if (Finger0_OffsetFromBegin.Size() <= Config.LongPressAllowedMovement && PressDuration > 0.15f && !bLongPressDetected && !bDoubleTapDetected)
			{
				OnTap.Broadcast(Events.GetPointerIndex(), Events.GetScreenSpacePosition(), Geometry.AbsoluteToLocal(Events.GetScreenSpacePosition()));
			}
		}

		if (Config.Gestures & (uint8)EGestureEvents::Swipe && !bFinger1_Detected && !bLongPressDetected && !bDragDetected && !bFingersCloseTogetherOnStart)
		{
			const float Velocity = Finger0_OffsetFromBegin.Size() / PressDuration;
			
			if (Finger0_OffsetFromBegin.Size() >= Config.SwipeThreshold && Velocity >= Config.SwipeVelocityThreshold)
			{
				const FVector2D Direction = Finger0_OffsetFromBegin.GetSafeNormal();
				const float TestX = FVector2D::DotProduct(FVector2D(1.0f, 0.0f), Direction);
				const float TestY = FVector2D::DotProduct(FVector2D(0.0f, 1.0f), Direction);

				ESwipeDirection SwipeDirection = ESwipeDirection::Right;
				
				if (TestX >= 0.98f)
				{
					//Swipe Right
					SwipeDirection = ESwipeDirection::Right;
				}
				else if (TestX <= -0.98f)
				{
					//Swipe Left
					SwipeDirection = ESwipeDirection::Left;
				}
				else if (TestY >= 0.98f)
				{
					//Swipe Down
					SwipeDirection = ESwipeDirection::Down;
				}
				else if (TestY <= -0.98f)
				{
					//Swipe Up
					SwipeDirection = ESwipeDirection::Up;
				}
				else
				{
					//Swipe in other direction (not yet supported)
#if WITH_EDITOR
					GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, TEXT("Swipe in this direction is not yet supported"));
#endif
				}
				
				FSwipeData SwipeData;
				SwipeData.Direction = SwipeDirection;
				SwipeData.BeginPosition = Finger0_BeginPosition;
				SwipeData.EndPosition = Finger0_LastPosition;
				SwipeData.Velocity = Velocity;
				SwipeData.Duration = PressDuration;
				SwipeData.Offset = Finger0_OffsetFromBegin;
				
				OnSwipe.Broadcast(Events.GetPointerIndex(), SwipeData);
			}
		}

		if (bDragDetected)
		{
			OnDragEnd.Broadcast(0, Events.GetScreenSpacePosition(), Geometry.AbsoluteToLocal(Events.GetScreenSpacePosition()));
		}
	}
	//Todo: Two finger swipe
	/*else if(Events.GetPointerIndex() == 1)
	{
		if (GesturesConfig.Gestures & (uint8)EGestureEvents::TwoFingerSwipe && bFingersCloseTogetherOnStart && !bLongPressDetected && !bDragDetected)
		{
			const float Closeness = FVector2D::Distance(Finger0_LastPosition, Geometry.AbsoluteToLocal(Events.GetScreenSpacePosition()));
			
			if (FMath::Abs(Closeness) <= GesturesConfig.TwoFingerSwipeClosenessThreshold)
			{
				const float Velocity = Finger0_OffsetFromBegin.Size() / PressDuration;
			
				if (Finger0_OffsetFromBegin.Size() >= GesturesConfig.SwipeThreshold && Velocity >= GesturesConfig.SwipeVelocityThreshold)
				{
					const FVector2D Direction = Finger0_OffsetFromBegin.GetSafeNormal();
					const float TestX = FVector2D::DotProduct(FVector2D(1.0f, 0.0f), Direction);
					const float TestY = FVector2D::DotProduct(FVector2D(0.0f, 1.0f), Direction);
				
					if (TestX >= 0.98f)
					{
						//Swipe Right
						//Use index to define direction. Up = 0, Right = 1, Down = 2, Left = 3
						OnGesture.Execute(EGestureEvents::TwoFingerSwipe, 1, 0.0f, FVector2D::ZeroVector);
					}
					else if (TestX <= -0.98f)
					{
						//Swipe Left
						OnGesture.Execute(EGestureEvents::TwoFingerSwipe, 3, 0.0f, FVector2D::ZeroVector);
					}
					else if (TestY >= 0.98f)
					{
						//Swipe Down
						OnGesture.Execute(EGestureEvents::TwoFingerSwipe, 2, 0.0f, FVector2D::ZeroVector);
					}
					else if (TestY <= -0.98f)
					{
						//Swipe Up
						OnGesture.Execute(EGestureEvents::TwoFingerSwipe, 0, 0.0f, FVector2D::ZeroVector);
					}
					else
					{
						//Swipe in other direction (not yet supported)
#if WITH_EDITOR
						GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, TEXT("Swipe in this direction is not yet supported"));
#endif
					}
				}
			}
		}
	}*/

	ResetGesture();
	//GestureData.Reset();

	UTouchInterfaceListener::OnTouchEnded(Geometry, Events);
}

void UGestureManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	UTouchInterfaceListener::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CurrentTime += DeltaTime;
	
	if (bFinger0_Detected && !bFinger1_Detected)
	{
		PressDuration += DeltaTime;
		if (PressDuration >= Config.LongPressDuration && !bLongPressDetected)
		{
			//Todo: Another bool should be created like LongPressChecked that is set to true when execution enter here to avoid continuous check
			if (Config.Gestures & (uint8)EGestureEvents::LongPress && Finger0_OffsetFromBegin.Size() <= Config.LongPressAllowedMovement)
			{
				OnLongPress.Broadcast(0, Finger0_BeginAbsolutePosition, Finger0_BeginPosition);
				bLongPressDetected = true;
			}
		}
	}
	if (bFinger1_Detected)
	{
		PressDuration += DeltaTime;
	}
}

void UGestureManager::HandleOnOrientationChanged()
{
	UTouchInterfaceListener::HandleOnOrientationChanged();
	
	//Call release
	
	//and reset
	ResetGesture();
}

void UGestureManager::ResetGesture()
{
	Finger0_BeginPosition = FVector2D::ZeroVector;
	Finger0_OffsetFromBegin = FVector2D::ZeroVector;
	Finger0_LastPosition = FVector2D::ZeroVector;
	Finger0_BeginAbsolutePosition = FVector2D::ZeroVector;
	
	Finger1_BeginPosition = FVector2D::ZeroVector;
	Finger1_OffsetFromBegin = FVector2D::ZeroVector;
	Finger1_LastPosition = FVector2D::ZeroVector;
	

	//LastTapTime = 0.0f;
	PressDuration = 0.0f;
	Finger0_TouchStartTime = 0.0f;
	//CurrentTime = 0.0f;

	StartOffsetBetweenFinger = 0.0f;
	LastZoomOffset = 0.0f;
	LastRotationOffset = 0.0f;

	TotalZoomOffset = 0.0f;
	TotalRotationAngle = 0.0f;

	//bFirstTouchDetected = false;
	bFinger0_Detected = false;
	bFinger1_Detected = false;

	bLongPressDetected = false;
	bDragDetected = false;
	bDoubleTapDetected = false;
}
