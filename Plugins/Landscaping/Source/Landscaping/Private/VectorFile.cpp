// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved



#include "VectorFile.h"

VectorFile::VectorFile(FString InFilename, UGISFileManager* InGisFileManager, FExtents Extents, bool InbLanduse)
{
    Filename = InFilename;
    GisFM = InGisFileManager;
    bCheckExtents = true;
    bLanduse = InbLanduse;
    GDALAllRegister();
    OpenVectorFile();
    FetchSourceSpaRef();
    FetchTargetSpaRef();
    ULandscapingSettings* Settings = GetMutableDefault<ULandscapingSettings>();
    if(Settings->bExtendedCRSCompatibility)
    {
        UE_LOG(LogTemp, Warning, TEXT("Landscaping: Using Extended CRS Compatibility which is slow. It can be changed in 'Project Settings -> Plugins -> Landscaping'"));
        FExtents GeogCSExtents = GisFM->GetCRS()->ConvertToGeogCS(Extents);
        ClipExtents = GeogCSExtents;
    }
    else
    {
        ClipExtents = Extents;
    }
    UE_LOG(LogTemp, Log, TEXT("Landscaping: Loading VectorFile %s within GeogCSExtents %s (Clip Extents %s) and %s check Extents"), *InFilename, *Extents.ToString(), *ClipExtents.ToString(), (bCheckExtents ? *FString() : *FString("do not")));
}

VectorFile::VectorFile(FString InFilename, UGISFileManager* InGisFileManager, bool InbLanduse)
{
    UE_LOG(LogTemp, Log, TEXT("Landscaping: Loading VectorFile %s"), *InFilename);
    Filename = InFilename;
    GisFM = InGisFileManager;
    bLanduse = InbLanduse;
    GDALAllRegister();
    OpenVectorFile();
    FetchSourceSpaRef();
    FetchTargetSpaRef();
    // Translate();
}

VectorFile::~VectorFile()
{
    Close();
}

void VectorFile::OpenMVT()
{
    bool bIsValid = false;
    FString Work = FPaths::GetBaseFilename(Filename);
    bIsValid = Work.RemoveFromStart("x");
    if(!bIsValid)
    {
        UE_LOG(LogTemp, Error, TEXT("Landscaping: [1] pbf filename invalid. Should be x<tilenr>y<tilenr>z<zoomlevel>.pbf e.g. x2968y6448z14.pbf"));
        return;
    }
    TArray<FString> Out;
    int32 PartNum = Work.ParseIntoArray(Out,TEXT("y"), true); // Out[0] = x tile
    if(PartNum != 2)
    {
        UE_LOG(LogTemp, Error, TEXT("Landscaping: [2] pbf filename invalid. Should be x<tilenr>y<tilenr>z<zoomlevel>.pbf e.g. x2968y6448z14.pbf"));
        return;
    }
    TArray<FString> Out2;
    PartNum = Out[1].ParseIntoArray(Out2,TEXT("z"), true); // Out2[0] = y tile, Out2[1] = zoom
    
    FString DestinationPath = FString::Printf(TEXT("%s/%s-%s-%s.pbf"), *FPaths::GetPath(Filename), *Out2[1], *Out[0], *Out2[0]);
    FString GpkgFilename = FPaths::GetPath(DestinationPath) + "/" + FPaths::GetBaseFilename(DestinationPath) + ".gpkg";
    if(FPaths::FileExists(GpkgFilename))
    {
        // translated geopackage exists, open it
        Filename = GpkgFilename;
        return OpenVectorFile();
    }

    // change the filename so that ogr understands it is a mapbox vector tile
    if (!FPaths::FileExists(DestinationPath) && !IPlatformFile::GetPlatformPhysical().CopyFile(*DestinationPath, *Filename))
    {
        Error = FString::Printf(TEXT("Could not copy %s to %s"), *Filename, *DestinationPath);
        return;
    }

    // open mapbox pbf
    char** AllowedDrivers = CSLAddString(nullptr, "MVT");
    VectorDataset = GDALOpenEx(TCHAR_TO_ANSI(*DestinationPath), GDAL_OF_VECTOR | GDAL_OF_READONLY | GDAL_OF_SHARED | GDAL_OF_VERBOSE_ERROR, AllowedDrivers, nullptr, nullptr);
    if(VectorDataset == nullptr)
    {
        Error = FString::Printf(TEXT("Open %s failed"), *DestinationPath);
        CSLDestroy(AllowedDrivers);
        return;
    }

    // translate to geopackage
    char** Argv = nullptr;
    Argv = CSLAddString(Argv, "-t_srs");
    Argv = CSLAddString(Argv, "EPSG:3857");
    GDALVectorTranslateOptions* Options = GDALVectorTranslateOptionsNew(Argv, nullptr);
    if(Options == nullptr)
    {
        Error = FString::Printf(TEXT("Creating Translate Options for %s failed"), *Filename);
        CSLDestroy(Argv);
        CSLDestroy(AllowedDrivers);
        return;
    }
    GDALDatasetH NewVectorDataset = GDALVectorTranslate(TCHAR_TO_ANSI(*GpkgFilename), nullptr, 1, &VectorDataset, Options, nullptr);
    if(NewVectorDataset != nullptr)
    {
        Close();
        VectorDataset = NewVectorDataset;
        Filename = GpkgFilename;
    }
    
    if (VectorDataset == nullptr)
    {
        Error = FString::Printf(TEXT("Converting to GeoPackage %s failed"), *DestinationPath);
    }
    GDALVectorTranslateOptionsFree(Options);
    CSLDestroy(Argv);
    CSLDestroy(AllowedDrivers);
}

