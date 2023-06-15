// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#include "VectorImporter.h"
#include "LandscapeSplineActor.h"
#include "LandscapeSplinesComponent.h"
#include "LandscapeSplineControlPoint.h"


VectorImporter::VectorImporter(UGISFileManager* InGisFileManager)
{
    GisFM = InGisFileManager;
    TileFactory = new VectorTileFactory(GisFM);
}

VectorImporter::~VectorImporter()
{
    GisFM = nullptr;
    delete TileFactory;
    TileFactory = nullptr;
}

TArray<FString> VectorImporter::LoadFiles(TArray<FString> InFilenames, int TileIndex)
{   
    delete TileFactory;
    TileFactory = new VectorTileFactory(GisFM);
    UE_LOG(LogTemp, Log, TEXT("Landscaping: Load vector data of %i files for Tile - %i"), InFilenames.Num(), TileIndex);
    for(int Index = 0; Index < InFilenames.Num(); Index++)
    {
        FString Error = TileFactory->AddFile(InFilenames[Index], TileIndex, false);
    }
    SetupObjects(TileIndex, GisFM->GetInfos()->bImportedThroughLandscaping, !GisFM->GetInfos()->bImportedThroughLandscaping);
    TArray<FString> TileFeatureClasses = TileFactory->GetAvailableFeatureClasses(TArray<FString>());
    for(FString FeatureClass : TileFeatureClasses)
    {
        FeatureClasses.AddUnique(FeatureClass);
    }
    return FeatureClasses;
}

bool VectorImporter::CreateBlueprints(VectorGeometrySpawnOptions Options, int TileIndex)
{  
    if(GetWorld() == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("Landscaping: No Level loaded"));
        return false;
    }
    return InstantiateBlueprintFromVectorGeometries(Options, TileIndex);
}

void VectorImporter::SetupObjects(int TileIndex, bool bCropToBounds, bool bAllObjects)
{
    if(TilesSetupFinished.Contains(TileIndex))
    {
        return;
    }
    if(bAllObjects)
    {
        GisFM->GetInfos()->Tiles[TileIndex].SplineGeometries = TileFactory->GetObjects();
    }
    else 
    {
        GisFM->GetInfos()->Tiles[TileIndex].SplineGeometries = TileFactory->GetShapes(TileIndex, bCropToBounds);
    }
    TilesSetupFinished.Add(TileIndex);
}

