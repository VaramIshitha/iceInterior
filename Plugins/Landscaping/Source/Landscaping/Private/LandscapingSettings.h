// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Defines.h"
#include "LandscapingStructs.h"
#include "LandscapingSettings.generated.h"

class UCoordinatereferenceSystem;

UCLASS(Config = Landscaping, DefaultConfig)
class LANDSCAPING_API ULandscapingSettings : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditAnywhere, Config, Category="Landscaping|Internal", meta=(Tooltip="Where temporary files are stored\nPlease only use forward slashes `/`"))
	FString CacheDirectory = FString("C:/Temp/Landscaping");
	UPROPERTY(EditAnywhere, Config, Category="Landscaping|DTM", meta=(Tooltip="Allowed DTM File Types.\nTrying to import other file types might work, but might also lead to a crash.\nUse with care."))
	FString AllowedDTMFileTypes = FString("*.tif;*.tiff;*.hgt;*.asc;*.gpkg;");
	UPROPERTY(EditAnywhere, Config, Category="Landscaping|DTM", meta=(Tooltip="Memory Limit for resampling Raster data (DTM and Satellite) in GB (GigaByte)"))
	int ResampleMemoryGigaByte = 8;
	UPROPERTY(EditAnywhere, Config, Category="Landscaping|DTM", meta=(Tooltip="Resample Algorithm"))
	ELandscapingResampleAlgorithm ResampleAlgorithm = ELandscapingResampleAlgorithm::ResampleAlgBilinear;
	UPROPERTY(EditAnywhere, Config, Category="Landscaping|DTM", meta=(Tooltip="DEPRECATED - moved to 'DTM Import Options -> Desired Max Tile Size'", EditCondition = "false"))
	int WorldPartitionMaxLandscapeSize = 8192;
	UPROPERTY(EditAnywhere, Config, Category="Landscaping|DTM", meta=(Tooltip="DEPRECATED - moved to 'DTM Import Options -> Resample To First Tile'", EditCondition = "false"))
	bool bResampleToFirstTile = true;
	UPROPERTY(EditAnywhere, Config, Category="Landscaping|DTM", meta=(Tooltip="DEPRECATED - moved to 'DTM Import Options -> Smooth Edges'", EditCondition = "false"))
	bool bSmoothEdges = true;
	UPROPERTY(EditAnywhere, Config, Category="Landscaping|Internal", meta=(Tooltip="Projection Mode for imported GIS data\nPlease see docs for options description"))
	ELandscapingProjectionMode ProjectionMode = ELandscapingProjectionMode::UseSourceCRS;
	UPROPERTY(EditAnywhere, Config, Category="Landscaping|Internal", 
		meta=(Tooltip="EPSG code of target CRS\nAll GIS data will be converted to this CRS on import\nThis can also be used, if the source file is not georeferenced (e.g. no .prj file on ASCII imports)\n\nIMPORTANT: only easting/northing CRS with unit meter will work.\nOther CRS will not be imported correctly.\nIf you want to import files with unit ft or degree, use 'Automatically reproject to appropriate UTM CRS'", EditCondition = "ProjectionMode==ELandscapingProjectionMode::CustomCRS"))
	int Projection = 3857;
	UPROPERTY(EditAnywhere, Config, Category="Landscaping|Vectordata", meta=(Tooltip="Connect Linestrings of Vector Data (from Shapefiles etc. or Mapbox) with the same name.\nNormally, you do not want this, because branching rivers and roads cannot be connected to one spline."))
	bool bConnectLinestrings = false;
	UPROPERTY(EditAnywhere, Config, Category="Landscaping|Vectordata", meta=(Tooltip="Multi step conversion if vector data cannot be converted to Level CRS in one step.\nUse this, if shapes cannot be found in the bound of the Landscape\nThis leads to longer import times of Vector data"))
	bool bExtendedCRSCompatibility = false;
	UPROPERTY(EditAnywhere, Config, Category="Landscaping|Internal", meta=(Tooltip="On large Worlds, World Partition cells will automatically loaded and unloaded during import to save memory.\nPlease see docs -> Max Landscape Size\nThis settings is for UE 5.0 only (has no effect on later versions)"))
	bool bEnableLargeWorlds = false;
	UPROPERTY(EditAnywhere, Config, Category="Landscaping|Internal", meta=(Tooltip="Experimental - This setting will significantly reduce the time needed when importing raster with different CRS"))
	bool bReadParallel = false;
	UPROPERTY(EditAnywhere, Config, Category="Landscaping|Internal", meta=(Tooltip="Create temporary files in memory instead creating them on the drive in the cache directory\nMapbox files are still created in the cache directory"))
	bool bUseInMemoryFiles = true;
    
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

};