void VectorFile::OpenVectorFile()
{
    if(Filename.EndsWith(".pbf") && !Filename.EndsWith(".osm.pbf")) // mapbox
    {
        return OpenMVT();
    }
    VectorDataset = GDALOpenEx(TCHAR_TO_ANSI(*Filename), GDAL_OF_VECTOR | GDAL_OF_READONLY | GDAL_OF_SHARED | GDAL_OF_VERBOSE_ERROR, nullptr, nullptr, nullptr);
    if (VectorDataset == nullptr)
    {
        Error = FString::Printf(TEXT("Open %s failed"), *Filename);
    }
}

void VectorFile::Close()
{
    if(VectorDataset != nullptr)
    {
        GDALClose(VectorDataset);
    }
    // GDALDestroyDriverManager();
}

FString VectorFile::GetError()
{
    return Error;
}

bool VectorFile::HasError()
{
    return !Error.IsEmpty();
}

TArray<FVectorData> VectorFile::GetShapes()
{
    FetchFeatures();
    UE_LOG(LogTemp, Log, TEXT("Landscaping: Get %i shapes from file %s"), Geometries.Num(), *Filename);
    return Geometries;
}

bool VectorFile::FetchSourceProjection()
{
    // we arrive here, after projection info fetch from VectorDataSet failed...
    FString FileExtension = FPaths::GetExtension(Filename);
    if(!FileExtension.Equals("pbf") || Filename.EndsWith(".osm.pbf"))
    {
        if(FileExtension.Equals("shp"))
        {
            const FString ProjFilePath = FPaths::ChangeExtension(Filename, "prj");
            if (!FFileHelper::LoadFileToString(SourceProj, *ProjFilePath))
            {
                FString ErrorMsg = FString::Printf(TEXT("Could not find GeoReference for vector data at %s. Please make sure you have also the .prj and .shx file in the same directory."), *ProjFilePath);
                UE_LOG(LogTemp, Error, TEXT("Landscaping: %s"), *ErrorMsg);
                return false;
            }
            return true;
        }
        else
        {
            SourceProj = FString("EPSG:4326");
            UE_LOG(LogTemp, Log, TEXT("Landscaping: %s file without projection info - assuming EPSG:4326"), *FileExtension);
            return true;
        }
    }
    if(FileExtension.Equals("pbf") && !Filename.EndsWith(".osm.pbf"))
    {
        // Filename has to be x<tilenr>y<tilenr>z<zoomlevel>.pbf
        // e.g. x2968y6448z14.pbf
        SourceProj = FString("EPSG:3857");
        UE_LOG(LogTemp, Log, TEXT("Landscaping: Set Source Projection to: %s"), *SourceProj);
        bool bIsValid = false;
        FString Work = FPaths::GetBaseFilename(Filename);
        bIsValid = Work.RemoveFromStart("x");
        if(!bIsValid)
        {
            UE_LOG(LogTemp, Error, TEXT("Landscaping: [1] pbf filename invalid. Should be x<tilenr>y<tilenr>z<zoomlevel>.pbf e.g. x2968y6448z14.pbf"));
            return false;
        }
        TArray<FString> Out;
		int32 PartNum = Work.ParseIntoArray(Out,TEXT("y"), true); // Out[0] = x tile
        if(PartNum != 2)
        {
            UE_LOG(LogTemp, Error, TEXT("Landscaping: [2] pbf filename invalid. Should be x<tilenr>y<tilenr>z<zoomlevel>.pbf e.g. x2968y6448z14.pbf"));
            return false;
        }
        TArray<FString> Out2;
        PartNum = Out[1].ParseIntoArray(Out2,TEXT("z"), true); // Out2[0] = y tile, Out2[1] = zoom
        int ZoomLevel = FCString::Atoi(*Out2[1]);
        OriginX = LandscapingUtils::TileXToLon(FCString::Atoi(*Out[0]), ZoomLevel);
        OriginY = LandscapingUtils::TileYToLat(FCString::Atoi(*Out2[0]), ZoomLevel);
        double MaxX = LandscapingUtils::TileXToLon(FCString::Atoi(*Out[0]) + 1, ZoomLevel);
        double MaxY = LandscapingUtils::TileYToLat(FCString::Atoi(*Out2[0]) + 1, ZoomLevel);
        FVector2D Origin = GisFM->GetCRS()->ConvertPointToGeogCS(OriginX, OriginY, "EPSG:3857", FString(), true);
        FVector2D Max = GisFM->GetCRS()->ConvertPointToGeogCS(MaxX, MaxY, "EPSG:3857", FString(), true);
        int VectorImportResolution = 4096; // valid for mapbox vector tiles
        OriginX = Origin.X;
        OriginY = Origin.Y;
        double SpanX = Max.X - OriginX;
        double SpanY = FMath::Abs(Max.Y - OriginY);
        ResolutionX = VectorImportResolution / SpanX;
        ResolutionY = VectorImportResolution / SpanY;
        OriginY = Max.Y;
        // ClipExtents = GisFM->GetCRS()->ConvertFromTo(ClipExtents, "EPSG:4326", "EPSG:3857");
        UE_LOG(LogTemp, Log, TEXT("Landscaping: pbf file x: %s y: %s zoom: %s - Origin: %f,%f - Max: %f,%f - ClipExtents: %s - Resolution: %f,%f"), *Out[0], *Out2[0], *Out2[1], OriginX, OriginY, Max.X, Max.Y, *ClipExtents.ToString(), ResolutionX, ResolutionY);
        bIsMapboxPbf = true;
        return true;
    }
    UE_LOG(LogTemp, Error, TEXT("Landscaping: Unsupported type: %s."), *FPaths::GetExtension(Filename));
    return false;
}

