// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "DrawDebugHelpers.h"
#include "LandscapingUtils.h"
#include "LandscapingStructs.h"
#include "Components/BoxComponent.h"
#include "CoordinateReferenceSystem.h"
#include "LandscapingInfo.h"
#include "ALandscapingInfos.generated.h"


DECLARE_MULTICAST_DELEGATE(FLandscapingInfosDeletedDelegate);

UCLASS(NonTransient, ClassGroup=(Landscaping), HideCategories = (Shape, Navigation, HLOD, Physics, Cooking, Input, Replication, Actor, Collision, Rendering, Tags, WorldPartition, Mobile, RayTracing, AssetUserData, DataLayers))
class ALandscapingInfos : public AActor
{
    GENERATED_BODY()
    
public:
    
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Landscaping|DTM", meta=(DisplayAfter = "Shape", ToolTip = "DEPRECATED - please go to Project Settings -> Plugins -> Landscaping -> DTM to change allowed DTM file types"))
    FString AllowedDTMFileTypes = FString("*.tif;*.tiff;*.hgt;*.asc;");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Landscaping|Scale", meta=(DisplayAfter = "Shape", ToolTip = "Scalefactor of the resulting CRS (Scale of the Landscapes, Meshes, Splines etc.) can be overwritten here before import or with the button below.\nScale of the Actors in the level will multiplied with this value"))
    double LandscapeScaleFactor = 1.0;
    UFUNCTION(CallInEditor, Category="Landscaping|Scale", meta=(DisplayAfter = "Shape", Tooltip="Change the scale of the Landscape and all Actors of the Level and let georeferencing intact.\nPositions are adjusted to keep Actor relative proportions.\n\nChanging the scale should be done as early as possible when working on a level"))
    void ChangeLandscapeScaleFactor();
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Landscaping|DTM", meta=(DisplayAfter = "Shape", ToolTip = "DEPRECATED - Scale of the Landscape will be calculated in double instead of integer.\n\nNOTICE: since Landscaping 7.0 this is default"))
    bool bUsePreciseScale = true;
    UPROPERTY(VisibleAnywhere, Category="Landscaping|DTM", meta=(DisplayAfter = "Shape", ToolTip = "Calculated Scale of the Landscape.\n'Tiles->Array Element' holds value specific to a Landscape"))
    FVector LandscapeScale = FVector(0);
    UPROPERTY(VisibleAnywhere, Category="Landscaping|DTM", meta=(DisplayAfter = "Shape", ToolTip = "Minimal altitude of the Landscape.\n'Tiles->Array Element' holds value specific to a Landscape"))
    double MinAltitude = 65535.0;
    UPROPERTY(VisibleAnywhere, Category="Landscaping|DTM", meta=(DisplayAfter = "Shape", ToolTip = "Maximal altitude of the Landscape.\n'Tiles->Array Element' holds value specific to a Landscape"))
    double MaxAltitude = MinAltitude;
    UPROPERTY(VisibleAnywhere, Category="Landscaping|DTM", meta=(DisplayAfter = "Shape", ToolTip = "Bounding rect of the Landscape in CRS units.\nThis always shows the last imported area.\n'Tiles->Array Element' holds value specific to a Tile (Landscape/Procedural Mesh/Nanite Mesh)"))
    FExtents Extents = FExtents();
    UPROPERTY(VisibleAnywhere, Category="Landscaping|DTM", meta=(DisplayAfter = "Shape", ToolTip = "Bounding rect chosen in 'DTM Options -> Corners of Bounding Box'.\nEPSG:4326\n\nThis always shows the last imported area"))
    FExtents CroppedExtents = FExtents(); // EPSG:4326
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Landscaping|DTM", meta=(DisplayAfter = "Shape", ToolTip = "Auto detect best possible Z Scale for Landscapes"))
    bool bHighDetailZScale = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Landscaping|DTM", meta=(DisplayAfter = "Shape", ToolTip = "Custom Z Scale for Landscapes"))
    double CustomZScale = 100.0;
   
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Landscaping|Vectordata", meta=(DisplayAfter = "Shape", ToolTip = "Vector data scale (from shapefiles, etc.) can be overwritten after choosing the file and before import"))
    FVector VectorDataScale = FVector(100);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Landscaping|Vectordata", meta=(DisplayAfter = "Shape", ToolTip = "Draw debug lines of vector data.\nArrows on the debug lines will indicate the direction"))
    bool bDrawVectorDataDebug = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Landscaping|Vectordata", meta=(DisplayAfter = "Shape", ToolTip = "Snap the debug lines to the surface ground.\nExpensive operation, do not use on large Landscapes"))
    bool bSnapToGround = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Landscaping|Vectordata", meta=(DisplayAfter = "Shape", ToolTip = "Update interval of the debug lines.\nSetting it to a higher value while working on large landscpes increases editor frame rate"))
    float DebugUpdateInterval = 1;
    UPROPERTY(VisibleAnywhere, Category="Landscaping|Vectordata", meta=(DisplayAfter = "Shape", ToolTip = "Offset from surface ground for vector data (from shapefiles, etc.)"))
    float OffsetFromGround = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Landscaping|Vectordata", meta=(DisplayAfter = "Shape", ToolTip = "XY offset of vector data (from shapefiles, etc.)"))
    FVector XYOffset = FVector(0);

