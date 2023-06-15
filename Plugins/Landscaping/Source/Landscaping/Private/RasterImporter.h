// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "GISFileManager.h"
#include "CoreMinimal.h"
#include "LevelEditor.h"
#include "UnrealEd.h"
#include "UnrealEdMisc.h"
#include "Editor.h"
#include "EditorLevelUtils.h"
#include "Landscape.h"
#include "LandscapeSubsystem.h"
#include "LandscapeFileFormatInterface.h"
#include "LandscapeEdit.h"
#include "LandscapeEditorUtils.h"
#include "LandscapeProxy.h"
#include "LandscapeStreamingProxy.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "EngineGlobals.h"
#include "RasterData.h"
#include "RasterTileFactory.h"
#include "GenericPlatform/GenericPlatformMath.h"
#include "LandscapingStructs.h"
#include "WeightmapImporter.h"
#include "LandscapingTileLoader.h"
#include "ALandscapingProcMeshLandscape.h"
#include "IAssetTools.h"
#include "AssetToolsModule.h"
#include "Materials/MaterialExpressionLandscapeLayerCoords.h"
#include "Materials/MaterialExpressionDivide.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant2Vector.h"
#include "ActorPartition/ActorPartitionSubsystem.h"

class WeightmapImporter;

class RasterImporter
{
public:
    RasterImporter(UGISFileManager* InGisFileManager, FString InWorkingDir);
    ~RasterImporter();
    
    // TileIndex is only relevant for satellite data, because through heightdata tiles get added
    FString LoadFiles(TArray<FString>& Filenames, int TileIndex = 0);
    TArray<FString> WriteFiles(TArray<RasterData> InDatas);
    ALandscapeProxy* CreateLandscape(RasterData RasterData, FGuid Guid, RasterImportOptions Options, int TileIndex = 0);
    bool CreateTerrains(UMaterialInterface* Material, TArray<TSharedPtr<MaterialLayerSettings>> LayerSettings, WeightmapImporter* InWeightmapCreator, RasterImportOptions Options);
    bool CreateSatelliteImages(int TileIndex, RasterImportOptions Options);
    bool HasRasterFile() const;
    void CreateDummyLandscape();
    void SetWorkingDir(FString Dir);
    FVector GetMeterPerPixel();

private:
    bool CheckBeforeImport(RasterData InData);
    void ImportFiles(TArray<FString> Filenames);
    bool CreateMeshes(RasterImportOptions Options, int FirstTileIndex = 0);
    bool CreateLandscapes(RasterImportOptions Options, int FirstTileIndex = 0);
    bool UpdateLandscapes(RasterImportOptions Options, int FirstTileIndex = 0);
    UMaterialInterface* CreateMaterial(UTexture* InitialTexture, FName Name, FVector2D Tiling, FName MaterialName, bool bImportSatImgAsDecal);
   
    UWorld* GetWorld();
    UMaterialInterface* LandscapeMaterial = nullptr;
    TArray<TSharedPtr<MaterialLayerSettings>> LayerSettingsList = TArray<TSharedPtr<MaterialLayerSettings>>();
    FString DefaultLayerName = FString();
    RasterTileFactory* TileFactory = nullptr;
    WeightmapImporter* WeightmapCreator = nullptr;
    UGISFileManager* GisFM = nullptr;
};