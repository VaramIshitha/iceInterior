// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "LandscapeSplineOptions.h"
#include "LandscapingPaintLayerInterface.generated.h"

UINTERFACE(Blueprintable)
class LANDSCAPING_API ULandscapingPaintLayerInterface : public UInterface
{
	GENERATED_BODY()
};

class ILandscapingPaintLayerInterface
{
    GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category=Landscaping)
	void OnPaintLayer(FLandscapeSplineOptions LandscapeSplineOptions);
};