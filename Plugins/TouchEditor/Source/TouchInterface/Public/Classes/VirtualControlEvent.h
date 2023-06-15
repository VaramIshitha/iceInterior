// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
//Needed for mobile
#include "Layout/Geometry.h"
#include "Input/Events.h"
//End
#include "VirtualControlSetup.h"
#include "VirtualControlEvent.generated.h"

class SVirtualControl;

/**
 * 
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class TOUCHINTERFACE_API UVirtualControlEvent : public UObject
{
	GENERATED_BODY()

public:
	UVirtualControlEvent();
	
	void SetButtonName(const FName Name) { LinkedButtonName = Name; }
	void SetPlayerIndex(const int32 Index) { PlayerIndex = Index; }
	
	UFUNCTION(Category="Virtual Control Event", BlueprintNativeEvent, meta=(DeprecatedFunction))
	void OnTouchBegan(const FGeometry& Geometry, const FPointerEvent& PointerEvent);

	UFUNCTION(Category="Virtual Control Event", BlueprintNativeEvent, meta=(DeprecatedFunction))
	void OnTouchMoved(const FGeometry& Geometry, const FPointerEvent& PointerEvent);

	UFUNCTION(Category="Virtual Control Event", BlueprintNativeEvent, meta=(DeprecatedFunction))
	void OnTouchEnded(const FGeometry& Geometry, const FPointerEvent& PointerEvent);

	
	// Called when user's finger begin touch virtual control
	UFUNCTION(Category="Virtual Control Event", BlueprintNativeEvent)
	void OnTouchBegin(const FGeometry& Geometry, const FPointerEvent& PointerEvent, const float CurrentTime);

	// Called when user's finger moves inside the virtual control
	UFUNCTION(Category="Virtual Control Event", BlueprintNativeEvent)
	void OnTouchMove(const FGeometry& Geometry, const FPointerEvent& PointerEvent, const float CurrentTime, const float PressDuration);

	// Called when user's finger stops touching the virtual control
	UFUNCTION(Category="Virtual Control Event", BlueprintNativeEvent)
	void OnTouchEnd(const FGeometry& Geometry, const FPointerEvent& PointerEvent, const float CurrentTime, const float PressDuration);

	/** Get Player Index associated to this virtual control event */
	UFUNCTION(Category="Virtual Control Event", BlueprintCallable)
	int32 GetPlayerIndex() const { return PlayerIndex; }
	
	/** Get the virtual control name that own this virtual control event */
	UFUNCTION(Category="Virtual Control Event", BlueprintCallable)
	FName GetButtonName() const { return LinkedButtonName; }

	TSharedPtr<SVirtualControl> GetVirtualControlWidget() { return VirtualControlWidget; }
	void SetVirtualControlWidget(const TSharedPtr<SVirtualControl>& InWidget) { VirtualControlWidget = InWidget; }

	/** Get Visual Layer data for the virtual control that own this virtual control event */
	UFUNCTION(Category="Virtual Control Event", BlueprintCallable, BlueprintPure)
	void GetVisualLayers(TArray<FVisualLayer>& VisualLayers);

	/** Set Visual Layer data of the virtual control that own this virtual control event
	 * The changes will be applied at the next tick
	 */
	UFUNCTION(Category="Virtual Control Event", BlueprintCallable)
	void SetVisualLayers(TArray<FVisualLayer> VisualLayers);
	
	virtual UWorld* GetWorld() const override;

private:
	UPROPERTY()
	UWorld* WorldContext;

	FName LinkedButtonName;
	int32 PlayerIndex;

	TSharedPtr<SVirtualControl> VirtualControlWidget;
};
