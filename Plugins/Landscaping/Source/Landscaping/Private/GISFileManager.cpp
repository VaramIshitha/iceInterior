// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#include "GISFileManager.h"
#include "GeoReferencingSystem.h"

void UGISFileManager::Initialize(FSubsystemCollectionBase& Collection)
{   
	
}

void UGISFileManager::Deinitialize()
{
	Cleanup();
}

void UGISFileManager::HandleMapChange(uint32 MapChangeFlags)
{
	UE_LOG(LogTemp, Log, TEXT("Landscaping: Handle Map Change... reinitializing Landscaping"));
	CurrentWorld = nullptr;
	Infos = nullptr;
	Init();
}

void UGISFileManager::HandleMapOpened(const FString& Arg1, bool Arg2)
{
	UE_LOG(LogTemp, Log, TEXT("Landscaping: Handle Map Opened... reinitializing Landscaping"));
	CurrentWorld = nullptr;
	Infos = nullptr;
	Init();
}

void UGISFileManager::HandleNewCurrentLevel()
{
	UE_LOG(LogTemp, Log, TEXT("Landscaping: Handle New Current Level... reinitializing Landscaping"));
	CurrentWorld = nullptr;
	Infos = nullptr;
	Init();
}

void UGISFileManager::Init(bool bReset)
{
	GetCurrentWorld();
	if (!this->MapChangeSubscription.IsValid()) 
	{
		UE_LOG(LogTemp, Log, TEXT("Landscaping: Subscribing to Map Change..."));
		this->MapChangeSubscription = FEditorDelegates::MapChange.AddUObject(this, &UGISFileManager::HandleMapChange);
	}
	if (!this->MapOpenedSubstription.IsValid()) 
	{
		this->MapOpenedSubstription = FEditorDelegates::OnMapOpened.AddUObject(this, &UGISFileManager::HandleMapOpened);
	}
	if (!this->NewCurrentLevelSubscription.IsValid()) 
	{
		this->NewCurrentLevelSubscription = FEditorDelegates::NewCurrentLevel.AddUObject(this, &UGISFileManager::HandleNewCurrentLevel);
	}
	//UE_LOG(LogTemp, Log, TEXT("Landscaping: Initializing LandscapingInfos in Level..."));
	if(CurrentWorld != nullptr) 
	{
		// Find existing LandscapingInfos Actor
		TArray<ALandscapingInfos*> LandscapingInfoActors;
		LandscapingUtils::FindAllActors<ALandscapingInfos>(CurrentWorld, LandscapingInfoActors);
		
		if(LandscapingInfoActors.Num() == 0)
		{
			// Create a new one
			UE_LOG(LogTemp, Log, TEXT("Landscaping: Could not find LandscapingInfos Actor: It's a new World."));
			Infos = CurrentWorld->SpawnActor<ALandscapingInfos>(FVector(0), FRotator(0));
			Infos->GetCRS()->SetAuthorityID(0);
#if ENGINE_MAJOR_VERSION > 4
			Infos->SetIsSpatiallyLoaded(false);
#endif
			Infos->bDrawVectorDataDebug = true;
			Infos->bSnapToGround = true;
			Infos->DebugUpdateInterval = 1;
		}
		else if(LandscapingInfoActors.Num() == 1)
		{
			Infos = LandscapingInfoActors[0];
		}
		else
		{
			FString Error("Multiple LandscapingInfos Actors found.\nPlease load only the Persistent Level and unload all other Levels.");
			UE_LOG(LogTemp, Error, TEXT("Landscaping: %s"), *Error);
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(*Error));
			Infos = nullptr;
			return;
		}
		if(!bReset)
		{
			TileLoader->RefreshTileList();
			return;
		}
		UE_LOG(LogTemp, Log, TEXT("Landscaping: Resetting temporary values in LandscapingInfos Actor"));
		for(int TileIndex = 0; TileIndex < Infos->Tiles.Num(); TileIndex++)
		{
			Infos->Tiles[TileIndex].LandcoverShapes.Empty();
			Infos->Tiles[TileIndex].SplineGeometries.Empty();
		}
		ULandscapingSettings* Settings = GetMutableDefault<ULandscapingSettings>();
		TerrainCreator = new RasterImporter(this, Settings->CacheDirectory);
		BlueprintCreator = new VectorImporter(this);
		WeightmapCreator = new WeightmapImporter(this);
		TileLoader = new FLandscapingTileLoader(this);
		FoliageAutomation = new FFoliageAutomation();
		TileLoader->RefreshTileList();
		Infos->Refresh();
		TileLoader->LoadWorldPartition();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Landscaping: No World found. Please load a level first. (Ignore this message on startup)"));
	}
}


void UGISFileManager::SetCacheDirectory(FString Dir)
{
	if(TerrainCreator != nullptr)
	{
		if(!FPaths::DirectoryExists(Dir))
		{
			IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
			if(!PlatformFile.CreateDirectoryTree(*Dir))
			{
				FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("Please set the Cache Directory in Project Settings -> Plugins -> Landscaping.\nMake sure it is writable."));
				return;
			}
		}
		TerrainCreator->SetWorkingDir(Dir);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Landscaping: Initialization Error - could not set Cache Directory in TerrainCreator"));
	}
}

void UGISFileManager::Cleanup(bool bClearVectorData)
{
	if (this->MapChangeSubscription.IsValid()) 
	{
		FEditorDelegates::MapChange.Remove(this->MapChangeSubscription);
		this->MapChangeSubscription.Reset();
	}
	if (this->MapOpenedSubstription.IsValid()) 
	{
		FEditorDelegates::OnMapOpened.Remove(this->MapOpenedSubstription);
		this->MapOpenedSubstription.Reset();
	}
	if (this->NewCurrentLevelSubscription.IsValid()) 
	{
		FEditorDelegates::NewCurrentLevel.Remove(this->NewCurrentLevelSubscription);
		this->NewCurrentLevelSubscription.Reset();
	}
	if(bClearVectorData)
	{
		for(int TileIndex = 0; TileIndex < Infos->Tiles.Num(); TileIndex++)
		{
			Infos->Tiles[TileIndex].LandcoverShapes.Empty();
			Infos->Tiles[TileIndex].SplineGeometries.Empty();
		}
	}
}

