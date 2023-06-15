// Copyright Lost in Game Studio. All Rights Reserved.


#include "VirtualControlEvent.h"
#include "SVirtualControl.h"


UVirtualControlEvent::UVirtualControlEvent()
{
	WorldContext = GetOuter()->GetWorld();
}

void UVirtualControlEvent::OnTouchBegan_Implementation(const FGeometry& Geometry, const FPointerEvent& PointerEvent)
{
	
}

void UVirtualControlEvent::OnTouchMoved_Implementation(const FGeometry& Geometry, const FPointerEvent& PointerEvent)
{
	
}

void UVirtualControlEvent::OnTouchEnded_Implementation(const FGeometry& Geometry, const FPointerEvent& PointerEvent)
{
	
}

void UVirtualControlEvent::OnTouchBegin_Implementation(const FGeometry& Geometry, const FPointerEvent& PointerEvent, const float CurrentTime)
{
	// Legacy
	OnTouchBegan(Geometry, PointerEvent);
}

void UVirtualControlEvent::OnTouchMove_Implementation(const FGeometry& Geometry, const FPointerEvent& PointerEvent, const float CurrentTime, const float PressDuration)
{
	// Legacy
	OnTouchMoved(Geometry, PointerEvent);
}

void UVirtualControlEvent::OnTouchEnd_Implementation(const FGeometry& Geometry, const FPointerEvent& PointerEvent, const float CurrentTime, const float PressDuration)
{
	// Legacy
	OnTouchEnded(Geometry, PointerEvent);
}

void UVirtualControlEvent::GetVisualLayers(TArray<FVisualLayer>& VisualLayers)
{
	VisualLayers = VirtualControlWidget->GetDataByRef().VisualLayers;
}

void UVirtualControlEvent::SetVisualLayers(TArray<FVisualLayer> VisualLayers)
{
	VirtualControlWidget->GetDataByRef().VisualLayers = VisualLayers;
}

UWorld* UVirtualControlEvent::GetWorld() const
{
	return WorldContext;
}
