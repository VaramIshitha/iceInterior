// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "LandscapeSplineControlPointOptions.h"
#include "LandscapeLayerInfoObject.h"
#include "LandscapeProxy.h"
#include "LandscapeSplineOptions.generated.h"

USTRUCT(BlueprintType)
struct FLandscapeSplineOptions
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Landscaping)
	float StartWidth = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Landscaping)
	float EndWidth = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Landscaping)
	float StartSideFalloff = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Landscaping)
	float EndSideFalloff = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Landscaping)
	float StartRoll = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Landscaping)
	float EndRoll = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Landscaping)
	int NumSubdivisions = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Landscaping)
	bool RaiseHeights = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Landscaping)
	bool LowerHeights = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Landscaping)
	ULandscapeLayerInfoObject* PaintLayer = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Landscaping)
	FName EditLayerName = TEXT("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Landscaping)
	ALandscapeProxy* Landscape = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Landscaping)
	bool bPaintMaterialLayer = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Landscaping)
	FLandscapeSplineControlPointOptions ControlPointOptions;

};