FDataLoadResult UGISFileManager::OpenFileDialog(const FString& DialogTitle, const FString& DefaultPath, const FString& FileTypes, TArray<FString>& OutFilenames, FLandscapingMapboxFinishedDelegate InMapboxFinishedDelegateHandle, int ZoomLevel)
{
	Init();
	MapboxFinishedDelegateHandle.Clear();
	FetchedData.Empty();
	if((FileTypes.Equals(VECTOR_FILE) || FileTypes.Equals(LANDUSE_FILE)) && HasMapboxExtension() && GetCRS()->GetAuthorityID() != 0)
	{
		bVectorDataForWeightmaps = FileTypes.Equals(LANDUSE_FILE);
		EAppReturnType::Type Answer = FMessageDialog::Open(
			EAppMsgType::YesNo,
			FText::FromString("Automatic load Vector Tiles from Mapbox?\n\nYes = Load from Mapbox\nNo = Load from files")
		);
		if(Answer == EAppReturnType::Yes)
		{
			MapboxFinishedDelegateHandle = InMapboxFinishedDelegateHandle;
			return FetchFromMapbox(ELandscapingRequestDataType::VECTOR, ZoomLevel);
		}
	}

	FDataLoadResult Result;
    void* ParentWindowPtr = FSlateApplication::Get().GetActiveTopLevelWindow()->GetNativeWindow()->GetOSWindowHandle();
    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
    if (DesktopPlatform == nullptr)
    {
		return FDataLoadResult("Init Desktop Platform failed");
	}

	const uint32 SelectionFlag = 1;
	if (DesktopPlatform->OpenFileDialog(ParentWindowPtr, DialogTitle, DefaultPath, FString(""), FileTypes,
										SelectionFlag, OutFilenames))
	{
		if (FileTypes.Equals(VECTOR_FILE))
		{
			for(int TileIndex = 0; TileIndex < GetInfos()->Tiles.Num(); TileIndex++)
			{
				GetInfos()->Tiles[TileIndex].SplineGeometries.Empty();
			}
			if(BlueprintCreator == nullptr)
			{
				BlueprintCreator = new VectorImporter(this);
			}
			if(!CheckSelectedTiles())
			{
				return FDataLoadResult("Please select the Terrain Actors in the Outliner on which the process should be performed.");
			}
			for(int TileIndex : SelectedTilesCached)
			{
				BlueprintCreator->LoadFiles(OutFilenames, TileIndex);
			}
			Result.VectorFeatureClasses = BlueprintCreator->GetFeatureClasses();
			if(Result.VectorFeatureClasses.IsEmpty())
			{
				ShowNotification("No Shapes found.\nPlease see Output Log for details.", SNotificationItem::ECompletionState::CS_Fail);
			}
		}
		else if (FileTypes.Equals(LANDUSE_FILE))
		{	
			for(int TileIndex = 0; TileIndex < GetInfos()->Tiles.Num(); TileIndex++)
			{
				GetInfos()->Tiles[TileIndex].LandcoverShapes.Empty();
			}
			if(WeightmapCreator == nullptr)
			{
				WeightmapCreator = new WeightmapImporter(this);
			}  
			if(!CheckSelectedTiles())
			{
				return FDataLoadResult("Please select the Terrain Actors in the Outliner on which the process should be performed.");
			}
			for(int TileIndex : SelectedTilesCached)
			{
				if(GetInfos()->Tiles[TileIndex].Landscape != nullptr)
				{
					WeightmapCreator->LoadFiles(OutFilenames, TileIndex);
				}
			}
			Result.VectorFeatureClasses = WeightmapCreator->GetFeatureClasses();
			if(Result.VectorFeatureClasses.IsEmpty())
			{
				ShowNotification("No Shapes found.\nPlease see Output Log for details.", SNotificationItem::ECompletionState::CS_Fail);
			}
		}
		else if(FileTypes.Equals("JPEG 2000|*.jp2;*.tif;*.tiff;"))
		{
			if(!CheckSelectedTiles())
			{
				return FDataLoadResult("Please select the Terrain Actors in the Outliner on which the process should be performed.");
			}
			if(TerrainCreator == nullptr)
			{
				ULandscapingSettings* Settings = GetMutableDefault<ULandscapingSettings>();
				TerrainCreator = new RasterImporter(this, Settings->CacheDirectory);
			}
			for(int TileIndex : SelectedTilesCached)
			{
				Result.ErrorMsg = TerrainCreator->LoadFiles(OutFilenames, TileIndex);
				if(!Result.ErrorMsg.IsEmpty())
				{
					UE_LOG(LogTemp, Warning, TEXT("Landscaping: Resetting Satellite Creator"));
					ULandscapingSettings* Settings = GetMutableDefault<ULandscapingSettings>();
					TerrainCreator = new RasterImporter(this, Settings->CacheDirectory);
					break;
				}
			}
		}
		else // heightdata
		{
			if(TerrainCreator == nullptr)
			{
				ULandscapingSettings* Settings = GetMutableDefault<ULandscapingSettings>();
				TerrainCreator = new RasterImporter(this, Settings->CacheDirectory);
			}
			Result.ErrorMsg = TerrainCreator->LoadFiles(OutFilenames);
			if(!Result.ErrorMsg.IsEmpty())
			{
				UE_LOG(LogTemp, Warning, TEXT("Landscaping: Resetting Terrain Creator"));
				ULandscapingSettings* Settings = GetMutableDefault<ULandscapingSettings>();
				TerrainCreator = new RasterImporter(this, Settings->CacheDirectory);
			}
		}
	}
	GetInfos()->Refresh();
    return Result;
}

