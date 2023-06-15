// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Core/Public/Async/ParallelFor.h"
#include "ImageUtils.h"
#include "Misc/FileHelper.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "BaseVectorImporter.h"
#include "VectorFile.h"
#include "RasterData.h"
#include "LandscapingStructs.h"
#include "FileHelpers.h"
#include "LandscapeInfo.h"
#include "LandscapeLayerInfoObject.h"
#include "LandscapeEdit.h"
#include "GISFileManager.h"
#include <vector>

class VectorTileFactory;
class UGISFileManager;

struct WeightmapCreationInfo
{
    TSharedPtr<MaterialLayerSettings> LayerSettings;
    TArray<FVectorData> LandcoverShapes;
    int32 LayerIndex = 0;
    TArray<uint8> ResampledNoise;
};

class WeightmapImporter : public BaseVectorImporter
{
public:
    WeightmapImporter(UGISFileManager* InGisFileManager);
    ~WeightmapImporter();
    TArray<FString> LoadFiles(TArray<FString> InFilenames, int TileIndex);
    bool CreateWeightmaps(UMaterialInterface* LandscapeMaterial, TArray<TSharedPtr<MaterialLayerSettings>> LayerSettingsList, int TileIndex, FString DefaultLayer);
    TArray<FLandscapeImportLayerInfo> GetLayerInfos(UMaterialInterface* LandscapeMaterial, TArray<TSharedPtr<MaterialLayerSettings>> LayerSettingsList, int TileIndex, FString DefaultLayer);
    ULandscapeLayerInfoObject* GetLandscapeLayerInfoObject(TSharedPtr<MaterialLayerSettings> LayerSettings, const FString& ContentPath, int TileIndex);
    ULandscapeLayerInfoObject* GetLandscapeLayerInfoObject(TSharedPtr<FString> PaintLayer, int TileIndex);
    bool HasVectorFile();
    void ClearLayers(TArray<int> SelectedTiles);

private:
    bool Import(ALandscapeProxy* LandscapeProxy, UMaterialInterface* LandscapeMaterial, TArray<TSharedPtr<MaterialLayerSettings>> LayerSettingsList, int TileIndex, FString DefaultLayer);
    TArray<FVectorData> GetLandcoverShapes(TArray<FString> InFeatureClasses, int TileIndex);
    TArray<uint8> CreateWeightData(FLandscapingInfo InInfo, TArray<FVectorData> Shapes);
    bool IsInShape(FBox ShapeBounds, FVectorData Shape, FVector Point);
    int CrossingNumber(TArray<FVector> Polygon, FVector Point);
    TArray<uint8> GetPixels(UTexture2D* NoiseTexture2D, FString ColorChannel, int Tiling);
    UWorld* GetWorld();

    VectorTileFactory* TileFactory = nullptr;
    TMap<int, TArray<FString>> FilesToLoad = TMap<int, TArray<FString>>();
    UGISFileManager* GisFM = nullptr;
    TArray<FString> FilterGeom = TArray<FString>();
    TArray<TArray<bool>> UsedCoordinates = TArray<TArray<bool>>();
    TArray<bool> UsedLayerIndizes = TArray<bool>();
    mutable FCriticalSection Guard;
};