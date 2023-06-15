
// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "LandscapeSplineControlPointOptions.generated.h"

USTRUCT(BlueprintType)
struct FLandscapeSplineControlPointOptions
{
    GENERATED_BODY()

    /** Half-Width of the spline at this point. */
	UPROPERTY(EditAnywhere, Category=LandscapeSpline, meta = (DisplayName = "Half-Width"))
	float Width = 500;

	/** Layer Width ratio of the spline at this point. */
	UPROPERTY(EditAnywhere, Category = LandscapeSpline)
	float LayerWidthRatio = 1.f;

	/** Falloff at the sides of the spline at this point. */
	UPROPERTY(EditAnywhere, Category=LandscapeSpline)
	float SideFalloff = 100;

	UPROPERTY(EditAnywhere, Category = LandscapeSpline, meta=(UIMin = 0, ClampMin = 0, UIMax = 1, ClampMax = 1))
	float LeftSideFalloffFactor = 1.f;

	UPROPERTY(EditAnywhere, Category = LandscapeSpline, meta = (UIMin = 0, ClampMin = 0, UIMax = 1, ClampMax = 1))
	float RightSideFalloffFactor = 1.f;

	UPROPERTY(EditAnywhere, Category = LandscapeSpline, meta = (UIMin = 0, ClampMin = 0, UIMax = 1, ClampMax = 1))
	float LeftSideLayerFalloffFactor = 0.5f;

	UPROPERTY(EditAnywhere, Category = LandscapeSpline, meta = (UIMin = 0, ClampMin = 0, UIMax = 1, ClampMax = 1))
	float RightSideLayerFalloffFactor = 0.5f;

	/** Falloff at the start/end of the spline (if this point is a start or end point, otherwise ignored). */
	UPROPERTY(EditAnywhere, Category=LandscapeSpline)
	float EndFalloff = 0;

#if WITH_EDITORONLY_DATA
	/** Vertical offset of the spline segment mesh. Useful for a river's surface, among other things. */
	UPROPERTY(EditAnywhere, Category=LandscapeSpline, meta=(DisplayName="Mesh Vertical Offset"))
	float SegmentMeshOffset = 0;
#endif
};