bool UGISFileManager::CheckSelectedTiles()
{
	TArray<int> SelectedTileIndizes = GetInfos()->GetSelectedTiles();
	if(GetInfos()->Tiles.Num() > 1 && GetInfos()->Tiles.Num() == SelectedTileIndizes.Num())
	{
		EAppReturnType::Type Answer = FMessageDialog::Open(
			EAppMsgType::OkCancel,
			FText::FromString("Process for all Landscapes?\n\nYou can import data for one or more landscapes by selecting them in the Outliner.\n\nOk = process for all")
		);
		if(Answer == EAppReturnType::Cancel)
		{
			return false;
		}
	}
	SelectedTilesCached = SelectedTileIndizes;
	if(SelectedTilesCached.IsEmpty())
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("No tiles selected or not loaded.\n\nPlease load the affected areas through World Partion"));
		return false;
	}
	return true;
}

FDataLoadResult UGISFileManager::FetchFromMapbox(ELandscapingRequestDataType Type, int ZoomLevel)
{
	FDataLoadResult Result;
	ILandscapingModuleInterface* MapboxModule = FModuleManager::Get().GetModulePtr<ILandscapingModuleInterface>("LandscapingMapbox");
	RasterImportOptionsCached.bMapboxImport = true;
	ULandscapingSettings* Settings = GetMutableDefault<ULandscapingSettings>();
	FString MapboxDir = Settings->CacheDirectory;
	MapboxDir.Append("/Mapbox");
	if(!FPaths::DirectoryExists(MapboxDir))
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		PlatformFile.CreateDirectoryTree(*MapboxDir);
	}
	if(!CheckSelectedTiles())
	{
		Result.ErrorMsg = FString("No Tiles found");
		return Result;
	}
	TotalMapboxRequests = SelectedTilesCached.Num();
	for(int TileIndex : SelectedTilesCached)
	{
		// do not fetch landuse data for static mesh
		if(bVectorDataForWeightmaps && GetInfos()->Tiles[TileIndex].Landscape == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Landscaping: Skipping Mesh Landscape when fetching data for Landscape Material Areas (Weightmaps for Landscape Material can only be created on Landscapes)"));
			continue;
		}
		ILandscapingDataSource* DataSource = MapboxModule->GetDataSource();
		if(DataSource == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("Landscaping: Mapbox DataSource not found"));
			Result.ErrorMsg = FString("Mapbox Data Source not found");
			return Result;
		}
		if(!DataSource->IsValid())
		{
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("No Mapbox API key set.\nPlease set it in Project Settings -> Plugins -> Landscaping Mapbox."));
			Result.ErrorMsg = FString("Mapbox API key not set. Set it in 'Project Settings -> Plugins -> Landscaping Mapbox'.");
			return Result;
		}
		DataSource->SetWorkingDir(MapboxDir);
		FExtents MapboxExtents = GetCRS()->ConvertToGeogCS(GetInfos()->Tiles[TileIndex].Extents);
		DataSource->SetExtents(MapboxExtents.Bottom, MapboxExtents.Left, MapboxExtents.Top, MapboxExtents.Right, TileIndex);
		FLandscapingDataSourceDelegate DSDelegate;
		DSDelegate.Clear();
		DSDelegate.AddUObject(this, &UGISFileManager::HandleDataFetched);
		DataSourceDelegateHandle.Add(DSDelegate);
		DataSource->FetchData(DSDelegate, Type, ZoomLevel);
	}
	Result.StatusMsg = FString("Fetching from Mapbox");
	return Result;
}

FString UGISFileManager::OpenFolderDialog(const FString& DialogTitle, const FString& DefaultPath)
{
    void* ParentWindowPtr = FSlateApplication::Get().GetActiveTopLevelWindow()->GetNativeWindow()->GetOSWindowHandle();
    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	FString* OutFolderName = new FString();
    if (DesktopPlatform)
    {
        const uint32 SelectionFlag = 1;
		if(DesktopPlatform->OpenDirectoryDialog(ParentWindowPtr, DialogTitle, DefaultPath, *OutFolderName))
		{
			return *OutFolderName;
		}
	}
	return *OutFolderName;
}

// satellite import
void UGISFileManager::CreateSatelliteImagery(UMaterialInterface* Material, RasterImportOptions Options)
{
	Init(false);
	// delete SatelliteFilenames so we can get new data
	for(int i = 0; i < GetInfos()->Tiles.Num(); i++)
	{
		GetInfos()->Tiles[i].SatelliteFilenames.Empty();
	}

	// clear delegate handle because we do not need it for satellite imagery
	MapboxFinishedDelegateHandle.Clear();
	FetchedData.Empty();
	if(HasMapboxExtension())
	{
		EAppReturnType::Type Answer = FMessageDialog::Open(
			EAppMsgType::YesNo,
			FText::FromString("Automatic load Satellite Images from Mapbox?\n\nYes = Load from Mapbox\nNo = Load from files")
		);
		if(Answer == EAppReturnType::Yes)
		{
			MaterialCached = Material;
			RasterImportOptionsCached = Options;
			FetchFromMapbox(ELandscapingRequestDataType::SATELLITE, Options.ZoomSatellite);
			return;
		}
	}

	TArray<FString> OutFilenames;
	OpenFileDialog(FString("Open Satellite or NAPI Images"), FString(), FString("JPEG 2000|*.jp2;*.tif;*.tiff;"), OutFilenames, MapboxFinishedDelegateHandle);
	
	if(!CheckSelectedTiles())
	{
		return;
	}	
	bool bApplySuccess = false;
	for(int TileIndex : SelectedTilesCached)
	{
		bApplySuccess = TerrainCreator->CreateSatelliteImages(TileIndex, Options);
		if(!bApplySuccess)
		{
			UE_LOG(LogTemp, Error, TEXT("Landscaping: No satellite image for Tile %i"), TileIndex);
			break;
		}
	}
	if(bApplySuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("Landscaping: Import satellite images successful"));
		GetInfos()->MarkPackageDirty();
		ShowNotification();
	}
	else
	{
		ShowNotification("Operation interrupted. Please see Output Log for details.", SNotificationItem::ECompletionState::CS_Fail);
	}
}