OGRSpatialReference VectorFile::FetchSourceSpaRef()
{
    if(SourceProj.IsEmpty())
    {
        SourceProj = GDALGetProjectionRef(VectorDataset);
        if(!SourceProj.IsEmpty())
        {
            UE_LOG(LogTemp, Log, TEXT("Landscaping: Found Projection in dataset: %s"), *SourceProj);
        }
    }
    if(SourceProj.IsEmpty())
    {
        const int64 LayerCount = GDALDatasetGetLayerCount(VectorDataset);
        UE_LOG(LogTemp, Log, TEXT("Landscaping: Looking for Projection in %i layers of file %s"), LayerCount, *Filename);
        for (int LayerIndex = 0; LayerIndex < LayerCount; LayerIndex++)
        {
            OGRLayerH Layer = GDALDatasetGetLayer(VectorDataset, LayerIndex);
            UE_LOG(LogTemp, Log, TEXT("Landscaping: Look for Spatial Ref in Layer: %s"), *FString(OGR_L_GetName(Layer)));
            OGRSpatialReferenceH SpaRef = OGR_L_GetSpatialRef(Layer);
            if(SpaRef)
            {
                SourceSpatialRef = *OGRSpatialReference::FromHandle(SpaRef);
                UE_LOG(LogTemp, Log, TEXT("Landscaping: Found Spatial Ref in Layer: %s"), *FString(OGR_L_GetName(Layer)));
                char* Wkt = nullptr;
                OGRErr E = SourceSpatialRef.exportToWkt(&Wkt);
                if(E == OGRERR_NONE)
                {
                    SourceProj = FString(Wkt);
                    UE_LOG(LogTemp, Log, TEXT("Landscaping: Set Source Projection from Layer: %s"), *FString(OGR_L_GetName(Layer)));
                    break;
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("Landscaping: Failed setting Source Projection from Layer: %s"), *GisFM->GetCRS()->GetOGRErrStr(E));
                }
            }
            else if(!Filename.EndsWith(".shp"))
            {
                UE_LOG(LogTemp, Warning, TEXT("Landscaping: No Projection Info in File %s found"), *Filename);
            }
        }
    }    
    if(SourceProj.IsEmpty() && !FetchSourceProjection())
    {
        UE_LOG(LogTemp, Error, TEXT("Landscaping: Could not find Source Projection for %s"), *Filename);
        return SourceSpatialRef;
    }

    if(SourceProj.StartsWith("EPSG:"))
    {
        UE_LOG(LogTemp, Log, TEXT("Landscaping: Set Source Spatial Reference from EPSG (%s)"), *FString(SourceProj));
        FString SourceProjection = SourceProj;
        SourceProjection.RemoveFromStart("EPSG:");
        int EPSG = FCString::Atoi(*SourceProjection);
        SourceSpatialRef.importFromEPSG(EPSG);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Landscaping: Set Source Spatial Reference from WKT"));
        SourceSpatialRef = OGRSpatialReference(TCHAR_TO_ANSI(*SourceProj));
    }

    // char* PrettyWkt;
    // OGRErr E = SourceSpatialRef.exportToPrettyWkt(&PrettyWkt);
    // if(E != OGRERR_NONE)
    // {
    //     UE_LOG(LogTemp, Error, TEXT("Landscaping: Source Spatial Reference Error: %s"), *FString(GisFM->GetCRS()->GetOGRErrStr(E)));
    //     Error = FString("Source Projection Error");
    //     return SourceSpatialRef;
    // }
    // UE_LOG(LogTemp, Log, TEXT("Landscaping: Source WKT:\n%s"), *FString(PrettyWkt));
    
    SourceGeogCSSpatialRef = SourceSpatialRef.CloneGeogCS();
    SourceGeogCSSpatialRef->SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
    if(SourceGeogCSSpatialRef == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Landscaping: SourceGeogCS Spatial Reference creation failed - falling back to EPSG:4326"));
        int Wgs84 = 4326;
        SourceGeogCSSpatialRef->importFromEPSG(Wgs84);
        SourceProj = FString::Printf(TEXT("EPSG:%i"), Wgs84);
    }
    SourceSpatialRef.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
    return SourceSpatialRef;
}

