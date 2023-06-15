// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved


#include "ALandscapingInfos.h"
#include "ProceduralMeshConversion.h"
#include "Engine/StaticMeshActor.h"
#include "PhysicsEngine/BodySetup.h"


ALandscapingInfos::ALandscapingInfos(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    if(WITH_EDITOR)
    {
        PrimaryActorTick.bCanEverTick = true;
        PrimaryActorTick.bStartWithTickEnabled = true;
    }
    bRelevantForLevelBounds = true;
    SetCanBeDamaged(false);
    Bounds = CreateEditorOnlyDefaultSubobject<UBoxComponent>(TEXT("Bounds"));
    RootComponent = Bounds;
    TransformLocationCached = GetActorLocation();
    SetActorScale3D(VectorDataScale);
    VectorDataScaleCached = VectorDataScale;
    XYOffsetCached = XYOffset;
}

bool ALandscapingInfos::ShouldTickIfViewportsOnly() const
{
    return true;
}

void ALandscapingInfos::Tick(float DeltaTime)
{
    AccumulatedTimeDelta += DeltaTime;
    if(bDrawVectorDataDebug && AccumulatedTimeDelta > DebugUpdateInterval  && CheckWorld())
    {
        for(FLandscapingInfo Tile : Tiles)
        {
            for(FVectorData Geometry : Tile.SplineGeometries)
            {   
                if(Geometry.FeatureClass.Equals(ShapeFClass) || ShapeFClass.IsEmpty() || ShapeFClass.Equals("ALL"))
                {
                    if(Geometry.GeometryName == "POINT")
                    {
                        const FVector PointPostition = bSnapToGround ? SnapToFloor(Geometry.Points[0]) : Geometry.Points[0];
                        DrawDebugSphere(World, PointPostition, 50, 6, FColor::Red, false, DebugUpdateInterval);
                    }
                    for(int j = 1; j < Geometry.Points.Num(); j++)
                    {
                        const FVector LineStart = bSnapToGround ? SnapToFloor(Geometry.Points[j-1]) : Geometry.Points[j-1];
                        const FVector LineEnd = bSnapToGround ? SnapToFloor(Geometry.Points[j]) : Geometry.Points[j];
                        if(j == 1)
                        {
                            DrawDebugDirectionalArrow(World, LineStart, LineEnd, 10000, FColor::Red, false, DebugUpdateInterval);
                        }
                        else 
                        {
                            DrawDebugLine(World, LineStart, LineEnd, FColor::Red, false, DebugUpdateInterval);
                        }
                    }
                }
            }
            for(FVectorData Geometry : Tile.LandcoverShapes)
            {
                for(TSharedPtr<MaterialLayerSettings> LayerData : LayerDataList)
                {
                    for(TSharedPtr<Landuse> Landuse : LayerData.Get()->LanduseList)
                    {
                        if(Landuse.Get()->LanduseType.Equals(Geometry.FeatureClass) && Landuse.Get()->bActive)
                        {
                            for(int j = 1; j < Geometry.Points.Num(); j++)
                            {
                                const FVector LineStart = bSnapToGround ? SnapToFloor(Geometry.Points[j-1]) : Geometry.Points[j-1];
                                const FVector LineEnd = bSnapToGround ? SnapToFloor(Geometry.Points[j]) : Geometry.Points[j];
                                DrawDebugLine(World, LineStart, LineEnd, FColor::Green, false, DebugUpdateInterval);
                            }
                        }
                    }
                }
            }
        }
        AccumulatedTimeDelta = 0;
    }
    
    if(GetActorLocation() != TransformLocationCached || XYOffset != XYOffsetCached)
    {
        if(FirstTick)  // workaround because PostLoad is not called in Editor
        {
            SetActorLocation(FVector(0));
            UpdateLandscapingBounds();
            TransformLocationCached = GetActorLocation();
            SetActorScale3D(VectorDataScale);
            VectorDataScaleCached = VectorDataScale;
            GetCRS()->SetVectorDataScale(VectorDataScale);
            XYOffsetCached = XYOffset;
            GetCRS()->SetXYOffset(XYOffset);
            if(!Tiles.IsEmpty())
            {
                SetOrigin(FVector(Tiles[0].Extents.Left, Tiles[0].Extents.Top, 0));
                GetCRS()->SetCroppedExtents(CroppedExtents);
            }
            if(!RootProjection.IsEmpty())
            {
                FString AuthorityID = RootProjection;
                AuthorityID.RemoveFromStart("EPSG:");
                GetCRS()->SetAuthorityID(FCString::Atoi(*AuthorityID), RootWkt);
            }
            GetCRS()->SetLandscapeScaleFactor(LandscapeScaleFactor);
            GetCRS()->SetUsePreciseScale(bUsePreciseScale);
            FirstTick = false;
        }
        else
        {
            FVector TransformLocationDelta;
            if(GetActorLocation() != TransformLocationCached)
            {
                TransformLocationDelta = GetActorLocation() - TransformLocationCached;
                // XYOffset = XYOffset + TransformLocationDelta;
                GetCRS()->SetXYOffset(XYOffset);
            }
            else
            {
                TransformLocationDelta = XYOffset - XYOffsetCached;
            }
            UpdateLandscapingBounds();
            UpdateLocationToGeometries(TransformLocationDelta);
            TransformLocationCached = GetActorLocation();
            XYOffsetCached = XYOffset;
        }
    }
    if(VectorDataScale != VectorDataScaleCached)
    {
        SetActorScale3D(VectorDataScale);
        GetCRS()->SetVectorDataScale(VectorDataScale);
        VectorDataScaleCached = VectorDataScale;
        UpdateLandscapingBounds();
        UpdateVectorScaleToGeometries();
    }
    if(LandscapeScaleFactor != LandscapeScaleFactorCached)
    {
        GetCRS()->SetLandscapeScaleFactor(LandscapeScaleFactor);
        LandscapeScaleFactorCached = LandscapeScaleFactor;
        UpdateLandscapingBounds();
        UpdateVectorScaleToGeometries();
        VectorDataScaleCached = VectorDataScale;
    }
    if(RootProjection.IsEmpty() && Tiles.Num() > 0)
    {
        GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, TEXT("Landscaping: Landscape detected but no CRS set."));
        GEngine->AddOnScreenDebugMessage(-2, DeltaTime, FColor::Turquoise, TEXT("Landscaping: You can ...\nLandscaping: a) Open a new Level with 'File -> New Level...' (recommended)\nLandscaping: b) Import a DTM or a Vectordata to set a CRS"));
    }
}