void UGISFileManager::CreateTerrains(UMaterialInterface* Material, TArray<TSharedPtr<MaterialLayerSettings>> LayerSettings, RasterImportOptions Options, FLandscapingMapboxFinishedDelegate InMapboxFinishedDelegateHandle)
{
	Init(false);
	if(TerrainCreator != nullptr && (TerrainCreator->HasRasterFile() || HasMapboxExtension()))
	{
		ULandscapingSettings* Settings = GetMutableDefault<ULandscapingSettings>();
		if(GetInfos()->Tiles.IsEmpty() && Options.bHighDetailZScale)
		{
			GetInfos()->bHighDetailZScale = true;
			GetInfos()->CustomZScale = Options.ZScale;
		}
		bool bApplySuccess = false;
		if(TerrainCreator->HasRasterFile())
		{
			bApplySuccess = TerrainCreator->CreateTerrains(Material, LayerSettings, WeightmapCreator, Options);
		}
		else if(HasMapboxExtension())
		{
			MapboxFinishedDelegateHandle = InMapboxFinishedDelegateHandle;
			MaterialCached = Material;
			LayerSettingsCached = LayerSettings;
			RasterImportOptionsCached = Options;
			UE_LOG(LogTemp, Log, TEXT("Landscaping: Heightmap import using LandscapingMapbox"));
			ILandscapingModuleInterface* MapboxModule = FModuleManager::Get().GetModulePtr<ILandscapingModuleInterface>("LandscapingMapbox");
			Options.DataSource = MapboxModule->GetDataSource();
			if(Options.DataSource == nullptr)
			{
				UE_LOG(LogTemp, Error, TEXT("Landscaping: Mapbox DataSource not found"));
				return;
			}
			if(!Options.DataSource->IsValid())
			{
				FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("No Mapbox API key set.\nPlease set it in Project Settings -> Plugins -> Landscaping Mapbox."));
				return;
			}
			RasterImportOptionsCached.bMapboxImport = true;
			FString MapboxDir = Settings->CacheDirectory;
			MapboxDir.Append("/Mapbox");
			if(!FPaths::DirectoryExists(MapboxDir))
			{
				UE_LOG(LogTemp, Error, TEXT("Landscaping: Creating Mapbox Cache Dir at %s"), *MapboxDir);
				IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
				PlatformFile.CreateDirectoryTree(*MapboxDir);
			}
			Options.DataSource->SetWorkingDir(MapboxDir);
			FExtents MapboxExtents = GetInfos()->GetCroppedExtents();
			Options.DataSource->SetExtents(MapboxExtents.Bottom, MapboxExtents.Left, MapboxExtents.Top, MapboxExtents.Right);
			FLandscapingDataSourceDelegate DSDelegate;
			DSDelegate.Clear();
			DSDelegate.AddUObject(this, &UGISFileManager::HandleDataFetched);
			DataSourceDelegateHandle.Add(DSDelegate);
			TotalMapboxRequests++;
			Options.DataSource->FetchData(DSDelegate, ELandscapingRequestDataType::TERRAIN, Options.ZoomLevel);
			return;
		}
		
		if(bApplySuccess)
		{
			GetInfos()->bImportedThroughLandscaping = true;
			GetInfos()->UpdateLandscapingBounds();
			GetInfos()->Material = Material;
			GetInfos()->RootProjection = GetCRS()->GetAuthorityIDStr();
			GetInfos()->RootWkt = GetCRS()->GetWkt();
			GetInfos()->MarkPackageDirty();
			delete TerrainCreator;
			TerrainCreator = new RasterImporter(this, Settings->CacheDirectory);
			ShowNotification();
		}
		else
		{
			ShowNotification("Operation interrupted. Please see Output Log for details.", SNotificationItem::ECompletionState::CS_Fail);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Landscaping: No Landscape to import"));
	}
}

void  UGISFileManager::CreateWeightmaps(UMaterialInterface* Material, TArray<TSharedPtr<MaterialLayerSettings>> LayerSettings, FString DefaultLayer)
{
	Init(false);
	if(GetInfos()->Tiles.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Landscaping: Select or import a heightmap first"));
		return;
	}
	if(WeightmapCreator == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Landscaping: Initialization failed. Close Landscaping tab and open it again"));
		return;
	}
	
	bool bApplySuccess = false;
	if(CheckSelectedTiles())
	{
		if(SelectedTilesCached.IsEmpty())
		{
			return;
		}
		FScopedSlowTask SlowTask(SelectedTilesCached.Num(), FText::FromString("Processing Tiles"));
		SlowTask.MakeDialog();
		WeightmapCreator->ClearLayers(SelectedTilesCached);
		for(int TileIndex : SelectedTilesCached)
		{
			FString ProgressStr = FString::Printf(TEXT("Processing Tile %i / %i"), (TileIndex + 1), GetInfos()->Tiles.Num());
			UE_LOG(LogTemp, Log, TEXT("Landscaping: Weightmaps - %s"), *ProgressStr);
			SlowTask.EnterProgressFrame(1.0, FText::FromString(ProgressStr));
			if(TileLoader->LoadWorldPartition(TileIndex))
			{
				if(GetInfos()->Tiles[TileIndex].Landscape != nullptr)
				{
					bApplySuccess = WeightmapCreator->CreateWeightmaps(Material, LayerSettings, TileIndex, DefaultLayer);
					if(bApplySuccess)
					{
						TileLoader->UnloadWorldPartition(TileIndex);
					}
					else
					{
						SlowTask.Destroy();
						ShowNotification("Operation interrupted. Please see Output Log for details.", SNotificationItem::ECompletionState::CS_Fail);
						break;
					}
				}
			}
		}
	}
	UE_LOG(LogTemp, Log, TEXT("Landscaping: Weightmaps finished %s"), bApplySuccess ? *FString("successfully") : *FString("with failure"));
	if(bApplySuccess)
	{
		ShowNotification();
		GetInfos()->Material = Material;
		GetInfos()->MarkPackageDirty();
	}
}

