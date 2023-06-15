// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#include "VectorTileFactory.h"

VectorTileFactory::VectorTileFactory(UGISFileManager* InGisFileManager, bool InbLanduse)
{
    GisFM = InGisFileManager;
    bLanduse = InbLanduse;
}

VectorTileFactory::~VectorTileFactory()
{

}

FString VectorTileFactory::AddFile(FString Filename, int TileIndex, bool bCheckExtents)
{
    if(GisFM->GetInfos()->Tiles.IsEmpty() || GisFM->GetInfos()->Tiles[TileIndex].Extents.IsEmpty())
    {
        if(GisFM->GetCRS()->IsModeCustomCRS())
        {   
            ULandscapingSettings* Settings = GetMutableDefault<ULandscapingSettings>();
            GisFM->GetCRS()->SetAuthorityID(Settings->Projection);
        }
        else if(GisFM->GetCRS()->IsModeAutoReprojectToUTM())
        {
            VectorFile* VectorFileTemp = new VectorFile(TCHAR_TO_ANSI(*Filename), GisFM, bLanduse);
            FVector ReferencePoint = VectorFileTemp->GetFirstPoint();
            delete VectorFileTemp;
            double Lat = ReferencePoint.X;
            double Lon = ReferencePoint.Y;
            if(Lon == 0 && Lat == 0)
            {
                return FString("Could not find valid reference point in VectorFile");
            }
            UE_LOG(LogTemp, Log, TEXT("Landscaping: VectorFile ReferencePoint: %s"), *ReferencePoint.ToString());
            int UTMAuthorityID = GisFM->GetCRS()->GetUTMAuthorityID(Lon, Lat);
            if(UTMAuthorityID == 0) 
            {
                return FString("Could not determine UTM AuthorityID");
            }
            GisFM->GetCRS()->SetAuthorityID(UTMAuthorityID);
        }
        else if(GisFM->GetCRS()->IsModeUseSourceCRS())
        {
            return FString("Projection Mode 'Use Source Projection' not available on VectorFile only import");
        }
        VectorFile *VectorFileTemp = new VectorFile(TCHAR_TO_ANSI(*Filename), GisFM, bLanduse);
        FVector TopLeftCorner = VectorFileTemp->GetFirstPoint();
        GisFM->GetInfos()->SetExtentsAndBoundsFromLeftTopCorner(TopLeftCorner.X, TopLeftCorner.Y);
        GisFM->GetInfos()->SetOffsetXYFromLeftTopMostTile();
        delete VectorFileTemp;
    }
    
    VectorFile *VectorFileObject = new VectorFile(TCHAR_TO_ANSI(*Filename), GisFM, GisFM->GetInfos()->Tiles[TileIndex].Extents, bLanduse);
    if(VectorFileObject->HasError()) 
    {
        FString Error = VectorFileObject->GetError();
        UE_LOG(LogTemp, Error, TEXT("Landscaping: Add Vectorfile - %s"), *Error);
        delete VectorFileObject;
        return Error;
    }
    
    // UE_LOG(LogTemp, Log, TEXT("Landscaping: Reproject and clip file %s to Extents - %s"), *Filename, *GeogCSExtents.ToString());
    // reproject and clip file to CroppedExtents
    TArray<FVectorData> NewShapes = VectorFileObject->GetShapes();
    for(int i = 0; i < NewShapes.Num(); i++)
    {
        Shapes.Add(NewShapes[i]);
    }
    if(Shapes.IsEmpty()) 
    {
        ULandscapingSettings* Settings = GetMutableDefault<ULandscapingSettings>();
        FString SettingHint = Settings->bExtendedCRSCompatibility ? FString() : FString(" - you might use 'Extended CRS Compatibility' in 'Project Settings -> Plugins -> Landscaping'");
        FString ErrorMsg = FString::Printf(TEXT("Could not find shapes in Bounds of the Landscape%s"), *SettingHint);
        UE_LOG(LogTemp, Error, TEXT("Landscaping: Add File failed: %s"), *ErrorMsg);
        delete VectorFileObject;
        return ErrorMsg;
    }
    bHasVectorFiles = true;
    delete VectorFileObject;
    return FString();
}

TArray<FString> VectorTileFactory::GetAvailableFeatureClasses(TArray<FString> FilterGeom, bool bStripGeomName)
{
    TArray<FString> FeatureClasses = TArray<FString>();
    for(int i = 0; i < Shapes.Num(); i++)
    {
        if(FilterGeom.Contains(Shapes[i].GeometryName) || FilterGeom.IsEmpty())
        {
            if(bStripGeomName)
            {
                FString StrToRemove = FString::Printf(TEXT(" (%s)"), *Shapes[i].GeometryName);
                Shapes[i].FeatureClass.RemoveFromEnd(StrToRemove);
            }
            FeatureClasses.AddUnique(Shapes[i].FeatureClass);
        }
    }
    return FeatureClasses;
}