void ALandscapingInfos::SetLayerDataList(TArray<TSharedPtr<MaterialLayerSettings>> InLayerDataList)
{
    LayerDataList = InLayerDataList;
}

UCoordinateReferenceSystem* ALandscapingInfos::GetCRS()
{
    if(CRS == nullptr)
    {
        UE_LOG(LogTemp, Log, TEXT("Landscaping: ALandscapingInfos - Init CRS"));
        CRS = NewObject<UCoordinateReferenceSystem>();
        CRS->SetVectorDataScale(VectorDataScale);
        CRS->SetXYOffset(XYOffset);
        if(!Tiles.IsEmpty())
        {
            CRS->SetOrigin(FVector(Tiles[0].Extents.Left, Tiles[0].Extents.Top, 0));
            CRS->SetCroppedExtents(CroppedExtents);
        }
        if(!RootProjection.IsEmpty())
        {
            FString AuthorityID = RootProjection;
            AuthorityID.RemoveFromStart("EPSG:");
            CRS->SetAuthorityID(FCString::Atoi(*AuthorityID), RootWkt);
        }
        CRS->SetLandscapeScaleFactor(LandscapeScaleFactor);
        CRS->SetUsePreciseScale(bUsePreciseScale);
        for(int TileIndex = 0; TileIndex < Tiles.Num(); TileIndex++)
        {
            if(Tiles[TileIndex].WGS84Extents.IsEmpty())
            {
                Tiles[TileIndex].WGS84Extents = CRS->ConvertToGeogCS(Tiles[TileIndex].Extents);
            }
        }
    }
    return CRS;
}

