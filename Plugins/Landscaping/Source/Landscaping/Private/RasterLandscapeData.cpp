// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "RasterLandscapeData.h"

RasterLandscapeData::RasterLandscapeData(UGISFileManager* InGisFileManager)
{
    GisFM = InGisFileManager;
}

void RasterLandscapeData::ResetRasterData()
{
    RasterDatas = TArray<RasterData>();
    MeterPerPixel = FVector(0);
    Extents.Reset();
    TileSize = FIntVector(0);
    NumberTilesX = 1;
    NumberTilesY = 1;
    RasterDataIndex = 0;
    MinAltitude = 65535.0;
    MaxAltitude = -65535.0;
}

FString RasterLandscapeData::ToString()
{
    return FString::Printf(TEXT("TileSize: %s - Meter Per Pixel: %s - Extents: %s Number of Tiles: %i %i - Altitude: %f to %f"),
    *TileSize.ToString(), *MeterPerPixel.ToString(), *Extents.ToString(), NumberTilesX, NumberTilesY, MinAltitude, MaxAltitude);
}

void RasterLandscapeData::UpdateLandscapeData()
{
    for(RasterData Data : RasterDatas)
    {
        //UE_LOG(LogTemp, Log, TEXT("Landscaping: Update Landscape Datas RasterData: %s"), *Data.ToString());
        if(Extents.Left == 0 || Data.Left < Extents.Left)
        {
            Extents.Left = Data.Left;
        }
        if(Extents.Right == 0 || Data.Right > Extents.Right) 
        {
            Extents.Right = Data.Right;
        }
        if(Extents.Top == 0 || Data.Top > Extents.Top)
        {
            Extents.Top = Data.Top;
        }
        if(Extents.Bottom == 0 || Data.Bottom < Extents.Bottom)
        {
            Extents.Bottom = Data.Bottom;
        }
        if(MeterPerPixel.X == 0 || Data.MeterPerPixel.X < MeterPerPixel.X)
        {
            MeterPerPixel.X = Data.MeterPerPixel.X;
        }
        if(MeterPerPixel.Y == 0 || FMath::Abs(Data.MeterPerPixel.Y) < FMath::Abs(MeterPerPixel.Y))
        {
            MeterPerPixel.Y = Data.MeterPerPixel.Y;
        }
        
        MinAltitude = FMath::Min(Data.MinAltitude, MinAltitude);
        MaxAltitude = FMath::Max(Data.MaxAltitude, MaxAltitude);
    }
}

bool RasterLandscapeData::CalculateLandscapeResolution()
{
    if(RasterDatas.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Landscaping: No RasterData for Landscape with Coordinates %s"), *GisFM->GetCRS()->ConvertToGeogCS(Extents).ToString());
        return false;
    }
    
    for(int Index = 0; Index < RasterDatas.Num(); Index++)
    {
        RasterData Data = RasterDatas[Index];
        
        int Width = Data.ImportResolution.X;// * Data.MeterPerPixel.X;
        int Height = Data.ImportResolution.Y;// * FMath::Abs(Data.MeterPerPixel.Y);
        int* Quads = new int[7]{15, 31, 63, 127, 255, 511, 1023};
        int* Sections = new int[2]{1, 2};  
        int QuadsPerSection = Quads[0];
        int NumberOfSections = Sections[0]; 
        int ComponentsX = 1;
        int ComponentsY = 1;

        bool bFoundWidth = false;
        bool bFoundHeight = false;
        for (int q = 0; q < 7; q++)
        {
            QuadsPerSection = Quads[q];
            for (int s = 0; s < 2; s++)
            {
                NumberOfSections = Sections[s];
                bFoundWidth = false;
                bFoundHeight = false;
                for (int c = 1; c < 256; c++)
                {
                    if (!bFoundWidth)
                    {
                        ComponentsX = c;
                    }
                    if (QuadsPerSection * NumberOfSections * ComponentsX + 1 >= Width)
                    {
                        RasterDatas[Index].NumberOfSections = NumberOfSections;
                        RasterDatas[Index].QuadsPerSection = QuadsPerSection;
                        bFoundWidth = true;
                    }
                    if (!bFoundHeight)
                    {
                        ComponentsY = c;
                    }
                    if (QuadsPerSection * NumberOfSections * ComponentsY + 1 >= Height)
                    {
                        RasterDatas[Index].NumberOfSections = NumberOfSections;
                        RasterDatas[Index].QuadsPerSection = QuadsPerSection;
                        bFoundHeight = true;
                    }
                    if (QuadsPerSection < c)
                    {
                        break;                    
                    }
                }
                if (QuadsPerSection < FMath::Max(ComponentsX, ComponentsY))
                {
                    break;
                }
            }
            if (bFoundWidth && bFoundHeight)
            {
                break;
            }
        }
        RasterDatas[Index].LandscapeResolution.X = QuadsPerSection * NumberOfSections * ComponentsX + 1;
        RasterDatas[Index].LandscapeResolution.Y = QuadsPerSection * NumberOfSections * ComponentsY + 1;
    }
    return true;
}

