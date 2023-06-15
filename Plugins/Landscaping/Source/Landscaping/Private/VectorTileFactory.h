// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "VectorFile.h"
#include "LandscapingStructs.h"
#include "RasterData.h"
#include "GISFileManager.h"
#include "LandscapingUtils.h"
#include "CoordinateReferenceSystem.h"
#include "ALandscapingInfos.h"
#include "LandscapingSettings.h"
#include "Misc/ScopedSlowTask.h"

class UGISFileManager;

class VectorTileFactory
{
public:
    VectorTileFactory(UGISFileManager* InGisFileManager, bool InbLanduse = false);
    ~VectorTileFactory();
    FString AddFile(FString Filename, int TileIndex, bool bCheckExtents);
    bool HasVectorFiles();
    TArray<FVectorData> GetShapes(int TileIndex, bool bCrop = true);
    TArray<FVectorData> GetObjects();
    TArray<FString> GetAvailableFeatureClasses(TArray<FString> FilterGeom, bool bStripGeomName = false);

private:
    TArray<FVectorData> GetObjectsInBounds(int TileIndex);
    TArray<FVectorData> GetMappedObjects(int TileIndex);
    
    TArray<FVectorData> Shapes = TArray<FVectorData>();
    bool bHasVectorFiles = false;
    UGISFileManager* GisFM = nullptr;
    bool bLanduse = false;
};