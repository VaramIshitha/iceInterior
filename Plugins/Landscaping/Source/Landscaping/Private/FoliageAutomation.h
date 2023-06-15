// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "EngineUtils.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Foliage/Public/InstancedFoliageActor.h"
#include "ProceduralFoliageComponent.h"
#include "ProceduralFoliageSpawner.h"
#include "ProceduralFoliageVolume.h"
#include "LandscapeHeightfieldCollisionComponent.h"
#include "LandscapeInfo.h"
#include "UObject/ObjectMacros.h"
#include "FoliageHelper.h"
#include "Builders/CubeBuilder.h"
#include "Engine/LevelBounds.h"
#include "Components/BrushComponent.h"
#include "LandscapingUtils.h"
#include "BSPOps.h"
#include "Misc/ScopedSlowTask.h"


#define NUM_INSTANCE_BUCKETS 10

//
// Painting filtering options
//
struct FFoliageGeometryFilter
{
    bool bAllowLandscape;
    bool bAllowStaticMesh;
    bool bAllowBSP;
    bool bAllowFoliage;
    bool bAllowTranslucent;

    FFoliageGeometryFilter()
        : bAllowLandscape(true)
        , bAllowStaticMesh(false)
        , bAllowBSP(false)
        , bAllowFoliage(false)
        , bAllowTranslucent(false)
    {
    }

    bool operator() (const UPrimitiveComponent* Component) const;
};

typedef TMap<FName, TMap<ULandscapeComponent*, TArray<uint8> > > LandscapeLayerCacheData;
class UProceduralFoliageSpawner;

struct FFoliageAutomationOptions
{
	FFoliageGeometryFilter FoliageGeometryFilter;
	bool bRemoveExistingFoliageVolumes = false;
	bool bReUseExistingFoliageVolume = false;
	bool bAddNewFoliageVolume = true;
	bool bUseLandscapeProxyBounds = false;
	bool bSpawnFoliage = true;
};

class FFoliageAutomation
{
public:	
	
	FFoliageAutomation();
	FIntVector GetCurrentLevelOffset();
	TArray<UProceduralFoliageComponent*> AddProceduralFoliageVolume(UProceduralFoliageSpawner* Spawner, FFoliageAutomationOptions InOptions, FBox LandscapeBounds, int TileNum = 0);
	bool GenerateFoliage(FFoliageAutomationOptions InOptions, TArray<UProceduralFoliageComponent*> ProceduralFoliageComponents);
	void SetCurrentLevel(ULevelStreamingDynamic* CurrentLevel);

	TSubclassOf<AProceduralFoliageVolume> FoliageVolumeToSpawn = AProceduralFoliageVolume::StaticClass();
	
private:

	bool IsWithinSlopeAngle(float NormalZ, float MinAngle, float MaxAngle, float Tolerance);
	bool CheckLocationForPotentialInstance_ThreadSafe(const UFoliageType* Settings, const FVector& Location, const FVector& Normal);
	bool LandscapeLayerCheck(const FHitResult& Hit, const UFoliageType* Settings, LandscapeLayerCacheData& LandscapeLayersCache, float& OutHitWeight);
	bool IsLandscapeLayersArrayValid(const TArray<FName>& LandscapeLayersArray);
	bool GetMaxHitWeight(const FVector& Location, UActorComponent* ActorComponent, const TArray<FName>& LandscapeLayersArray, LandscapeLayerCacheData* LandscapeLayerCaches, float& OutMaxHitWeight);
	bool IsFilteredByWeight(float Weight, float TestValue, bool bExclusionTest);
	void CreateBrushForVolumeActor(AVolume* NewActor, UBrushBuilder* BrushBuilder);
	UWorld* GetWorld() const;

	ULevelStreamingDynamic* LoadedLevel = nullptr;
	TArray<ULevel*> CurrentFoliageTraceBrushAffectedLevels = TArray<ULevel*>();
};