bool VectorImporter::InstantiateBlueprintFromVectorGeometries(VectorGeometrySpawnOptions Options, int TileIndex)
{
    FLandscapingInfo InInfo = GisFM->GetInfos()->Tiles[TileIndex];
    int i = FMath::Min(Options.StartEntityIndex, (InInfo.SplineGeometries.Num() - 1));
    int Num = FMath::Min((i + Options.MaxEntities), InInfo.SplineGeometries.Num());
    Num = Num == 0 ? InInfo.SplineGeometries.Num() : Num;
    for(; i < Num; i++)
    {
        if(InInfo.SplineGeometries[i].FeatureClass.Equals(*Options.ShapeFClass.Get()) || 
            InInfo.SplineGeometries[i].FeatureClass.IsEmpty() || FString("ALL").Equals(*Options.ShapeFClass.Get()))
        {
            if(InInfo.SplineGeometries[i].GeometryName.Equals("LINESTRING") || InInfo.SplineGeometries[i].GeometryName.Equals("POLYGON"))
            {
                if(InInfo.SplineGeometries[i].Points.Num() == 0)
                {
                    continue;
                }

                // location and start index
                int StartIndex = 0;
                int LookAtIndex = InInfo.SplineGeometries[i].Points.Num() > 1 ? 1 : 0;
                if(Options.bRevertSplineDirection)
                {
                    StartIndex = InInfo.SplineGeometries[i].Points.Num() > 1 ? InInfo.SplineGeometries[i].Points.Num() - 1 : 0;
                    LookAtIndex = InInfo.SplineGeometries[i].Points.Num() > 1 ? StartIndex - 1 : 0;
                }
                FVector Location = GisFM->GetInfos()->SnapToFloor(InInfo.SplineGeometries[i].Points[StartIndex]);
                FVector LookAt = GisFM->GetInfos()->SnapToFloor(InInfo.SplineGeometries[i].Points[LookAtIndex]);
                FRotator Rotation = FRotator::ZeroRotator;
                float zValue = Location.Z;
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
                // do we deal with Landscape Splines on a Landscape?
                ALandscapeSplineActor* LandscapeSplineActor = nullptr;
                ULandscapeSplinesComponent* LandscapeSplinesComponent = nullptr;
                if(Options.bLandscapeSplines && InInfo.Landscape != nullptr)
                {
                    ALandscape* Landscape = (ALandscape*)InInfo.Landscape;
                    // toggle edit layers
                    if(!Landscape->CanHaveLayersContent())
                    {
                        Landscape->ToggleCanHaveLayersContent();
                        if (GEditor)
                        {
                            GEditor->ResetTransaction(FText::FromString(FString("Toggling Landscape Edit Layers")));
                        }
                    }
                    
                    // create landscape spline actor
                    LandscapeSplineActor = Landscape->GetLandscapeInfo()->CreateSplineActor(Landscape->GetActorLocation());
                    FString ActorLabel = FString::Printf(TEXT("%s (%i-Shape #%i)"), (InInfo.SplineGeometries[i].Name.IsEmpty() ? *InInfo.SplineGeometries[i].FeatureClass: *InInfo.SplineGeometries[i].Name), TileIndex, i);
                    LandscapeSplineActor->SetActorLabel(ActorLabel);
                    LandscapeSplinesComponent = LandscapeSplineActor->GetSplinesComponent();

                    LandscapeSplinesComponent->Modify();
                    
                    ULandscapeSplineControlPoint* ControlPoint = AddControlPoint(InInfo.SplineGeometries[i].Points[StartIndex], Landscape, LandscapeSplinesComponent, Options, zValue);
                
                    if(Options.bRevertSplineDirection)
                    {
                        for(int j = InInfo.SplineGeometries[i].Points.Num() - 1; j > 0; j--)
                        {
                            ULandscapeSplineControlPoint* NewControlPoint = AddControlPoint(InInfo.SplineGeometries[i].Points[j], Landscape, LandscapeSplinesComponent, Options, zValue);
                            AddSegment(ControlPoint, NewControlPoint);
                            ControlPoint = NewControlPoint;
                        }
                    }
                    else
                    {
                        for(int j = 1; j < InInfo.SplineGeometries[i].Points.Num(); j++)
                        {
                            ULandscapeSplineControlPoint* NewControlPoint = AddControlPoint(InInfo.SplineGeometries[i].Points[j], Landscape, LandscapeSplinesComponent, Options, zValue);
                            AddSegment(ControlPoint, NewControlPoint);
                            ControlPoint = NewControlPoint;
                        }
                    }
                    // AutoUpdateDirtyLandscapeSplines
                    if (Landscape->HasLayersContent() && GEditor->IsTransactionActive())
                    {
                        // Only auto-update if a layer is reserved for landscape splines
                        if (Landscape->GetLandscapeSplinesReservedLayer())
                        {
                            // TODO : Only update dirty regions
                            Landscape->RequestSplineLayerUpdate();
                        }
                    }
                    if (!LandscapeSplinesComponent->IsRegistered())
                    {
                        LandscapeSplinesComponent->RegisterComponent();
                    }
                    else
                    {
                        LandscapeSplinesComponent->MarkRenderStateDirty();
                    }

                    TArray<TObjectPtr<ULandscapeSplineSegment>> Segments = LandscapeSplinesComponent->GetSegments();
                    for(int SegIndex = 0; SegIndex < Segments.Num(); SegIndex++)
                    {
                        Segments[SegIndex].Get()->bRaiseTerrain = Options.LandscapeSplineOptions.RaiseHeights;
                        Segments[SegIndex].Get()->bLowerTerrain = Options.LandscapeSplineOptions.LowerHeights;
                        // Set mesh
                        if(Options.SplineMesh != nullptr)
                        {
                            FLandscapeSplineMeshEntry MeshEntry;
                            MeshEntry.Mesh = Cast<UStaticMesh>(Options.SplineMesh);
                            MeshEntry.Scale = Options.Scale;
                            Segments[SegIndex].Get()->SplineMeshes.Add(MeshEntry);
                        }
                        if(Options.PaintLayer.IsValid())
                        {
                            FName LayerName = FName(*Options.PaintLayer.Get());
                            UE_LOG(LogTemp, Log, TEXT("Landscaping: Segment Layername %s"), *LayerName.ToString());
                            Segments[SegIndex].Get()->LayerName = LayerName;
                        }
                    }
                    for (ULandscapeSplineControlPoint* InControlPoint : LandscapeSplinesComponent->GetControlPoints())
                    {
                        InControlPoint->Modify(true);
                    }
                    for (ULandscapeSplineSegment* InSegment : LandscapeSplinesComponent->GetSegments())
                    {
                        InSegment->Modify(true);
                    }
                    
                    FPropertyChangedEvent PropertyChangedEventControlPoints(FindFieldChecked<FProperty>(LandscapeSplinesComponent->GetClass(), FName("ControlPoints")));
                    LandscapeSplinesComponent->PostEditChangeProperty(PropertyChangedEventControlPoints);
                    FPropertyChangedEvent PropertyChangedEventSegments(FindFieldChecked<FProperty>(LandscapeSplinesComponent->GetClass(), FName("Segments")));
                    LandscapeSplinesComponent->PostEditChangeProperty(PropertyChangedEventSegments);
                    LandscapeSplinesComponent->MarkPackageDirty();
                    LandscapeSplinesComponent->PostEditChange();
                    LandscapeSplinesComponent->RebuildAllSplines();
                    Landscape->GetLandscapeInfo()->RequestSplineLayerUpdate();
                    continue;
                }
#endif
                AActor* NewObj = nullptr;
                ALandscapingLandscapeSplines* AuxActor = nullptr;
                ALandscapingSplineActor * NewSplineActor = nullptr;
                USplineComponent* SplineComponent = nullptr;

                // begin blueprint spline
                if(Options.ActorOrBlueprintClass != nullptr)
                {   
                    NewObj = GetWorld()->SpawnActor<AActor>(Options.ActorOrBlueprintClass, Location, Rotation);
                    NewObj->SetActorScale3D(Options.Scale);
                    FString ActorLabel = FString::Printf(TEXT("%s (%i-Shape #%i)"), (InInfo.SplineGeometries[i].Name.IsEmpty() ? *InInfo.SplineGeometries[i].FeatureClass: *InInfo.SplineGeometries[i].Name), TileIndex, i);
                    NewObj->SetActorLabel(ActorLabel);
                    if(NewObj->GetClass()->ImplementsInterface(ULandscapingVectorInterface::StaticClass()))
                    {
                        FEditorScriptExecutionGuard ScriptGuard;
                        ILandscapingVectorInterface::Execute_OnVectorData(NewObj, InInfo.SplineGeometries[i]);
                        NewObj->Modify(true);
                        NewObj->MarkComponentsRenderStateDirty();
                        NewObj->MarkPackageDirty();
                        continue;
                    }
                    if(Options.bSpawnAux)
                    {
                        UClass* ClassToSpawn = StaticLoadClass(AActor::StaticClass(), nullptr, *FString("/Landscaping/Blueprints/BP_LandscapeSplines.BP_LandscapeSplines_C"));
                        AuxActor = GetWorld()->SpawnActor<ALandscapingLandscapeSplines>(ClassToSpawn, Location, Rotation);
                        AuxActor->bIsEditorOnlyActor = true;
                        ActorLabel.Append(" Aux (Editor Only)");
                        AuxActor->SetActorLabel(ActorLabel);
                    }
                    FProperty* Prop = FindFieldChecked<FProperty>(NewObj->GetClass(), FName("SplineComponent"));
                    if(Prop == nullptr)
                    {
                        Prop = FindFieldChecked<FProperty>(NewObj->GetClass(), FName("Spline"));
                    }
                    if(Prop == nullptr)
                    {
                        UE_LOG(LogTemp, Error, TEXT("Landscaping: Could not find `Spline` or `SplineComponent` on the Blueprint. Please make sure to name the spline component `Spline` or `SplineComponent` to perform the post updates properly."))
                    }
                    else
                    {
                        FPropertyChangedEvent NewObjPropertyChangedEvent(Prop);
                        NewObj->PostEditChangeProperty(NewObjPropertyChangedEvent);
                    }
                }
                // end blueprint spline
                // begin spline mesh / paint layer
                else if(Options.SplineMesh != nullptr || Options.LandscapeSplineOptions.bPaintMaterialLayer)
                {
                    // ALandscapingSplineActor is here for legacy reasons to set the mesh and other properties on the BP_Spline (see BP_LandscapingSplineActor)
                    UClass* ClassToSpawnSplineActor = StaticLoadClass(AActor::StaticClass(), nullptr, *FString("/Landscaping/Blueprints/BP_LandscapingSplineAuxActor.BP_LandscapingSplineAuxActor_C"));
                    NewSplineActor = GetWorld()->SpawnActor<ALandscapingSplineActor>(ClassToSpawnSplineActor, Location, Rotation);
                    FString ActorLabel = FString::Printf(TEXT("%s (%i-Shape #%i)"), (InInfo.SplineGeometries[i].Name.IsEmpty() ? *InInfo.SplineGeometries[i].FeatureClass: *InInfo.SplineGeometries[i].Name), TileIndex, i);
                    if(Options.SplineMesh == nullptr)
                    {
                        Options.SplineMesh = StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *FString("/Engine/BasicShapes/Cube.Cube"));
                    }
                    if(Options.SplineMesh == nullptr)
                    {
                        UE_LOG(LogTemp, Error, TEXT("Landscaping: Could not load SplineMesh /Engine/BasicShapes/Cube.Cube"));
                        break;
                    }
                    if(Options.SplineMesh != nullptr)
                    {
                        UClass* ClassToSpawn = StaticLoadClass(AActor::StaticClass(), nullptr, *FString("/Landscaping/Blueprints/BP_Spline.BP_Spline_C"));
                        NewObj = GetWorld()->SpawnActor<AActor>(ClassToSpawn, Location, Rotation);
                        NewObj->SetActorLabel(ActorLabel);
                    }
                    NewSplineActor->StartScale = Options.StartWidth;
                    NewSplineActor->EndScale = Options.EndWidth;
                    FSplineMeshDetails DefaultDetails = FSplineMeshDetails();
                    DefaultDetails.Mesh = Cast<UStaticMesh>(Options.SplineMesh);
                    NewSplineActor->SplineMeshDetails = DefaultDetails;
                    NewSplineActor->bIsEditorOnlyActor = true;
                    ActorLabel.Append(" (Editor Only)");
                    NewSplineActor->SetActorLabel(ActorLabel);
                }
                // end spline mesh / paint layer

                SplineComponent = Cast<USplineComponent>(NewObj->GetComponentByClass(USplineComponent::StaticClass()));
                if(SplineComponent == nullptr)
                {
                    UE_LOG(LogTemp, Error, TEXT("Landscaping: No spline compontent found"));
                    FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("Operation interrupted.\nNo spline compontent found on selected Actor/Blueprint."));
                    return false;
                }
                
                SplineComponent->ClearSplinePoints();
                if(Options.bRevertSplineDirection)
                {
                    for(int j = InInfo.SplineGeometries[i].Points.Num() - 1; j >= 0; j--)
                    {
                        FVector SplinePointLocation = GisFM->GetInfos()->SnapToFloor(InInfo.SplineGeometries[i].Points[j]);
                        if(Options.bIsBuilding)
                        {
                            SplinePointLocation.Z = zValue;
                        }
                        SplineComponent->AddSplinePoint(SplinePointLocation, ESplineCoordinateSpace::World);
                    }
                }
                else
                {
                    for(int j = 0; j < InInfo.SplineGeometries[i].Points.Num(); j++)
                    {
                        FVector SplinePointLocation = GisFM->GetInfos()->SnapToFloor(InInfo.SplineGeometries[i].Points[j]);
                        if(Options.bIsBuilding)
                        {
                            SplinePointLocation.Z = zValue;
                        }
                        SplineComponent->AddSplinePoint(SplinePointLocation, ESplineCoordinateSpace::World);
                    }
                }
                const int32 SplinePoints = SplineComponent->GetNumberOfSplinePoints();
                for(int SplineCount = 0; SplineCount < SplinePoints; SplineCount++)
                {
                    const FVector StartTangent = SplineComponent->GetTangentAtSplinePoint(SplineCount > 0 ? SplineCount - 1 : SplineCount, ESplineCoordinateSpace::Type::Local);
                    const FVector EndTangent = SplineComponent->GetTangentAtSplinePoint(SplineCount < SplinePoints - 1 ? SplineCount + 1 : SplineCount, ESplineCoordinateSpace::Type::Local);
                    SplineComponent->SetTangentsAtSplinePoint(SplineCount, StartTangent, EndTangent, ESplineCoordinateSpace::Type::Local);
                    SplineComponent->SetSplinePointType(SplineCount, Options.SplinePointType, true);
                }
                if(Options.ActorOrBlueprintClass != nullptr && AuxActor != nullptr)
                {
                    FEditorScriptExecutionGuard ScriptGuard;
                    AuxActor->OnSplineCreated(NewObj);
                    AuxActor->Modify(true);
                }
                else if(NewSplineActor != nullptr)
                {
                    FEditorScriptExecutionGuard ScriptGuard;
                    NewSplineActor->OnSplineCreated(SplineComponent);
                    NewSplineActor->Modify(true);
                }
                SplineComponent->Modify(true);
                SplineComponent->UpdateSpline();
	            SplineComponent->bSplineHasBeenEdited = true;

                FProperty* PropCurves = FindFieldChecked<FProperty>(SplineComponent->GetClass(), FName("SplineCurves"));
                if(PropCurves != nullptr)
                {
                    FPropertyChangedEvent PropertyChangedEvent(PropCurves);
                    SplineComponent->PostEditChangeProperty(PropertyChangedEvent);
                    SplineComponent->MarkPackageDirty();
                    SplineComponent->PostEditChange();
                }
               
                Options.LandscapeSplineOptions.Landscape = InInfo.Landscape;
                if(AuxActor != nullptr && AuxActor->GetClass()->ImplementsInterface(ULandscapingPaintLayerInterface::StaticClass()))
                {
                    FEditorScriptExecutionGuard ScriptGuard;
                    AuxActor->Execute_OnPaintLayer(AuxActor, Options.LandscapeSplineOptions);
                    AuxActor->Modify(true);
                }
                if(NewSplineActor != nullptr && NewSplineActor->GetClass()->ImplementsInterface(ULandscapingPaintLayerInterface::StaticClass()))
                {
                    FEditorScriptExecutionGuard ScriptGuard;
                    NewSplineActor->Execute_OnPaintLayer(NewSplineActor, Options.LandscapeSplineOptions);
                    NewSplineActor->Modify(true);
                }
                if(SplineComponent && SplineComponent->GetName().Equals("SplineComponent"))
                {
                    FProperty* Prop = FindFieldChecked<FProperty>(NewObj->GetClass(), FName("SplineComponent"));
                    if(Prop != nullptr)
                    {
                        FPropertyChangedEvent NewObjPropertyChangedEvent(Prop);
                        NewObj->PostEditChangeProperty(NewObjPropertyChangedEvent);
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("Landscaping: Could not find `SplineComponent` on the Blueprint. Please make sure to name the spline component `SplineComponent` to perform the post updates properly."));
                }
                NewObj->Modify(true);
                
            }
            else if(InInfo.SplineGeometries[i].GeometryName.Equals("POINT"))
            {
                for(int j = 0; j < InInfo.SplineGeometries[i].Points.Num(); j++)
                {
                    FVector Location = GisFM->GetInfos()->SnapToFloor(InInfo.SplineGeometries[i].Points[j]);
                    if(Options.ActorOrBlueprintClass != nullptr)
                    {   
                        AActor* NewObj = GetWorld()->SpawnActor<AActor>(Options.ActorOrBlueprintClass, Location, FRotator(0));
                        FString ActorLabel = FString::Printf(TEXT("%s (Point #%i)"), (InInfo.SplineGeometries[i].Name.IsEmpty() ? *InInfo.SplineGeometries[i].FeatureClass: *InInfo.SplineGeometries[i].Name), i);
                        NewObj->SetActorLabel(ActorLabel);
                        if(NewObj->GetClass()->ImplementsInterface(ULandscapingVectorInterface::StaticClass()))
                        {
                            FEditorScriptExecutionGuard ScriptGuard;
                            ILandscapingVectorInterface::Execute_OnVectorData(NewObj, InInfo.SplineGeometries[i]);
                            break;
                        }
                        NewObj->Modify(true);
                    }
                    else if(Options.SplineMesh != nullptr)
                    {
                        AActor* NewObj = GetWorld()->SpawnActor<AStaticMeshActor>(Location, FRotator(0));
                        AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(NewObj);
                        UStaticMeshComponent* StaticMeshComponent = MeshActor->GetStaticMeshComponent();
                        StaticMeshComponent->SetStaticMesh(Cast<UStaticMesh>(Options.SplineMesh));
                        FString ActorLabel = FString::Printf(TEXT("%s (Point #%i)"), (InInfo.SplineGeometries[i].Name.IsEmpty() ? *InInfo.SplineGeometries[i].FeatureClass: *InInfo.SplineGeometries[i].Name), i);
                        NewObj->SetActorLabel(ActorLabel);
                        NewObj->Modify(true);
                    }
                }
            }
        }
    }
    return true;
}

