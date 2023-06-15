// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MotionManager.generated.h"

UENUM(BlueprintType)
enum class EShake : uint8
{
	Horizontal,
	Vertical,
	BackAndForth
};

UENUM(BlueprintType)
enum class ETapType : uint8
{
	Left,
	Right,
	//OnScreen
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnSimpleMotionDelegate, int32, PlayerIndex, FVector, Acceleration, FVector, Gravity, FVector, RotationRate, FVector, Tilt);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShakeMotionDelegate, int32, PlayerIndex, EShake, ShakeType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTapMotionDelegate, int32, PlayerIndex, ETapType, Type /** Todo: Add value (force) */);

/**
 * A component that intercept IMU data and process it to easy configurable events
 */
UCLASS(ClassGroup=(TouchInterfaceDesigner), meta=(BlueprintSpawnableComponent))
class TOUCHINTERFACE_API UMotionManager : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UMotionManager();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Calibrate tilt
	UFUNCTION(Category="Motion Manager", BlueprintCallable)
	virtual void Calibrate();
	
	// Tries to counteract the parasitic rotation of the sensor
	UFUNCTION(Category="Motion Manager", BlueprintCallable)
	virtual void StabilizeGyroscope();

private:
	void TapDetector();
	void ShakeDetector();
	
public:
	UPROPERTY(Category="Input", EditAnywhere, BlueprintReadOnly)
	TEnumAsByte<EAutoReceiveInput::Type> AutoReceiveInput;

	UPROPERTY(Category="Tap Motion", EditAnywhere, BlueprintReadOnly)
	float MinDeltaToDetectTap;

	UPROPERTY(Category="Tap Motion", EditAnywhere, BlueprintReadOnly, meta=(ClampMin=0.25f))
	float DelayBetweenTapDetection;
	
	//Todo: Control What data is process
	//Todo: Bitflag to enable type of event

	// EVENT

	UPROPERTY(Category="Motion Manager", BlueprintAssignable)
	FOnSimpleMotionDelegate OnIMUInputState;

	UPROPERTY(Category="Motion Manager", BlueprintAssignable)
	FOnShakeMotionDelegate OnShakeStart;

	UPROPERTY(Category="Motion Manager", BlueprintAssignable)
	FOnShakeMotionDelegate OnShakeOnGoing;

	UPROPERTY(Category="Motion Manager", BlueprintAssignable)
	FOnShakeMotionDelegate OnShakeEnd;

	UPROPERTY(Category="Motion Manager", BlueprintAssignable)
	FOnTapMotionDelegate OnTapOnDevice;
	
private:
	UPROPERTY()
	APlayerController* PlayerController;

	FVector Tilt;
	FVector Gravity;
	FVector RotationRate;
	FVector Acceleration;

	uint8 bCanDetectTap:1;
	uint8 bRightTapDetected:1;
	uint8 bLeftTapDetected:1;

	FTimerHandle TapDetectorTimerHandle;
};
