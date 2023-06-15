// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved



#include "WeightmapImporter.h"

WeightmapImporter::WeightmapImporter(UGISFileManager* InGisFileManager)
{
    GisFM = InGisFileManager;
    TileFactory = new VectorTileFactory(GisFM, true);
    FilterGeom.Add(FString("POLYGON"));
}

WeightmapImporter::~WeightmapImporter()
{
    delete TileFactory;
    TileFactory = nullptr;
}

// full load of files with geometry
TArray<FString> WeightmapImporter::LoadFiles(TArray<FString> InFilenames, int TileIndex)
{
    delete TileFactory;
    TileFactory = new VectorTileFactory(GisFM, true);
    UE_LOG(LogTemp, Log, TEXT("Landscaping: Load vector data of %i files for Tile - %i"), InFilenames.Num(), TileIndex);
    for(int Index = 0; Index < InFilenames.Num(); Index++)
    {
        FString Error = TileFactory->AddFile(InFilenames[Index], TileIndex, true);
    }

    TArray<FString> TileFeatureClasses = TileFactory->GetAvailableFeatureClasses(FilterGeom, true);
    for(FString FeatureClass : TileFeatureClasses)
    {
        FeatureClasses.AddUnique(FeatureClass);
    }
    
    GisFM->GetInfos()->Tiles[TileIndex].LandcoverShapes = GetLandcoverShapes(FeatureClasses, TileIndex);
    return FeatureClasses;
}

TArray<FVectorData> WeightmapImporter::GetLandcoverShapes(TArray<FString> InFeatureClasses, int TileIndex)
{
    TArray<FVectorData> Geoms = TileFactory->GetShapes(TileIndex, false);
    TArray<FVectorData> OutGeom;
    
    for(FVectorData Geom : Geoms)
    {
        for(FString FeatureClass : InFeatureClasses)
        {   
            if(Geom.FeatureClass.Equals(FeatureClass))
            {
                OutGeom.Add(Geom);
            }
        }
    }
    return OutGeom;
}

// create weightmaps
bool WeightmapImporter::CreateWeightmaps(UMaterialInterface* LandscapeMaterial, TArray<TSharedPtr<MaterialLayerSettings>> LayerSettingsList, int TileIndex, FString DefaultLayer)
{
    ALandscapeProxy* SelectedLandscape = GisFM->GetInfos()->Tiles[TileIndex].Landscape;
    if(SelectedLandscape == nullptr)
    {
        if(GisFM->GetInfos()->Tiles[TileIndex].LandscapeSM == nullptr) 
        {
            UE_LOG(LogTemp, Error, TEXT("Landscaping: No Landscape or LandscapeProxy found - Please assign the Landscape in 'LandscapingInfos Actor -> Tiles'"));
        }
        return false;
    }

    // save level so we do not have 'Untitled' folders in the project
    UEditorLoadingAndSavingUtils::SaveCurrentLevel();
   
    if(GetWorld() == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("Landscaping: No Level loaded"));
        return false;
    }
    FLandscapingInfo InInfo = GisFM->GetInfos()->Tiles[TileIndex];
    
    if(InInfo.LandscapeResolution.X == 0 || InInfo.LandscapeResolution.Y == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Landscaping: LandscapeResolution not valid - abort"));
        return false;
    }
    SelectedLandscape->LandscapeMaterial = LandscapeMaterial;
    bool bSuccess = Import(SelectedLandscape, LandscapeMaterial, LayerSettingsList, TileIndex, DefaultLayer);
    if(bSuccess)
    {
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
                    Proxies[i]->LandscapeMaterial = LandscapeMaterial;
                    FPropertyChangedEvent PropertyChangedEventProxy(FindFieldChecked<FProperty>(Proxies[i]->GetClass(), FName("LandscapeMaterial")));
                    Proxies[i]->PostEditChangeProperty(PropertyChangedEventProxy);
                    Proxies[i]->MarkPackageDirty();
                    Proxies[i]->PostEditChange();
                }
            }
        }
    }
    return bSuccess;
}