OGRSpatialReference VectorFile::FetchTargetSpaRef()
{
    // might use TargetSpatialRef.SetFromUserInput() instead
    if(!GisFM->GetCRS()->GetWkt().IsEmpty())
    {
        TargetSpatialRef.importFromWkt(TCHAR_TO_ANSI(*GisFM->GetCRS()->GetWkt()));
    }
    else if(GisFM->GetCRS()->GetAuthorityID() != 0)
    {
        TargetSpatialRef.importFromEPSG(GisFM->GetCRS()->GetAuthorityID());
    }
    else
    {
        TargetSpatialRef = *SourceGeogCSSpatialRef;
    }
    TargetSpatialRef.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
    return TargetSpatialRef;
}

void VectorFile::FetchFeatures()
{
    if(Filename.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Reading failed, filename not specified"));
        return;
    }

    if(VectorDataset == nullptr)
    {
        OpenVectorFile();
        FetchSourceSpaRef();
        FetchTargetSpaRef();
    }

    OGRCoordinateTransformationOptions Options;
    if(!ClipExtents.IsEmpty())
    {
        Options.SetAreaOfInterest(ClipExtents.Left, ClipExtents.Bottom, ClipExtents.Right, ClipExtents.Top);
    }
    OGRCoordinateTransformation* CoordinateTransformation = nullptr;
    ULandscapingSettings* Settings = GetMutableDefault<ULandscapingSettings>();
    if(Settings->bExtendedCRSCompatibility)
    {
        CoordinateTransformation = OGRCreateCoordinateTransformation(&SourceSpatialRef, SourceGeogCSSpatialRef, Options);
    }
    else
    {
        CoordinateTransformation = OGRCreateCoordinateTransformation(&SourceSpatialRef, &TargetSpatialRef, Options);
    }
    if(CoordinateTransformation == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("Landscaping: Transformation creation for %s failed"), *Filename);
        return;
    }

    const int64 LayerCount = GDALDatasetGetLayerCount(VectorDataset);
    UE_LOG(LogTemp, Log, TEXT("Landscaping: Found %i layers in %s"), LayerCount, *Filename);
    for (int LayerIndex = 0; LayerIndex < LayerCount; LayerIndex++)
    {
        OGRLayerH Layer = GDALDatasetGetLayer(VectorDataset, LayerIndex);
        const OGRFeatureDefnH FeatureDefinition = OGR_L_GetLayerDefn(Layer);
        OGR_L_ResetReading(Layer);
        OGRFeatureH Feature;
        int64_t FeatureCount = OGR_L_GetFeatureCount(Layer, 0);
        FString Layername = FString(OGR_L_GetName(Layer));
        FScopedSlowTask SlowTask(FeatureCount, FText::FromString(FString::Printf(TEXT("Reading Vector data layer %s"), *Layername)));
        SlowTask.MakeDialog();
        int FeatureCounter = 0;
        while ((Feature = OGR_L_GetNextFeature(Layer)) != nullptr)
        {
            if(FeatureCounter < FeatureCount)
            {
                SlowTask.EnterProgressFrame();
                FeatureCounter++;
            }
            FString Name = FString();
            for (int FieldCount = 0; FieldCount < OGR_FD_GetFieldCount(FeatureDefinition); FieldCount++)
            {
                OGRFieldDefnH FieldDefinition = OGR_FD_GetFieldDefn(FeatureDefinition, FieldCount);
                if(OGR_Fld_GetType(FieldDefinition) == OFTString)
                {
                    FString FieldStr = UTF8_TO_TCHAR(OGR_F_GetFieldAsString(Feature, FieldCount));
                    Name.Append(FieldStr);
                }
            }
            
            OGRGeometryH GeometryReference = OGR_F_GetGeometryRef(Feature);
            const OGRwkbGeometryType GeometryType = wkbFlatten(OGR_G_GetGeometryType(GeometryReference));
            if(bLanduse && !(GeometryType == wkbPolygon || GeometryType == wkbMultiPolygon || GeometryType == wkbGeometryCollection))
            {
                continue;
            }
            if (GeometryReference == nullptr)
            {
                continue;
            }
            if(!bIsMapboxPbf && OGR_G_Transform(GeometryReference, CoordinateTransformation) != OGRERR_NONE)
            {
                UE_LOG(LogTemp, Error, TEXT("Landscaping: Transforming %s for %s failed"), *Name, *Filename);
                continue;
            }
            FString GeomName = FString(OGR_G_GetGeometryName(GeometryReference));
            GetPointsFromGeometry(GeometryReference, Feature, LayerIndex, Name);
        }
    }
    OCTDestroyCoordinateTransformation(CoordinateTransformation);

    if(Settings->bExtendedCRSCompatibility)
    {
        FScopedSlowTask TransformSlowTask(Geometries.Num(), FText::FromString("Transform Vector data"));
        TransformSlowTask.MakeDialog();
        // mapbox import - set geogcs if geopackage could not be created
        if(bIsMapboxPbf)
        {
            SourceGeogCSSpatialRef->importFromEPSG(3857);
            SourceGeogCSSpatialRef->SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
        }
        // // to transform points in parallel is faster than transform geometries in parallel
        // // since gdal is not threadsafe, we can only use it if the the source has the same 
        // // authority id as the target CRS
        for(int GeomIndex = 0; GeomIndex < Geometries.Num(); GeomIndex++)
        {
            FString ProgressStr = FString::Printf(TEXT("Transform Vector data %i / %i"), GeomIndex, Geometries.Num());
            TransformSlowTask.EnterProgressFrame(1.0, FText::FromString(ProgressStr));
            for(int i = 0; i < Geometries[GeomIndex].Points.Num(); i++)
            {
                Geometries[GeomIndex].Points[i] = ConvertPoint(Geometries[GeomIndex].Points[i]);
            }
        }
    }

    if(!Settings->bConnectLinestrings || Geometries.IsEmpty())
    {
        return;
    }
    
    // sort and merge fragmented objects only for linestrings
    TArray<FVectorData> FinalGeometries = TArray<FVectorData>();
    TMap<FString, TArray<FVectorData>> GeometriesMap = TMap<FString, TArray<FVectorData>>();
    
    // group TArray objects together by name
    for(int GeomIndex = 0; GeomIndex < Geometries.Num(); GeomIndex++)
    {
        FVectorData GeometryObject = Geometries[GeomIndex];
        if(GeometryObject.Name.IsEmpty()) 
        {
            FinalGeometries.Add(GeometryObject);
        }
        else if(!GeometriesMap.Contains(GeometryObject.Name))
        {
            TArray<FVectorData> GeomMapVector = TArray<FVectorData>();
            GeomMapVector.Add(GeometryObject);
            GeometriesMap.Add(GeometryObject.Name, GeomMapVector);
        }
        else
        {
            GeometriesMap[GeometryObject.Name].Add(GeometryObject);
        }
    }
    for (TPair<FString, TArray<FVectorData>>& Kvp : GeometriesMap)
    {
        // merge objects into single object
        double FarestSquaredDistance = 0; 
        int FirstObjIndex = GetFirstObjectIndex(Kvp.Value, FarestSquaredDistance);
        FVectorData FinalGeometryObject = Kvp.Value[FirstObjIndex];
        TArray<int> UsedIndizes = TArray<int>();
        TArray<FVector> UsedPoints = TArray<FVector>();
        UsedIndizes.Add(FirstObjIndex);
        int CurrentObjIndex = FirstObjIndex;
        while(UsedIndizes.Num() < Kvp.Value.Num())
        {
            int NextObjIndex = GetNextObjectIndex(CurrentObjIndex, UsedIndizes, FarestSquaredDistance, Kvp.Value);
            UsedIndizes.Add(NextObjIndex);
            for(int PointIndex = 1; PointIndex < Kvp.Value[NextObjIndex].Points.Num(); PointIndex++)
            {
                if(UsedPoints.Contains(Kvp.Value[NextObjIndex].Points[PointIndex]))
                {
                    continue;
                }
                FinalGeometryObject.Points.Add(Kvp.Value[NextObjIndex].Points[PointIndex]);
                UsedPoints.Add(Kvp.Value[NextObjIndex].Points[PointIndex]);
            }
            CurrentObjIndex = NextObjIndex;
        }
        
        FinalGeometries.Add(FinalGeometryObject);
    }
    Geometries = FinalGeometries;
}