bool VectorImporter::HasVectorFile()
{
    return TileFactory->HasVectorFiles();
}

UWorld* VectorImporter::GetWorld()
{
	return GEditor ? GEditor->GetEditorWorldContext(false).World() : nullptr;
}

ULandscapeSplineControlPoint* VectorImporter::AddControlPoint(FVector Point, ALandscape* Landscape, ULandscapeSplinesComponent* LandscapeSplinesComponent, VectorGeometrySpawnOptions Options, double zValue)
{
    FVector SplinePointLocation = GisFM->GetInfos()->SnapToFloor(Point);
    if(Options.bIsBuilding)
    {
        SplinePointLocation.Z = zValue;
    }
    TObjectPtr<ULandscapeSplineControlPoint> NewControlPoint = NewObject<ULandscapeSplineControlPoint>(LandscapeSplinesComponent, NAME_None, RF_Transactional);
    NewControlPoint->Location = SplinePointLocation - Landscape->GetActorLocation();
    NewControlPoint->Width = Options.LandscapeSplineOptions.ControlPointOptions.Width;
    NewControlPoint->LayerWidthRatio = Options.LandscapeSplineOptions.ControlPointOptions.LayerWidthRatio;
    NewControlPoint->SideFalloff = Options.LandscapeSplineOptions.ControlPointOptions.SideFalloff;
    NewControlPoint->LeftSideFalloffFactor = Options.LandscapeSplineOptions.ControlPointOptions.LeftSideFalloffFactor;
    NewControlPoint->RightSideFalloffFactor = Options.LandscapeSplineOptions.ControlPointOptions.RightSideFalloffFactor;
    NewControlPoint->LeftSideLayerFalloffFactor = Options.LandscapeSplineOptions.ControlPointOptions.LeftSideLayerFalloffFactor;
    NewControlPoint->RightSideLayerFalloffFactor = Options.LandscapeSplineOptions.ControlPointOptions.RightSideLayerFalloffFactor;
    NewControlPoint->EndFalloff = Options.LandscapeSplineOptions.ControlPointOptions.EndFalloff;
    NewControlPoint->SegmentMeshOffset = Options.LandscapeSplineOptions.ControlPointOptions.SegmentMeshOffset;
    if(Options.PaintLayer.IsValid())
    {
        FName LayerName = FName(*Options.PaintLayer.Get());
        UE_LOG(LogTemp, Log, TEXT("Landscaping: Controlpoint Layername %s"), *LayerName.ToString());
        NewControlPoint->LayerName = LayerName;
        FPropertyChangedEvent PropertyChangedEventControlPoint(FindFieldChecked<FProperty>(NewControlPoint->GetClass(), FName("LayerName")));
        NewControlPoint->PostEditChangeProperty(PropertyChangedEventControlPoint);
    }
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
    LandscapeSplinesComponent->GetControlPoints().Add(NewControlPoint);
#endif
    return NewControlPoint;
}

