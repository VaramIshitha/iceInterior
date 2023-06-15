// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "LandscapingPaintLayerInterface.h"
#include "LandscapingUtils.h"
#include "LandscapingLandscapeSplines.generated.h"

UCLASS(ClassGroup=(Landscaping), BlueprintType)
class LANDSCAPING_API ALandscapingLandscapeSplines : public AActor, public ILandscapingPaintLayerInterface
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category=Landscaping)
	void OnSplineCreated(AActor* ActorWithSplineComp);

	UFUNCTION(BlueprintCallable, Category=Landscaping)
	FVector SnapToFloor(FVector Location, float OffsetFromFloor, AActor* ActorWithSplineComp = nullptr);

	UFUNCTION(BlueprintCallable, Category=Landscaping)
	bool ToggleCollisionEnabled(AActor* ActorWithSplineComp);

	UFUNCTION(BlueprintCallable, Category=Landscaping)
	void PostSplineUpdate(AActor* ActorWithSplineComp);
};