int VectorFile::GetFirstObjectIndex(TArray<FVectorData> InGeometries, double &FarestSquaredDistance)
{
    int FirstObjIndex = 0;
    for(int GeomIndex = 0; GeomIndex < InGeometries.Num(); GeomIndex++)
    {
        FVectorData Obj1 = InGeometries[GeomIndex];
        for(int Geom2Index = 0; Geom2Index < InGeometries.Num(); Geom2Index++)
        {
            FVectorData Obj2 = InGeometries[Geom2Index];
            double DistanceX = FMath::Abs(Obj1.Points[Obj1.Points.Num()-1].X-Obj2.Points[0].X);
            double DistanceY = FMath::Abs(Obj1.Points[Obj1.Points.Num()-1].Y-Obj2.Points[0].Y);
            double SquaredDistance = DistanceX * DistanceX + DistanceY * DistanceY;
            if(SquaredDistance > FarestSquaredDistance)
            {
                FarestSquaredDistance = SquaredDistance;
                FirstObjIndex = Geom2Index;
            }
        }
    }
    return FirstObjIndex;
}

int VectorFile::GetNextObjectIndex(int InCurrentIndex, TArray<int> UsedIndizes, double FarestSquaredDistance, TArray<FVectorData> InGeometries)
{
    double NearestSquaredDistance = FarestSquaredDistance;
    int NextObjIndex = 0;
    
    FVectorData Obj1 = InGeometries[InCurrentIndex];
    for(int GeomIndex = 0; GeomIndex < InGeometries.Num(); GeomIndex++)
    {
        if(UsedIndizes.Contains(GeomIndex))
        {
            continue;
        }
        FVectorData Obj2 = InGeometries[GeomIndex];
        double DistanceX = FMath::Abs(Obj1.Points[Obj1.Points.Num()-1].X-Obj2.Points[0].X);
        double DistanceY = FMath::Abs(Obj1.Points[Obj1.Points.Num()-1].Y-Obj2.Points[0].Y);
        double SquaredDistance = DistanceX * DistanceX + DistanceY * DistanceY;
        if(SquaredDistance < FarestSquaredDistance)
        {
            NearestSquaredDistance = SquaredDistance;
            NextObjIndex = GeomIndex;
        }
    }
    return NextObjIndex;
}

