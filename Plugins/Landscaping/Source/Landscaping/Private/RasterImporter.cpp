// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved



#include "RasterImporter.h"
#include "LandscapeConfigHelper.h"


RasterImporter::RasterImporter(UGISFileManager* InGisFileManager, FString InWorkingDir)
{
	GisFM = InGisFileManager;
	TArray<FString> SearchPaths = TArray<FString>();
	const FString& RelProjectPluginsDir = FPaths::ProjectPluginsDir();
	const FString& ProjectPluginsDir = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*RelProjectPluginsDir);
	const FString& RelEnginePluginsDir = FPaths::EnginePluginsDir();
	const FString& EnginePluginsDir = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*RelEnginePluginsDir);
	const FString& RelEnterprisePluginsDir = FPaths::EnterprisePluginsDir();
	const FString& EnterprisePluginsDir = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*RelEnterprisePluginsDir);
	SearchPaths.Add(ProjectPluginsDir);
	SearchPaths.Add(EnginePluginsDir);
	SearchPaths.Add(EnterprisePluginsDir);
	SearchPaths.Add(FString::Printf(TEXT("%sMarketplace/"), *EnginePluginsDir));
	SearchPaths.Add(FString::Printf(TEXT("%sMarketplace/"), *EnterprisePluginsDir));
	TArray<FString> ProjDataPaths = TArray<FString>();
	FString GdalDataPath = FString();
	for(int SearchPathIndex = 0; SearchPathIndex < SearchPaths.Num(); SearchPathIndex++)
	{
		for(int i = 0; i < 2; i++)
		{
			FString ProjDataDir = SearchPaths[SearchPathIndex];
			if(i == 0)
			{
				ProjDataDir.Append("Landscaping/Binaries/Win64/proj");
			}
			else
			{
				ProjDataDir.Append("Landscaping/Binaries/Win64/proj7");
			}
			if(FPaths::DirectoryExists(ProjDataDir) && ProjDataPaths.Num() < 2)
			{
				ProjDataPaths.Add(ProjDataDir);
			}
		}
		FString GdalDataDir = SearchPaths[SearchPathIndex];
		GdalDataDir.Append("Landscaping/Binaries/Win64/gdal");
		if(FPaths::DirectoryExists(GdalDataDir))
		{
			GdalDataPath = GdalDataDir;
		}
	}
	
	TileFactory = new RasterTileFactory(ProjDataPaths, GdalDataPath, GisFM);
	SetWorkingDir(InWorkingDir);
}

RasterImporter::~RasterImporter()
{
	LandscapeMaterial = nullptr;
	GisFM = nullptr;
	delete TileFactory;
}

FString RasterImporter::LoadFiles(TArray<FString>& Filenames, int TileIndex)
{   
	if(Filenames.Num() == 0)
	{
		return FString();
	}

	for(FString Filename : Filenames)
	{
		if(Filename.EndsWith(".hgt"))
		{
			string FilenameToCheck = string(TCHAR_TO_ANSI(*FPaths::GetCleanFilename(Filename)));
			bool bValid = FilenameToCheck.length() >= 11;
			bValid = bValid && (FilenameToCheck[0] == 'N' || FilenameToCheck[0] == 'S');
			bValid = bValid && (FilenameToCheck[3] == 'E' || FilenameToCheck[3] == 'W');
			bValid = bValid && FilenameToCheck[7] == '.';
			if(!bValid)
			{
				return FString("Open hgt file failed because filename is invalid!\nPlease refer to https://jorop.github.io/landscaping-docs/#/get-data?id=raster-data for valid hgt file naming.");
			}
		}
	}
	
	TArray<FString> Errors = TileFactory->ReadFiles(Filenames, TileIndex);

	if(!Errors.IsEmpty())
	{
		for(FString Error : Errors)
		{
			UE_LOG(LogTemp, Error, TEXT("Landscaping: %s"), *Error);
		}
		GisFM->GetInfos()->ResetCRS();
		return FString("A problem has occurred.\nThere is an additional action required.\n\nSee Ouput Log for details.");
	}

	return FString();
}

TArray<FString> RasterImporter::WriteFiles(TArray<RasterData> InData)
{
	return TileFactory->WriteFiles(InData);
}

