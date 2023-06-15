// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

using namespace std;

struct RasterData
{
    double MinAltitude = 65535.0;
    double MaxAltitude = -65535.0;
    double Left = 0;
    double Top = 0;
    double Right = 0;
    double Bottom = 0;
    double LocationX = 0;
    double LocationY = 0;
    double LocationZ = 0;
    int NumberOfSections = 0;
    int QuadsPerSection = 0;
    FIntVector ImportResolution = FIntVector(0);
    FIntVector LandscapeResolution = FIntVector(0);
    FVector MeterPerPixel = FVector(0);
    FString Filename = FString();
    FString OriginalFilename = FString();
    FString Projection = FString();
    FString ProjectionWkt = FString();
    TArray<TArray<double>> RasterBandData = TArray<TArray<double>>();
    TArray<TArray<FColor>> ColorBandData = TArray<TArray<FColor>>();
    double NoDataValue = 0;
    FString Error = FString();
    bool bHasNoData0 = false;
    bool bMapboxImport = false;
    int BandCount = 1;

    // overlaps and touches are counted as overlaps
    bool IsOverlapping(RasterData Other) const
    {
        if(Left > Other.Right || Other.Left > Right)
        {
            return false;
        }
        if(Top < Other.Bottom || Other.Top < Bottom)
        {
            return false;
        }
        return true;
    }

    FString ToString()
    {
        return FString::Printf(TEXT("Extents: %f,%f,%f,%f - %s - Import Res: %s - Landscape Res: %s - Meter Per Pixel: %s - Altitude: %f to %f - Location: %f,%f,%f - BandCout: %i - %s"), 
            Bottom, Left, Top, Right, *Projection, *ImportResolution.ToString(), *LandscapeResolution.ToString(), *MeterPerPixel.ToString(), MinAltitude, MaxAltitude, LocationX, LocationY, LocationZ, BandCount, *Filename);
    }
};