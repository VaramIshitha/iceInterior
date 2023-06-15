// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

#include "UObject/Object.h"
#include "CustomTouchInterfaceScaling.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class TOUCHINTERFACE_API UCustomTouchInterfaceScaling : public UObject
{
	GENERATED_BODY()

public:
	
	virtual float GetScaleFactor(const FVector2D GeometrySize) const;
};