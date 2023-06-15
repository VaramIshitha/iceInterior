// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "Defines.h"
#include <regex>
#include "CoreMinimal.h"
#include "ALandscapingInfos.h"
#include "VectorImporter.h"
#include "WeightmapImporter.h"
#include "RasterImporter.h"
#include "FoliageAutomation.h"
#include "CoordinateReferenceSystem.h"
#include "LandscapingUtils.h"
#include "LandscapingTileLoader.h"
#include "VectorFile.h"
#include "ILandscapingModuleInterface.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "EditorLevelUtils.h"
#include "LandscapingSettings.h"
#include "EditorSubsystem.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "GISFileManager.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FLandscapingMapboxFinishedDelegate, FDataLoadResult&);

class FLandscapingTileLoader;
class VectorImporter;
struct VectorGeometrySpawnOptions;

UCLASS()
class LANDSCAPING_API UGISFileManager : public UEditorSubsystem
{
    GENERATED_BODY()
    
public:
    TArray<TSharedPtr<FString>> MaterialLayers = TArray<TSharedPtr<FString>>();
    TArray<FLandscapingDataSourceDelegate> DataSourceDelegateHandle;
    FLandscapingMapboxFinishedDelegate MapboxFinishedDelegateHandle;

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
    void Init(bool bReset = true);
    void Cleanup(bool bClearVectorData = false);
    // import files
    FDataLoadResult OpenFileDialog(const FString& DialogTitle, const FString& DefaultPath, const FString& FileTypes, TArray<FString>& OutFilenames, FLandscapingMapboxFinishedDelegate InMapboxFinishedDelegateHandle, int ZoomLevel = -1);
    FDataLoadResult FetchFromMapbox(ELandscapingRequestDataType Type, int ZoomLevel = -1);
    FString OpenFolderDialog(const FString& DialogTitle, const FString& DefaultPath);
    // import DTMs and Vectordata for weightmaps
    void CreateTerrains(UMaterialInterface* Material, TArray<TSharedPtr<MaterialLayerSettings>> LayerSettings, RasterImportOptions Options, FLandscapingMapboxFinishedDelegate InMapboxFinishedDelegateHandle);
    // import satellite data
    void CreateSatelliteImagery(UMaterialInterface* Material, RasterImportOptions Options);
    // import Vectordata for weightmaps (create/update weightmaps)
    void CreateWeightmaps(UMaterialInterface* Material, TArray<TSharedPtr<MaterialLayerSettings>> LayerSettings, FString DefaultLayer);
    // import Vectordata for splines / blueprints
    void CreateBlueprints(VectorGeometrySpawnOptions Options);
    // generate procedural foliage
    void CreateFoliage(UProceduralFoliageSpawner* Spawner, FFoliageAutomationOptions InOptions);
    bool HasLandscape(int MinTiles = 1);
    bool HasLandscapeSM();
    bool HasVectorData();
    bool HasRasterData();
    void SetShapeFClass(FString FClassName);
    float GetOffsetFromGround();
    void SetOffsetFromGround(float Offset);
    FString GetAllowedDTMFileTypes();
    bool ExtentsValid();
    void SetIsWorldPartition(bool bIsWorldPartition);
    bool GetIsWorldPartition();
    UMaterialInterface* GetLandscapeMaterial();
    bool IsImportedThroughLandscaping();
    void CreateDummyLandscape();
    void HandleDataFetched(TArray<RasterData>& InData, int TileIndex);
    void HandleMapChange(uint32 MapChangeFlags = 0);
    void HandleMapOpened(const FString& Arg1, bool Arg2);
    void HandleNewCurrentLevel();
    void SetCacheDirectory(FString Dir);
    void WriteLandscapingInfosTxt(FString TxtPath);
    ALandscapingInfos* GetInfos();
    UCoordinateReferenceSystem* GetCRS();
    UWorld* GetCurrentWorld();
    FReply InspectDTM();
    FReply SpawnGeoReferencingSystem();
    bool ReadGeoReferencingSystem();
    double GetAvgMeterPerPixel();
    bool HasMapboxExtension();

private:
    ALandscapingInfos* Infos = nullptr;
    UWorld* CurrentWorld = nullptr;
    FDelegateHandle MapChangeSubscription;
    FDelegateHandle MapOpenedSubstription;
    FDelegateHandle NewCurrentLevelSubscription;
    bool CheckSelectedTiles();
    void ShowNotification(FString Message = FString(), SNotificationItem::ECompletionState State = SNotificationItem::ECompletionState::CS_Success);
    FDataLoadResult HandleImport();
    void FinishMapboxData(FDataLoadResult ResultInfo);

    string NUMBER_REGEX = "^(-?)(0|([1-9][0-9]*))(\\.[0-9]+)?$";
    RasterImporter* TerrainCreator = nullptr;
    VectorImporter* BlueprintCreator = nullptr;
    WeightmapImporter* WeightmapCreator = nullptr;
    FLandscapingTileLoader* TileLoader = nullptr;
    FFoliageAutomation* FoliageAutomation = nullptr;
    bool bInitializing = false;
    UMaterialInterface* MaterialCached = nullptr;
    TArray<TSharedPtr<MaterialLayerSettings>> LayerSettingsCached = TArray<TSharedPtr<MaterialLayerSettings>>();
    RasterImportOptions RasterImportOptionsCached = RasterImportOptions();
    bool bVectorDataForWeightmaps = true;
    TArray<int> SelectedTilesCached = TArray<int>();
    int FinishedMapboxRequests = 0;
    int TotalMapboxRequests = 0;
    TMap<int, TArray<RasterData>> FetchedData = TMap<int, TArray<RasterData>>();
};