void UGISFileManager::CreateBlueprints(VectorGeometrySpawnOptions Options)
{
	Init(false);
	if(BlueprintCreator != nullptr && BlueprintCreator->HasVectorFile() && (Options.SplineMesh != nullptr || Options.ActorOrBlueprintClass != nullptr || Options.LandscapeSplineOptions.bPaintMaterialLayer))
	{
		bool bApplySuccess = false;
		FScopedSlowTask SlowTask(SelectedTilesCached.Num(), FText::FromString("Processing Tiles"));
		SlowTask.MakeDialog();
		for(int TileIndex : SelectedTilesCached)
		{
			if(Options.PaintLayer.Get() != nullptr && !Options.PaintLayer.Get()->IsEmpty())
			{
				UEditorLoadingAndSavingUtils::SaveCurrentLevel();
				if(GetInfos()->Tiles[TileIndex].Landscape != nullptr)
				{
					UE_LOG(LogTemp, Log, TEXT("Landscaping: Creating Paint Layer %s for Tile %i"), *Options.LandscapeSplineOptions.EditLayerName.ToString(), TileIndex);
					Options.LandscapeSplineOptions.PaintLayer = WeightmapCreator->GetLandscapeLayerInfoObject(Options.PaintLayer, TileIndex);
				}
			}
			if(Options.bCropToBounds)
			{
				BlueprintCreator->SetupObjects(TileIndex, true);
			}
			FString ProgressStr = FString::Printf(TEXT("Processing Tile %i / %i"), (TileIndex + 1), GetInfos()->Tiles.Num());
			UE_LOG(LogTemp, Log, TEXT("Landscaping: Actor from Vector data - %s"), *ProgressStr);
			SlowTask.EnterProgressFrame(1, FText::FromString(ProgressStr));
			if(TileLoader->LoadWorldPartition(TileIndex))
			{
				bApplySuccess = BlueprintCreator->CreateBlueprints(Options, TileIndex);
				if(bApplySuccess)
				{
					if(GetInfos()->Tiles[TileIndex].Landscape != nullptr)
					{
						ALandscape* Landscape = (ALandscape*)GetInfos()->Tiles[TileIndex].Landscape;
						FLandscapeLayer* CurrentLayer = Landscape->GetLayer(0);
						FGuid Guid = CurrentLayer ? CurrentLayer->Guid : FGuid();
						FScopedSetLandscapeEditingLayer Scope(Landscape, Guid, [&] { check(Landscape); Landscape->RequestLayersContentUpdate(ELandscapeLayerUpdateMode::Update_Weightmap_All); });
						Landscape->EditorLayerSettings.Add(FLandscapeEditorLayerSettings(Options.LandscapeSplineOptions.PaintLayer));
						FLandscapeEditDataInterface LandscapeEdit(Landscape->GetLandscapeInfo());
						LandscapeEdit.Flush();
						GetInfos()->Tiles[TileIndex].Landscape->MarkPackageDirty();
					}
					TileLoader->UnloadWorldPartition(TileIndex);
				}
				else 
				{
					SlowTask.Destroy();
					ShowNotification("Operation interrupted. Please see Output Log for details.", SNotificationItem::ECompletionState::CS_Fail);
					break;
				}
			}
		}
		UE_LOG(LogTemp, Log, TEXT("Landscaping: Actor from Vector data finished %s"), bApplySuccess ? *FString("successfully") : *FString("with failure"));
		if(bApplySuccess)
		{
			ShowNotification();
		}
	}
	else if(BlueprintCreator == nullptr || !BlueprintCreator->HasVectorFile())
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("Please select files first."));
	}
	else if(Options.SplineMesh == nullptr && Options.ActorOrBlueprintClass == nullptr && !Options.LandscapeSplineOptions.bPaintMaterialLayer) 
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("No Mesh or Actor or Paint Material Layer selected."));
	}
	else 
	{
		UE_LOG(LogTemp, Error, TEXT("Landscaping: Could not import"));
	}
}

void UGISFileManager::CreateFoliage(UProceduralFoliageSpawner* Spawner, FFoliageAutomationOptions InOptions)
{
	Init(false);
	if(FoliageAutomation == nullptr || GetInfos()->Tiles.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Landscaping: Could not generate Foliage. Initialization failed or no Landscape found."));
		return;
	}
	bool bApplySuccess = true;
    if(!CheckSelectedTiles())
	{
		return;
	}
	for(int TileIndex : SelectedTilesCached)
	{
		if(TileLoader->LoadWorldPartition(TileIndex))
		{
			TArray<FBox> ProxyBounds = TArray<FBox>();
			if(GetInfos()->bWorldPartition && InOptions.bUseLandscapeProxyBounds)
			{
				TArray<ALandscapeStreamingProxy*> LandscapeProxies;
				LandscapingUtils::FindAllActors<ALandscapeStreamingProxy>(CurrentWorld, LandscapeProxies);
				for(ALandscapeStreamingProxy* Proxy : LandscapeProxies)
				{
					if(Proxy->GetLandscapeActor() == GetInfos()->Tiles[TileIndex].Landscape)
					{
						ProxyBounds.Add(Proxy->GetComponentsBoundingBox(false, false));
					}
				}
			}
			else
			{
				ProxyBounds.Add(GetInfos()->Tiles[TileIndex].Bounds);
			}
			TArray<TArray<UProceduralFoliageComponent*>> ProceduralFoliageComponents;

			for(FBox InBounds : ProxyBounds)
			{
				TArray<UProceduralFoliageComponent*> ProcFoliageComp = FoliageAutomation->AddProceduralFoliageVolume(Spawner, InOptions, InBounds, TileIndex);
				if(ProcFoliageComp.Num() > 0 && InOptions.bSpawnFoliage)
				{
					bApplySuccess = bApplySuccess && FoliageAutomation->GenerateFoliage(InOptions, ProcFoliageComp);
				}
			}
			TileLoader->UnloadWorldPartition(TileIndex);
		}
    }
	if(bApplySuccess)
	{
		ShowNotification("Operation successful");
	}
	else 
	{
		ShowNotification("Operation interrupted. Please see Output Log for further details.", SNotificationItem::ECompletionState::CS_Fail);
	}
}

