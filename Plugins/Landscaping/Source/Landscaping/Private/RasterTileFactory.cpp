// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#include "RasterTileFactory.h"


RasterTileFactory::RasterTileFactory(TArray<FString> ProjDataPaths, FString GdalDataPath, UGISFileManager* InGisFileManager)
{
	if(ProjDataPaths.Num() == 2)
	{
		const char* const ProjPaths[3] = {
			TCHAR_TO_ANSI(*ProjDataPaths[0]),
			TCHAR_TO_ANSI(*ProjDataPaths[1]),
			NULL
		};
		OSRSetPROJSearchPaths(ProjPaths);
		//UE_LOG(LogTemp, Log, TEXT("Landscaping: proj at %s %s"), *ProjDataPaths[0], *ProjDataPaths[1]);
		//int Major = 0, Minor = 0, Patch = 0;
		//OSRGetPROJVersion(&Major, &Minor, &Patch);
		//UE_LOG(LogTemp, Log, TEXT("Landscaping: Using proj %i.%i.%i"), Major, Minor, Patch);
	}
	else 
	{
		UE_LOG(LogTemp, Error, TEXT("Landscaping: proj not found. Projection transformation not supported."));
	}
	CPLSetConfigOption("GDAL_DISABLE_READDIR_ON_OPEN", "TRUE");
	if(!GdalDataPath.IsEmpty())
	{
		CPLSetConfigOption("GDAL_DATA", TCHAR_TO_ANSI(*GdalDataPath));
		//UE_LOG(LogTemp, Log, TEXT("Landscaping: gdal-data at %s"), *GdalDataPath);
	}
	else 
	{
		UE_LOG(LogTemp, Error, TEXT("Landscaping: gdal-data not found. OSM filetypes not supported."));
	}
	GisFM = InGisFileManager;
}

RasterTileFactory::~RasterTileFactory()
{
    GisFM = nullptr;
	for(FString InMemoryFile : InMemoryFiles)
	{
		VSIUnlink(TCHAR_TO_ANSI(*InMemoryFile));
	}
}

void RasterTileFactory::SetWorkingDir(FString InWorkingDir)
{
	//UE_LOG(LogTemp, Log, TEXT("Landscaping: Set Working Directory to %s"), *InWorkingDir);
    WorkingDir = InWorkingDir;
}

TArray<FString> RasterTileFactory::ReadFiles(TArray<FString> FilesToRead, int TileIndex)
{	
	TArray<FString> Errors;
	if(FilesToRead.IsEmpty())
	{
		Errors.Add(FString("No Files to read from"));
		return Errors;
	}
	if(GisFM->GetInfos()->Tiles.Num() == 0)
	{
		if(!FetchAuthorityID(FilesToRead[0]))
		{
			Errors.Add(FString::Printf(TEXT("File %s does not contain valid GIS information or could not be projected to desired CRS"), *FilesToRead[0]));
			return Errors;
		}
	}

	LandscapeDatas = TArray<RasterLandscapeData>();
	FScopedSlowTask SlowTask(2, FText::FromString("Reading Files"));
	SlowTask.MakeDialog();
	SlowTask.EnterProgressFrame();
	RasterLandscapeData LandscapeData = RasterLandscapeData(GisFM);
	ULandscapingSettings* Settings = GetMutableDefault<ULandscapingSettings>();
	if(Settings->bReadParallel && FilesToRead.Num() > 1)
	{
		ParallelFor(FilesToRead.Num(),  [&](int Index)
		{
			FString FileToRead = FilesToRead[Index];
			LandscapeData.AddRasterData(FileToRead, WorkingDir, TileIndex);
		});
	}
	else
	{
		for(FString FileToRead : FilesToRead)
		{
			UE_LOG(LogTemp, Log, TEXT("Landscaping: Read file %s"), *FileToRead);
			LandscapeData.AddRasterData(FileToRead, WorkingDir, TileIndex);
		}
	}

	SlowTask.EnterProgressFrame();
	
	for (RasterData Data : LandscapeData.RasterDatas)
	{
		if(!Data.Error.IsEmpty())
		{
            Errors.Add(Data.Error);
		}
	}
	if(!Errors.IsEmpty())
	{
		return Errors;
	}

	// heightdata first or additional import
	// check if we are dealing with color data for satellite (BandCount >= 3)
	if(!LandscapeData.RasterDatas.IsEmpty() && LandscapeData.RasterDatas[0].BandCount == 1) 
	{
		LandscapeData.UpdateLandscapeData();
		FExtents CroppedExtentsGeogCS = GisFM->GetCRS()->ConvertToGeogCS(LandscapeData.Extents);
		UE_LOG(LogTemp, Log, TEXT("Landscaping: LandscapeData.Extents: %s - CroppedExtentsGeogCS: %s"), *LandscapeData.Extents.ToString(), *CroppedExtentsGeogCS.ToString());
		FExtents CroppedExtentsProj = GisFM->GetCRS()->ConvertFromGeogCS(CroppedExtentsGeogCS);
		if(!CroppedExtentsProj.ToString().Equals(LandscapeData.Extents.ToString()))
		{
			UE_LOG(LogTemp, Warning, TEXT("Landscaping: CroppedExtentsProj: %s - CroppedExtentsGeogCS: %s"), *CroppedExtentsProj.ToString(), *CroppedExtentsGeogCS.ToString());
			FString Warn = FString::Printf(TEXT("Something is wrong with the spatial reference of file %s. Projection is not reversable."), *FilesToRead[0]);
			UE_LOG(LogTemp, Warning, TEXT("Landscaping: %s"), *Warn);
			Errors.Add(FString::Printf(TEXT("Settings do not fit the input file - is the file unit 'meter'?\nIf it shows 'CRS not set', please use 'Automatically reproject to appropriate UTM CRS' or 'Use custom CRS specified below' in 'Project Settings -> Plugins -> Landscaping -> Projection Mode'")));
		}
		GisFM->GetInfos()->SetCroppedExtents(CroppedExtentsGeogCS);
		GisFM->GetInfos()->Extents = LandscapeData.Extents;
	}
	LandscapeDatas.Add(LandscapeData);
	
    return Errors;
}