void ALandscapingInfos::SetOrigin(FVector InOrigin)
{
    // only set origin, if there is not already one
    if(!GetCRS()->IsOriginValid())
    {
        UE_LOG(LogTemp, Log, TEXT("Landscaping: Set Origin to %s"), *InOrigin.ToString());
        GetCRS()->SetOrigin(InOrigin);
    }
}

void ALandscapingInfos::Refresh()
{
    UpdateLandscapingBounds();
    TransformLocationCached = GetActorLocation();
    VectorDataScaleCached = VectorDataScale;
    XYOffsetCached = XYOffset;
}

void ALandscapingInfos::ResetCRS()
{
    if(Tiles.IsEmpty())
    {
        RootProjection = FString();
        RootWkt = FString();
        CroppedExtents = FExtents();
        Extents = FExtents();
        GetCRS()->ConditionalBeginDestroy();
        CRS = nullptr;
        UE_LOG(LogTemp, Warning, TEXT("Landscaping: Reset CRS successful."));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Landscaping: Reset CRS called, but there are Tiles in this Level. Ignoring call."));
    }
}

int ALandscapingInfos::SetCRS(FString AuthorityIDStr, FVector Origin)
{
    FString AuthorityID = AuthorityIDStr;
    if(AuthorityID.StartsWith("EPSG:"))
    {
        Tiles.Empty();
        ResetCRS();
        RootProjection = AuthorityIDStr;
        RootWkt = FString();
        AuthorityID.RemoveFromStart("EPSG:");
        GetCRS()->SetOrigin(Origin);
    }
    else
    {
        return -1;
    }
    return GetCRS()->GetAuthorityID();
}

void ALandscapingInfos::UpdateLocationToGeometries(FVector TransformLocationDelta)
{
    for(int t = 0; t < Tiles.Num(); t++)
    {
        for(int i = 0; i < Tiles[t].SplineGeometries.Num(); i++)
        {   
            for(int j = 0; j < Tiles[t].SplineGeometries[i].Points.Num(); j++)
            {
                Tiles[t].SplineGeometries[i].Points[j] = Tiles[t].SplineGeometries[i].Points[j] + TransformLocationDelta;
            }
        }
        for(int i = 0; i < Tiles[t].LandcoverShapes.Num(); i++)
        {
            for(int j = 0; j < Tiles[t].LandcoverShapes[i].Points.Num(); j++)
            {
                Tiles[t].LandcoverShapes[i].Points[j] = Tiles[t].LandcoverShapes[i].Points[j] + TransformLocationDelta;
            }
        }
    }
}

void ALandscapingInfos::UpdateVectorScaleToGeometries()
{
    for(int t = 0; t < Tiles.Num(); t++)
    {
        for(int i = 0; i < Tiles[t].SplineGeometries.Num(); i++)
        {   
            for(int j = 0; j < Tiles[t].SplineGeometries[i].Points.Num(); j++)
            {
                Tiles[t].SplineGeometries[i].Points[j] = GetCRS()->ConvertCRSPointToUnreal(Tiles[t].SplineGeometries[i].PointsOriginal[j]);
            }
        }
        for(int i = 0; i < Tiles[t].LandcoverShapes.Num(); i++)
        {
            for(int j = 0; j < Tiles[t].LandcoverShapes[i].Points.Num(); j++)
            {
                Tiles[t].LandcoverShapes[i].Points[j] = GetCRS()->ConvertCRSPointToUnreal(Tiles[t].LandcoverShapes[i].PointsOriginal[j]);
            }
        }
    }
}