    UPROPERTY(VisibleAnywhere, Category="Landscaping|Internal")
    UBoxComponent* Bounds = nullptr;
    UPROPERTY(VisibleAnywhere, Category="Landscaping|Internal", meta=(DisplayAfter = "Shape", ToolTip = "Wheter or not the Landscape in this Level is imported through the Landscaping plugin"))
    bool bImportedThroughLandscaping = false;
    UPROPERTY(VisibleAnywhere, Category="Landscaping|Internal", meta=(DisplayAfter = "Shape", ToolTip = "Wheter or not the Landscape in this Level is a World Partition Landscape"))
    bool bWorldPartition = false;
    UPROPERTY(VisibleAnywhere, Category="Landscaping|Internal", meta=(DisplayAfter = "Shape", ToolTip = "Grid Size of the Landscape (if World Partition is used)"))
    int WorldPartitionGridSize = 16;
    UPROPERTY(VisibleAnywhere, Category="Landscaping|Internal", meta=(DisplayAfter = "Shape", ToolTip = "Maximum Size of a single Landscape"))
    int DesiredMaxTileSize = 8192;
    UPROPERTY(VisibleAnywhere, Category="Landscaping|Internal", meta=(DisplayAfter = "Shape", ToolTip = "CRS used in this Level (EPSG)"))
    FString RootProjection = FString();
    UPROPERTY(VisibleAnywhere, Category="Landscaping|Internal", meta=(DisplayAfter = "Shape", ToolTip = "CRS used in this Level (WKT)"))
    FString RootWkt = FString();
    UPROPERTY(VisibleAnywhere, Category="Landscaping|Internal", meta=(DisplayAfter = "Shape", ToolTip = "Landscape Material"))
    UMaterialInterface* Material;
    UPROPERTY(VisibleAnywhere, Category="Landscaping|Internal", meta=(DisplayAfter = "Shape", ToolTip = "Metadata for Landscapes"))
    TArray<FLandscapingInfo> Tiles = TArray<FLandscapingInfo>();
    UPROPERTY(VisibleAnywhere, Category="Landscaping|Internal", meta=(DisplayAfter = "Shape", ToolTip = "CRS"))
    UCoordinateReferenceSystem* CRS;

    FString ShapeFClass = FString();
private:
    UWorld* World = nullptr;
    FVector TransformLocationCached = FVector(0);
    FVector XYOffsetCached = FVector(0);
    FVector VectorDataScaleCached = VectorDataScale;
    double LandscapeScaleFactorCached = LandscapeScaleFactor;
    float AccumulatedTimeDelta = 0;
    bool FirstTick = true;
    TArray<TSharedPtr<MaterialLayerSettings>> LayerDataList = TArray<TSharedPtr<MaterialLayerSettings>>();

public:
    ALandscapingInfos(const FObjectInitializer& ObjectInitializer);
   
    virtual bool ShouldTickIfViewportsOnly() const override;
    
    virtual void Tick(float DeltaTime) override;

    void SetLayerDataList(TArray<TSharedPtr<MaterialLayerSettings>> InLayerDataList);

    // get CRS Reference, creates and initializes one, if there is none
    UCoordinateReferenceSystem* GetCRS();

    // only set origin, if there is not already one
    void SetOrigin(FVector InOrigin);

    void Refresh();

    // reset CRS - only works if there are not tiles in the level
    void ResetCRS();

    // force reset CRS and replace with AuthorityIDStr and Origin
    // AuthorityIDStr is an epsg code e.g. 'EPSG:3758'
    int SetCRS(FString AuthorityIDStr, FVector Origin);

    void UpdateLocationToGeometries(FVector TransformLocationDelta);

    void UpdateVectorScaleToGeometries();

    bool CheckWorld();

    void UpdateLandscapingBounds();

	FVector SnapToFloor(FVector Location);

    // Left: X, Top: Y
    void SetExtentsAndBoundsFromLeftTopCorner(double Left, double Top);

    void SetBoundsFromTiles();

    void SetOffsetXYFromLeftTopMostTile();

    void SetCroppedExtents(FExtents InExtents);

    // set cropped extents as comma separeted double values: bottom,left,top,right
    bool SetCroppedExtents(FString InBBox);

    // cropped extents of last area imported
    FExtents GetCroppedExtents();

    // get area to import if file or area via map is selected, or area already imported otherwise
    double GetArea() const;

    // get width and height of area to import if file or area via map is selected, or width and height of already imported area otherwise
    FVector GetAreaDimension();

    // get indizes of currenctly selected tiles (landscapes) in the outliner
    TArray<int> GetSelectedTiles() const;

    void CalculateTileBounds();

    void SaveStaticMeshAndReplace();

    FString SaveStaticMesh(ALandscapingProcMeshLandscape* LandscapeSM);
};