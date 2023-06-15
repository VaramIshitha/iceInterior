// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CoordinateReferenceSystem.h"
#include "GISFileManager.h"
#include "RasterData.h"

using namespace std;

class UGISFileManager;

class RasterLandscapeData
{
private:
    UGISFileManager* GisFM = nullptr;

public:
    TArray<RasterData> RasterDatas = TArray<RasterData>();
    FVector MeterPerPixel = FVector(0);
    FIntVector TileSize = FIntVector(0);
    int NumberTilesX = 1;
    int NumberTilesY = 1;
    int RasterDataIndex = 0;
    FExtents Extents = FExtents();
    double MinAltitude = 65535.0;
    double MaxAltitude = -65535.0;

    RasterLandscapeData(UGISFileManager* InGisFileManager);
    void ResetRasterData();
    FString ToString();
    void UpdateLandscapeData();
    bool CalculateLandscapeResolution();
    void AddRasterData(FString FilePath, FString WorkingDir, int TileIndex = 0);
    RasterData GetRasterData(RasterFile* RasterFileObject, FString OriginalFilename, FString ActualFilename, bool bWithRasterbandData = false);
    FVector GetRasterPixelSize(RasterImportOptions Options, bool bPreserveSign = false);
    // Scale
    FVector GetScale();
    double GetScaleZ();


};