bool ALandscapingInfos::CheckWorld()
{
    World = GEditor ? GEditor->GetEditorWorldContext(false).World() : nullptr;
    return World != nullptr;
}

void ALandscapingInfos::UpdateLandscapingBounds()
{
    CalculateTileBounds();
    if(Bounds != nullptr && !Tiles.IsEmpty())
    {
        FBox OverallBounds = FBox(ForceInit);
        for(int i = 0; i < Tiles.Num(); i++)
        {
            OverallBounds += Tiles[i].Bounds;
        }
        if(OverallBounds.IsValid)
        {
            Bounds->SetWorldLocation(OverallBounds.GetCenter());
            Bounds->SetBoxExtent(OverallBounds.GetExtent());
            Bounds->SetWorldScale3D(FVector(1));
        }
        else
        {
            FVector LeftTop = GetCRS()->ConvertCRSPointToUnreal(FVector(Tiles[0].Extents.Left, Tiles[0].Extents.Top, 0));
            FVector RightBottom = GetCRS()->ConvertCRSPointToUnreal(FVector(Tiles[0].Extents.Right, Tiles[0].Extents.Bottom, 0));
            FVector WorldLocation = FVector((LeftTop.X + RightBottom.X) / 2, (LeftTop.Y + RightBottom.Y) / 2, 0);
            Bounds->SetWorldLocation(WorldLocation);
            Bounds->SetBoxExtent((FVector(LeftTop.X, LeftTop.Y, 0) - FVector(RightBottom.X, RightBottom.Y, 0)) / 2);
            Bounds->SetWorldScale3D(VectorDataScale * 0.01);
            GEngine->AddOnScreenDebugMessage(-3, 5.f, FColor::Red, TEXT("Landscaping: Landscape Bounds not valid, falling back to vector bounds of first tile."));
        }

    }
}

FVector ALandscapingInfos::SnapToFloor(FVector Location)
{
    CheckWorld();
    return LandscapingUtils::SnapToGround(Location, OffsetFromGround, World);
}

// Left: X, Top: Y
void ALandscapingInfos::SetExtentsAndBoundsFromLeftTopCorner(double Left, double Top)
{
    Extents.Left = Left;
    Extents.Top = Top;
    TArray<double> XTiles = TArray<double>();
    TArray<double> YTiles = TArray<double>();
    for(int i = 0; i < Tiles.Num(); i++)
    {
        XTiles.AddUnique(Tiles[i].LocationX);
        YTiles.AddUnique(Tiles[i].LocationY);
        FVector TileScale = GetCRS()->GetLandscapeScale(Tiles[i]);
        FVector BoundsCenter = FVector(Tiles[i].LocationX + Tiles[0].LandscapeResolution.X * 0.5 * TileScale.X, Tiles[i].LocationY + Tiles[0].LandscapeResolution.Y * 0.5 * TileScale.Y, 0);
        FVector BoundsExtents = FVector(Tiles[0].LandscapeResolution.X * 0.5 * TileScale.X, Tiles[0].LandscapeResolution.Y * 0.5 * TileScale.Y, 1);
        Tiles[i].Bounds = Tiles[i].Bounds.BuildAABB(BoundsCenter, BoundsExtents);
    }
    Extents.Right = Extents.Left + XTiles.Num() * Tiles[0].LandscapeResolution.X;
    Extents.Bottom = Extents.Top - YTiles.Num() * Tiles[0].LandscapeResolution.Y;
    UpdateLandscapingBounds();
}