void WeightmapImporter::ClearLayers(TArray<int> SelectedTiles)
{
    for(int TileIndex : SelectedTiles)
    {
        if(GisFM->GetInfos()->Tiles[TileIndex].Landscape == nullptr)
        {
            continue;
        }
        ALandscapeProxy* LandscapeProxy = GisFM->GetInfos()->Tiles[TileIndex].Landscape;
        ULandscapeInfo* LandscapeInfo = LandscapeProxy->GetLandscapeInfo();

        ALandscape* Landscape = (ALandscape*)LandscapeProxy;
        FLandscapeLayer* CurrentLayer = Landscape->GetLayer(0);
        FGuid Guid = CurrentLayer ? CurrentLayer->Guid : FGuid();
        
        FScopedSetLandscapeEditingLayer Scope(Landscape, Guid, [&] { check(Landscape); Landscape->RequestLayersContentUpdate(ELandscapeLayerUpdateMode::Update_Weightmap_All); });

        for(int i = 0; i < LandscapeProxy->EditorLayerSettings.Num(); i++)
        {
            if(LandscapeProxy->EditorLayerSettings[i].LayerInfoObj != nullptr)
            {
                LandscapeInfo->DeleteLayer(LandscapeProxy->EditorLayerSettings[i].LayerInfoObj,  LandscapeProxy->EditorLayerSettings[i].LayerInfoObj->LayerName);
            }
        }
    }
}

bool WeightmapImporter::Import(ALandscapeProxy* LandscapeProxy, UMaterialInterface* LandscapeMaterial, TArray<TSharedPtr<MaterialLayerSettings>> LayerSettingsList, int TileIndex, FString DefaultLayer)
{
    UE_LOG(LogTemp, Log, TEXT("Landscaping: Import Weightmap for Tile %i"), TileIndex);
    if(LandscapeProxy == nullptr)
    {   
        UE_LOG(LogTemp, Error, TEXT("Landscaping: Landscape not valid - no Level loaded?"));
        return false;
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
        UE_LOG(LogTemp, Error, TEXT("Landscaping: LandscapeProxy not valid or extent of Landscape unknown."))
        return false;
    }

    FLandscapeEditDataInterface LandscapeEdit(LandscapeInfo);
    TArray<FLandscapeImportLayerInfo> LayerInfos = GetLayerInfos(LandscapeMaterial, LayerSettingsList, TileIndex, DefaultLayer);
    FScopedSlowTask CreateSlowTask(LayerSettingsList.Num(), FText::FromString("Applying Weightmaps"));
    CreateSlowTask.MakeDialog();
    for(FLandscapeImportLayerInfo LayerInfo : LayerInfos)
    {
        FString ProgressStr = FString::Printf(TEXT("Applying Weightmap for Layer %s"), *LayerInfo.LayerName.ToString());
        UE_LOG(LogTemp, Log, TEXT("Landscaping: Vector Weightmap Import - %s"), *ProgressStr);
		CreateSlowTask.EnterProgressFrame(1.0, FText::FromString(ProgressStr));
        LandscapeProxy->EditorLayerSettings.Add(FLandscapeEditorLayerSettings(LayerInfo.LayerInfo));
        // FAlphamapAccessor<false, false> AlphamapAccessor(LandscapeInfo, LayerInfo.LayerInfo);
        if(LayerInfo.LayerData.Num() > 0)
        {
            //AlphamapAccessor.SetData(MinX, MinY, MaxX, MaxY, LayerInfo.LayerData.GetData(), ELandscapeLayerPaintingRestriction::None);
            LandscapeEdit.SetAlphaData(LayerInfo.LayerInfo, MinX, MinY, MaxX, MaxY, LayerInfo.LayerData.GetData(), 0, ELandscapeLayerPaintingRestriction::None, true, false);
        }
    }

    LandscapeEdit.Flush();
    
    LandscapeProxy->MarkPackageDirty();
    return true;
}