bool RasterImporter::CreateTerrains(UMaterialInterface* Material, TArray<TSharedPtr<MaterialLayerSettings>> LayerSettings, WeightmapImporter* InWeightmapCreator, RasterImportOptions Options)
{
	if(!HasRasterFile())
	{
		UE_LOG(LogTemp, Error, TEXT("Landscaping: Not ready to import: no file or data"));
		return false;
	}

	WeightmapCreator = InWeightmapCreator;
	LayerSettingsList = LayerSettings;
	LandscapeMaterial = Material;
	DefaultLayerName = Options.DefaultLayer;
	int FirstTileIndex = GisFM->GetInfos()->Tiles.Num();
	FString PrepareError = TileFactory->PrepareImport(Options);
	if(!PrepareError.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Landscaping: %s"), *PrepareError);
		return false;
	}
	
	if(Options.bUpdateLandscape)
	{
		return UpdateLandscapes(Options, FirstTileIndex);
	}
	if(Options.bImportAsMesh)
	{
		return CreateMeshes(Options, FirstTileIndex);
	}
	return CreateLandscapes(Options, FirstTileIndex);
}

bool RasterImporter::CheckBeforeImport(RasterData InRasterData)
{
	if(InRasterData.LandscapeResolution.X < 16 || InRasterData.LandscapeResolution.Y < 16 || InRasterData.NumberOfSections == 0 || InRasterData.QuadsPerSection == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Landscaping: Size of Landscape to import not valid. Raster Data probably corrupted:"));
		UE_LOG(LogTemp, Error, TEXT("Landscaping: Sections: %i Quads: %i"), InRasterData.NumberOfSections,  InRasterData.QuadsPerSection);
		UE_LOG(LogTemp, Error, TEXT("Landscaping: Import Resolution (from source, should be at least 16x16 pixels): %i %i"), InRasterData.ImportResolution.X, InRasterData.ImportResolution.Y);
		UE_LOG(LogTemp, Error, TEXT("Landscaping: Lanscape Resolution: %i %i"), InRasterData.LandscapeResolution.X, InRasterData.LandscapeResolution.Y);
		return false;
	}
	return true;
}

bool RasterImporter::CreateMeshes(RasterImportOptions Options, int FirstTileIndex)
{
	FLandscapingTileLoader* TileLoader = new FLandscapingTileLoader(GisFM);
	int TileIndex = 0;
	for(; TileIndex <= FirstTileIndex; TileIndex++)
	{
		TileLoader->UnloadWorldPartition(TileIndex);
	}
	FScopedSlowTask LandscapeDataSlowTask(TileFactory->GetLandscapeDataCount(), FText::FromString("Creating Mesh"));
	LandscapeDataSlowTask.MakeDialog();
	for(int LandscapeDataIndex = 0; LandscapeDataIndex < TileFactory->GetLandscapeDataCount(); LandscapeDataIndex++)
	{
		LandscapeDataSlowTask.EnterProgressFrame();
		FScopedSlowTask SlowTask(TileFactory->GetRasterDataCount(LandscapeDataIndex), FText::FromString("Creating Static Mesh"));
		SlowTask.MakeDialog();
		int DataCounter = 0;
		for(int RasterDataIndex = 0; RasterDataIndex < TileFactory->GetRasterDataCount(LandscapeDataIndex); RasterDataIndex++)
		{
			TileIndex = FirstTileIndex + RasterDataIndex + LandscapeDataIndex;
			if(GisFM->GetInfos()->bWorldPartition)
			{
				TileLoader->LoadWorldPartition(TileIndex);
			}
			FString ProgressStr2 = FString::Printf(TEXT("Creating Static Mesh Part %i / %i"), ++DataCounter, TileFactory->GetRasterDataCount(LandscapeDataIndex));
			SlowTask.EnterProgressFrame(1.0, FText::FromString(ProgressStr2));
			RasterData Data = TileFactory->GetNextRasterData(LandscapeDataIndex, RasterDataIndex);
			if(!Data.Error.IsEmpty())
			{
				UE_LOG(LogTemp, Error, TEXT("Landscaping: %s"), *Data.Error);
				continue;
			}
			
			if(CheckBeforeImport(Data))
			{
				FVector Location = GisFM->GetCRS()->GetLandscapeLocation(GisFM->GetInfos()->Tiles[TileIndex]);
				Location.Z = 0;
				FRotator Rotation = FRotator(0);
				ALandscapingProcMeshLandscape* NewProcMesh = (ALandscapingProcMeshLandscape*)(GetWorld()->SpawnActor(ALandscapingProcMeshLandscape::StaticClass(), &Location, &Rotation));
				
				if(NewProcMesh) 
				{
					NewProcMesh->SetActorLabel(FString::Printf(TEXT("LandscapeSM_Tile_%i"), TileIndex));
					FLandscapingInfo TileInfo = GisFM->GetInfos()->Tiles[TileIndex];
					TArray<double> TempHeightData;
					TempHeightData.AddDefaulted(TileInfo.ImportResolution.X * TileInfo.ImportResolution.Y);
					ParallelFor (TileInfo.ImportResolution.Y, [&](int32 Y)
					{
						for (int X = 0; X < TileInfo.ImportResolution.X; X++)
						{
							TempHeightData[Y * TileInfo.ImportResolution.X + X] = Data.RasterBandData[Y][X];
						}
					});
					NewProcMesh->CreateMesh(TileInfo.ImportResolution, TileInfo.MeterPerPixelX, TileInfo.MeterPerPixelY, TempHeightData, Options.bMeshCollision);
					NewProcMesh->SetActorScale3D(FVector(1.0) * GisFM->GetCRS()->GetLandscapeScaleFactor());
					GisFM->GetInfos()->Tiles[TileIndex].Bounds = NewProcMesh->GetComponentsBoundingBox(true);
					GisFM->GetInfos()->Tiles[TileIndex].LandscapeSM = NewProcMesh;
				}
				NewProcMesh->MarkPackageDirty();
				if(GisFM->GetInfos()->bWorldPartition)
				{
					TileLoader->UnloadWorldPartition(TileIndex);
				}
			}
		}
	}
	return true;
}