void UGISFileManager::ShowNotification(FString Message, SNotificationItem::ECompletionState State)
{
	Message = Message.IsEmpty() ? "Import successful! Please save the Level" : Message;
	Message = FString::Printf(TEXT("Landscaping\n%s"), *Message);
	FNotificationInfo Info(FText::FromString(*Message));
	Info.bUseLargeFont = false;
	Info.bFireAndForget = true;
	Info.bUseThrobber = false;
	Info.bUseSuccessFailIcons = true;
	auto NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
	NotificationItem->SetCompletionState(State);
}

bool UGISFileManager::HasLandscape(int MinTiles)
{
	if(TileLoader != nullptr)
	{
		TileLoader->RefreshTileList();
	}
	return Infos != nullptr && GetInfos()->Tiles.Num() >= MinTiles;
}

bool UGISFileManager::HasLandscapeSM()
{
	if(TileLoader != nullptr)
	{
		TileLoader->RefreshTileList();
	}
	if(Infos != nullptr && GetInfos()->Tiles.Num() >= 1)
	{
		TArray<int> SelectedTiles = GetInfos()->GetSelectedTiles();
		for(int TileIndex : SelectedTiles)
		{
			if(GetInfos()->Tiles[TileIndex].LandscapeSM != nullptr)
			{
				return true;
			}
		}
	}
	return false;
}

bool UGISFileManager::HasRasterData()
{
	return TerrainCreator != nullptr && TerrainCreator->HasRasterFile();
}

bool UGISFileManager::HasVectorData()
{
	return BlueprintCreator != nullptr && BlueprintCreator->HasVectorFile();
}

FString UGISFileManager::GetAllowedDTMFileTypes()
{
	ULandscapingSettings* Settings = GetMutableDefault<ULandscapingSettings>();
	return Settings->AllowedDTMFileTypes;
}

void UGISFileManager::SetIsWorldPartition(bool bIsWorldPartition)
{
	if(bIsWorldPartition && CurrentWorld != nullptr && !CurrentWorld->GetWorldSettings()->GetWorldPartition())
	{
		UWorldPartition::CreateOrRepairWorldPartition(CurrentWorld->GetWorldSettings());
		if(!CurrentWorld->GetWorldSettings()->GetWorldPartition())
		{
			FMessageDialog::Open(
					EAppMsgType::Ok,
					FText::FromString("World Partition could not be created for this Level.\nConsider creating a new Level from Open World Template.")
				);
			return;
		}
	}
	GetInfos()->bWorldPartition = bIsWorldPartition;
}

bool UGISFileManager::GetIsWorldPartition()
{
	if(Infos == nullptr)
	{
		return false;
	}
	if(CurrentWorld != nullptr)
	{
		GetInfos()->bWorldPartition = CurrentWorld->GetWorldPartition() != nullptr;
	}
	return GetInfos()->bWorldPartition;
}

UMaterialInterface* UGISFileManager::GetLandscapeMaterial()
{
	return GetInfos()->Material;
}

void UGISFileManager::SetShapeFClass(FString FClassName)
{
	GetInfos()->ShapeFClass = FClassName;
}

void UGISFileManager::SetOffsetFromGround(float Offset)
{
	GetInfos()->OffsetFromGround = Offset;
}

float UGISFileManager::GetOffsetFromGround()
{
	return GetInfos()->OffsetFromGround;
}

bool UGISFileManager::ExtentsValid()
{
	return !GetInfos()->Extents.IsEmpty();
}

bool UGISFileManager::IsImportedThroughLandscaping()
{
	return GetInfos()->bImportedThroughLandscaping;
}

void UGISFileManager::CreateDummyLandscape()
{
	if(TerrainCreator != nullptr)
	{
		TerrainCreator->CreateDummyLandscape();
	}
}

void UGISFileManager::HandleDataFetched(TArray<RasterData>& InDatas, int TileIndex)
{
	FinishedMapboxRequests++;
	if(InDatas.IsEmpty())
	{
		FinishMapboxData(FDataLoadResult("Data fetch failed"));
		ShowNotification("Mapbox didn't send valid data.\nPlease see Output Log for details.", SNotificationItem::ECompletionState::CS_Fail);
		return;
	}
    UE_LOG(LogTemp, Log, TEXT("Landscaping: Handle %i Mapbox tiles fetched"), InDatas.Num());
	for(int i = 0; i < InDatas.Num(); i++)
	{
		if(!InDatas[i].Error.IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("Landscaping: %i RasterData Error: %s"), i, *InDatas[i].Error);
			FinishMapboxData(FDataLoadResult("Data fetch failed"));
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString::Printf(TEXT("Mapbox Error:\n\n%s"), *InDatas[i].Error)));
			return;
		}
	}
	UE_LOG(LogTemp, Log, TEXT("Adding %i datasets for Tile %i"), InDatas.Num(), TileIndex);
	FetchedData.Add(TileIndex, InDatas);
	FinishMapboxData(FDataLoadResult());
}