TArray<FLandscapeImportLayerInfo> WeightmapImporter::GetLayerInfos(UMaterialInterface* LandscapeMaterial, TArray<TSharedPtr<MaterialLayerSettings>> LayerSettingsList, int TileIndex, FString DefaultLayer)
{
    TArray<FLandscapeImportLayerInfo> LayerInfos;
    if(GetWorld() == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("Landscaping: No Level loaded"));
        return LayerInfos;
    }
    FLandscapingInfo InInfo = GisFM->GetInfos()->Tiles[TileIndex];
    
    UsedLayerIndizes.Empty();
    for(int i = 0; i < InInfo.LandscapeResolution.X * InInfo.LandscapeResolution.Y; i++)
    {
        UsedLayerIndizes.Add(false);
    }

    int DefaultLayerIndex = 0;
    for (TSharedPtr<MaterialLayerSettings> LayerSettings : LayerSettingsList)
    {	
            FString ProgressStr = FString::Printf(TEXT("Creating Weightmap for Layer %s"), *LayerSettings.Get()->Name.ToString());
            UE_LOG(LogTemp, Log, TEXT("Landscaping: Weightmaps - %s"), *ProgressStr);
            FLandscapeImportLayerInfo LayerInfo = FLandscapeImportLayerInfo(LayerSettings.Get()->Name);
            LayerInfo.LayerInfo = GetLandscapeLayerInfoObject(LayerSettings, GetWorld()->GetOutermost()->GetName(), TileIndex);
            
            LayerInfo.LayerData.AddZeroed(InInfo.LandscapeResolution.X * InInfo.LandscapeResolution.Y * sizeof(uint8));
            
        if(LayerSettings.Get()->bWeightBlended && !LayerSettings.Get()->Name.ToString().Equals(DefaultLayer))
        {
            TArray<FString> LandcoverTypes;
            for(TSharedPtr<Landuse> LanduseItem : LayerSettings.Get()->LanduseList)
            {
                if(LanduseItem.Get()->bActive)
                {
                    LandcoverTypes.Add(LanduseItem.Get()->LanduseType);
                }
            }
            TArray<FVectorData> LandcoverShapes;
            for(FVectorData LandcoverShape : InInfo.LandcoverShapes)
            {
                for(FString LandcoverType : LandcoverTypes)
                {
                    if(LandcoverShape.FeatureClass.Equals(LandcoverType))
                    {
                        LandcoverShapes.Add(LandcoverShape);
                    }
                }
            }
            // create weight data
            if(LandcoverShapes.Num() > 0)
            {
                LayerInfo.LayerData = CreateWeightData(InInfo, LandcoverShapes);
                if(LayerSettings.Get()->NoiseTexture != nullptr)
                {   
                    TArray<uint8> NoisePixels = GetPixels(LayerSettings.Get()->NoiseTexture, *LayerSettings.Get()->ColorChannel.Get(), LayerSettings.Get()->Tiling);
                    FTexture2DMipMap* MipMap = &LayerSettings.Get()->NoiseTexture->GetPlatformData()->Mips[0];
                    TArray<uint8> ResampledNoise = LandscapingUtils::ResampleData<uint8>(            
                                                NoisePixels,
                                                MipMap->SizeX * LayerSettings.Get()->Tiling,
                                                MipMap->SizeY * LayerSettings.Get()->Tiling,
                                                InInfo.LandscapeResolution.X,
                                                InInfo.LandscapeResolution.Y);
                    for(int i = 0; i < LayerInfo.LayerData.Num(); i++)
                    {
                        if(LayerInfo.LayerData[i] > 0)
                        {
                            LayerInfo.LayerData[i] = FMath::Max(LayerSettings.Get()->MinWeight, ResampledNoise[i]);
                            LayerInfo.LayerData[DefaultLayerIndex] = 255 - LayerInfo.LayerData[i];
                        }
                    }
                }
            }
        }
        LayerInfos.Add(LayerInfo);
        if(LayerSettings.Get()->Name.ToString().Equals(DefaultLayer))
        {
            DefaultLayerIndex = LayerInfos.Num() - 1;
        }
    }

    if(LayerInfos.Num() > 0)
    {
        for(int i = 0; i < LayerInfos[DefaultLayerIndex].LayerData.Num(); i++)
        {
            if(!UsedLayerIndizes[i])
            {
                LayerInfos[DefaultLayerIndex].LayerData[i] = 255;
            }
        }
    }
    return LayerInfos;
}