void ALandscapingInfos::SetBoundsFromTiles()
{
    for(int i = 0; i < Tiles.Num(); i++)
    {
        FVector TileScale = GetCRS()->GetLandscapeScale(Tiles[i]);
        FVector BoundsCenter = FVector(Tiles[i].LocationX + Tiles[0].LandscapeResolution.X * 0.5 * TileScale.X, Tiles[i].LocationY + Tiles[0].LandscapeResolution.Y * 0.5 * TileScale.Y, 0);
        FVector BoundsExtents = FVector(Tiles[0].LandscapeResolution.X * 0.5 * TileScale.X, Tiles[0].LandscapeResolution.Y * 0.5 * TileScale.Y, 1);
        Tiles[i].Bounds = Tiles[i].Bounds.BuildAABB(BoundsCenter, BoundsExtents);
    }
}

void ALandscapingInfos::SetOffsetXYFromLeftTopMostTile()
{
    if(Tiles.Num() == 0)
    {
        return;
    }
    int LeftMostIndex = 0;
    int TopMostIndex = 0;
    for(int i = 0; i < Tiles.Num(); i++)
    {
        if(Tiles[i].LocationX < Tiles[LeftMostIndex].LocationX)
        {
            LeftMostIndex = i;
        }
        if(Tiles[i].LocationY < Tiles[TopMostIndex].LocationY)
        {
            TopMostIndex = i;
        }
    }

    XYOffset = FVector(Tiles[LeftMostIndex].LocationX, Tiles[TopMostIndex].LocationY, 0);
}

void ALandscapingInfos::SetCroppedExtents(FExtents InExtents)
{
    CroppedExtents = InExtents;
    GetCRS()->SetCroppedExtents(InExtents);
}

bool ALandscapingInfos::SetCroppedExtents(FString InBBox)
{
    bool bIsValid = GetCRS()->SetCroppedExtents(InBBox);
    CroppedExtents = GetCRS()->GetCroppedExtents();
    return bIsValid;
}

FExtents ALandscapingInfos::GetCroppedExtents()
{
    return GetCRS()->GetCroppedExtents();
}

double ALandscapingInfos::GetArea() const
{
    double OverallArea = 0;
    for(auto Tile : Tiles)
    {
        OverallArea += Tile.Extents.GetArea();
    }
    return OverallArea;
}

FVector ALandscapingInfos::GetAreaDimension()
{
    FVector AreaDim = FVector(0);
    FExtents NewExtents = GetCRS()->ConvertFromGeogCS(GetCroppedExtents());
    AreaDim.X = FMath::Abs(NewExtents.Left - NewExtents.Right);
    AreaDim.Y = FMath::Abs(NewExtents.Top - NewExtents.Bottom);
    return AreaDim;
}

TArray<int> ALandscapingInfos::GetSelectedTiles() const
{
    TArray<int> SelectedIndizes;
    TArray<ALandscapeProxy*> LandscapeProxies = LandscapingUtils::GetSelectedActors<ALandscapeProxy>();
    for(ALandscapeProxy* Proxy : LandscapeProxies)
    {
        for(int i = 0; i < Tiles.Num(); i++)
        {
            if(Tiles[i].Landscape == Proxy)
            {
                SelectedIndizes.Add(i);
            }
        }
    }
    TArray<ALandscapingProcMeshLandscape*> LandscapeProcMeshes = LandscapingUtils::GetSelectedActors<ALandscapingProcMeshLandscape>();
    for(ALandscapingProcMeshLandscape* ProcMesh : LandscapeProcMeshes)
    {
        for(int i = 0; i < Tiles.Num(); i++)
        {
            if(Tiles[i].LandscapeSM == ProcMesh)
            {
                SelectedIndizes.Add(i);
            }
        }
    }
    TArray<AStaticMeshActor*> LandscapeMeshes = LandscapingUtils::GetSelectedActors<AStaticMeshActor>();
    for(AStaticMeshActor* StaticMesh : LandscapeMeshes)
    {
        for(int i = 0; i < Tiles.Num(); i++)
        {
            if(Tiles[i].LandscapeNaniteMesh == StaticMesh)
            {
                SelectedIndizes.Add(i);
            }
        }
    }
    if(SelectedIndizes.IsEmpty())
    {
        for(int i = 0; i < Tiles.Num(); i++)
        {
            SelectedIndizes.Add(i);
        }
    }
    return SelectedIndizes;
}