void RasterLandscapeData::AddRasterData(FString FilePath, FString WorkingDir, int TileIndex)
{
    FScopedSlowTask SlowTask(5, FText::FromString("Projecting to desired CRS"));
    SlowTask.MakeDialog();
    
    // check projection
    SlowTask.EnterProgressFrame();
    RasterFile* RasterFileObjectTemp = new RasterFile(FilePath);
    bool bIsAscii = FPaths::GetExtension(FilePath).ToLower().Equals("asc");
    int EPSGOfRasterFile = GisFM->GetCRS()->FindAuthorityID(RasterFileObjectTemp->GetProjection());
    if(EPSGOfRasterFile == 0 && !bIsAscii)
    {
        delete RasterFileObjectTemp;
        RasterData Data;
        Data.Error = FString::Printf(TEXT("No projection info for file %s"), *FilePath);
        RasterDatas.Add(Data);
        return;
    }

    // let's check if we dealing with satellite data and do not add the file, if it is outside
    int* FileDim = RasterFileObjectTemp->GetDimensions();
    if(FileDim[2] >= 3)
    {
        if(GisFM->GetInfos()->Tiles.IsEmpty())
        {
            delete RasterFileObjectTemp;
            RasterData Data;
            Data.Error = FString::Printf(TEXT("Multi-band raster detected. Please use only single-band raster for heightmap import %s"), *FilePath);
            RasterDatas.Add(Data);
            return;
        }
        double* GeoTransform = RasterFileObjectTemp->GetGeoTransform();
        int EpsgOfRaster = GisFM->GetCRS()->FindAuthorityID(RasterFileObjectTemp->GetProjection());
        FExtents FileExtents = RasterFileObjectTemp->GetExtents();
        bool bIsOverlapping = false;
        if(!GisFM->GetCRS()->IsSameAsLevelCRS(RasterFileObjectTemp->GetProjection()))// && EpsgOfRaster != GisFM->GetCRS()->GetAuthorityID())
        {
            FExtents FileGeogCSExtents = GisFM->GetCRS()->ConvertToGeogCS(FileExtents, FString(RasterFileObjectTemp->GetProjection()));
            FExtents TileGeogCSExtents = GisFM->GetCRS()->ConvertToGeogCS(GisFM->GetInfos()->Tiles[TileIndex].Extents);
            bIsOverlapping = FileGeogCSExtents.IsEmpty() || TileGeogCSExtents.IsEmpty() || FileGeogCSExtents.IsOverlapping(TileGeogCSExtents);
        }
        else
        {
            bIsOverlapping = FileExtents.IsOverlapping(GisFM->GetInfos()->Tiles[TileIndex].Extents);
        }
        if(!bIsOverlapping)
        {
            return;
        }
    }

    // reproject to CRS::AuthorityID when neccessary
    bool bIsSameCRS = GisFM->GetCRS()->IsSameAsLevelCRS(RasterFileObjectTemp->GetProjection());
    if(bIsSameCRS)
    {   
        SlowTask.EnterProgressFrame();
        RasterData Data = GetRasterData(RasterFileObjectTemp, FilePath, FilePath);
        delete RasterFileObjectTemp;
        SlowTask.EnterProgressFrame();
        RasterDatas.Add(Data);
        return;
    }

    // reproject
    SlowTask.EnterProgressFrame();
    FString WarpedFilePath;
    ULandscapingSettings* Settings = GetMutableDefault<ULandscapingSettings>();
    if(Settings->bUseInMemoryFiles)
    {
        WarpedFilePath = FString("/vsimem");
        WarpedFilePath.Append("/").Append(LandscapingUtils::GetUniqueFilename(FilePath)).Append(".tif");
    }
    else
    {
        WarpedFilePath = WorkingDir;
        WarpedFilePath.Append("/").Append(LandscapingUtils::GetUniqueFilename(FilePath)).Append(".tif");
    }

    if(RasterFileObjectTemp->Warp(WarpedFilePath, GisFM->GetCRS()->GetWkt()))
    {
        delete RasterFileObjectTemp;
        // fetch and store rasterdata
        SlowTask.EnterProgressFrame();
        RasterFile* RasterFileObject = new RasterFile(WarpedFilePath);
        SlowTask.EnterProgressFrame();
        RasterData Data = GetRasterData(RasterFileObject, FilePath, WarpedFilePath);
        delete RasterFileObject;
        SlowTask.EnterProgressFrame();
        RasterDatas.Add(Data);
        return;
    }
    else 
    {
        delete RasterFileObjectTemp;
        RasterData Data;
        Data.Error = FString::Printf(TEXT("Could not reproject from EPSG:%i to EPSG:%i on file %s"), EPSGOfRasterFile, GisFM->GetCRS()->GetAuthorityID(), *FilePath);
        RasterDatas.Add(Data);
        return;
    }
}