// return all shapes which touch the bounds (extents) or are inside 
// mapped to unreal 
// crops geometry which is outside
TArray<FVectorData> VectorTileFactory::GetObjectsInBounds(int TileIndex)
{
    TArray<FVectorData> ObjectsInBounds;
    FBox Bounds = GisFM->GetInfos()->Tiles[TileIndex].Bounds;
    if(Bounds.IsValid == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Landscaping: Get Objects In Bounds - Bounds not valid for Tile %i"), TileIndex);
        return ObjectsInBounds;
    }
    FVector PUpperLeft = FVector(Bounds.Min.X, Bounds.Min.Y, 0);
    FVector PUpperRight = FVector(Bounds.Max.X, Bounds.Min.Y, 0);
    FVector PLowerLeft = FVector(Bounds.Min.X, Bounds.Max.Y, 0);
    FVector PLowerRight = FVector(Bounds.Max.X, Bounds.Max.Y, 0);
    FScopedSlowTask SlowTask(Shapes.Num(), FText::FromString("Check bounds for shapes"));
    SlowTask.MakeDialog();
    for(int i = 0; i < Shapes.Num(); i++)
    {
        SlowTask.EnterProgressFrame();
        FVectorData Object;
        Object.Name = Shapes[i].Name;
        Object.Id = Shapes[i].Id;
        Object.GeometryName = Shapes[i].GeometryName;
        Object.FeatureClass = Shapes[i].FeatureClass;
        Object.StringAttributes = Shapes[i].StringAttributes;
        Object.IntegerAttributes = Shapes[i].IntegerAttributes;
        Object.Integer64Attributes = Shapes[i].Integer64Attributes;
        Object.DoubleAttributes = Shapes[i].DoubleAttributes;
        for(int j = 0; j < Shapes[i].Points.Num(); j++)
        {   
            bool AddPoint = false;
            FVector Point = GisFM->GetCRS()->ConvertCRSPointToUnreal(Shapes[i].Points[j]);
            AddPoint = Bounds.IsInsideOrOnXY(Point);

            // next point will be in bounds - add this point on the border intersection of the bounds
            if(!AddPoint && j < Shapes[i].Points.Num() - 1)
            {
                FVector NextPoint = GisFM->GetCRS()->ConvertCRSPointToUnreal(Shapes[i].Points[j + 1]);
                if(Bounds.IsInsideOrOnXY(NextPoint))
                {
                    FVector IntersectPoint;
                    if(FMath::SegmentIntersection2D(Point, NextPoint, PUpperLeft, PUpperRight, IntersectPoint))
                    {
                        Point = IntersectPoint;
                    }
                    else if(FMath::SegmentIntersection2D(Point, NextPoint, PUpperRight, PLowerRight, IntersectPoint))
                    {
                        Point = IntersectPoint;
                    }
                    else if(FMath::SegmentIntersection2D(Point, NextPoint, PLowerRight, PLowerLeft, IntersectPoint))
                    {
                        Point = IntersectPoint;
                    }
                    else if(FMath::SegmentIntersection2D(Point, NextPoint, PLowerLeft, PUpperLeft, IntersectPoint))
                    {
                        Point = IntersectPoint;
                    }
                    AddPoint = true;
                }
                else
                {
                    AddPoint = false;
                }
            }
            // last point was in bounds - add this point on the border intersection of the bounds
            if(!AddPoint && j > 0)
            {
                FVector PreviousPoint = GisFM->GetCRS()->ConvertCRSPointToUnreal(Shapes[i].Points[j - 1]);
                if(Bounds.IsInsideOrOnXY(PreviousPoint))
                {
                    FVector IntersectPoint;
                    if(FMath::SegmentIntersection2D(Point, PreviousPoint, PUpperLeft, PUpperRight, IntersectPoint))
                    {
                        Point = IntersectPoint;
                    }
                    else if(FMath::SegmentIntersection2D(Point, PreviousPoint, PUpperRight, PLowerRight, IntersectPoint))
                    {
                        Point = IntersectPoint;
                    }
                    else if(FMath::SegmentIntersection2D(Point, PreviousPoint, PLowerRight, PLowerLeft, IntersectPoint))
                    {
                        Point = IntersectPoint;
                    }
                    else if(FMath::SegmentIntersection2D(Point, PreviousPoint, PLowerLeft, PUpperLeft, IntersectPoint))
                    {
                        Point = IntersectPoint;
                    }
                    AddPoint = true;
                }
                else
                {
                    AddPoint = false;
                }
            }
            if(AddPoint)
            {
                Object.PointsOriginal.Add(Shapes[i].Points[j]);
                Object.Points.Add(Point);
            }
        }

        if(Object.Points.Num() > 0)
        {
            ObjectsInBounds.Add(Object);
        }
    }
    UE_LOG(LogTemp, Log, TEXT("Landscaping: Got %i shapes in Bounds %s for Tile %i"), ObjectsInBounds.Num(), *Bounds.ToString(), TileIndex);
    return ObjectsInBounds;
}