void VectorImporter::AddSegment(ULandscapeSplineControlPoint* Start, ULandscapeSplineControlPoint* End, bool bAutoRotateStart, bool bAutoRotateEnd)
{
    FScopedTransaction Transaction(FText::FromString(FString("Add Landscape Spline Segment")));

    if (Start == End)
    {
        //UE_LOG( TEXT("Can't join spline control point to itself.") );
        return;
    }

    if (Start->GetOuterULandscapeSplinesComponent() != End->GetOuterULandscapeSplinesComponent())
    {
        //UE_LOG( TEXT("Can't join spline control points across different terrains.") );
        return;
    }

    for (const FLandscapeSplineConnection& Connection : Start->ConnectedSegments)
    {
        // if the *other* end on the connected segment connects to the "end" control point...
        if (Connection.GetFarConnection().ControlPoint == End)
        {
            //UE_LOG( TEXT("Spline control points already joined connected!") );
            return;
        }
    }

    ULandscapeSplinesComponent* SplinesComponent = Start->GetOuterULandscapeSplinesComponent();
    SplinesComponent->Modify();
    Start->Modify();
    End->Modify();

    TObjectPtr<ULandscapeSplineSegment> NewSegment = NewObject<ULandscapeSplineSegment>(SplinesComponent, NAME_None, RF_Transactional);
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
    SplinesComponent->GetSegments().Add(NewSegment);
#endif

    NewSegment->Connections[0].ControlPoint = Start;
    NewSegment->Connections[1].ControlPoint = End;

    NewSegment->Connections[0].SocketName = Start->GetBestConnectionTo(End->Location);
    NewSegment->Connections[1].SocketName = End->GetBestConnectionTo(Start->Location);

    FVector StartLocation; FRotator StartRotation;
    Start->GetConnectionLocationAndRotation(NewSegment->Connections[0].SocketName, StartLocation, StartRotation);
    FVector EndLocation; FRotator EndRotation;
    End->GetConnectionLocationAndRotation(NewSegment->Connections[1].SocketName, EndLocation, EndRotation);

    // Set up tangent lengths
    NewSegment->Connections[0].TangentLen = (EndLocation - StartLocation).Size();
    NewSegment->Connections[1].TangentLen = NewSegment->Connections[0].TangentLen;

    NewSegment->AutoFlipTangents();

    // set up other segment options
    ULandscapeSplineSegment* CopyFromSegment = nullptr;
    if (Start->ConnectedSegments.Num() > 0)
    {
        CopyFromSegment = Start->ConnectedSegments[0].Segment;
    }
    else if (End->ConnectedSegments.Num() > 0)
    {
        CopyFromSegment = End->ConnectedSegments[0].Segment;
    }
    else
    {
        // Use defaults
    }

    if (CopyFromSegment != nullptr)
    {
        NewSegment->LayerName = CopyFromSegment->LayerName;
        NewSegment->SplineMeshes = CopyFromSegment->SplineMeshes;
        NewSegment->LDMaxDrawDistance = CopyFromSegment->LDMaxDrawDistance;
        NewSegment->bRaiseTerrain = CopyFromSegment->bRaiseTerrain;
        NewSegment->bLowerTerrain = CopyFromSegment->bLowerTerrain;
        NewSegment->bPlaceSplineMeshesInStreamingLevels = CopyFromSegment->bPlaceSplineMeshesInStreamingLevels;
        NewSegment->BodyInstance = CopyFromSegment->BodyInstance;
        NewSegment->bCastShadow = CopyFromSegment->bCastShadow;
        NewSegment->TranslucencySortPriority = CopyFromSegment->TranslucencySortPriority;
        NewSegment->RuntimeVirtualTextures = CopyFromSegment->RuntimeVirtualTextures;
        NewSegment->VirtualTextureLodBias = CopyFromSegment->VirtualTextureLodBias;
        NewSegment->VirtualTextureCullMips = CopyFromSegment->VirtualTextureCullMips;
        NewSegment->VirtualTextureRenderPassType = CopyFromSegment->VirtualTextureRenderPassType;
        NewSegment->bRenderCustomDepth = CopyFromSegment->bRenderCustomDepth;
        NewSegment->CustomDepthStencilWriteMask = CopyFromSegment->CustomDepthStencilWriteMask;
        NewSegment->CustomDepthStencilValue = CopyFromSegment->CustomDepthStencilValue;
    }

    Start->ConnectedSegments.Add(FLandscapeSplineConnection(NewSegment, 0));
    End->ConnectedSegments.Add(FLandscapeSplineConnection(NewSegment, 1));
    
    Start->AutoCalcRotation();
    Start->UpdateSplinePoints();
    End->AutoCalcRotation();
    End->UpdateSplinePoints();
}