FVector VectorFile::GetFirstPoint()
{
    FVector ShpPoint = FVector(0);
    if(VectorDataset == nullptr)
    {
        VectorDataset = GDALOpenEx(TCHAR_TO_ANSI(*Filename), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
    }
    const int LayerCount = GDALDatasetGetLayerCount(VectorDataset);
    OGRCoordinateTransformation* CoordinateTransformation = OGRCreateCoordinateTransformation(&SourceSpatialRef, &TargetSpatialRef);
    if(CoordinateTransformation == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("Landscaping: Could not create transformation for %s"), *Filename);
        return ShpPoint;
    }
    OGRLayerH Layer = GDALDatasetGetLayer(VectorDataset, 0);
    OGRFeatureH Feature;
    bool bCheckExtentsCached = bCheckExtents;
    bCheckExtents = false;
    while ((Feature = OGR_L_GetNextFeature(Layer)) != nullptr)
    {
        OGRGeometryH GeometryReference = OGR_F_GetGeometryRef(Feature);
        GetPointsFromGeometry(GeometryReference, Feature);
        if(Geometries.Num() > 0)
        {
            ShpPoint = Geometries[0].Points[0];
            break;
        }
    }
    OCTDestroyCoordinateTransformation(CoordinateTransformation);
    bCheckExtents = bCheckExtentsCached;
    return ShpPoint;
}

