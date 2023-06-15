// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "LandscapingStructs.h"
#include "LandscapingSettings.h"
#include <math.h>
#include <regex>
#include "gdal.h"
#include "ogr_spatialref.h"
#include "ogr_geometry.h"
#include "Defines.h"
#include "CoordinateReferenceSystem.generated.h"

struct FLandscapingInfo;

UCLASS()
class UCoordinateReferenceSystem : public UObject
{
    GENERATED_BODY()

private:
    
    // EPSG Code of the Level
    UPROPERTY(VisibleAnywhere, Category="Landscaping|Internal", meta=(Tooltip="EPSG Code of the Level"))
    int AuthorityID = 0;
    // optional Projection Str from File as wkt, proj str, epsg, ...
    UPROPERTY(VisibleAnywhere, Category="Landscaping|Internal", meta=(Tooltip="Projection Str from File as wkt"))
    FString Wkt = FString();
    // Origin in AuthorityID CRS
    UPROPERTY(VisibleAnywhere, Category="Landscaping|Internal", meta=(Tooltip="Origin in AuthorityID CRS"))
    FVector Origin = FVector(0);
    // last selected cropped extents from the map
    // will be initialized on RasterFile selection to extents of the RasterFile
    // always in GeogCS of Wkt
    UPROPERTY(VisibleAnywhere, Category="Landscaping|Internal", meta=(Tooltip="Last selected cropped extents from the map or file"))
    FExtents CurrentCroppedExtents = FExtents();
    // Landscape scale
    UPROPERTY(VisibleAnywhere, Category="Landscaping|Internal", meta=(Tooltip="DEPRECATED Landscape scale. See Tiles->LandscapeScale"))
    FVector LandscapeScale = FVector(0);
    // Landscape scale factor
    UPROPERTY(VisibleAnywhere, Category="Landscaping|Internal", meta=(Tooltip="Landscape scale factor\nThis factor is applied to all Landscape in the level"))
    double LandscapeScaleFactor = 1.0;
    // Vector data scale
    UPROPERTY(VisibleAnywhere, Category="Landscaping|Internal", meta=(Tooltip="Vector data scale\nNormally you won't touch this"))
    FVector VectorScale = FVector(100);
    // XY Offset for vector data
    UPROPERTY(VisibleAnywhere, Category="Landscaping|Internal", meta=(Tooltip="XY Offset of vector data\nNormally you won't touch this"))
    FVector XYOffset = FVector(0);

    // DEPRECATED
    UPROPERTY(VisibleAnywhere, Category="Landscaping|Internal", meta=(Tooltip="DEPRECATED From Version 7.0 this is true and cannot be changed"))
    bool bUsePreciseScale = true;

    UPROPERTY(VisibleAnywhere, Category="Landscaping|Internal", meta=(Tooltip="Current projection mode.\nTo change the mode go to Project Settings -> Plugins -> Landscaping"))
    ELandscapingProjectionMode ProjectionMode = ELandscapingProjectionMode::AutoReprojectToUTM;

public:
    UCoordinateReferenceSystem(const FObjectInitializer& ObjectInitializer);

    bool IsValid();

    void FetchAuthorityIDFromSettings();

    // EPSG code of the Level
    int GetAuthorityID();

    // EPSG code of the Level with preceeding 'EPSG:', e.g.: 'EPSG:3857'
    FString GetAuthorityIDStr();

    // Origin in AuthorityID CRS
    FVector GetOrigin() const;

    void SetOrigin(FVector InOrigin);

    bool IsOriginValid() const;

    void SetAuthorityID(int EPSG, FString InProjectionStr = FString());

    FString GetWkt() const;

    bool IsEpsgIDValid(int Epsg, bool bCheckIfProjected = true) const;

    bool IsWktValid(FString InWkt, bool bCheckIfProjected = true) const;

    bool IsAuthorityIDValid();

    bool IsModeAutoReprojectToUTM();

    void FetchModeFromSetting();

    bool IsModeUseSourceCRS();

    bool IsModeCustomCRS();

    void SetCroppedExtents(FExtents InExtents);

    bool SetCroppedExtents(FString ImportBBox);

    FExtents GetCroppedExtents() const;

    bool GetUsePreciseScale() const;

    void SetUsePreciseScale(bool InPreciseScale);

    bool IsSameAsLevelCRS(const char* Proj) const;

    // if LeftBottom or RightTop is zero, this will return FExtents::Empty
    FExtents GetValidExtents(FVector2D LeftBottom, FVector2D RightTop) const;

    // converts between given wkt projections, returns FExtents::Empty if not successful
    FExtents ConvertFromTo(FExtents InExtents, FString InWktFrom, FString InWktTo) const;

    // converts to lon lat using provided wkt, returns FExtents::Empty if not successful
    FExtents ConvertToGeogCS(FExtents InExtents, FString InFromWkt) const;

    // converts to crs from lon lat using provided wkt, returns FExtents::Empty if not successful
    FExtents ConvertFromGeogCS(FExtents InExtents, FString InWkt) const;

    // converts to lon lat using level wkt, returns FExtents::Empty if not successful
    FExtents ConvertToGeogCS(FExtents InExtents) const;

    // converts to crs from lon lat using level wkt (reverse to ConvertToGeogCS), returns FExtents::Empty if not successful
    FExtents ConvertFromGeogCS(FExtents InExtents) const;

    // convert to and from GeogCS, returns FVector2D::Zero if not successful
    FVector2D ConvertPointToGeogCS(double X, double Y, FString From, FString To, bool SwitchSourceAndTarget = false) const;

    // Proj can be epsg, wkt, projstring, ...
    int FindAuthorityID(const char* Proj) const;

    // dump projection to output log
    void DumpProjectionToLog(const char* Proj, FString Filename) const;

    // value to multiply by linear distances to transform them to meters
    double GetLinearUnits() const;

    // this should automaticly determine the right utm
    // utm will have the hightest accuracy
    int GetUTMAuthorityID(double Lon, double Lat) const;

    // get landscape scale considering IsPreciseScale and LandscapeScaleFactor
    FVector GetLandscapeScale(FLandscapingInfo InInfo) const;

    // get landscape location considering IsPreciseScale and LandscapeScaleFactor
    FVector GetLandscapeLocation(FLandscapingInfo InInfo) const;

    void SetLandscapeScale(FVector InLandscapeScale);
    
    void SetLandscapeScaleFactor(double InLandscapeScaleFactor);

    double GetLandscapeScaleFactor() const;
    
    void SetVectorDataScale(FVector InVectorScale);

    // only for vector data which is already converted to same CRS as raster data (using CoordinateReferenceSystem::AuthorityID)
    FVector ConvertCRSPointToUnreal(FVector Point) const;

    void SetXYOffset(FVector InXYOffset);

    FString GetOGRErrStr(OGRErr InError) const;
};