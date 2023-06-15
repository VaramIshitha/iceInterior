// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "BaseVectorImporter.h"
#include "VectorFile.h"
#include "VectorTileFactory.h"
#include "Components/SplineComponent.h"
#include "Misc/Paths.h"
#include "Core/Public/Misc/FileHelper.h"
#include "ALandscapingInfos.h"
#include "LandscapeSplinesComponent.h"
#include "DesktopPlatformModule.h"
#include "Engine/StaticMeshActor.h"
#include "DrawDebugHelpers.h"
#include "IDesktopPlatform.h"
#include "Kismet/KismetMathLibrary.h"
#include "LandscapingVectorInterface.h"
#include "LandscapingSplineActor.h"
#include "LandscapingLandscapeSplines.h"
#include "WeightmapImporter.h"
#include "LandscapingUtils.h"
#include "Misc/ScopedSlowTask.h"
#include "LandscapingSettings.h"
#include "LandscapeSplineOptions.h"

struct VectorGeometrySpawnOptions
{   
    float StartWidth = 1;
    float EndWidth = 1;
    TSharedPtr<FString> ShapeFClass = nullptr;
    FLandscapeSplineOptions LandscapeSplineOptions = FLandscapeSplineOptions();
    TSharedPtr<FString> PaintLayer = nullptr;
    bool bRevertSplineDirection = false;
    UObject* SplineMesh = nullptr;
    UClass* ActorOrBlueprintClass = nullptr;
    bool bCropToBounds = true;
    bool bIsBuilding = false;
    ESplinePointType::Type SplinePointType = ESplinePointType::Curve;
    TSharedPtr<FString> SplinePointTypeStr = nullptr;
    FVector Scale = FVector(1);
    bool bSpawnAux = true;
    int MaxEntities = 0;
    int StartEntityIndex = 0;
    bool bLandscapeSplines = false;
    int ZoomLevel = 15;
};

class VectorImporter : public BaseVectorImporter
{
public:
    VectorImporter(UGISFileManager* InGisFileManager);
    ~VectorImporter();
    TArray<FString> LoadFiles(TArray<FString> InFilenames, int TileIndex);
    bool CreateBlueprints(VectorGeometrySpawnOptions Options, int TileIndex);
    bool HasVectorFile();
    void SetupObjects(int TileIndex, bool bCropToBounds = false, bool bAllObjects = false);
    
private:
    bool InstantiateBlueprintFromVectorGeometries(VectorGeometrySpawnOptions Options, int TileIndex);
    ULandscapeSplineControlPoint* AddControlPoint(FVector Point, ALandscape* Landscape, ULandscapeSplinesComponent* LandscapeSplinesComponent, VectorGeometrySpawnOptions Options, double zValue);
    void AddSegment(ULandscapeSplineControlPoint* Start, ULandscapeSplineControlPoint* End, bool bAutoRotateStart = true, bool bAutoRotateEnd = true);
    UWorld* GetWorld();
    UGISFileManager* GisFM = nullptr;
    VectorTileFactory* TileFactory = nullptr;
    TArray<int> TilesSetupFinished = TArray<int>();
};