FDataLoadResult UGISFileManager::HandleImport()
{
	FDataLoadResult Result;
	bool bApplySuccess = true;
	bool bVectorDataImport = false;
	for (auto& Elem : FetchedData)
	{
		int TileIndex = Elem.Key;
		TArray<RasterData> InDatas = Elem.Value;
		// check if we are dealing with vector data
		if(InDatas[0].Filename.EndsWith(".pbf"))
		{
			bVectorDataImport = true;
			TArray<FString> OutFilenames;
			for(int i = 0; i < InDatas.Num(); i++)
			{
				OutFilenames.Add(InDatas[i].Filename);
			}

			if(bVectorDataForWeightmaps)
			{
				if(WeightmapCreator == nullptr)
				{
					UE_LOG(LogTemp, Error, TEXT("Landscaping: Weightmap Importer not properly initialized"));
					return FDataLoadResult("Data read failed");
				}
				WeightmapCreator->LoadFiles(OutFilenames, TileIndex);
			}
			else 
			{
				if(BlueprintCreator == nullptr)
				{
					UE_LOG(LogTemp, Error, TEXT("Landscaping: Vector Importer not properly initialized"));
					return FDataLoadResult("Data read failed");
				}
				BlueprintCreator->LoadFiles(OutFilenames, TileIndex);
			}
			continue;
		}
		// height and satellite data
		if(TerrainCreator == nullptr)
		{
			ULandscapingSettings* Settings = GetMutableDefault<ULandscapingSettings>();
			TerrainCreator = new RasterImporter(this, Settings->CacheDirectory);
		}
		TArray<FString> Filenames = TerrainCreator->WriteFiles(InDatas);
		if(Filenames.IsEmpty())
		{
			ShowNotification("Could not write file fetched from Mapbox.\nPlease see Output Log for details.", SNotificationItem::ECompletionState::CS_Fail);
			return FDataLoadResult("Data read failed - no files");
		}
		if(InDatas[0].BandCount == 3)  // satellite data with color
		{
			if(TileIndex >= 0 && TileLoader->LoadWorldPartition(TileIndex))
			{
				GetInfos()->Tiles[TileIndex].SatelliteFilenames = Filenames;
				bApplySuccess = TerrainCreator->CreateSatelliteImages(TileIndex, RasterImportOptionsCached);
				if(!bApplySuccess)
				{
					UE_LOG(LogTemp, Error, TEXT("Landscaping: Apply Satellite Image for Tile %i failed"), TileIndex);	
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Landscaping: TileIndex out of range: %i"), TileIndex);
				bApplySuccess = false;
			}
			Init();
		}
		else // heightdata
		{
			if(!GetCRS()->IsAuthorityIDValid() && GetCRS()->IsModeUseSourceCRS())
			{
				GetCRS()->SetAuthorityID(3857); // use web mercator for mapbox
				UE_LOG(LogTemp, Log, TEXT("Landscaping: Set Authority ID for Mapbox heightmap import to %i"), GetCRS()->GetAuthorityID());
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("Landscaping: Authority ID already set for Mapbox heightmap import to %i"), GetCRS()->GetAuthorityID());
			}
			FExtents ExtentsCached = GetInfos()->GetCroppedExtents();
			// UE_LOG(LogTemp, Log, TEXT("Landscaping: Load Files with Cropped Extents %s"), *ExtentsCached.ToString());
			TerrainCreator->LoadFiles(Filenames);
			GetInfos()->SetCroppedExtents(ExtentsCached);
			//GetInfos()->SetOrigin(FVector(GetInfos()->Extents.Left, GetInfos()->Extents.Top, 0));
			UE_LOG(LogTemp, Log, TEXT("Landscaping: Loading successful, import Heightdata..."));
			bApplySuccess = TerrainCreator->CreateTerrains(MaterialCached, LayerSettingsCached, WeightmapCreator, RasterImportOptionsCached);
			if(bApplySuccess)
			{
				GetInfos()->bImportedThroughLandscaping = true;
				GetInfos()->UpdateLandscapingBounds();
				GetInfos()->Material = MaterialCached;
				GetInfos()->RootProjection = GetCRS()->GetAuthorityIDStr();
				GetInfos()->RootWkt = GetCRS()->GetWkt();
			}
			Init();
		}
	}
	if(bApplySuccess)
	{
		GetInfos()->MarkPackageDirty();
		if(bVectorDataImport)
		{
			Result.VectorFeatureClasses = bVectorDataForWeightmaps ? WeightmapCreator->GetFeatureClasses() : BlueprintCreator->GetFeatureClasses();
			if(Result.VectorFeatureClasses.IsEmpty())
			{
				ShowNotification("No Shapes found.\nPlease see Output Log for details.", SNotificationItem::ECompletionState::CS_Fail);
			}
			return Result;
		}
		UE_LOG(LogTemp, Log, TEXT("Landscaping: Import successful"));
		return Result;
	}
	ShowNotification("Operation interrupted.\nPlease see Output Log for details.", SNotificationItem::ECompletionState::CS_Fail);
	Result.ErrorMsg = FString("Data fetch error");
	return Result;
}

void UGISFileManager::FinishMapboxData(FDataLoadResult ResultInfo)
{
	UE_LOG(LogTemp, Log, TEXT("Landscaping: FinishedMapboxRequests %i/%i"), FinishedMapboxRequests, TotalMapboxRequests);
	if(FinishedMapboxRequests == TotalMapboxRequests)
	{
		FinishedMapboxRequests = 0;
		TotalMapboxRequests = 0;
		for(auto DSDelegateHandle : DataSourceDelegateHandle)
		{
			DSDelegateHandle.Clear();
		}
		DataSourceDelegateHandle.Empty();
		if(ResultInfo.ErrorMsg.IsEmpty())
		{
			ResultInfo = HandleImport();
		}
		if(MapboxFinishedDelegateHandle.IsBound())
		{
			MapboxFinishedDelegateHandle.Broadcast(ResultInfo);
		}
	}
}

