// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#include "LandscapingInfo.h"

// scale //
void FLandscapingInfo::CalculateScale(RasterImportOptions Options) 
{
    if(LandscapeResolution.X == 0 || LandscapeResolution.Y == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Landscaping: Landscape Resolution not valid - unable to calculate Scale"));
        LandscapeScale = FVector(100.0);
    }
    double X = (double)(ImportResolution.X) * MeterPerPixelX / (double)(LandscapeResolution.X) * 100;
    double Y = (double)(ImportResolution.Y) * FMath::Abs(MeterPerPixelY) / (double)(LandscapeResolution.Y) * 100;
    double ScaleZ = (Options.bHighDetailZScale ? GetScaleZ() : Options.ZScale);
    LandscapeScale = FVector(X, Y, ScaleZ);
}

double FLandscapingInfo::GetScaleZ()
{
    // 512 meters is heightmap max, * 100 to get cm -> meter scale, * 2 because we use only half the range of the heightmap (see RasterTileFactory->Import)
    return (MaxAltitude - MinAltitude) / 5.12 * 2;
}

void FLandscapingInfo::CalculateLocation(FVector Origin)
{
    if(LandscapeScale.X == 0 || LandscapeScale.Y == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Landscaping: Landscape Scale not valid - unable to calculate Location! Call CalculateScale() before!"));
        return;
    }
    LocationX = (Extents.Left - Origin.X) * 100;
    LocationY = -(Extents.Top - Origin.Y) * 100;
    LocationZ = (MaxAltitude - (MaxAltitude - MinAltitude) * 0.5) * 100;
}

FVector FLandscapingInfo::GetLocation() const
{
    return FVector(LocationX, LocationY, LocationZ);
}

bool FLandscapingInfo::ContainsSplineGeometry(FString InId) const
{
    for(int i = 0; i < SplineGeometries.Num(); i++)
    {
        if(SplineGeometries[i].Id.Equals(InId))
        {
            return true;
        }
    }
    return false;
}

bool FLandscapingInfo::ContainsLandcoverShape(FString InId) const
{
    for(int i = 0; i < LandcoverShapes.Num(); i++)
    {
        if(LandcoverShapes[i].Id.Equals(InId))
        {
            return true;
        }
    }
    return false;
}

// stringify
FString FLandscapingInfo::ToString() const
{
    return FString::Printf(TEXT("LandscapingInfo - Location: %f,%f,%f - Bounds: %s - LandscapeRes: %s - ImportRes: %s - Scale: %s - Landcover Shapes: %i - Prop Shapes: %i"),
        LocationX, LocationY, LocationZ, *Bounds.ToString(), *LandscapeResolution.ToString(), *ImportResolution.ToString(), *LandscapeScale.ToString(), LandcoverShapes.Num(), SplineGeometries.Num());
}
