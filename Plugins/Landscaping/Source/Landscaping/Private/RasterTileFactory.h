// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include <math.h>
#include "gdalwarper.h"
#include "RasterFile.h"
#include "RasterData.h"
#include "RasterLandscapeData.h"
#include "VectorFile.h"
#include "CoordinateReferenceSystem.h"
#include "GISFileManager.h"
#include "Misc/ScopedSlowTask.h"
#include "LandscapeEditorModule.h"
#include "Runtime/Core/Public/Async/ParallelFor.h"
#include "FileHelpers.h"
#include "ImageUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "LandscapingSettings.h"
#include <chrono>
#include <numeric>
#include "Blur.h"

using namespace std;
using namespace std::chrono;

class UGISFileManager;
class RasterLandscapeData;

struct ColorData
{
    TArray<FColor> Data = TArray<FColor>();
    int32 Width = 0;
    int32 Height = 0;
    FString BaseFilename;
    int NumberTilesX = 1;
    int NumberTilesY = 1;
};

class RasterTileFactory
{
    public:
    RasterTileFactory(TArray<FString> ProjDataPath, FString GdalDataPath, UGISFileManager* InGisFileManager);
    ~RasterTileFactory();
    void SetWorkingDir(FString WorkingDir);
    FString PrepareImport(RasterImportOptions Options);
    TArray<FTileImportConfiguration> GetSquareConfigurations();
    TArray<FString> ReadFiles(TArray<FString> FilesToRead, int TileIndex = 0);
    bool FetchAuthorityID(FString FileToRead);
    TArray<FString> WriteFiles(TArray<RasterData> InDatas);
    bool HasRasterFile(int LandscapeDataIndex) const;
    RasterData GetFirstRasterData(int LandscapeDataIndex, bool bWithRasterBandData = false);
    RasterData GetNextRasterData(int LandscapeDataIndex, int RasterDataIndex);
    int GetRasterDataCount(int LandscapeDataIndex);
    int GetLandscapeDataCount();
    FVector GetMeterPerPixel();
    TArray<uint16> Import(TArray<TArray<double>> HeightInfo, FIntVector ImportResolution, int TileIndex, RasterImportOptions Options);
    TArray<ColorData> GetColorData(int TileIndex, bool bImportSatImgAsDecal);
    TileConfig CreateTexture(int TileIndex, bool bImportSatImgAsDecal);

    TArray<RasterLandscapeData> LandscapeDatas = TArray<RasterLandscapeData>();

    private:
    RasterLandscapeData MergeOverlapping(int Index, RasterImportOptions Options);
    RasterData CreateBlankTile(int Index, int x, int y, double TilesLeft, double TilesTop, RasterImportOptions Options, bool bLogError);
    void RefreshLandscapingInfos(int LandscapeDataIndex, RasterImportOptions Options);
    
    FString WorkingDir = FString();
    UGISFileManager* GisFM = nullptr;
    TArray<FString> InMemoryFiles = TArray<FString>();
};