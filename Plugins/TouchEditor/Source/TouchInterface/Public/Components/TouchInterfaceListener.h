// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

//#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TouchInterfaceListener.generated.h"

class UTouchInterfaceSettings;
class ULocalPlayer;
class UTouchInterfaceSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRegisterFailSignature);

UCLASS(Abstract)
class TOUCHINTERFACE_API UTouchInterfaceListener : public UActorComponent
{
	GENERATED_BODY()

public:
	UTouchInterfaceListener();

protected:
	virtual void BeginPlay() override;

public:
	// Called by STouchInterface when user begin touch screen
	virtual bool OnTouchStarted(const FGeometry& Geometry, const FPointerEvent& Events);

	// Called by STouchInterface when user move finger on screen
	virtual void OnTouchMoved(const FGeometry& Geometry, const FPointerEvent& Events);

	// Called by STouchInterface when user leave finger from screen
	virtual void OnTouchEnded(const FGeometry& Geometry, const FPointerEvent& Events);
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**  */
	bool HasStarted() const { return bHasStarted; }

	// Called by STouchInterface when device orientation changed
	virtual void HandleOnOrientationChanged();

	// Called by STouchInterface to reset component
	virtual void Reset();

protected:
	ULocalPlayer* GetLocalPlayer();
	UTouchInterfaceSubsystem* GetSubsystem();
	const UTouchInterfaceSettings* GetSettings() const { return Settings; }

public:	
	/** Define the priority of inputs for this component.
	 * STouchInterface will send the inputs to the components with the highest priority level
	 */
	UPROPERTY(Category="Input", EditAnywhere, BlueprintReadWrite, meta=(ClampMin=0, UIMin=0))
	int32 Priority;

	// Event that is called if the registration fails. It allows you to manage this failure
	UPROPERTY(Category="Gesture Manager|Event", BlueprintAssignable)
	FOnRegisterFailSignature OnRegisterFail;

protected:
	/** If true, the component try to register on begin play but it can fail because Virtual Control Manager isn't ready
	 * For online multiplayer, use Touch Interface subsystem and register this component manually
	 */
	UPROPERTY(Category="Registration", EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	bool bRegisterOnStart;

	/** Automatically registers this actor to receive input from a player. */
	UPROPERTY(Category="Registration", EditAnywhere, BlueprintReadOnly, meta=(EditCondition="bRegisterOnStart", EditConditionHides))
	TEnumAsByte<EAutoReceiveInput::Type> AutoReceiveInput;

	// If this component has a priority level higher than 0, you can tell STouchInterface not to send inputs to components with a lower priority level.
	UPROPERTY(Category="Input", EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess="true"), meta=(EditCondition="Priority > 0"))
	bool bBlockLowerPriority;

	// Whether or not, debug information is displayed (log, draw...)
	UPROPERTY(Category="Debug", EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	bool bShowDebug;

private:
	uint8 bHasStarted:1;

	UPROPERTY()
	const UTouchInterfaceSettings* Settings;
};