// checks if point is inside polygon and creates the weight data
TArray<uint8> WeightmapImporter::CreateWeightData(FLandscapingInfo InInfo, TArray<FVectorData> Shapes)
{
    double LandscapeScaleFactor = GisFM->GetCRS()->GetLandscapeScaleFactor();
	FVector LandscapeScale = GisFM->GetCRS()->GetLandscapeScale(InInfo);
    UE_LOG(LogTemp, Log, TEXT("Landscaping: Create WeightData for %i Shapes with Scale %s"), Shapes.Num(), *LandscapeScale.ToString());
	TArray<uint8> Data;
    Data.AddZeroed(InInfo.LandscapeResolution.X * InInfo.LandscapeResolution.Y);
	TArray<FBox> PolygonBounds;
	
	for(int ShapeIndex = 0; ShapeIndex < Shapes.Num(); ShapeIndex++)
	{
		FBox ShapeBounds = FBox(ForceInit);
		for(int PointIndex = 0; PointIndex < Shapes[ShapeIndex].Points.Num(); PointIndex++)
		{
			FVector Point = Shapes[ShapeIndex].Points[PointIndex];
			ShapeBounds += FVector(Point.X, Point.Y, PointIndex%2 ? -1 : 1);
		}
		PolygonBounds.Add(ShapeBounds);
	}
	
    FScopedSlowTask SlowTask(InInfo.LandscapeResolution.Y, FText::FromString("Creating Weightmap Texture"));
    SlowTask.MakeDialog();
	for (int HeightIndex = 0; HeightIndex < InInfo.LandscapeResolution.Y; HeightIndex++)
	{
        FString ProgressStr = FString::Printf(TEXT("Creating Weightmap Texture Line %i / %i"), (HeightIndex + 1), InInfo.LandscapeResolution.Y);
		SlowTask.EnterProgressFrame(1.0, FText::FromString(ProgressStr));
        double Y = InInfo.LocationY * LandscapeScaleFactor + (double)(HeightIndex) * LandscapeScale.Y;
        int32 DataOffset = HeightIndex * InInfo.LandscapeResolution.X;
		ParallelFor (InInfo.LandscapeResolution.X, [&](int32 WidthIndex)
		{
            // FScopeLock Lock(&Guard);
            if(!UsedLayerIndizes[DataOffset + WidthIndex])
            {
                double X = InInfo.LocationX * LandscapeScaleFactor + (double)(WidthIndex) * LandscapeScale.X;
                for(int ShapeIndex = 0; ShapeIndex < Shapes.Num(); ShapeIndex++)
                {
                    // this operation is slow, so we use ParallelFor
                    if(IsInShape(PolygonBounds[ShapeIndex], Shapes[ShapeIndex], FVector(X, Y, 0)))
                    {
                        Data[DataOffset + WidthIndex] = 255;
                        UsedLayerIndizes[DataOffset + WidthIndex] = true;
                        break;
                    }
                }
            }
		}, EParallelForFlags::Unbalanced);
	}
	return Data;
}

// actually checks if point is inside polygon
bool WeightmapImporter::IsInShape(FBox ShapeBounds, FVectorData Shape, FVector Point)
{
	if(ShapeBounds.IsInsideOrOn(Point))
	{
		return CrossingNumber(Shape.Points, Point) > 0;
	}
	return false;
}

// point in polygon inclusion algorithm
int WeightmapImporter::CrossingNumber(TArray<FVector> Points, FVector Point)
{
	int CrossingNumber = 0;
	int ShapePoints = Points.Num();
    FVector FirstPoint = Points[0];
	Points.Add(FirstPoint);
	for(int PointIndex = 0; PointIndex < ShapePoints; PointIndex++)
	{
		if((Points[PointIndex].Y <= Point.Y && Points[PointIndex + 1].Y > Point.Y) 
			|| (Points[PointIndex].Y > Point.Y && Points[PointIndex + 1].Y <= Point.Y))
		{
			float v = (float)(Point.Y - Points[PointIndex].Y) / (Points[PointIndex + 1].Y - Points[PointIndex].Y);
			if(Point.X < Points[PointIndex].X + v * (Points[PointIndex + 1].X - Points[PointIndex].X))
			{
				CrossingNumber++;
			}
		}
	}
	return CrossingNumber%2 == 1;
}