void ALandscapingInfos::ChangeLandscapeScaleFactor()
{
#if WITH_EDITOR
    GetCRS()->SetLandscapeScaleFactor(LandscapeScaleFactor);
    double ScaleRatio = 1.0; // used to find out what we should scale actors not referenced by Landscaping
    if(Tiles.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Landscaping: No Landscape to scale."));
        return;
    }
    for(int TileIndex = 0; TileIndex < Tiles.Num(); TileIndex++)
    {
        if(Tiles[TileIndex].Landscape != nullptr)
        {
            double CurrentScaleRatio = Tiles[0].Landscape->GetActorScale3D().X / Tiles[0].LandscapeScale.X;
            ScaleRatio = GetCRS()->GetLandscapeScaleFactor() / CurrentScaleRatio;
            break;
        }
        if(Tiles[TileIndex].LandscapeSM != nullptr)
        {
            double CurrentScaleRatio = Tiles[0].LandscapeSM->GetActorScale3D().X / Tiles[0].LandscapeScale.X;
            ScaleRatio = GetCRS()->GetLandscapeScaleFactor() / CurrentScaleRatio;
            break;
        }
        if(Tiles[TileIndex].LandscapeNaniteMesh != nullptr)
        {
            double CurrentScaleRatio = Tiles[0].LandscapeNaniteMesh->GetActorScale3D().X / Tiles[0].LandscapeScale.X;
            ScaleRatio = GetCRS()->GetLandscapeScaleFactor() / CurrentScaleRatio;
            break;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Landscaping: Change Scale and Location of Level - ScaleFactor: %f - ScaleRatio: %f"), GetCRS()->GetLandscapeScaleFactor(), ScaleRatio);
    
    TArray<AActor*> FoundActors;
    CheckWorld();
    if(World != nullptr)
    {
        LandscapingUtils::FindAllActors<AActor>(World, FoundActors);
        for(AActor* FoundActor : FoundActors)
        {   
            FVector NewActorScale = FoundActor->GetActorScale3D() * ScaleRatio;
            FVector NewActorLocation = FoundActor->GetActorLocation() * ScaleRatio;
            FoundActor->SetActorScale3D(NewActorScale);
            FoundActor->SetActorLocation(NewActorLocation);
            FoundActor->Modify(true);
        }
        CalculateTileBounds();
        Modify(true);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Landscaping: Cannot find Level World. Please try again."));
    }
#endif
}

void ALandscapingInfos::CalculateTileBounds()
{
    TArray<ALandscapeStreamingProxy*> Proxies;
    CheckWorld();
    if(World != nullptr)
    {
        LandscapingUtils::FindAllActors<ALandscapeStreamingProxy>(World, Proxies);
        for(int TileIndex = 0; TileIndex < Tiles.Num(); TileIndex++)
        {
            if(Tiles[TileIndex].Bounds.IsValid || !Tiles[TileIndex].Bounds.Min.Equals(Tiles[TileIndex].Bounds.Max))
            {
                continue;
            }
            if(Tiles[TileIndex].Landscape != nullptr)
            {
                Tiles[TileIndex].Bounds = FBox(ForceInit);
                for(auto Proxy : Proxies)
                {
                    if(Proxy->GetLandscapeActor() == Tiles[TileIndex].Landscape)
                    {
                        Tiles[TileIndex].Bounds += Proxy->GetComponentsBoundingBox(false, true);
                    }
                }
                if(!Tiles[TileIndex].Bounds.IsValid)
                {
                    Tiles[TileIndex].Bounds += Tiles[TileIndex].Landscape->GetComponentsBoundingBox(false, true);
                }
            }
            else if(Tiles[TileIndex].LandscapeSM != nullptr)
            {
                Tiles[TileIndex].Bounds = Tiles[TileIndex].LandscapeSM->GetComponentsBoundingBox(true, true);
            }
            else if(Tiles[TileIndex].LandscapeNaniteMesh != nullptr)
            {
                Tiles[TileIndex].Bounds = Tiles[TileIndex].LandscapeNaniteMesh->GetComponentsBoundingBox(true, true);
            }
        }
    }
}

void ALandscapingInfos::SaveStaticMeshAndReplace()
{
#if WITH_EDITOR
	TArray<int> SelectedTiles = GetSelectedTiles();
    for(int TileIndex : SelectedTiles)
    {
        if(Tiles[TileIndex].LandscapeSM == nullptr)
        {
            continue;
        }
    
        FString NewNameSuggestion = SaveStaticMesh(Tiles[TileIndex].LandscapeSM);
        FString PackageName = FString(TEXT("/Game/Landscaping/Meshes/")) + NewNameSuggestion;
        if(PackageName.IsEmpty())
        {
            return;
        }
        UStaticMesh* Mesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *PackageName));
        CheckWorld();
        if(World != nullptr)
        {
            AStaticMeshActor *MeshActor = World->SpawnActor<AStaticMeshActor>(Tiles[TileIndex].LandscapeSM->GetActorLocation(), FRotator(0));
            MeshActor->SetActorScale3D(FVector(1.0) * GetCRS()->GetLandscapeScaleFactor());
            UStaticMeshComponent* StaticMeshComponent = MeshActor->GetStaticMeshComponent();
            StaticMeshComponent->SetStaticMesh(Mesh);
            MeshActor->SetActorLabel(NewNameSuggestion);
            Tiles[TileIndex].LandscapeSM->Destroy();
            Tiles[TileIndex].LandscapeNaniteMesh = MeshActor;
            Tiles[TileIndex].LandscapeSM = nullptr;
        }
    }
#endif
}