bool RasterTileFactory::FetchAuthorityID(FString FileToRead)
{
	UE_LOG(LogTemp, Log, TEXT("Landscaping: Fetch AuthorityID with file %s"), *FileToRead);
	if(GisFM->GetCRS()->IsAuthorityIDValid())
	{
		UE_LOG(LogTemp, Log, TEXT("Landscaping: Already got a valid Authority ID EPSG:%i"), GisFM->GetCRS()->GetAuthorityID());
		return true;
	}

	if(GisFM->GetCRS()->IsModeCustomCRS())
	{
		ULandscapingSettings* Settings = GetMutableDefault<ULandscapingSettings>();
		GisFM->GetCRS()->SetAuthorityID(Settings->Projection);
		UE_LOG(LogTemp, Log, TEXT("Landscaping: Mode Custom CRS -> Authority ID EPSG:%i"), GisFM->GetCRS()->GetAuthorityID());
		return true;
	}

	RasterFile* RasterFileObjectTemp = new RasterFile(TCHAR_TO_ANSI(*FileToRead));
	int EPSGOfRasterFile = GisFM->GetCRS()->FindAuthorityID(RasterFileObjectTemp->GetProjection());
	UE_LOG(LogTemp, Log, TEXT("Landscaping: Authority of %s is EPSG:%i WKT: %s"), *FileToRead, EPSGOfRasterFile, *FString(RasterFileObjectTemp->GetProjection()));
	if(!GisFM->GetCRS()->IsEpsgIDValid(EPSGOfRasterFile, false))
	{
		delete RasterFileObjectTemp;
		FString Error = FString::Printf(TEXT("No projection info or invalid info for file %s \nIf you are trying to import ASCII, make sure to provide a corresponding .prj file with projection info in the same folder or set a custom CRS in 'Project Settings -> Plugins -> Landscaping'."), *FileToRead);
		UE_LOG(LogTemp, Error, TEXT("Landscaping: Fetch Authority ID: %s"), *Error);
		return false;
	}
	
	if(GisFM->GetCRS()->IsModeAutoReprojectToUTM())
	{
		double* GeoTransform = RasterFileObjectTemp->GetGeoTransform();
		FVector2D GeogCSPoint = GisFM->GetCRS()->ConvertPointToGeogCS(GeoTransform[0], GeoTransform[3], RasterFileObjectTemp->GetProjection(), FString());
		UE_LOG(LogTemp, Log, TEXT("Landscaping: GeogCSPoint: %s"), *GeogCSPoint.ToString());
		int UTMAuthorityID = GisFM->GetCRS()->GetUTMAuthorityID(GeogCSPoint.X, GeogCSPoint.Y);
		FString UTMEPSG = FString::Printf(TEXT("EPSG:%i"), UTMAuthorityID);
		delete RasterFileObjectTemp;
		if(!UTMEPSG.IsEmpty()) 
		{
			GisFM->GetCRS()->SetAuthorityID(UTMAuthorityID);
			UE_LOG(LogTemp, Log, TEXT("Landscaping: Mode Auto Reproject to UTM -> Authority ID EPSG:%i"), GisFM->GetCRS()->GetAuthorityID());
			return true;
		}
		UE_LOG(LogTemp, Error, TEXT("Landscaping: Mode Auto Reproject to UTM -> failed to find UTM Authority ID"));
		return false;
	}
	if(GisFM->GetCRS()->IsModeUseSourceCRS())
	{
		GisFM->GetCRS()->SetAuthorityID(EPSGOfRasterFile, RasterFileObjectTemp->GetProjection());
		UE_LOG(LogTemp, Log, TEXT("Landscaping: Mode Use Source CRS -> Authority ID EPSG:%i"), GisFM->GetCRS()->GetAuthorityID());
		return true;
	}
	return false;
}