RasterData RasterLandscapeData::GetRasterData(RasterFile* RasterFileObject, FString OriginalFilename, FString ActualFilename, bool bWithRasterbandData)
{
    RasterData Data = RasterData();
    if(!RasterFileObject->IsValid())
    {
        Data.Error = FString::Printf(TEXT("Raster file object not valid: %s"), *Data.Filename);
        // UE_LOG(LogTemp, Error, TEXT("Landscaping: %s"), *Data.Error);
        return Data;
    }

    int EpsgOfRaster = GisFM->GetCRS()->FindAuthorityID(RasterFileObject->GetProjection());
    if(!GisFM->GetCRS()->IsSameAsLevelCRS(RasterFileObject->GetProjection()) && EpsgOfRaster != GisFM->GetCRS()->GetAuthorityID())
    {
        Data.Error = FString::Printf(TEXT("Authority ID of %s does not match: EPSG:%i - It should be EPSG:%i"), *ActualFilename, EpsgOfRaster, GisFM->GetCRS()->GetAuthorityID());
        // UE_LOG(LogTemp, Error, TEXT("Landscaping: %s"), *Data.Error);
        return Data;
    }
    Data.Projection = FString::Printf(TEXT("EPSG:%i"), GisFM->GetCRS()->FindAuthorityID(RasterFileObject->GetProjection()));
    Data.ProjectionWkt = RasterFileObject->GetProjection();
    Data.Filename = ActualFilename;
    Data.OriginalFilename = OriginalFilename;
    if(bWithRasterbandData) // only used for heightdata
    {
        TArray<TArray<double>> RasterBandData = RasterFileObject->GetRasterBand(1);
        if(RasterBandData.IsEmpty())
        {
            Data.Error = FString::Printf(TEXT("Invalid data at band 1 in file %s"), *Data.Filename);
            // UE_LOG(LogTemp, Error, TEXT("Landscaping: %s"), *Data.Error);
            return Data;
        }
        Data.RasterBandData = RasterBandData;
    }
    double* GeoTransform = RasterFileObject->GetGeoTransform();
    Data.Left = GeoTransform[0];
    Data.Top = GeoTransform[3];
    Data.MeterPerPixel.X = GeoTransform[1];
    Data.MeterPerPixel.Y = GeoTransform[5];
    int* Dimensions = RasterFileObject->GetDimensions();
    Data.ImportResolution.X = Dimensions[1]; // rows
    Data.ImportResolution.Y = Dimensions[0]; // columns
    Data.BandCount = Dimensions[2]; // bands
    Data.Right = Data.Left + Data.MeterPerPixel.X * Data.ImportResolution.X;
    Data.Bottom = Data.Top + Data.MeterPerPixel.Y * Data.ImportResolution.Y;
    double* MinMax = RasterFileObject->GetMinAndMaxAltitude(1, Data.Error);
    if(!Data.Error.IsEmpty())
    {
        // UE_LOG(LogTemp, Error, TEXT("Landscaping: AltitudeError: %s"), *Data.Error);
        return Data;
    }
    Data.MinAltitude = MinMax[0];
    Data.MaxAltitude = MinMax[1];
    Data.NoDataValue = RasterFileObject->GetNoDataValue();
    // UE_LOG(LogTemp, Log, TEXT("Landscaping: Got Raster Data: %s"), *Data.ToString());
    return Data;
}

FVector RasterLandscapeData::GetRasterPixelSize(RasterImportOptions Options, bool bPreserveSign)
{
	FVector PixelSize = FVector(1.0, -1.0, 0);
    double Y = bPreserveSign ? MeterPerPixel.Y : FMath::Abs(MeterPerPixel.Y);
    double Sign = bPreserveSign ? FMath::Sign(MeterPerPixel.Y) : 1.0;
	PixelSize.X = Options.bNativeRasterPixelSize ? MeterPerPixel.X : Options.CustomRasterPixelSize;
	PixelSize.Y = Options.bNativeRasterPixelSize ? Y : Options.CustomRasterPixelSize * Sign;
	return PixelSize;
}