void VectorFile::GetPointsFromGeometry(OGRGeometryH GeometryReference, OGRFeatureH Feature, int LayerNr, FString FormerGeometryTypeStr)
{
    const OGRwkbGeometryType GeometryType = wkbFlatten(OGR_G_GetGeometryType(GeometryReference));
    FString GeometryTypeStr = OGR_G_GetGeometryName(GeometryReference);
    if (GeometryType == wkbPolygon || GeometryType == wkbMultiPoint || GeometryType == wkbMultiLineString
        || GeometryType == wkbMultiPolygon || GeometryType == wkbGeometryCollection)
    {

        const int GeometryNum = OGR_G_GetGeometryCount(GeometryReference);
        for (int iSubGeom = 0; iSubGeom < GeometryNum; iSubGeom++)
        {
            const OGRGeometryH SubGeom = OGR_G_GetGeometryRef(GeometryReference, iSubGeom);
            FString GeometryTypeStrToPass = GeometryTypeStr;
            if(!GeometryTypeStrToPass.Equals("POLYGON"))
            {
                GeometryTypeStrToPass = FormerGeometryTypeStr;
            }
            GetPointsFromGeometry(SubGeom, Feature, LayerNr, GeometryTypeStrToPass);
        }
    }
    else if (GeometryType == wkbPoint || GeometryType == wkbLineString)
    {
        FVectorData GeometryObject;
        int FClassFieldIndex = INDEX_NONE;
        FString FeatureClassStr = FString();
        for(FString FieldName : FIELD_NAMES)
        {
            int FieldIndex = OGR_F_GetFieldIndex(Feature, TCHAR_TO_ANSI(*FieldName));
            if(FieldIndex > INDEX_NONE)
            {
                FeatureClassStr = UTF8_TO_TCHAR(OGR_F_GetFieldAsString(Feature, FieldIndex));
                if(!FeatureClassStr.IsEmpty() 
                    && !FeatureClassStr.ToLower().TrimStartAndEnd().Equals("true") 
                    && !FeatureClassStr.ToLower().TrimStartAndEnd().Equals("false"))
                {
                    GeometryObject.FeatureClass = FeatureClassStr;
                    break;
                }
            }
        }
        if(GeometryObject.FeatureClass.IsEmpty())
        {
            OGRLayerH Layer = GDALDatasetGetLayer(VectorDataset, LayerNr);
            FString Layername = FString(OGR_L_GetName(Layer));
            GeometryObject.FeatureClass = Layername;
        }
        if(bLanduse)
        {
            FString FieldName = FString("name");
            int FieldIndex = OGR_F_GetFieldIndex(Feature, TCHAR_TO_ANSI(*FieldName));
            if(FieldIndex > INDEX_NONE)
            {
                GeometryObject.Name = UTF8_TO_TCHAR(OGR_F_GetFieldAsString(Feature, FieldIndex));
            }
        }
        else
        {
            OGRLayerH Layer = GDALDatasetGetLayer(VectorDataset, LayerNr);
            const OGRFeatureDefnH FeatureDefinition = OGR_L_GetLayerDefn(Layer);
            for(int FieldCount = 0; FieldCount < OGR_FD_GetFieldCount(FeatureDefinition); FieldCount++)
            {
                OGRFieldDefnH FieldDefinition = OGR_FD_GetFieldDefn(FeatureDefinition, FieldCount);
                FString FieldName = FString(OGR_Fld_GetNameRef(FieldDefinition));
                int FieldValueInt;
                int64 FieldValueInt64;
                double FieldValueDouble;
                FString FieldValueStr;
                switch(OGR_Fld_GetType(FieldDefinition))
                {
                    case OFTInteger:
                        FieldValueInt = OGR_F_GetFieldAsInteger(Feature, FieldCount);
                        GeometryObject.IntegerAttributes.Add(FieldName, FieldValueInt);
                        break;
                    case OFTInteger64:
                        FieldValueInt64 = OGR_F_GetFieldAsInteger64(Feature, FieldCount);
                        GeometryObject.Integer64Attributes.Add(FieldName, FieldValueInt64);
                        break;
                    case OFTReal:
                        FieldValueDouble = OGR_F_GetFieldAsDouble(Feature, FieldCount);
                        GeometryObject.DoubleAttributes.Add(FieldName, FieldValueDouble);
                        break;
                    case OFTString:
                        FieldValueStr = UTF8_TO_TCHAR(OGR_F_GetFieldAsString(Feature, FieldCount));
                        GeometryObject.StringAttributes.Add(FieldName, FieldValueStr);
                        if(FieldName.Equals("name"))
                        {
                            GeometryObject.Name = FieldValueStr;
                        }
                        else if(GeometryObject.FeatureClass.IsEmpty() 
                            && !FieldName.Equals("name") 
                            && !FieldName.Equals("id") 
                            && !FieldName.Equals("type")
                            && !FieldValueStr.ToLower().TrimStartAndEnd().Equals("true") 
                            && !FieldValueStr.ToLower().TrimStartAndEnd().Equals("false")
                            && !FieldValueStr.IsEmpty())
                        {
                            GeometryObject.FeatureClass = FieldValueStr;
                        }
                        break;
                    default:
                        FieldValueStr = UTF8_TO_TCHAR(OGR_F_GetFieldAsString(Feature, FieldCount));
                        GeometryObject.StringAttributes.Add(FieldName, FieldValueStr);
                        break;
                }
            }
        }

        if(GeometryObject.FeatureClass.IsEmpty())
        {
            return;
        }

        if(GeometryType == wkbPoint)
        {
            GeometryObject.GeometryName = "POINT";
        }
        else
        {
            GeometryObject.GeometryName = FormerGeometryTypeStr.Equals("POLYGON") ? FormerGeometryTypeStr : "LINESTRING";
        }

        const int PointNum = OGR_G_GetPointCount(GeometryReference);
        GeometryObject.Points = TArray<FVector>();
        bool bPointsInExtents = !bCheckExtents;
        for (int iPoint = 0; iPoint < PointNum; iPoint++)
        {   
            FVector ShpPoint = FVector(0);
            ShpPoint.X = OGR_G_GetX(GeometryReference, iPoint);
            ShpPoint.Y = OGR_G_GetY(GeometryReference, iPoint);
            ShpPoint.Z = OGR_G_GetZ(GeometryReference, iPoint);
         
            // add origin on pbf vector tiles from Mapbox
            if(bIsMapboxPbf)
            {
                ShpPoint.X = ShpPoint.X / ResolutionX + OriginX;
                ShpPoint.Y = ShpPoint.Y / ResolutionY + OriginY;
            }
            bPointsInExtents = bPointsInExtents || IsInArea(ShpPoint); // check if shape intersects ClipExtents
            GeometryObject.Points.Add(ShpPoint);
        }
        if(GeometryObject.Points.Num() > 0 && bPointsInExtents)
        {
            GeometryObject.FeatureClass = FString::Printf(TEXT("%s (%s)"), *GeometryObject.FeatureClass, *GeometryObject.GeometryName);
            GeometryObject.Id = FString::Printf(TEXT("%s_%s_%s"), *Filename, *GeometryObject.FeatureClass, *GeometryObject.Points[0].ToString());
            Geometries.Add(GeometryObject);
        }
    }
}

bool VectorFile::IsInArea(FVector Point) 
{
    if(Point.X < ClipExtents.Left || Point.X > ClipExtents.Right)
    {
        return false;
    }
    if(Point.Y > ClipExtents.Top || Point.Y < ClipExtents.Bottom)
    {
        return false;
    }
    return true;
}

// converts point from source GeogCS to target CRS (basically lon lat to CRS)
FVector VectorFile::ConvertPoint(FVector Point)
{
    OGRPoint p2(Point.X, Point.Y, Point.Z);
    p2.assignSpatialReference(SourceGeogCSSpatialRef);
    OGRErr E = p2.transformTo(&TargetSpatialRef);
    FVector ConvertedPoint = FVector(p2.getX(), p2.getY(), p2.getZ());
    if(E != OGRERR_NONE)
    {
        //UE_LOG(LogTemp, Error, TEXT("Landscaping: Convert Point to Target CRS failed (%s) Original: %s - Converted: %s"), *GisFM->GetCRS()->GetOGRErrStr(E), *Point.ToString(), *ConvertedPoint.ToString());
        return Point;
    }
    return ConvertedPoint;
}