bool RasterImporter::UpdateLandscapes(RasterImportOptions Options, int FirstTileIndex)
{
	FLandscapingTileLoader* TileLoader = new FLandscapingTileLoader(GisFM);
	int TileIndex = 0;
	for(; TileIndex <= FirstTileIndex; TileIndex++)
	{
		TileLoader->UnloadWorldPartition(TileIndex);
	}
	TileIndex = FirstTileIndex;
	FScopedSlowTask LandscapeDataSlowTask(TileFactory->GetLandscapeDataCount(), FText::FromString("Updating Landscapes"));
	LandscapeDataSlowTask.MakeDialog();
	for(int LandscapeDataIndex = 0; LandscapeDataIndex < TileFactory->GetLandscapeDataCount(); LandscapeDataIndex++)
	{
		LandscapeDataSlowTask.EnterProgressFrame();
		FScopedSlowTask SlowTask(TileFactory->GetRasterDataCount(LandscapeDataIndex), FText::FromString("Updating Landscapes"));
		SlowTask.MakeDialog();
		int DataCounter = 0;
		for(int RasterDataIndex = 0; RasterDataIndex < TileFactory->GetRasterDataCount(LandscapeDataIndex); RasterDataIndex++)
		{
			TileIndex = FirstTileIndex + RasterDataIndex + LandscapeDataIndex;
			if(GisFM->GetInfos()->bWorldPartition)
			{
				TileLoader->LoadWorldPartition(TileIndex);
			}
			FString ProgressStr2 = FString::Printf(TEXT("Updating Landscapes %i / %i"), ++DataCounter, TileFactory->GetRasterDataCount(LandscapeDataIndex));
			SlowTask.EnterProgressFrame(1.0, FText::FromString(ProgressStr2));
			RasterData Data = TileFactory->GetNextRasterData(LandscapeDataIndex, RasterDataIndex);
			if(!Data.Error.IsEmpty())
			{
				UE_LOG(LogTemp, Error, TEXT("Landscaping: %s"), *Data.Error);
				continue;
			}
			
			if(CheckBeforeImport(Data))
			{
				FVector TileLocation = GisFM->GetInfos()->Tiles[TileIndex].GetLocation();
				ALandscapeProxy* LandscapeProxy = nullptr;
				FIntVector LandscapeResolution = FIntVector(0);
				int ExistingTileIndex = 0;
				for(; ExistingTileIndex < GisFM->GetInfos()->Tiles.Num(); ExistingTileIndex++)
				{
					FVector ExistingTileLocation = GisFM->GetInfos()->Tiles[ExistingTileIndex].GetLocation();
					if(GisFM->GetInfos()->Tiles[ExistingTileIndex].Landscape != nullptr && FMath::IsNearlyEqual(TileLocation.X, ExistingTileLocation.X, 10.0) && FMath::IsNearlyEqual(TileLocation.Y, ExistingTileLocation.Y, 10.0))
					{
						LandscapeProxy = GisFM->GetInfos()->Tiles[ExistingTileIndex].Landscape;
						LandscapeResolution = GisFM->GetInfos()->Tiles[ExistingTileIndex].LandscapeResolution;
						break;
					}
				}
				if(LandscapeProxy == nullptr)
				{
					UE_LOG(LogTemp, Error, TEXT("Landscaping: Update Landscape - no LandscapeProxy for this area found."))
					continue;
				}
				if(LandscapeProxy->CanHaveLayersContent())
				{
					FLandscapeLayer* Layer = LandscapeProxy->GetLandscapeActor()->GetLayer(0);
					LandscapeProxy->GetLandscapeActor()->SetEditingLayer(Layer->Guid);
				}
				ULandscapeInfo* LandscapeInfo = LandscapeProxy->GetLandscapeInfo();
				int32 MinX = 0, MinY = 0, MaxX = 0, MaxY = 0;
				if (LandscapeInfo == nullptr || !LandscapeInfo->GetLandscapeExtent(MinX, MinY, MaxX, MaxY))
				{
					UE_LOG(LogTemp, Error, TEXT("Landscaping: Update Landscape - LandscapeProxy not valid or extent of Landscape unknown."))
					continue;
				}
				FLandscapeEditDataInterface LandscapeEdit(LandscapeInfo);
				TArray<uint16> ResampledData;    
				TArray<uint16> ImportData = TileFactory->Import(Data.RasterBandData, Data.ImportResolution, ExistingTileIndex, Options);
				if (Data.ImportResolution.X != Data.LandscapeResolution.X)
				{
					UE_LOG(LogTemp, Log, TEXT("Landscaping: Update Landscape - Resample Data (ImportResolution: %s - LandscapeResolution: %s - ImportData.Num: %i)"), 
						*Data.ImportResolution.ToString(), 
						*Data.LandscapeResolution.ToString(), 
						ImportData.Num());
						
					ResampledData = LandscapingUtils::ResampleData<uint16>(            
						ImportData,
						Data.ImportResolution.X,
						Data.ImportResolution.Y,
						Data.LandscapeResolution.X,
						Data.LandscapeResolution.Y);
				}
				else
				{
					ResampledData = ImportData;
				}
				if(Options.bFillMissing)
				{
					TArray<uint16> OriginalData;
					OriginalData.AddZeroed(LandscapeResolution.X * LandscapeResolution.Y);
					LandscapeEdit.GetHeightDataFast(MinX, MinY, MaxX, MaxY, OriginalData.GetData(), 0);
					uint16 Minimum = 65535;
					for(int i = 0; i < OriginalData.Num(); i++)
					{
						Minimum = FMath::Min(OriginalData[i], Minimum);
					}
					UE_LOG(LogTemp, Log, TEXT("Landscaping: Start updating landscape"));
					FScopedSlowTask FillGapSlowTask(OriginalData.Num(), FText::FromString("Updating Landscapes"));
					FillGapSlowTask.MakeDialog();
					for(int i = 0; i < OriginalData.Num(); i++)
					{
						FString ProgressStr3 = FString::Printf(TEXT("Fill Gaps %i / %i"), i+1, OriginalData.Num());
						FillGapSlowTask.EnterProgressFrame(1.0, FText::FromString(ProgressStr3));
						if(!FMath::IsNearlyEqual((float)OriginalData[i], (float)Minimum, (double)Options.MinHeightTolerance))
						{
							ResampledData[i] = OriginalData[i];
						}
					}
				}
				UE_LOG(LogTemp, Log, TEXT("Landscaping: Updating Landscape"));
				LandscapeEdit.SetHeightData(MinX, MinY, MaxX, MaxY, ResampledData.GetData(), 0, true);
			}
		}
	}
	return true;
}