// write mapbox downloaded data into geotiff
TArray<FString> RasterTileFactory::WriteFiles(TArray<RasterData> InDatas)
{
	UE_LOG(LogTemp, Log, TEXT("Landscaping: Write %i Files"), InDatas.Num());
	int CachedFiles = 0;
	GDALAllRegister();
	TArray<FString> OutFilenames = TArray<FString>();
	if(InDatas.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Landscaping: No Data to write"));
		return OutFilenames;
	}
	
	FScopedSlowTask SlowTask(2, FText::FromString("Writing Files"));
	SlowTask.MakeDialog();
	SlowTask.EnterProgressFrame();
	int BandCount = InDatas[0].BandCount;
	for(int i = 0; i < InDatas.Num(); i++)
	{
		InDatas[i].Projection = "EPSG:3857";
		FExtents Extents3857 = GisFM->GetCRS()->ConvertFromTo(FExtents(InDatas[i].Bottom, InDatas[i].Left, InDatas[i].Top, InDatas[i].Right), FString("EPSG:4326"), InDatas[i].Projection);
		InDatas[i].Bottom = Extents3857.Bottom;
		InDatas[i].Left = Extents3857.Left; 
		InDatas[i].Top = Extents3857.Top;
		InDatas[i].Right = Extents3857.Right;
		InDatas[i].MeterPerPixel.X = FMath::Abs(Extents3857.Right - Extents3857.Left) / InDatas[i].ImportResolution.X;
		InDatas[i].MeterPerPixel.Y = (Extents3857.Bottom - Extents3857.Top) / InDatas[i].ImportResolution.Y;
		if(BandCount == 3) // satellite data
		{
			OutFilenames.Add(InDatas[i].Filename);
		}
		if(FPaths::FileExists(InDatas[i].Filename))
		{
			CachedFiles++;
			continue;
		} 
		if(!InDatas[i].Filename.IsEmpty())
		{
			RasterFile* NewFile = new RasterFile(InDatas[i]);
			delete NewFile;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Landscaping: Wrote %i files (found %i in cache)"), InDatas.Num() - CachedFiles, CachedFiles);
	SlowTask.EnterProgressFrame();
	if(BandCount == 1) // heightdata
	{
		FScopedSlowTask SlowTaskMerge(InDatas.Num(), FText::FromString("Merging Files"));
		SlowTaskMerge.MakeDialog();
		// create empty geotiff
		double Left = InDatas[0].Left;
		double Top = InDatas[0].Top;
		TArray<double> Lefts = TArray<double>();
		TArray<double> Tops = TArray<double>();
		for(int i = 0; i < InDatas.Num(); i++)
		{
			Lefts.AddUnique(InDatas[i].Left);
			Tops.AddUnique(InDatas[i].Top);
			Left = InDatas[i].Left < Left ? InDatas[i].Left : Left;
			Top = InDatas[i].Top > Top ? InDatas[i].Top : Top;
		}
		RasterData NewRasterData;
		NewRasterData.ImportResolution.X = InDatas[0].ImportResolution.X * Lefts.Num();
		NewRasterData.ImportResolution.Y = InDatas[0].ImportResolution.Y * Tops.Num();
		NewRasterData.Filename = FString::Printf(TEXT("%s/%s.tif"), *WorkingDir, *LandscapingUtils::GetUniqueFilename("Landscaping/Mapbox.box"));
		NewRasterData.BandCount = InDatas[0].BandCount;
		NewRasterData.Left = Left;
		NewRasterData.Top = Top;
		NewRasterData.MeterPerPixel = InDatas[0].MeterPerPixel;
		NewRasterData.Projection = InDatas[0].Projection;
		
		RasterFile* MosaickedFile = new RasterFile(NewRasterData);
		for(int i = 0; i < InDatas.Num(); i++)
		{
			SlowTaskMerge.EnterProgressFrame();
			RasterFile* TempFile = new RasterFile(InDatas[i].Filename);
			TempFile->Mosaic(MosaickedFile, InDatas[i].Projection);
			delete TempFile;
		}

		OutFilenames.Add(NewRasterData.Filename);
		delete MosaickedFile;
	}
	return OutFilenames;
}

// before actual landscape import
FString RasterTileFactory::PrepareImport(RasterImportOptions Options)
{	
	// crop extents
	if(!GisFM->GetInfos()->GetCroppedExtents().IsEmpty() /*&& !Options.bMapboxImport*/ && !LandscapeDatas.IsEmpty() && !LandscapeDatas[0].RasterDatas.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("Landscaping: Crop extents: %s"), *GisFM->GetInfos()->GetCroppedExtents().ToString());
		GisFM->GetInfos()->Extents = GisFM->GetCRS()->ConvertFromGeogCS(GisFM->GetInfos()->GetCroppedExtents(), LandscapeDatas[0].RasterDatas[0].ProjectionWkt);
	}
	
	if(GisFM->GetInfos()->Tiles.IsEmpty())
	{
		GisFM->GetInfos()->SetOrigin(FVector(GisFM->GetInfos()->Extents.Left, GisFM->GetInfos()->Extents.Top, 0));
	}

	// calculate number of tiles and tile sizes
	for(int i = 0; i < LandscapeDatas.Num(); i++)
	{
		if(!GisFM->GetInfos()->GetCroppedExtents().IsEmpty())
		{
			LandscapeDatas[i].Extents.Left = FMath::Max(LandscapeDatas[i].Extents.Left, GisFM->GetInfos()->Extents.Left);
			LandscapeDatas[i].Extents.Top = FMath::Min(LandscapeDatas[i].Extents.Top, GisFM->GetInfos()->Extents.Top);
			LandscapeDatas[i].Extents.Right = FMath::Min(LandscapeDatas[i].Extents.Right, GisFM->GetInfos()->Extents.Right);
			LandscapeDatas[i].Extents.Bottom = FMath::Max(LandscapeDatas[i].Extents.Bottom, GisFM->GetInfos()->Extents.Bottom);
		}
		double TotalWidthXInMeters = FMath::Abs(LandscapeDatas[i].Extents.Right - LandscapeDatas[i].Extents.Left);
		double TotalWidthYInMeters = FMath::Abs(LandscapeDatas[i].Extents.Bottom - LandscapeDatas[i].Extents.Top);
		if(GisFM->GetInfos()->Tiles.IsEmpty())
		{
			double WidthXDivided = TotalWidthXInMeters / LandscapeDatas[i].NumberTilesX;
			while(WidthXDivided > Options.DesiredMaxTileSize)
			{
				LandscapeDatas[i].NumberTilesX++;
				WidthXDivided = TotalWidthXInMeters / LandscapeDatas[i].NumberTilesX;
			}
			
			double WidthYDivided = TotalWidthYInMeters / LandscapeDatas[i].NumberTilesY;
			while(WidthYDivided > Options.DesiredMaxTileSize)
			{
				LandscapeDatas[i].NumberTilesY++;
				WidthYDivided = TotalWidthYInMeters / LandscapeDatas[i].NumberTilesY;
			}
			LandscapeDatas[i].TileSize.X = FMath::CeilToInt(WidthXDivided / LandscapeDatas[i].GetRasterPixelSize(Options).X);

			// UE tiles landscape import/export of pngs requires specific configs
			if(Options.bSquareTiles)
			{
				TArray<FTileImportConfiguration> SquareConfigs = GetSquareConfigurations();
				for(int SquareConfigsIndex = 0; SquareConfigsIndex < SquareConfigs.Num(); SquareConfigsIndex++)
				{
					FTileImportConfiguration Config = SquareConfigs[SquareConfigsIndex];
					if(Config.SizeX >= LandscapeDatas[i].TileSize.X)
					{
						LandscapeDatas[i].TileSize.X = Config.SizeX - 1;
						break;
					}
				}
				LandscapeDatas[i].TileSize.Y = LandscapeDatas[i].TileSize.X;
			}
			else
			{
				LandscapeDatas[i].TileSize.Y = FMath::CeilToInt(WidthYDivided / LandscapeDatas[i].GetRasterPixelSize(Options).Y);
			}
			UE_LOG(LogTemp, Log, TEXT("Landscaping: TotalWidthXInMeters: %f, WidthXDivided: %f, WidthYDivided: %f, TileSize: %s, Number Of Tiles: %i, %i"), TotalWidthXInMeters, WidthXDivided, WidthYDivided, *LandscapeDatas[i].TileSize.ToString(), LandscapeDatas[i].NumberTilesX, LandscapeDatas[i].NumberTilesY);
		}
		else
		{
			if(Options.bResampleToFirstTile)
			{
				LandscapeDatas[i].TileSize = GisFM->GetInfos()->Tiles[0].ImportResolution;
				LandscapeDatas[i].MeterPerPixel = FVector(GisFM->GetInfos()->Tiles[0].MeterPerPixelX, GisFM->GetInfos()->Tiles[0].MeterPerPixelY, 0);
			}
			else
			{
				LandscapeDatas[i].TileSize.X = FMath::CeilToInt((double)GisFM->GetInfos()->Tiles[0].ImportResolution.X * GisFM->GetInfos()->Tiles[0].MeterPerPixelX / LandscapeDatas[i].GetRasterPixelSize(Options).X);
				LandscapeDatas[i].TileSize.Y = FMath::CeilToInt((double)GisFM->GetInfos()->Tiles[0].ImportResolution.Y * FMath::Abs(GisFM->GetInfos()->Tiles[0].MeterPerPixelY) / LandscapeDatas[i].GetRasterPixelSize(Options).Y);
			}
			LandscapeDatas[i].NumberTilesX = FMath::CeilToInt(TotalWidthXInMeters / ((double)LandscapeDatas[i].TileSize.X * LandscapeDatas[i].GetRasterPixelSize(Options).X));
			LandscapeDatas[i].NumberTilesY = FMath::CeilToInt(TotalWidthYInMeters / ((double)LandscapeDatas[i].TileSize.Y * LandscapeDatas[i].GetRasterPixelSize(Options).Y));
		}
		UE_LOG(LogTemp, Log, TEXT("Landscaping: TileSize: %s Number of Tiles X=%i Y=%i, PixelSize: %s"), *LandscapeDatas[i].TileSize.ToString(), LandscapeDatas[i].NumberTilesX, LandscapeDatas[i].NumberTilesY, *LandscapeDatas[i].GetRasterPixelSize(Options).ToString());
		auto PixelCount = LandscapeDatas[i].TileSize.X * LandscapeDatas[i].TileSize.Y;
		if(PixelCount > INT_MAX || (Options.bImportAsMesh && PixelCount * 6 > INT_MAX))
		{
			UE_LOG(LogTemp, Error, TEXT("Landscaping: Exeeding size limit for Array - please set the 'Desired Max Landscape Size' to a lower value or uncheck 'Resample to first Tile'. Aborting."));
			return FString("Tile size too big - cannot import.");
		}
		LandscapeDatas[i] = MergeOverlapping(i, Options);
		if(LandscapeDatas[i].RasterDatas.IsEmpty())
		{
			return FString("No valid data found");
		}
		RefreshLandscapingInfos(i, Options);
	}
	return FString();
}

void RasterTileFactory::RefreshLandscapingInfos(int LandscapeDataIndex, RasterImportOptions Options)
{
	if(!LandscapeDatas[LandscapeDataIndex].CalculateLandscapeResolution())
	{
		return;
	}
	for(RasterData InRasterData : LandscapeDatas[LandscapeDataIndex].RasterDatas)
	{
		FLandscapingInfo LandscapingInfo = FLandscapingInfo();
		LandscapingInfo.Extents = FExtents(InRasterData.Bottom, InRasterData.Left, InRasterData.Top, InRasterData.Right);
		LandscapingInfo.WGS84Extents = GisFM->GetCRS()->ConvertToGeogCS(LandscapingInfo.Extents, LandscapeDatas[0].RasterDatas[0].ProjectionWkt);
		LandscapingInfo.ImportResolution.X = InRasterData.ImportResolution.X;
		LandscapingInfo.ImportResolution.Y = InRasterData.ImportResolution.Y;
		LandscapingInfo.LandscapeResolution.X = InRasterData.LandscapeResolution.X;
		LandscapingInfo.LandscapeResolution.Y = InRasterData.LandscapeResolution.Y;
		LandscapingInfo.MeterPerPixelX = InRasterData.MeterPerPixel.X;
		LandscapingInfo.MeterPerPixelY = InRasterData.MeterPerPixel.Y;
		LandscapingInfo.MinAltitude = LandscapeDatas[LandscapeDataIndex].MinAltitude;
		LandscapingInfo.MaxAltitude = LandscapeDatas[LandscapeDataIndex].MaxAltitude;
		LandscapingInfo.NumberOfSections = InRasterData.NumberOfSections;
		LandscapingInfo.QuadsPerSection = InRasterData.QuadsPerSection;
		LandscapingInfo.CalculateScale(Options);
		LandscapingInfo.CalculateLocation(GisFM->GetCRS()->GetOrigin());
		LandscapingInfo.Bounds = FBox(ForceInit);
		// bounds are for loading /unloading, exact bounds are derived from landscape after import
		LandscapingInfo.Bounds.Min = FVector(LandscapingInfo.LocationX *GisFM->GetInfos()->LandscapeScaleFactor, LandscapingInfo.LocationY * GisFM->GetInfos()->LandscapeScaleFactor, -HALF_WORLD_MAX);
		LandscapingInfo.Bounds.Max = LandscapingInfo.Bounds.Min + FVector((InRasterData.Right - InRasterData.Left) * GisFM->GetInfos()->LandscapeScaleFactor, (InRasterData.Top - InRasterData.Bottom) * GisFM->GetInfos()->LandscapeScaleFactor, HALF_WORLD_MAX);
		GisFM->GetInfos()->Tiles.Add(LandscapingInfo);
	}
}

TArray<FTileImportConfiguration> RasterTileFactory::GetSquareConfigurations() // from STiledLandscapeImportDlg.cpp GenerateAllPossibleTileConfigurations()
{
	TArray<FTileImportConfiguration> AllConfigurations = TArray<FTileImportConfiguration>();
	for (int32 ComponentsNum = 1; ComponentsNum <= 32; ComponentsNum++)
	{
		for (int32 SectionsPerComponent = 1; SectionsPerComponent <= 2; SectionsPerComponent++)
		{
			for (int32 QuadsPerSection = 3; QuadsPerSection <= 8; QuadsPerSection++)
			{
				FTileImportConfiguration Entry;
				Entry.NumComponents				= ComponentsNum;
				Entry.NumSectionsPerComponent	= SectionsPerComponent;
				Entry.NumQuadsPerSection		= (1 << QuadsPerSection) - 1;
				Entry.SizeX = Entry.NumComponents * Entry.NumSectionsPerComponent * Entry.NumQuadsPerSection + 1;

				AllConfigurations.Add(Entry);
			}
		}
	}

	// Sort by resolution
	AllConfigurations.Sort([](const FTileImportConfiguration& A, const FTileImportConfiguration& B){
		if (A.SizeX == B.SizeX)
		{
			return A.NumComponents < B.NumComponents;
		}
		return A.SizeX < B.SizeX;
	});
	return AllConfigurations;
}

bool RasterTileFactory::HasRasterFile(int LandscapeDataIndex) const
{
    return LandscapeDatas.Num() > LandscapeDataIndex && LandscapeDatas[LandscapeDataIndex].RasterDatas.Num() > 0;
}

RasterData RasterTileFactory::GetFirstRasterData(int LandscapeDataIndex, bool bWithRasterbandData)
{
	if(LandscapeDatas[LandscapeDataIndex].RasterDatas.Num() > 0)
	{
		RasterFile* RasterFileObject = new RasterFile(TCHAR_TO_ANSI(*LandscapeDatas[LandscapeDataIndex].RasterDatas[0].Filename));
		RasterData Data = LandscapeDatas[LandscapeDataIndex].RasterDatas[0];
		if(bWithRasterbandData)
		{
			TArray<TArray<double>> RasterBandData = RasterFileObject->GetRasterBand(1);
			if(RasterBandData.IsEmpty())
			{
				Data.Error = FString::Printf(TEXT("Invalid raster data in file %s"), *Data.Filename);
			}
			else
			{
				Data.RasterBandData = RasterBandData;
			}
		}
		delete RasterFileObject;
		return Data;
	}
	RasterData Data = RasterData();
	Data.Error = "No Data";
	return Data;
}

int RasterTileFactory::GetRasterDataCount(int LandscapeDataIndex)
{
	return LandscapeDatas[LandscapeDataIndex].RasterDatas.Num();
}

int RasterTileFactory::GetLandscapeDataCount()
{
	return LandscapeDatas.Num();
}

FVector RasterTileFactory::GetMeterPerPixel()
{
	if(!LandscapeDatas.IsEmpty() && !LandscapeDatas[0].RasterDatas.IsEmpty())
	{
		return LandscapeDatas[0].MeterPerPixel;
	}
	return FVector(1.0);
}

RasterData RasterTileFactory::GetNextRasterData(int LandscapeDataIndex, int RasterDataIndex)
{
	if(RasterDataIndex < LandscapeDatas[LandscapeDataIndex].RasterDatas.Num())
	{
		FString Filename = LandscapeDatas[LandscapeDataIndex].RasterDatas[RasterDataIndex].Filename;
		RasterFile* RasterFileObject = new RasterFile(TCHAR_TO_ANSI(*Filename));
		TArray<TArray<double>> RasterBandData = RasterFileObject->GetRasterBand(1);
		RasterData Data = LandscapeDatas[LandscapeDataIndex].RasterDatas[RasterDataIndex];
		if(RasterBandData.IsEmpty())
		{
			Data.Error = FString::Printf(TEXT("Invalid raster data in file %s"), *Data.Filename);
		}
		else
		{
			Data.RasterBandData = RasterBandData;
		}
		delete RasterFileObject;
		return Data;
	}
	RasterData Data = RasterData();
	Data.Error = "No Data";
	return Data;
}

TArray<uint16> RasterTileFactory::Import(TArray<TArray<double>> HeightInfo, FIntVector ImportResolution, int TileIndex, RasterImportOptions Options)
{
    // smooth
	if(Options.SmoothSteps > 0)
	{
		double* OutHeightInfo = new double[ImportResolution.X * ImportResolution.Y];
		double* InHeightInfo = new double[ImportResolution.X * ImportResolution.Y];
		ParallelFor (ImportResolution.Y, [&](int IndexY)
		{
			for (int IndexX = 0; IndexX < ImportResolution.X; IndexX++)
			{
				InHeightInfo[IndexY * ImportResolution.X + IndexX] = HeightInfo[IndexY][IndexX];
			}
		});
		FScopedSlowTask SlowTask(Options.SmoothSteps, FText::FromString("Smoothing Landscape"));
		SlowTask.MakeDialog();
		for(int SmoothStep = 0; SmoothStep < Options.SmoothSteps; SmoothStep++)
		{
			FString ProgressStr = FString::Printf(TEXT("Smoothing Landscape %i / %i"), (SmoothStep + 1), Options.SmoothSteps);
			SlowTask.EnterProgressFrame(1.0, FText::FromString(ProgressStr));
			Blur::fast_gaussian_blur(InHeightInfo, OutHeightInfo, ImportResolution.X, ImportResolution.Y, 3.0);
			if(SmoothStep < Options.SmoothSteps)
			{
				std::swap(InHeightInfo, OutHeightInfo);
			}
		}
		
		ParallelFor (ImportResolution.Y, [&](int IndexY)
		{
			for (int IndexX = 0; IndexX < ImportResolution.X; IndexX++)
			{
				// do not smooth the edges, so we can have seamless multiple landscapes
				double AlphaX = IndexX <= (double)ImportResolution.X * 0.5 ? (double)IndexX / ((double)(ImportResolution.X - 1) * 0.5) : (double)(ImportResolution.X - IndexX - 1) / ((double)(ImportResolution.X - 1) * 0.5);
				double AlphaY = IndexY <= (double)ImportResolution.Y * 0.5 ? (double)IndexY / ((double)(ImportResolution.Y - 1) * 0.5) : (double)(ImportResolution.Y - IndexY - 1) / ((double)(ImportResolution.Y - 1) * 0.5);
				double Alpha = AlphaX * AlphaY;
				HeightInfo[IndexY][IndexX] = Options.bSmoothEdges ? OutHeightInfo[IndexY * ImportResolution.X + IndexX] : FMath::InterpEaseOut(HeightInfo[IndexY][IndexX], OutHeightInfo[IndexY * ImportResolution.X + IndexX], Alpha, 15);
			}
		});
		delete OutHeightInfo;
	}

	// import
	// NewMin is the Min value of Height Range; NewMax is the Max value
	// The total height range is 65535, but we only use half of it to have slack above and below
	int NewMin = 16383;
    int NewMax = 49152;
	double MaxMinDiff = GisFM->GetInfos()->Tiles[TileIndex].MaxAltitude - GisFM->GetInfos()->Tiles[TileIndex].MinAltitude;
	double MinAltitude = GisFM->GetInfos()->Tiles[TileIndex].MinAltitude;
	if(!Options.bHighDetailZScale)
	{
		NewMin = 0;
		NewMax = 65535;
		int NewMaxTemp = MaxMinDiff / Options.ZScale * 100 * 128;
		if(NewMaxTemp > 60000) // with 60k we have still 5535 values in the heightrange
		{
			NewMin = 16383;
			NewMax = 49152;
			UE_LOG(LogTemp, Warning, TEXT("Landscaping: invalid Custom Z-Scale. Falling back to High Detail Z-Scale for Tile %i"), TileIndex);
			Options.bHighDetailZScale = true;
			GisFM->GetInfos()->Tiles[TileIndex].CalculateScale(Options);
		}
		else
		{
			int NewRangeRemainder = NewMax - NewMaxTemp;
			NewMax = NewMax - NewRangeRemainder / 2;
			NewMin = NewRangeRemainder / 2;
		}
	}
    TArray<uint16> Data;
	Data.AddDefaulted(ImportResolution.X * ImportResolution.Y);
	if(ImportResolution.Y > ImportResolution.X)
	{
		ParallelFor (ImportResolution.Y, [&](int32 IndexY)
		{
			for (int IndexX = 0; IndexX < ImportResolution.X; IndexX++)
			{
				double HeightValue = HeightInfo[IndexY][IndexX];
				uint16 Height = (NewMax - NewMin)*(HeightValue - MinAltitude) / MaxMinDiff + NewMin;
				Data[IndexY * ImportResolution.X + IndexX] = Height;
			}
		});
	}
	else
	{
		for (int IndexY = 0; IndexY < ImportResolution.Y; IndexY++)
		{
			ParallelFor (ImportResolution.X, [&](int32 IndexX)
			{
				double HeightValue = HeightInfo[IndexY][IndexX];
				uint16 Height = (NewMax - NewMin)*(HeightValue - MinAltitude) / MaxMinDiff + NewMin;
				Data[IndexY * ImportResolution.X + IndexX] = Height;
			});
		}
	}

	ParallelFor (ImportResolution.Y, [&](int32 IndexY)
	{
		HeightInfo[IndexY] = TArray<double>();
	});
	HeightInfo = TArray<TArray<double>>();
    return Data;
}


RasterLandscapeData RasterTileFactory::MergeOverlapping(int Index, RasterImportOptions Options)
{
    // create blank tiles
    TArray<RasterData> BlankTiles = TArray<RasterData>();
    
    double TilesLeft = LandscapeDatas[Index].Extents.Left;
    double TilesTop = LandscapeDatas[Index].Extents.Top;

    if(!GisFM->GetInfos()->Tiles.IsEmpty())
    {
		LandscapeDatas[Index].TileSize = GisFM->GetInfos()->Tiles[0].ImportResolution;
		LandscapeDatas[Index].MeterPerPixel = FVector(GisFM->GetInfos()->Tiles[0].MeterPerPixelX, GisFM->GetInfos()->Tiles[0].MeterPerPixelY, 0);
        bool bShifted = false;
        FExtents CRSCroppedExtents = GisFM->GetCRS()->ConvertFromGeogCS(GisFM->GetCRS()->GetCroppedExtents());
		double RightMostTile = GisFM->GetInfos()->Tiles[0].Extents.Right;
		double BottomMostTile = GisFM->GetInfos()->Tiles[0].Extents.Bottom;
		for(FLandscapingInfo Tile : GisFM->GetInfos()->Tiles)
		{
			RightMostTile = FMath::Max(RightMostTile, Tile.Extents.Left);
			BottomMostTile = FMath::Min(BottomMostTile, Tile.Extents.Top);
		}
        TilesLeft = RightMostTile;
		double ShiftLeft = (double)(GisFM->GetInfos()->Tiles[0].ImportResolution.X - 1) * GisFM->GetInfos()->Tiles[0].MeterPerPixelX + GisFM->GetInfos()->Tiles[0].MeterPerPixelX * 0.5;
		double ShiftTop = (double)(GisFM->GetInfos()->Tiles[0].ImportResolution.Y - 1) * GisFM->GetInfos()->Tiles[0].MeterPerPixelY + GisFM->GetInfos()->Tiles[0].MeterPerPixelY * 0.5;
        while(CRSCroppedExtents.Left < TilesLeft)
        {
            TilesLeft -= ShiftLeft;
			LandscapeDatas[Index].NumberTilesX++;
            bShifted = true;
        }
        TilesTop = BottomMostTile;
        while(CRSCroppedExtents.Top > TilesTop)
        {
            TilesTop -= ShiftTop;
			LandscapeDatas[Index].NumberTilesY++;
            bShifted = true;
        }
    }
    ULandscapingSettings* Settings = GetMutableDefault<ULandscapingSettings>();
	if(Settings->bReadParallel && LandscapeDatas[Index].NumberTilesX * LandscapeDatas[Index].NumberTilesY > 1)
    {
        BlankTiles.AddDefaulted(LandscapeDatas[Index].NumberTilesX * LandscapeDatas[Index].NumberTilesY);
        if(LandscapeDatas[Index].NumberTilesX < LandscapeDatas[Index].NumberTilesY)
        {
            ParallelFor(LandscapeDatas[Index].NumberTilesY, [&](int y)
            {
                for(int x = 0; x < LandscapeDatas[Index].NumberTilesX; x++)
                {
                    RasterData NewRasterFile = CreateBlankTile(Index, x, y, TilesLeft, TilesTop, Options, false);
                    BlankTiles[y * LandscapeDatas[Index].NumberTilesX + x] = NewRasterFile;
                }
            });
        }
        else
        {
            ParallelFor(LandscapeDatas[Index].NumberTilesX, [&](int x)
            {
                for(int y = 0; y < LandscapeDatas[Index].NumberTilesY; y++)
                {
                    RasterData NewRasterFile = CreateBlankTile(Index, x, y, TilesLeft, TilesTop, Options, false);
                    BlankTiles[y * LandscapeDatas[Index].NumberTilesX + x] = NewRasterFile;
                }
            });
        }
    }
    else
    {
        FScopedSlowTask SlowTask(LandscapeDatas[Index].NumberTilesX * LandscapeDatas[Index].NumberTilesY, FText::FromString("Preparing Tiles"));
        SlowTask.MakeDialog();
        for(int x = 0; x < LandscapeDatas[Index].NumberTilesX; x++)
        {
            for(int y = 0; y < LandscapeDatas[Index].NumberTilesY; y++)
            {
                SlowTask.EnterProgressFrame();
                RasterData NewRasterFile = CreateBlankTile(Index, x, y, TilesLeft, TilesTop, Options, true);
                if(NewRasterFile.Error.IsEmpty())
                {
                    BlankTiles.Add(NewRasterFile);
                }
            }
        }
    }
    // merge raster data into blank tiles
    if(LandscapeDatas[Index].RasterDatas.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Landscaping: Cant't merge because we got no data"));
        return RasterLandscapeData(GisFM);
    }
    
    if(LandscapeDatas[Index].RasterDatas.Num() > 1)
    {
        // we merge the lower resolution rasters first, the better resolution ontop to get the best quality
        LandscapeDatas[Index].RasterDatas.Sort([](const RasterData& A, const RasterData& B) { return FMath::Abs(A.MeterPerPixel.X) > FMath::Abs(B.MeterPerPixel.X); });
    }
    FScopedSlowTask SlowTaskMerge(BlankTiles.Num(), FText::FromString("Merging Files into Tiles"));
    SlowTaskMerge.MakeDialog();
    RasterLandscapeData NewLandscapeData = RasterLandscapeData(GisFM);
    for(int BlankTileIndex = 0; BlankTileIndex < BlankTiles.Num(); BlankTileIndex++)
    {
        SlowTaskMerge.EnterProgressFrame();
        RasterData BlankTile = BlankTiles[BlankTileIndex];
        if(!BlankTile.Error.IsEmpty())
        {
            UE_LOG(LogTemp, Error, TEXT("Landscaping: %s"), *BlankTile.Error);
            continue;
        }
        RasterFile* MosaickedFile = nullptr;
        for(RasterData Data : LandscapeDatas[Index].RasterDatas)
        {
            bool bIsOverlapping = BlankTile.IsOverlapping(Data);
            if(bIsOverlapping) 
            {
                if(MosaickedFile == nullptr)
                {
                    MosaickedFile = new RasterFile(TCHAR_TO_ANSI(*BlankTile.Filename));
                }
                RasterFile* RasterFileObjectTemp = new RasterFile(TCHAR_TO_ANSI(*Data.Filename));
				if(Data.Filename.StartsWith("/vsimem/"))
				{
					InMemoryFiles.AddUnique(Data.Filename);
				}
                if(!RasterFileObjectTemp->Mosaic(MosaickedFile, TCHAR_TO_ANSI(*Data.ProjectionWkt)))
                {
                    UE_LOG(LogTemp, Error, TEXT("Landscaping: Mosaicking failed"));
                }
                delete RasterFileObjectTemp;
            }
        }
        if(MosaickedFile != nullptr)
        {
            RasterData NewData = LandscapeDatas[Index].GetRasterData(MosaickedFile, BlankTile.Filename, BlankTile.Filename);
            if(NewData.MinAltitude != NewData.MaxAltitude && NewData.Error.IsEmpty())
            {
                NewLandscapeData.RasterDatas.Add(NewData);
            }
            delete MosaickedFile;
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("Landscaping: No merging for %s (there was no overlap)"), *BlankTile.Filename);
        }
    }
    
    NewLandscapeData.UpdateLandscapeData();
    return NewLandscapeData;
}