ALandscapingInfos* UGISFileManager::GetInfos()
{
	// we check again, if we have to spawn the LandscapingInfos Actor
	if(Infos == nullptr)
	{
		// Create a new one
		UE_LOG(LogTemp, Log, TEXT("Landscaping: Could not find LandscapingInfos Actor: Let's create a new one."));
		Init();
	}
	return Infos;
}

UCoordinateReferenceSystem* UGISFileManager::GetCRS()
{
	return GetInfos()->GetCRS();
}

UWorld* UGISFileManager::GetCurrentWorld()
{
	CurrentWorld = GEditor != nullptr ? GEditor->GetEditorWorldContext(false).World() : nullptr;
	if(CurrentWorld == nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("Landscaping: World is not set"));
	}
	return CurrentWorld;
}

FReply UGISFileManager::InspectDTM()
{
	void* ParentWindowPtr = FSlateApplication::Get().GetActiveTopLevelWindow()->GetNativeWindow()->GetOSWindowHandle();
    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	FString Result = FString();
	TArray<FString> OutFilenames;
	ULandscapingSettings* Settings = GetMutableDefault<ULandscapingSettings>();
    if (DesktopPlatform)
    {
        const uint32 SelectionFlag = 1;
        if (DesktopPlatform->OpenFileDialog(ParentWindowPtr, FString("Choose file(s) to inspect"), FString(), FString(), GetAllowedDTMFileTypes(), SelectionFlag, OutFilenames))
		{
			for(int i = 0; i < OutFilenames.Num(); i++)
			{
				RasterFile* FileToInspect = new RasterFile(OutFilenames[i]);
				GetCRS()->DumpProjectionToLog(FileToInspect->GetProjection(), OutFilenames[i]);
				delete FileToInspect;
			}
		}
	}
	ShowNotification("See Output Log for inspection results");
	return FReply::Handled();
}

FReply UGISFileManager::SpawnGeoReferencingSystem()
{
	if(GetCRS()->IsValid())
	{
		AGeoReferencingSystem* ReferencingSystem = GetCurrentWorld()->SpawnActor<AGeoReferencingSystem>(FVector(0), FRotator(0));
		ReferencingSystem->ProjectedCRS = GetCRS()->GetAuthorityIDStr();
		ReferencingSystem->OriginProjectedCoordinatesEasting = GetCRS()->GetOrigin().X;
		ReferencingSystem->OriginProjectedCoordinatesNorthing = GetCRS()->GetOrigin().Y;
		ShowNotification("Geo Referencing System ready to use");
	}
	else
	{
		ShowNotification("Import a DTM to have an origin and CRS for the Geo Referencing System", SNotificationItem::ECompletionState::CS_Fail);
	}
	return FReply::Handled();
}

bool UGISFileManager::ReadGeoReferencingSystem()
{
	bool bFetchFromGeoRef = true;
	if(GetCRS()->IsValid())
	{
		EAppReturnType::Type Answer = FMessageDialog::Open(
			EAppMsgType::YesNo,
			FText::FromString("Overwrite valid CRS of this Level?\n\nYes = Overwrite with values from Geo Referencing System\nNo = Cancel")
		);
		bFetchFromGeoRef = (Answer == EAppReturnType::Yes);
	}

	if(bFetchFromGeoRef)
	{
		TArray<AGeoReferencingSystem*> GeoRefs;
		LandscapingUtils::FindAllActors<AGeoReferencingSystem>(CurrentWorld, GeoRefs);
		if(GeoRefs.Num() == 1)
		{
			AGeoReferencingSystem* ReferencingSystem = GeoRefs[0];
			int Result = GetInfos()->SetCRS(ReferencingSystem->ProjectedCRS, FVector(ReferencingSystem->OriginProjectedCoordinatesEasting, ReferencingSystem->OriginProjectedCoordinatesNorthing, 0));
			if(Result > 0)
			{
				ULandscapingSettings* Settings = GetMutableDefault<ULandscapingSettings>();
				Settings->ProjectionMode = ELandscapingProjectionMode::CustomCRS;
				Settings->Projection = Result;
				ShowNotification("Successfully set CRS from Geo Referencing System");
				ShowNotification("Setting changed to 'Use Custom CRS'");
				ShowNotification(FString::Printf(TEXT("CRS set to %s"), *ReferencingSystem->ProjectedCRS));
				return true;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Landscaping: Projected CRS not valid"));
			}
		}
		else if(GeoRefs.IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("Landscaping: There is not Geo Referencing System in this Level"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Landscaping: Multiple Geo Referencing Systems found. Please make sure to have only one in the Level"));
		}

	}
	ShowNotification("Setting CRS from Geo Referencing System failed", SNotificationItem::ECompletionState::CS_Fail);
	return false;
}

double UGISFileManager::GetAvgMeterPerPixel()
{
	if(HasLandscape())
	{
		return (FMath::Abs(GetInfos()->Tiles[0].MeterPerPixelX) + FMath::Abs(GetInfos()->Tiles[0].MeterPerPixelY)) / 2;
	}
	else if(HasRasterData())
	{
		FVector MeterPerPixel = TerrainCreator->GetMeterPerPixel();
		return (FMath::Abs(MeterPerPixel.X) + FMath::Abs(MeterPerPixel.Y)) / 2;
	}
	return 5.0; // let's assume zoom level 14 on mapbox imports
}

bool UGISFileManager::HasMapboxExtension()
{
	return FModuleManager::Get().IsModuleLoaded("LandscapingMapbox");
}