bool RasterImporter::CreateLandscapes(RasterImportOptions Options, int FirstTileIndex)
{	
	FLandscapingTileLoader* TileLoader = new FLandscapingTileLoader(GisFM);
	int TileIndex = 0;
	for(; TileIndex <= FirstTileIndex; TileIndex++)
	{
		TileLoader->UnloadWorldPartition(TileIndex);
	}
	TileIndex = FirstTileIndex;
	FScopedSlowTask LandscapeDataSlowTask(TileFactory->GetLandscapeDataCount(), FText::FromString("Creating Landscapes"));
	LandscapeDataSlowTask.MakeDialog();
	for(int LandscapeDataIndex = 0; LandscapeDataIndex < TileFactory->GetLandscapeDataCount(); LandscapeDataIndex++)
	{
		LandscapeDataSlowTask.EnterProgressFrame();
		FScopedSlowTask SlowTask(TileFactory->GetRasterDataCount(LandscapeDataIndex), FText::FromString("Creating Landscapes"));
		SlowTask.MakeDialog();
		int DataCounter = 0;
		for(int RasterDataIndex = 0; RasterDataIndex < TileFactory->GetRasterDataCount(LandscapeDataIndex); RasterDataIndex++)
		{
			TileIndex = FirstTileIndex + RasterDataIndex + LandscapeDataIndex;
			if(GisFM->GetInfos()->bWorldPartition)
			{
				TileLoader->LoadWorldPartition(TileIndex);
			}
			FGuid LandscapeGuid = FGuid::NewGuid();
			FString ProgressStr2 = FString::Printf(TEXT("Creating Landscapes %i / %i"), ++DataCounter, TileFactory->GetRasterDataCount(LandscapeDataIndex));
			SlowTask.EnterProgressFrame(1.0, FText::FromString(ProgressStr2));
			RasterData Data = TileFactory->GetNextRasterData(LandscapeDataIndex, RasterDataIndex);
			if(!Data.Error.IsEmpty())
			{
				UE_LOG(LogTemp, Error, TEXT("Landscaping: %s"), *Data.Error);
				continue;
			}
			
			check(LandscapeGuid.IsValid());
			if(CheckBeforeImport(Data))
			{
				UE_LOG(LogTemp, Log, TEXT("Max Altitude: %f Min Altitude: %f"), GisFM->GetInfos()->Tiles[TileIndex].MinAltitude, GisFM->GetInfos()->Tiles[TileIndex].MaxAltitude);
				ALandscapeProxy* NewLandscape = CreateLandscape(Data, LandscapeGuid, Options, TileIndex);

				if(NewLandscape) 
				{
					NewLandscape->SetActorLabel(FString::Printf(TEXT("Landscape_Tile_%i"), TileIndex));
					GisFM->GetInfos()->Tiles[TileIndex].Bounds = NewLandscape->GetComponentsBoundingBox(false, true);
					GisFM->GetInfos()->Tiles[TileIndex].Landscape = NewLandscape;
					NewLandscape->RecreateCollisionComponents();
			#if ENGINE_MAJOR_VERSION > 4
					if(GisFM->GetInfos()->bWorldPartition)
					{
						ULandscapeInfo* LandscapeInfo = NewLandscape->GetLandscapeInfo();
						GetWorld()->GetSubsystem<ULandscapeSubsystem>()->ChangeGridSize(LandscapeInfo, Options.WorldPartitionGridSize);
					}
			#endif
					NewLandscape->MarkPackageDirty();
				}
				if(GisFM->GetInfos()->bWorldPartition)
				{
					TileLoader->UnloadWorldPartition(TileIndex);
				}
			}
		}
	}
	return true;
}