RasterData RasterTileFactory::CreateBlankTile(int Index, int x, int y, double TilesLeft, double TilesTop, RasterImportOptions Options, bool bLogError)
{
    FString TimestampStr = to_string(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count()).c_str();
    RasterData NewRasterData;
    NewRasterData.Left = TilesLeft + x * ((LandscapeDatas[Index].TileSize.X - 1) * LandscapeDatas[Index].GetRasterPixelSize(Options).X);
    NewRasterData.Top = TilesTop + y * ((LandscapeDatas[Index].TileSize.Y - 1) * LandscapeDatas[Index].GetRasterPixelSize(Options, true).Y);
    ULandscapingSettings* Settings = GetMutableDefault<ULandscapingSettings>();
    if(Settings->bUseInMemoryFiles)
    {
        NewRasterData.Filename = FString::Printf(TEXT("/vsimem/Tile%s_x%i_y%i.tif"), *TimestampStr, x, y); // in memory
		InMemoryFiles.AddUnique(NewRasterData.Filename);
    }
    else
    {
        NewRasterData.Filename = FString::Printf(TEXT("%s/Tile%s_x%i_y%i.tif"), *WorkingDir, *TimestampStr, x, y);
    }
    NewRasterData.MeterPerPixel = LandscapeDatas[Index].GetRasterPixelSize(Options, true);
    NewRasterData.ImportResolution = FIntVector(LandscapeDatas[Index].TileSize.X, LandscapeDatas[Index].TileSize.Y, 0);
    NewRasterData.BandCount = !LandscapeDatas[Index].RasterDatas.IsEmpty() ? LandscapeDatas[Index].RasterDatas[0].BandCount : 1; // rasterdatas should not be empty here...
    NewRasterData.Projection = GisFM->GetCRS()->GetAuthorityIDStr();
    NewRasterData.ProjectionWkt = GisFM->GetCRS()->GetWkt();
    NewRasterData.NoDataValue = 0; //LandscapeDatas[Index].RasterDatas[0].NoDataValue;
    RasterFile* RasterFileObject = new RasterFile(NewRasterData);
    if(RasterFileObject->IsValid())
    {
        RasterData BlankTile = LandscapeDatas[Index].GetRasterData(RasterFileObject, NewRasterData.Filename, NewRasterData.Filename);
        delete RasterFileObject;
        if(BlankTile.Error.IsEmpty())
        {
            return BlankTile;
        }
    }
    delete RasterFileObject;
    NewRasterData.Error = FString::Printf(TEXT("Raster file creation error %s"), *NewRasterData.Filename);
    if(bLogError)
    {
        UE_LOG(LogTemp, Error, TEXT("Landscaping: Creating blank tiles error. %s"), *FString(NewRasterData.Filename));
    }
    return NewRasterData;
}