// creates and return the layerinfoobject for the landscape paint layer
ULandscapeLayerInfoObject* WeightmapImporter::GetLandscapeLayerInfoObject(TSharedPtr<MaterialLayerSettings> LayerSettings, const FString& ContentPath, int TileIndex)
{
	// Build default layer object name and package name
	FString LayerObjectName = FString::Printf(TEXT("%s_%i_LayerInfo"), *LayerSettings.Get()->Name.ToString(), TileIndex);
	FString Path = ContentPath + TEXT("_sharedassets/");
	if (Path.StartsWith("/Temp/"))
	{
		Path = FString("/Game/") + Path.RightChop(FString("/Temp/").Len());
	}
	
	FString PackageName = Path + LayerObjectName;
    UPackage* Package = FindPackage(nullptr, *PackageName);
	if (Package == nullptr)
	{
		Package = CreatePackage(*PackageName);
	}

	ULandscapeLayerInfoObject* LayerInfo = FindObject<ULandscapeLayerInfoObject>(Package, *LayerObjectName);
	if (LayerInfo == nullptr)
	{
		LayerInfo = NewObject<ULandscapeLayerInfoObject>(Package, FName(*LayerObjectName), RF_Public | RF_Standalone | RF_Transactional);
		LayerInfo->LayerName = LayerSettings.Get()->Name;
		LayerInfo->bNoWeightBlend = false; //!LayerSettings.Get()->bWeightBlended;
		// Notify the asset registry
		FAssetRegistryModule::AssetCreated(LayerInfo);
		// Mark the package dirty...
		Package->MarkPackageDirty();
		TArray<UPackage*> PackagesToSave; 
		PackagesToSave.Add(Package);
        UEditorLoadingAndSavingUtils::SavePackages(PackagesToSave, false);
    }
	
	return LayerInfo;
}

// creates and return the layerinfoobject for the landscape paint layer only by name of the paint layer
ULandscapeLayerInfoObject* WeightmapImporter::GetLandscapeLayerInfoObject(TSharedPtr<FString> PaintLayer, int TileIndex)
{
    if(!PaintLayer.IsValid())
    {
        return nullptr;
    }
    if(GetWorld() == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("Landscaping: No Level loaded"));
        return nullptr;
    }
    TSharedPtr<MaterialLayerSettings> LayerSettings = MakeShared<MaterialLayerSettings>();
    LayerSettings.Get()->Name = FName(*PaintLayer.Get());
    return GetLandscapeLayerInfoObject(LayerSettings, GetWorld()->GetOutermost()->GetName(), TileIndex);
}

bool WeightmapImporter::HasVectorFile()
{
    return TileFactory->HasVectorFiles();
}

TArray<uint8> WeightmapImporter::GetPixels(UTexture2D* NoiseTexture2D, FString ColorChannel, int Tiling)
{    
    TextureCompressionSettings OldCompressionSettings = NoiseTexture2D->CompressionSettings; TextureMipGenSettings OldMipGenSettings = NoiseTexture2D->MipGenSettings; bool OldSRGB = NoiseTexture2D->SRGB;

    NoiseTexture2D->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
    NoiseTexture2D->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
    NoiseTexture2D->SRGB = false;
    NoiseTexture2D->UpdateResource();
    FTexture2DMipMap* MipMap = &NoiseTexture2D->GetPlatformData()->Mips[0];
    const FColor* FormatedImageData = static_cast<const FColor*>( NoiseTexture2D->GetPlatformData()->Mips[0].BulkData.LockReadOnly());
    TArray<uint8> NoisePixels;
    for(int x = 0; x < Tiling; x++)
    {
        for(int32 X = 0; X < MipMap->SizeX; X++)
        {
            for(int y = 0; y < Tiling; y++)
            {
                for (int32 Y = 0; Y < MipMap->SizeY; Y++)
                {
                    if(ColorChannel.Equals("A"))
                    {
                        NoisePixels.Add(FormatedImageData[Y * MipMap->SizeX + X].A);
                    }
                    else if(ColorChannel.Equals("R"))
                    {
                        NoisePixels.Add(FormatedImageData[Y * MipMap->SizeX + X].R);
                    }
                    else if(ColorChannel.Equals("B"))
                    {
                        NoisePixels.Add(FormatedImageData[Y * MipMap->SizeX + X].B);
                    }
                    else if(ColorChannel.Equals("G"))
                    {
                        NoisePixels.Add(FormatedImageData[Y * MipMap->SizeX + X].G);
                    }
                }
            }
        }
    }

    NoiseTexture2D->GetPlatformData()->Mips[0].BulkData.Unlock();

    NoiseTexture2D->CompressionSettings = OldCompressionSettings;
    NoiseTexture2D->MipGenSettings = OldMipGenSettings;
    NoiseTexture2D->SRGB = OldSRGB;
    NoiseTexture2D->UpdateResource();
    return NoisePixels;
}

UWorld* WeightmapImporter::GetWorld()
{
	return GEditor ? GEditor->GetEditorWorldContext(false).World() : nullptr;
}