// return all shapes which touch the bounds (extents) or are inside 
// mapped to unreal 
// does not crop geometry
TArray<FVectorData> VectorTileFactory::GetMappedObjects(int TileIndex)
{
    FBox Bounds = GisFM->GetInfos()->Tiles[TileIndex].Bounds;
    TArray<FVectorData> ShapesInOrIntersectingBounds;
    if(Bounds.IsValid == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Landscaping: Get Mapped Objects - Bounds not valid for Tile %i"), TileIndex);
        return ShapesInOrIntersectingBounds;
    }
    FScopedSlowTask SlowTask(Shapes.Num(), FText::FromString("Get mapped shapes"));
    SlowTask.MakeDialog();
    for(int i = 0; i < Shapes.Num(); i++)
    {
        SlowTask.EnterProgressFrame();
        if(!Shapes[i].GeometryName.Equals("POLYGON"))
        {
            continue;
        }
        FVectorData Object;
        Object.Name = Shapes[i].Name;
        Object.Id = Shapes[i].Id;
        Object.GeometryName = Shapes[i].GeometryName;
        Object.FeatureClass = Shapes[i].FeatureClass;
        if(Shapes[i].Points.Num() > 0)
        {
            bool bIsInsideOrOnXY = false;
            Object.Bounds = FBox(ForceInit);
            for(int j = 0; j < Shapes[i].Points.Num(); j++)
            {
                FVector Point = GisFM->GetCRS()->ConvertCRSPointToUnreal(Shapes[i].Points[j]);
                bIsInsideOrOnXY = bIsInsideOrOnXY || Bounds.IsInsideOrOnXY(Point);
                Object.Bounds += Point;
                Object.PointsOriginal.Add(Shapes[i].Points[j]);
                Object.Points.Add(Point);
            }
            if(bIsInsideOrOnXY)
            {
                ShapesInOrIntersectingBounds.Add(Object);
            }
        }
    }
    ShapesInOrIntersectingBounds.Sort([](const FVectorData& Lhs, const FVectorData& Rhs) { return Lhs.Bounds.Min.Y < Rhs.Bounds.Min.Y; });
    UE_LOG(LogTemp, Log, TEXT("Landscaping: Got %i shapes in or intersecting Bounds %s for Tile %i"), ShapesInOrIntersectingBounds.Num(), *Bounds.ToString(), TileIndex);
    return ShapesInOrIntersectingBounds;
}

// return all shapes mapped to unreal 
TArray<FVectorData> VectorTileFactory::GetObjects()
{
    UE_LOG(LogTemp, Log, TEXT("Landscaping: Get all shapes of file"));
    TArray<FVectorData> ShapesInFile;
    FScopedSlowTask SlowTask(Shapes.Num(), FText::FromString("Get all shapes"));
    SlowTask.MakeDialog();
    for(int i = 0; i < Shapes.Num(); i++)
    {
        SlowTask.EnterProgressFrame();
        FVectorData Object;
        Object.Name = Shapes[i].Name;
        Object.Id = Shapes[i].Id;
        Object.GeometryName = Shapes[i].GeometryName;
        Object.FeatureClass = Shapes[i].FeatureClass;
        Object.StringAttributes = Shapes[i].StringAttributes;
        Object.IntegerAttributes = Shapes[i].IntegerAttributes;
        Object.Integer64Attributes = Shapes[i].Integer64Attributes;
        Object.DoubleAttributes = Shapes[i].DoubleAttributes;
        if(Shapes[i].Points.Num() > 0)
        {
            FVector ObjectExtents = Shapes[i].Points[0];
            for(int j = 0; j < Shapes[i].Points.Num(); j++)
            {   
                FVector Point = GisFM->GetCRS()->ConvertCRSPointToUnreal(Shapes[i].Points[j]);
                Object.PointsOriginal.Add(Shapes[i].Points[j]);
                Object.Points.Add(Point);
            }
            ShapesInFile.Add(Object);
        }
    }
    ShapesInFile.Sort([](const FVectorData& Lhs, const FVectorData& Rhs) { return Lhs.Bounds.Min.Y < Rhs.Bounds.Min.Y; });
    UE_LOG(LogTemp, Log, TEXT("Landscaping: Got %i shapes from file"), ShapesInFile.Num());
    return ShapesInFile;
}

bool VectorTileFactory::HasVectorFiles()
{
    return bHasVectorFiles;
}

TArray<FVectorData> VectorTileFactory::GetShapes(int TileIndex, bool bCrop)
{
    GisFM->GetInfos()->CalculateTileBounds();
    if(bCrop)
    {
        return GetObjectsInBounds(TileIndex);
    }
    return GetMappedObjects(TileIndex);
}
