// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "cpl_conv.h"
#include "cpl_error.h"
#include "cpl_multiproc.h"
#include "cpl_string.h"
#include "cpl_vsi.h"
#include "gdal_version.h"
#include "gdal.h"
#include "gdal_priv.h"
#include "gdal_version.h"
#include "ogr_api.h"
#include "ogr_core.h"
#include "gdal_utils_priv.h"
#include "ogr_feature.h"
#include "ogr_geometry.h"
#include "ogr_p.h"
#include "ogr_spatialref.h"
#include "ogrsf_frmts.h"
#include "CoreMinimal.h"
#include "Misc/ScopedSlowTask.h"
#include "GISFileManager.h"
#include "LandscapingStructs.h"
#include "Misc/Paths.h"
#include "Core/Public/Misc/FileHelper.h"
#include "CoordinateReferenceSystem.h"
#include "LandscapingSettings.h"

class UGISFileManager;

const TArray<FString> GEOJSON {"json", "geojson", "geojsonl", "geojsons"};
const TArray<FString> FIELD_NAMES {"fclass", "natural", "leisure", "landuse", "landarea", "class"};

class VectorFile
{
public:
    // Origin and Extent are in Level CRS
    VectorFile(FString InFilename, UGISFileManager* InGisFileManager, FExtents Extents, bool InbLanduse = false);
    VectorFile(FString InFilename, UGISFileManager* InGisFileManager, bool InbLanduse = false);
    ~VectorFile();
    void Close();
    bool HasError();
    FString GetError();
    TArray<FVectorData> GetShapes();
    FVector GetFirstPoint();

private:
    void OpenVectorFile();
    void OpenMVT();
    bool FetchSourceProjection();
    OGRSpatialReference FetchSourceSpaRef();
    OGRSpatialReference FetchTargetSpaRef();
    // void Translate();
    void FetchFeatures();
    TArray<FVectorData> Geometries = TArray<FVectorData>();
    void GetPointsFromGeometry(OGRGeometryH hGeometry, OGRFeatureH hFeature, int LayerNr = 0, FString FormerGeometryTypeStr = FString());
    bool IsInArea(FVector Point);
    int GetFirstObjectIndex(TArray<FVectorData> InGeometries, double &FarestSquaredDistance);
    int GetNextObjectIndex(int InCurrent, TArray<int> UsedIndizes, double FarestSquaredDistance, TArray<FVectorData> InGeometries);
    FVector ConvertPoint(FVector Point);
    
    double GeoTransform[6];
    bool bCheckExtents = true;
    GDALDatasetH VectorDataset = nullptr;
    FString SourceProj = FString();
    OGRSpatialReference SourceSpatialRef;
    OGRSpatialReference* SourceGeogCSSpatialRef = nullptr;
    OGRSpatialReference TargetSpatialRef;
    FExtents ClipExtents = FExtents();
    FString Filename = FString();
    FString Error = FString();
    UGISFileManager* GisFM = nullptr;
    bool bIsMapboxPbf = false; // used on Mapbox .pbf import
    double OriginX;
    double OriginY;
    double Resolution = 1.0;
    double ResolutionX = 1.0;
    double ResolutionY = 1.0;
    bool bLanduse = false;
};