ALandscapeProxy* RasterImporter::CreateLandscape(RasterData InRasterData, FGuid Guid, RasterImportOptions Options, int TileIndex)
{   
	FLandscapingInfo LandscapingInfo = FLandscapingInfo();
    TArray<uint16> Data;    
    TArray<uint16> ImportData = TileFactory->Import(InRasterData.RasterBandData, InRasterData.ImportResolution, TileIndex, Options);
    if (InRasterData.ImportResolution.X != InRasterData.LandscapeResolution.X || InRasterData.ImportResolution.Y != InRasterData.LandscapeResolution.Y)
    {
		UE_LOG(LogTemp, Log, TEXT("Landscaping: Resample Data (ImportResolution: %s - LandscapeResolution: %s - ImportData.Num: %i)"), 
			*InRasterData.ImportResolution.ToString(), 
			*InRasterData.LandscapeResolution.ToString(), 
			ImportData.Num());
			
        Data = LandscapingUtils::ResampleData<uint16>(            
            ImportData,
            InRasterData.ImportResolution.X,
            InRasterData.ImportResolution.Y,
            InRasterData.LandscapeResolution.X,
            InRasterData.LandscapeResolution.Y);
    }
    else
    {
        Data = ImportData;
    }
    ALandscapeProxy* LandscapeProxy;
    FVector Location = GisFM->GetCRS()->GetLandscapeLocation(GisFM->GetInfos()->Tiles[TileIndex]);
    FRotator Rotation = FRotator(0);
    LandscapeProxy = (ALandscapeProxy*)(GetWorld()->SpawnActor(ALandscape::StaticClass(), &Location, &Rotation));
	if(LandscapeProxy == nullptr)
	{
		return LandscapeProxy;
	}
	// landscape scale
	FVector LandscapeScale = GisFM->GetCRS()->GetLandscapeScale(GisFM->GetInfos()->Tiles[TileIndex]);
    LandscapeProxy->SetActorScale3D(LandscapeScale);

	TArray<FLandscapeImportLayerInfo> LayerInfos;
	if (LandscapeMaterial)
	{
		UEditorLoadingAndSavingUtils::SaveCurrentLevel(); // save level so we do not have 'Untitled' folders in the project
		LandscapeProxy->LandscapeMaterial = LandscapeMaterial;
		LayerInfos = WeightmapCreator->GetLayerInfos(LandscapeMaterial, LayerSettingsList, TileIndex, DefaultLayerName);
	}

    TMap<FGuid, TArray<uint16>> HeightDataPerLayers;
    HeightDataPerLayers.Add(FGuid(), Data);
    TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayersInfo;
    MaterialLayersInfo.Add(FGuid(), LayerInfos);
	
    LandscapeProxy->Import(
        Guid,
        0,
        0,
        InRasterData.LandscapeResolution.X - 1,
        InRasterData.LandscapeResolution.Y - 1,
        InRasterData.NumberOfSections,
        InRasterData.QuadsPerSection,
        HeightDataPerLayers,
        nullptr,
        MaterialLayersInfo,
        ELandscapeImportAlphamapType::Additive);
		
	
	for (FLandscapeImportLayerInfo ImportLayerInfo : LayerInfos)
	{
		LandscapeProxy->EditorLayerSettings.Add(FLandscapeEditorLayerSettings(ImportLayerInfo.LayerInfo));
	}
	LandscapeProxy->CreateLandscapeInfo();
	
	return LandscapeProxy;
}

