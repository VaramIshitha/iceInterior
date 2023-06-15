// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "LandscapingStructs.h"
#include "ALandscapingProcMeshLandscape.h"
#include "LandscapingInfo.generated.h"

USTRUCT()
struct FLandscapingInfo
{
	GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, Category=Landscaping, meta=(ToolTip = "Bounding rect of the Landscape in CRS units"))
	FExtents Extents = FExtents();
	UPROPERTY(VisibleAnywhere, Category=Landscaping, meta=(ToolTip = "Bounding rect of the Landscape in WGS 84 units"))
	FExtents WGS84Extents = FExtents();
	UPROPERTY(VisibleAnywhere, Category=Landscaping, meta=(ToolTip = "Minimal altitude of the Landscape"))
	double MinAltitude = 65535.0;  
	UPROPERTY(VisibleAnywhere, Category=Landscaping, meta=(ToolTip = "Maximal altitude of the Landscape"))
	double MaxAltitude = -65535.0;
    UPROPERTY(VisibleAnywhere, Category=Landscaping, meta=(ToolTip = "Calculated Scale of the Landscape"))
    FVector LandscapeScale = FVector(0);
	UPROPERTY(VisibleAnywhere, Category=Landscaping, meta=(ToolTip = "X Location of the Landscape"))
	double LocationX = 0;
	UPROPERTY(VisibleAnywhere, Category=Landscaping, meta=(ToolTip = "Y Location of the Landscape"))
	double LocationY = 0;
	UPROPERTY(VisibleAnywhere, Category=Landscaping, meta=(ToolTip = "Z Location of the Landscape"))
	double LocationZ = 0;
	UPROPERTY(VisibleAnywhere, Category=Landscaping, meta=(ToolTip = "Number of Sections of the Landscape"))
	int NumberOfSections = 0;  
	UPROPERTY(VisibleAnywhere, Category=Landscaping, meta=(ToolTip = "Quads per Sections of the Landscape"))
	int QuadsPerSection = 0;  
	UPROPERTY(VisibleAnywhere, Category=Landscaping, meta=(ToolTip = "Resolution of the imported DTM file in pixel"))
	FIntVector ImportResolution = FIntVector(0);
	UPROPERTY(VisibleAnywhere, Category=Landscaping, meta=(ToolTip = "Resolution of the Landscape"))
	FIntVector LandscapeResolution = FIntVector(0);
	UPROPERTY(VisibleAnywhere, Category=Landscaping, meta=(ToolTip = "Unreal Engine Bounds of the Landscape"))
	FBox Bounds = FBox(ForceInit);
	UPROPERTY(VisibleAnywhere, Category=Landscaping, meta=(ToolTip = "Meter per pixel X in of the imported DTM file"))
	double MeterPerPixelX = 0;  
    UPROPERTY(VisibleAnywhere, Category=Landscaping, meta=(ToolTip = "Meter per pixel Y in of the imported DTM file"))
	double MeterPerPixelY = 0;  
	UPROPERTY(VisibleAnywhere, Category=Landscaping)
	FString Filename = FString();  
	UPROPERTY(VisibleAnywhere, Category=Landscaping)
	FString OriginalFilename = FString(); 
    UPROPERTY(VisibleAnywhere, Category=Landscaping)
	TArray<FString> SatelliteFilenames = TArray<FString>();  
	UPROPERTY(VisibleAnywhere, Category=Landscaping, meta=(ToolTip = "CRS used on import"))
	FString Projection = FString();
	UPROPERTY(VisibleAnywhere, Category=Landscaping)
	FString MapName = FString();
    UPROPERTY(EditAnywhere, Category=Landscaping)
	ALandscapeProxy* Landscape = nullptr;
	UPROPERTY(EditAnywhere, Category=Landscaping)
	ALandscapingProcMeshLandscape* LandscapeSM = nullptr;
	UPROPERTY(EditAnywhere, Category=Landscaping)
	AStaticMeshActor* LandscapeNaniteMesh = nullptr;
	UPROPERTY(VisibleAnywhere, Category=Landscaping)
	UMaterialInterface* Material = nullptr;
	TArray<FVectorData> LandcoverShapes = TArray<FVectorData>();
    TArray<FVectorData> SplineGeometries = TArray<FVectorData>();

    // calculate the scale of the tile
    void CalculateScale(RasterImportOptions Options);

    double GetScaleZ();

	// calculate the location of the tile
    void CalculateLocation(FVector Origin);

    // get location as FVector without considering LandscapeScaleFactor
    FVector GetLocation() const;

	bool ContainsSplineGeometry(FString InId) const;

	bool ContainsLandcoverShape(FString InId) const;

    // stringify
    FString ToString() const;
};