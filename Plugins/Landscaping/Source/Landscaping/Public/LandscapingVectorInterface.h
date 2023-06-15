// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "LandscapingVectorInterface.generated.h"


UINTERFACE(Blueprintable)
class ULandscapingVectorInterface : public UInterface
{
	GENERATED_BODY()
};

class ILandscapingVectorInterface
{
	GENERATED_BODY()
	
public:
	// classes using this interface must implement OnVectorData
	// it will be called right after instantiation of the Actor which 
	// implements this interface and contains a FVectorData Object
	// with a TArray<FVector> Points
	// see LandscapingStructs.h -> FVectorData
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category=Landscaping)
	void OnVectorData(FVectorData VectorData);

};