FString ALandscapingInfos::SaveStaticMesh(ALandscapingProcMeshLandscape* LandscapeSM)
{
#if WITH_EDITOR 
	
	if (LandscapeSM->ProceduralMesh != nullptr)
	{
		// /** 
		//  *	Slice the ProceduralMeshComponent (including simple convex collision) using a plane. Optionally create 'cap' geometry. 
		// *	@param	InProcMesh				ProceduralMeshComponent to slice
		// *	@param	PlanePosition			Point on the plane to use for slicing, in world space
		// *	@param	PlaneNormal				Normal of plane used for slicing. Geometry on the positive side of the plane will be kept.
		// *	@param	bCreateOtherHalf		If true, an additional ProceduralMeshComponent (OutOtherHalfProcMesh) will be created using the other half of the sliced geometry
		// *	@param	OutOtherHalfProcMesh	If bCreateOtherHalf is set, this is the new component created. Its owner will be the same as the supplied InProcMesh.
		// *	@param	CapOption				If and how to create 'cap' geometry on the slicing plane
		// *	@param	CapMaterial				If creating a new section for the cap, assign this material to that section
		// */
		// UFUNCTION(BlueprintCallable, Category = "Components|ProceduralMesh")
		// static void SliceProceduralMesh(UProceduralMeshComponent* InProcMesh, FVector PlanePosition, FVector PlaneNormal, bool bCreateOtherHalf, UProceduralMeshComponent*& OutOtherHalfProcMesh, EProcMeshSliceCapOption CapOption, UMaterialInterface* CapMaterial);

		FString NewNameSuggestion = FString::Printf(TEXT("%s_%s"), *GetActorLabel(), *LandscapeSM->ProcMeshId);
		FString PackageName = FString(TEXT("/Game/Landscaping/Meshes/")) + NewNameSuggestion;
		
		FName MeshName(*FPackageName::GetLongPackageAssetName(PackageName));
		FMeshDescription MeshDescription = BuildMeshDescription(LandscapeSM->ProceduralMesh);

		// If we got some valid data.
		if (MeshDescription.Polygons().Num() > 0)
		{
			// Then find/create it.
			UPackage* Package = CreatePackage(*PackageName);
			check(Package);

			// Create StaticMesh object
			UStaticMesh* StaticMesh = NewObject<UStaticMesh>(Package, MeshName, RF_Public | RF_Standalone);
			StaticMesh->InitResources();

			StaticMesh->SetLightingGuid();

			// Add source to new StaticMesh
			FStaticMeshSourceModel& SrcModel = StaticMesh->AddSourceModel();
			SrcModel.BuildSettings.bRecomputeNormals = false;
			SrcModel.BuildSettings.bRecomputeTangents = false;
			SrcModel.BuildSettings.bRemoveDegenerates = false;
			SrcModel.BuildSettings.bUseHighPrecisionTangentBasis = false;
			SrcModel.BuildSettings.bUseFullPrecisionUVs = false;
			SrcModel.BuildSettings.bGenerateLightmapUVs = true;
			SrcModel.BuildSettings.SrcLightmapIndex = 0;
			SrcModel.BuildSettings.DstLightmapIndex = 1;
			StaticMesh->CreateMeshDescription(0, MoveTemp(MeshDescription));
			StaticMesh->CommitMeshDescription(0);

			//// SIMPLE COLLISION
			if (!LandscapeSM->ProceduralMesh->bUseComplexAsSimpleCollision)
			{
				StaticMesh->CreateBodySetup();
				UBodySetup* NewBodySetup = StaticMesh->GetBodySetup();
				NewBodySetup->BodySetupGuid = FGuid::NewGuid();
				NewBodySetup->AggGeom.ConvexElems = LandscapeSM->ProceduralMesh->ProcMeshBodySetup->AggGeom.ConvexElems;
				NewBodySetup->bGenerateMirroredCollision = false;
				NewBodySetup->bDoubleSidedGeometry = true;
				NewBodySetup->CollisionTraceFlag = CTF_UseDefault;
				NewBodySetup->CreatePhysicsMeshes();
			}

			//// MATERIALS
			TSet<UMaterialInterface*> UniqueMaterials;
			const int32 NumSections = LandscapeSM->ProceduralMesh->GetNumSections();
			for (int32 SectionIdx = 0; SectionIdx < NumSections; SectionIdx++)
			{
				FProcMeshSection *ProcSection = LandscapeSM->ProceduralMesh->GetProcMeshSection(SectionIdx);
				UMaterialInterface *ProcMaterial = LandscapeSM->ProceduralMesh->GetMaterial(SectionIdx);
				UniqueMaterials.Add(ProcMaterial);
			}
			// Copy materials to new mesh
			for (auto* ProcMaterial : UniqueMaterials)
			{
				StaticMesh->GetStaticMaterials().Add(FStaticMaterial(ProcMaterial));
			}

			//Set the Imported version before calling the build
			StaticMesh->ImportVersion = EImportStaticMeshVersion::LastVersion;
			FMeshNaniteSettings NaniteSettings;
			NaniteSettings.bEnabled = true;
			NaniteSettings.FallbackRelativeError = 0;
			StaticMesh->NaniteSettings = NaniteSettings;

			// Build mesh from source
			StaticMesh->Build(false);
			StaticMesh->PostEditChange();

			// Notify asset registry of new asset
			FAssetRegistryModule::AssetCreated(StaticMesh);
			return NewNameSuggestion;
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Landscaping: No valid data for static mesh %s"), *GetActorLabel());
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Landscaping: No procedural mesh for static mesh %s"), *GetActorLabel());
	}
#endif
	return FString();
}
