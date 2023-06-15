// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LandscapingMapboxSettings.generated.h"

UENUM()
enum class EMapboxHeightData : uint8
{
	MapboxTerrainDEMv1 UMETA(DisplayName = "Mapbox Terrain-DEM v1"),
	MapboxTerrainRGBv1 UMETA(DisplayName = "Mapbox Terrain-RGB v1")
};
UCLASS(Config = LandscapingMapbox, DefaultConfig)
class LANDSCAPINGMAPBOX_API ULandscapingMapboxSettings : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditAnywhere, Config, Category = Mapbox, meta=(Tooltip="Api Key for Mapbox - get a free API key at https://mapbox.com"))
    FString ApiKey;
	UPROPERTY(EditAnywhere, Config, Category = Mapbox, meta=(Tooltip="Zoom for heightmaps.\n14 is the best possible and default value"))
	int Zoom = 14;
	UPROPERTY(EditAnywhere, Config, Category = Mapbox, meta=(Tooltip="Query heightdata from 'Mapbox Terrain-DEM v1' or 'Mapbox Terrain-RGB v1' data.\n'Mapbox Terrain-DEM v1' is the optimized version but 'Mapbox Terrain-RGB v1' offers data until zoom level 15.\nSee docs for further details!"))
	EMapboxHeightData HeightDataAPI = EMapboxHeightData::MapboxTerrainDEMv1;
	UPROPERTY(EditAnywhere, Config, Category = Mapbox, meta=(Tooltip="Zoom for vector tiles.\n14 is the default value and has world coverage.\n30 is the heighest, most detailed, but will download a massive amount of data even on small areas.\n\nNOTE: If you see areas without vector data in the debug lines view, the zoom level is to high, and mapbox cannot provide data for that area."))
	int ZoomVector = 14;
	UPROPERTY(EditAnywhere, Config, Category = Mapbox, meta=(Tooltip="Zoom for satellite images. 16 is the default value and has world coverage, 18 has a resolution of 0.3-0.6 meter almost everywhere.\n21 is the best (7.5 centimeter, some areas of US, Canada, Europe, and Australia) - but it will download a lot of data even on small areas and will take quite some time.\n\nCAUTION: you might exceed your mapbox free tier when downloading at zoom 18 - 21.\nAlso it only make sense on small areas, because the texture resolution in Unreal Engine is limited with 8k\n\nNOTE: If you see black areas in the result, the zoom level is to high, and mapbox cannot provide data for that area."))
	int ZoomSatellite = 16;
	UPROPERTY(EditAnywhere, Config, Category = Mapbox, meta=(Tooltip="Display a warning message popup when downloading more than a certain number of tiles. E.g. more than 1000 tiles.\n0 means no warn messages."))
	int TileDownloadWarnLimit = 0;
    

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

};