////////////////////////////////
/////// Satellite Import ///////
////////////////////////////////
TArray<ColorData> RasterTileFactory::GetColorData(int TileIndex, bool bImportSatImgAsDecal)
{
	TArray<ColorData> ColorDatas;
	if(!(TileIndex < GisFM->GetInfos()->Tiles.Num()))
	{
		UE_LOG(LogTemp, Error, TEXT("Landscaping: Invalid TileIndex when attempting to create texture: %i"), TileIndex);
		return ColorDatas;
	}

	FScopedSlowTask SlowTask(3, FText::FromString(FString::Printf(TEXT("Get Colors for Tile %i"), TileIndex)));
	SlowTask.MakeDialog();
	SlowTask.EnterProgressFrame();

	double MeterPerPixelX = GisFM->GetInfos()->Tiles[TileIndex].MeterPerPixelX;
	double MeterPerPixelY = GisFM->GetInfos()->Tiles[TileIndex].MeterPerPixelY;
	// warp to rootprojection
	TArray<FString> WarpedFilePaths = TArray<FString>();
	if(!GisFM->GetInfos()->Tiles[TileIndex].SatelliteFilenames.IsEmpty()) // SatelliteFilename was set on mapbox import
	{
		for(FString SatelliteFilename : GisFM->GetInfos()->Tiles[TileIndex].SatelliteFilenames)
		{
			FString BaseFilename = LandscapingUtils::GetUniqueFilename("SatTex.tex"); // filename for package
			UE_LOG(LogTemp, Log, TEXT("Landscaping: Create Satellite Texture from Mapbox for Tile %i"), TileIndex);
			RasterFile* TempRasterFile = new RasterFile(SatelliteFilename);
			FString WarpedFilePath;
			ULandscapingSettings* Settings = GetMutableDefault<ULandscapingSettings>();
			if(Settings->bUseInMemoryFiles)
			{
				WarpedFilePath = FString::Printf(TEXT("/vsimem/%s.tif"), *BaseFilename); // in memory
				InMemoryFiles.AddUnique(WarpedFilePath);
			}
			else
			{
				WarpedFilePath = FString::Printf(TEXT("%s/%s.vrt"), *WorkingDir, *BaseFilename);
			}
			if(GisFM->GetCRS()->IsSameAsLevelCRS(TempRasterFile->GetProjection()))
			{
				WarpedFilePath = SatelliteFilename;
			}
			else if(!TempRasterFile->Warp(WarpedFilePath, GisFM->GetCRS()->GetWkt()))
			{
				UE_LOG(LogTemp, Log, TEXT("Landscaping: Projecting Texture %s to EPSG:%i failed"), *SatelliteFilename, GisFM->GetCRS()->GetAuthorityID());
				delete TempRasterFile;
				return ColorDatas;
			}
			delete TempRasterFile;
			RasterFile* WarpedFile = new RasterFile(TCHAR_TO_ANSI(*WarpedFilePath));
			// we have to make sure, we do not downgrade a sat image
			// FYI: MeterPerPixelX = GeoTransform[1] and MeterPerPixelY = GeoTransform[5]
			MeterPerPixelX = FMath::Abs(MeterPerPixelX) <= FMath::Abs(WarpedFile->GetGeoTransform()[1]) ? MeterPerPixelX : WarpedFile->GetGeoTransform()[1];
			MeterPerPixelY = FMath::Abs(MeterPerPixelY) <= FMath::Abs(WarpedFile->GetGeoTransform()[5]) ? MeterPerPixelY : WarpedFile->GetGeoTransform()[5];
			delete WarpedFile;
			UE_LOG(LogTemp, Log, TEXT("Landscaping: Add file (mapbox) %s"), *WarpedFilePath);
			WarpedFilePaths.Add(WarpedFilePath);
		}
	} 
	else if(!LandscapeDatas.IsEmpty() && !LandscapeDatas[0].RasterDatas.IsEmpty()) // file import
	{
		UE_LOG(LogTemp, Log, TEXT("Landscaping: Create Satellite Texture from files for Tile %i"), TileIndex);
		// files are already warped, just add them
		for(int i = 0; i < LandscapeDatas.Num(); i++) // should be only one anyway
		{
			for(int j = 0; j < LandscapeDatas[i].RasterDatas.Num(); j++)
			{
				UE_LOG(LogTemp, Log, TEXT("Landscaping: Add file (selected) %s"), *LandscapeDatas[i].RasterDatas[j].Filename);
				WarpedFilePaths.Add(LandscapeDatas[i].RasterDatas[j].Filename);
				// we have to make sure, we do not downgrade a sat image
				MeterPerPixelX = FMath::Abs(MeterPerPixelX) <= FMath::Abs(LandscapeDatas[i].RasterDatas[j].MeterPerPixel.X) ? MeterPerPixelX : LandscapeDatas[i].RasterDatas[j].MeterPerPixel.X;
				MeterPerPixelY = FMath::Abs(MeterPerPixelY) <= FMath::Abs(LandscapeDatas[i].RasterDatas[j].MeterPerPixel.Y) ? MeterPerPixelY : LandscapeDatas[i].RasterDatas[j].MeterPerPixel.Y;
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Landscaping: No file or data for creating a texture"));
		return ColorDatas;
	}
	if(WarpedFilePaths.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Landscaping: No files or data for this area"));
		return ColorDatas;
	}

	SlowTask.EnterProgressFrame();
	FString BaseFilename = LandscapingUtils::GetUniqueFilename("SatTex.tex"); // filename for package
	// create blank tile of heightmap tile extents and resolution
	RasterData NewRasterData;
	ULandscapingSettings* Settings = GetMutableDefault<ULandscapingSettings>();
	if(Settings->bUseInMemoryFiles)
	{
		NewRasterData.Filename = FString::Printf(TEXT("/vsimem/%s_Tile_%i.tif"), *BaseFilename, TileIndex); // in memory
		InMemoryFiles.AddUnique(NewRasterData.Filename);
	}
	else
	{
		NewRasterData.Filename = FString::Printf(TEXT("%s/%s_Tile_%i.tif"), *WorkingDir, *BaseFilename, TileIndex);
	}
	NewRasterData.Left = GisFM->GetInfos()->Tiles[TileIndex].Extents.Left;
	NewRasterData.Top = GisFM->GetInfos()->Tiles[TileIndex].Extents.Top;
	if(FMath::Abs(MeterPerPixelX) < FMath::Abs(GisFM->GetInfos()->Tiles[TileIndex].MeterPerPixelX))
	{
		NewRasterData.MeterPerPixel = FVector(MeterPerPixelX, MeterPerPixelY, 0);
		NewRasterData.ImportResolution.X = (double)GisFM->GetInfos()->Tiles[TileIndex].ImportResolution.X * GisFM->GetInfos()->Tiles[TileIndex].MeterPerPixelX / MeterPerPixelX;
		NewRasterData.ImportResolution.Y = FMath::Abs((double)GisFM->GetInfos()->Tiles[TileIndex].ImportResolution.Y * GisFM->GetInfos()->Tiles[TileIndex].MeterPerPixelY / MeterPerPixelY);
	}
	else
	{
		NewRasterData.MeterPerPixel = FVector(GisFM->GetInfos()->Tiles[TileIndex].MeterPerPixelX, GisFM->GetInfos()->Tiles[TileIndex].MeterPerPixelY, 0);
		NewRasterData.ImportResolution = GisFM->GetInfos()->Tiles[TileIndex].ImportResolution;
	}
	NewRasterData.BandCount = 3;
	NewRasterData.Projection = GisFM->GetCRS()->GetAuthorityIDStr();
	NewRasterData.ProjectionWkt = GisFM->GetCRS()->GetWkt();
	RasterFile* NewTile = new RasterFile(NewRasterData);
	if(NewTile == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Landscaping: There was a problem creating the target image - new image is null"))
		return ColorDatas;
	}
	if(!NewTile->IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Landscaping: There was a problem creating the target image - new image is not valid"))
		return ColorDatas;
	}

	FScopedSlowTask MergeSlowTask(WarpedFilePaths.Num(), FText::FromString("Merge Textures"));
	MergeSlowTask.MakeDialog();
	// merge warped file into blank tile
	for(int i = 0; i < WarpedFilePaths.Num(); i++)
	{
		//  UE_LOG(LogTemp, Log, TEXT("Landscaping ---- WARPED FILE[%i]: %s"),i, *WarpedFilePaths[i]);
		MergeSlowTask.EnterProgressFrame();
		RasterFile* RasterFileObjectTemp = new RasterFile(WarpedFilePaths[i]);
		if(!RasterFileObjectTemp->Mosaic(NewTile, GisFM->GetCRS()->GetWkt()))
		{
			UE_LOG(LogTemp, Error, TEXT("Landscaping: Merging %s failed"), *WarpedFilePaths[i]);
		}
		delete RasterFileObjectTemp;
	}

	// get colors from the file
	TArray<TArray<FColor>> ColorBandData = NewTile->GetColorBand();
	TArray<FColor> Data = TArray<FColor>();
	int32 Width = NewTile->GetDimensions()[1];
	int32 Height = NewTile->GetDimensions()[0];
	// UE_LOG(LogTemp, Log, TEXT("Landscaping: Add Color data with w=%i and h=%i"), Width, Height);
	FScopedSlowTask ReadMergedSlowTask(2, FText::FromString(FString::Printf(TEXT("Apply best resolution for Tile %i"), TileIndex)));
	ReadMergedSlowTask.MakeDialog();
	ReadMergedSlowTask.EnterProgressFrame();
	// tiling for decal high res
	int NumberTilesX = 1;
	int NumberTilesY = 1;
	if(bImportSatImgAsDecal)
	{
		int32 NewDim = 8192;
		while(Width / NumberTilesX > NewDim)
		{
			NumberTilesX++;
		}
		while(Height / NumberTilesY > NewDim)
		{
			NumberTilesY++;
		}
	}
	int TileWidth = Width / NumberTilesX;
	int TileHeight = Height / NumberTilesY;
 
	for(int yTile = 0; yTile < NumberTilesY; yTile++)
	{
		for(int xTile = 0; xTile < NumberTilesX; xTile++)
		{
			int xOffset = xTile * TileWidth;
			int yOffset = yTile * TileHeight;
			ColorData NewColorData;
			NewColorData.Data.AddDefaulted(TileHeight * TileWidth);
			for(int y = 0; y < TileHeight; y++)
			{
				for(int x = 0; x < TileWidth; x++)
				{
					NewColorData.Data[y * TileWidth + x] = ColorBandData[y + yOffset][x + xOffset];
				}
			}
			NewColorData.Width = TileWidth;
			NewColorData.Height = TileHeight;
			NewColorData.BaseFilename = BaseFilename;
			NewColorData.NumberTilesX = NumberTilesX;
			NewColorData.NumberTilesY = NumberTilesY;
			ColorDatas.Add(NewColorData);
		}
	}
	ReadMergedSlowTask.EnterProgressFrame();
	SlowTask.EnterProgressFrame();
	delete NewTile;
	
	return ColorDatas;
}

// for satellite imagery
TileConfig RasterTileFactory::CreateTexture(int TileIndex, bool bImportSatImgAsDecal) 
{
	TileConfig SatelliteTiles;
	TArray<ColorData> NewDatas = GetColorData(TileIndex, bImportSatImgAsDecal);
	if(NewDatas.IsEmpty())
	{
		return SatelliteTiles;
	}
	SatelliteTiles.NumberTilesX = NewDatas[0].NumberTilesX;
	SatelliteTiles.NumberTilesY = NewDatas[0].NumberTilesY;
	int32 NewDim = 8192;
	TArray<TArray<FColor>> TexDatas;
	if(bImportSatImgAsDecal)
	{
		for(ColorData TileData : NewDatas)
		{
			TArray<FColor> TexData;
			FImageUtils::ImageResize(TileData.Width, TileData.Height, TileData.Data, NewDim, NewDim, TexData, true);
			TexDatas.Add(TexData);
		}
	}
	else
	{
		TArray<FColor> TexData;
		FImageUtils::ImageResize(NewDatas[0].Width, NewDatas[0].Height, NewDatas[0].Data, NewDim, NewDim, TexData, true);
		TexDatas.Add(TexData);
	}
	TArray<UTexture2D*> Textures;
	for(int i = 0; i < TexDatas.Num(); i++)
	{
		TArray<FColor> TexData = TexDatas[i];
		// Create Package
		FString Filename = FString::Printf(TEXT("T_%s_%i"), *NewDatas[0].BaseFilename, i);
		FString PackageName = FString::Printf(TEXT("/Game/Landscaping/Satellite/%s"), *Filename);
		UPackage* Package = FindPackage(nullptr, *PackageName);
		if (Package == nullptr)
		{
			Package = CreatePackage(*PackageName);
		}
		Package->FullyLoad();
		Package->Modify();

		// create Texture
		FCreateTexture2DParameters Params;
		Params.bDeferCompression = true;
		Params.bSRGB = false;
		Params.MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
		Params.CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
		UTexture2D* Texture = FImageUtils::CreateTexture2D(NewDim, NewDim, TexData, Package, Filename, RF_Public | RF_Standalone | RF_Transactional, Params);

		// save package of texture
		FAssetRegistryModule::AssetCreated(Texture);
		Texture->MarkPackageDirty();
		TArray<UPackage*> PackagesToSave; 
		PackagesToSave.Add(Package);
		FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, false, false);
		FAssetRegistryModule::AssetCreated(Package);
		Package->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT( "Texture created: %s" ), *PackageName);
		SatelliteTiles.Textures.Add(Texture);
	}
	return SatelliteTiles;
}