void RasterImporter::SetWorkingDir(FString Dir)
{
	if(!Dir.IsEmpty())
	{
		TileFactory->SetWorkingDir(Dir);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Landscaping: Cannot set empty Working Directory!"))
	}
}

FVector RasterImporter::GetMeterPerPixel()
{
	return TileFactory->GetMeterPerPixel();
}

bool RasterImporter::CreateSatelliteImages(int TileIndex, RasterImportOptions Options)
{
	FLandscapingInfo Tile = GisFM->GetInfos()->Tiles[TileIndex];
	if(Options.bImportSatImgAsVertexColor && GisFM->GetInfos()->Tiles[TileIndex].LandscapeSM == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Landscaping: Vertex Color chosen but not Static Mesh - falling back to decal!"));
		Options.bImportSatImgAsVertexColor = false;
		Options.bImportSatImgAsDecal = true;
	}
	
	if(Options.bImportSatImgAsDecal)
	{
		FName TemplateMaterialName = FName("/Landscaping/Materials/M_LandscapingSatDecalMat");
		TileConfig SatelliteTiles = TileFactory->CreateTexture(TileIndex, Options.bImportSatImgAsDecal);
		if(SatelliteTiles.Textures.IsEmpty())
		{
			FString ErrorMsg = FString::Printf(TEXT("Could not create satellite textures for Tile %i"), TileIndex);
			UE_LOG(LogTemp, Error, TEXT("Landscaping: %s"), *ErrorMsg);
			return false;
		}
		int SatTileIndex = 0;
		for(int y  = 0; y < SatelliteTiles.NumberTilesY; y++)
		{
			for(int x  = 0; x < SatelliteTiles.NumberTilesX; x++)
			{
				UTexture2D* Texture = SatelliteTiles.Textures[SatTileIndex++];
				FName MatName = FName(*FString::Printf(TEXT("M_%s"), *Texture->GetName()));
				UMaterialInterface* Material = CreateMaterial(Texture, MatName, FVector2D(Tile.LandscapeResolution.X / SatelliteTiles.NumberTilesX, Tile.LandscapeResolution.Y / SatelliteTiles.NumberTilesY), TemplateMaterialName, true);
				if(Material != nullptr)
				{
					ADecalActor* DecalActor = GetWorld()->SpawnActor<ADecalActor>();
					DecalActor->SetActorLabel(Texture->GetName());
					double SatTileWidth = (Tile.Bounds.Max.X - Tile.Bounds.Min.X) / SatelliteTiles.NumberTilesX;
					double SatTileHeight = (Tile.Bounds.Max.Y - Tile.Bounds.Min.Y) / SatelliteTiles.NumberTilesY;
					// set the decal actor to the center of the tile and to the max bounds z
					double LocationX = Tile.Bounds.Min.X + x * SatTileWidth + SatTileWidth / 2;
					double LocationY =  Tile.Bounds.Min.Y + y * SatTileHeight + SatTileHeight / 2;
					FVector Location = FVector(LocationX, LocationY, Tile.Bounds.Max.Z);
					DecalActor->SetActorLocation(Location);
					UDecalComponent* DecalComponent = DecalActor->GetDecal();
					DecalComponent->DecalSize = FVector(128, 256, 256);
					// scale 1 of the decal is 5.12 cm
					// the decal is rotated (0,-90,0) - so we have to scale y and z
					// x * 512 = scale * (landscaperes - 1)
					// we use the bounds for vertical scale - since we placing the decal on Tile.Bounds.Max.Z - we double the vertial scale, therefore 0.02
					double ScaleFactor = GisFM->GetCRS()->GetLandscapeScaleFactor();
					FVector Scale = FVector((Tile.Bounds.Max.Z - Tile.Bounds.Min.Z) * 0.02, Tile.LandscapeScale.Y * ScaleFactor * (Tile.LandscapeResolution.Y / SatelliteTiles.NumberTilesY) / 512, Tile.LandscapeScale.X * ScaleFactor * (Tile.LandscapeResolution.X / SatelliteTiles.NumberTilesX) / 512);
					DecalActor->SetActorScale3D(Scale);
					DecalActor->SetDecalMaterial(Material);
					FPropertyChangedEvent PropertyChangedEvent(FindFieldChecked<FProperty>(DecalComponent->GetClass(), FName("DecalMaterial")));
					DecalComponent->PostEditChangeProperty(PropertyChangedEvent);
					DecalComponent->MarkPackageDirty();
					DecalComponent->PostEditChange();
					FPropertyChangedEvent DecalActorPropertyChangedEvent(FindFieldChecked<FProperty>(DecalActor->GetClass(), FName("Decal")));
					DecalActor->PostEditChangeProperty(DecalActorPropertyChangedEvent);
					DecalActor->MarkPackageDirty();
					DecalActor->PostEditChange();
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("Landscaping: Could not create Material for satellite texture for decal %s"), *Texture->GetName());
				}
			}
		}
	}
	else if(GisFM->GetInfos()->Tiles[TileIndex].Landscape)
	{
		FName TemplateMaterialName = FName("/Landscaping/Materials/M_LandscapingSatellite");
		TileConfig SatelliteTiles = TileFactory->CreateTexture(TileIndex, Options.bImportSatImgAsDecal);
		if(SatelliteTiles.Textures.IsEmpty())
		{
			FString ErrorMsg = FString::Printf(TEXT("Could not create Texture for Tile %i"), TileIndex);
			UE_LOG(LogTemp, Error, TEXT("Landscaping: %s"), *ErrorMsg);
			return false;
		}
		UTexture* Texture = SatelliteTiles.Textures[0];
		FName MatName = FName(*FString::Printf(TEXT("M_%s"), *Texture->GetName()));
		UMaterialInterface* Material = CreateMaterial(Texture, MatName, FVector2D(Tile.LandscapeResolution.X, Tile.LandscapeResolution.Y), TemplateMaterialName, false);
		if(Material != nullptr)
		{
			ALandscapeProxy* SelectedLandscape = GisFM->GetInfos()->Tiles[TileIndex].Landscape;
			SelectedLandscape->LandscapeMaterial = Material;
			FPropertyChangedEvent PropertyChangedEvent(FindFieldChecked<FProperty>(SelectedLandscape->GetClass(), FName("LandscapeMaterial")));
			SelectedLandscape->PostEditChangeProperty(PropertyChangedEvent);
			SelectedLandscape->MarkPackageDirty();
			SelectedLandscape->PostEditChange();
			if(GisFM->GetInfos()->bWorldPartition)
			{
				TArray<ALandscapeProxy*> Proxies = LandscapingUtils::GetLandscapeCells(GetWorld());
				for(int i = 0; i < Proxies.Num(); i++)
				{
					if(Proxies[i]->GetLandscapeActor() == SelectedLandscape)
					{
						Proxies[i]->LandscapeMaterial = Material;
						FPropertyChangedEvent PropertyChangedEventProxy(FindFieldChecked<FProperty>(Proxies[i]->GetClass(), FName("LandscapeMaterial")));
						Proxies[i]->PostEditChangeProperty(PropertyChangedEventProxy);
						Proxies[i]->MarkPackageDirty();
						Proxies[i]->PostEditChange();
					}
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Landscaping: Could not create Material for satellite texture for Landscape"));
			return false;
		}
	}
	else if(GisFM->GetInfos()->Tiles[TileIndex].LandscapeSM || GisFM->GetInfos()->Tiles[TileIndex].LandscapeNaniteMesh)
	{
		ALandscapingProcMeshLandscape* SelectedLandscape = GisFM->GetInfos()->Tiles[TileIndex].LandscapeSM;
		if(Options.bImportSatImgAsVertexColor && GisFM->GetInfos()->Tiles[TileIndex].LandscapeSM)
		{
			FName TemplateMaterialName = FName("/Landscaping/Materials/M_LandscapingVertexColor");
			TArray<ColorData> NewData = TileFactory->GetColorData(TileIndex, Options.bImportSatImgAsDecal);
			SelectedLandscape->AddVertexColor(NewData[0].Data, NewData[0].Width, NewData[0].Height);
			FName MatName = FName(*FString::Printf(TEXT("M_%s"), *LandscapingUtils::GetUniqueFilename("SatTex.tex")));
			UMaterialInterface* Material = CreateMaterial(nullptr, MatName, FVector2D(1.0), TemplateMaterialName, false);
			if(Material != nullptr)
			{
				SelectedLandscape->SetMaterial(Material);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Landscaping: Could not create Material for satellite vertex color for SM"));
				return false;
			}
		}
		else
		{
			FName TemplateMaterialName = FName("/Landscaping/Materials/M_LandscapingSatSMMat");
			TileConfig SatelliteTiles = TileFactory->CreateTexture(TileIndex, Options.bImportSatImgAsDecal);
			if(SatelliteTiles.Textures.IsEmpty())
			{
				FString ErrorMsg = FString::Printf(TEXT("Could not create Texture for Tile %i"), TileIndex);
				UE_LOG(LogTemp, Error, TEXT("Landscaping: %s"), *ErrorMsg);
				return false;
			}
			UTexture* Texture = SatelliteTiles.Textures[0];
			FName MatName = FName(*FString::Printf(TEXT("M_%s"), *Texture->GetName()));
			UMaterialInterface* Material = CreateMaterial(Texture, MatName, FVector2D(1.0), TemplateMaterialName, false);
			if(Material != nullptr && GisFM->GetInfos()->Tiles[TileIndex].LandscapeSM != nullptr)
			{
				SelectedLandscape->SetMaterial(Material);
			}
			else if(Material != nullptr)
			{
				UStaticMeshComponent* MeshComp = GisFM->GetInfos()->Tiles[TileIndex].LandscapeNaniteMesh->GetStaticMeshComponent();
				MeshComp->GetStaticMesh()->SetMaterial(0, Material);
				MeshComp->MarkPackageDirty();
				GisFM->GetInfos()->Tiles[TileIndex].LandscapeNaniteMesh->MarkPackageDirty();
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Landscaping: Could not create Material for satellite texture for SM"));
				return false;
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Landscaping: Applying Satellite images failed - Landscape references missing for Tile %i"), TileIndex);
		UE_LOG(LogTemp, Error, TEXT("Landscaping: Please assign the Landscape or LandscapingProcMeshLandscape for Tile %i in LandscapingInfos -> Tiles"), TileIndex);
		return false;
	}
	return true;
}

UMaterialInterface* RasterImporter::CreateMaterial(UTexture* InitialTexture, FName Name, FVector2D Tiling, FName MaterialName, bool bImportSatImgAsDecal)
{
	
	UMaterial* MaterialOriginal = (UMaterial*)LandscapingUtils::LoadMatFromPath(MaterialName);
	if(MaterialOriginal == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Landscaping: Material not found"));
		return nullptr;
	}
	FString PackagePath = "/Game/Landscaping/Satellite/";
	FString PackageName = FString::Printf(TEXT("%s%s"), *PackagePath, *Name.ToString());
	UPackage* Package = CreatePackage(*PackageName);
	IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
	UMaterial* NewMaterial = (UMaterial*)AssetTools.DuplicateAsset(Name.ToString(), PackagePath, MaterialOriginal);
	if(InitialTexture != nullptr)
	{
		NewMaterial->SetTextureParameterValueEditorOnly(FName("Satellite"), InitialTexture);
	}
	if(!bImportSatImgAsDecal)
	{
		NewMaterial->SetScalarParameterValueEditorOnly(FName("TilingX"), Tiling.X);
		NewMaterial->SetScalarParameterValueEditorOnly(FName("TilingY"), Tiling.Y);
	}
	return NewMaterial;
}

bool RasterImporter::HasRasterFile() const
{
	return TileFactory->HasRasterFile(0);
}

void RasterImporter::CreateDummyLandscape()
{
	FGuid LandscapeGuid = FGuid::NewGuid();
	ALandscape* Landscape = GetWorld()->SpawnActor<ALandscape>();
	Landscape->SetActorTransform(FTransform(FQuat::Identity, FVector::ZeroVector, FVector(100)));
	Landscape->SetLandscapeGuid(LandscapeGuid);
	Landscape->CreateLandscapeInfo();
}

UWorld* RasterImporter::GetWorld()
{
	return GEditor ? GEditor->GetEditorWorldContext(false).World() : nullptr;
}
