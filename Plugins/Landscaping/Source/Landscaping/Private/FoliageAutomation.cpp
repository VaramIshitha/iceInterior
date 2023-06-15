// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#include "FoliageAutomation.h"

FFoliageAutomation::FFoliageAutomation()
{
}

void FFoliageAutomation::SetCurrentLevel(ULevelStreamingDynamic* CurrentLevel)
{
	LoadedLevel = CurrentLevel;
}

//
// Painting filtering options
//
bool FFoliageGeometryFilter::operator() (const UPrimitiveComponent* Component) const
{
	if (Component)
	{
		bool bFoliageOwned = Component->GetOwner() && FFoliageHelper::IsOwnedByFoliage(Component->GetOwner());

		// Whitelist
		bool bAllowed =
            (bAllowLandscape   && Component->IsA(ULandscapeHeightfieldCollisionComponent::StaticClass())) ||
            (bAllowStaticMesh  && Component->IsA(UStaticMeshComponent::StaticClass()) && !Component->IsA(UFoliageInstancedStaticMeshComponent::StaticClass()) && !bFoliageOwned) ||
            bAllowBSP ||
            (bAllowFoliage     && (Component->IsA(UFoliageInstancedStaticMeshComponent::StaticClass()) || bFoliageOwned));

		// Blacklist
		bAllowed &=
            (bAllowTranslucent || !(Component->GetMaterial(0) && IsTranslucentBlendMode(Component->GetMaterial(0)->GetBlendMode())));

		return bAllowed;
	}

	return false;
}


TArray<UProceduralFoliageComponent*> FFoliageAutomation::AddProceduralFoliageVolume(UProceduralFoliageSpawner* Spawner, FFoliageAutomationOptions InOptions, FBox LandscapeBounds, int TileNum)
{
	if(Spawner == nullptr && !InOptions.bRemoveExistingFoliageVolumes)
	{
		UE_LOG(LogTemp, Error, TEXT("Landscaping: Spawner not set. Aborting."));
		return TArray<UProceduralFoliageComponent*>();
	}
	
	if(LoadedLevel == nullptr && !LandscapeBounds.IsValid)
	{
		UE_LOG(LogTemp, Error, TEXT("Landscaping: Create Procedural Foliage Volume - Level nor Bounds valid. Aborting."));
		return TArray<UProceduralFoliageComponent*>();
	}
	TArray<UProceduralFoliageComponent*> ProceduralFoliageComponents = TArray<UProceduralFoliageComponent*>();
	TArray<AProceduralFoliageVolume*> NewVolumes;
	if(InOptions.bRemoveExistingFoliageVolumes || InOptions.bReUseExistingFoliageVolume)
	{
		TArray<AProceduralFoliageVolume*> Volumes;
		LandscapingUtils::FindAllActors(GetWorld(), Volumes);
		for(AProceduralFoliageVolume* Volume : Volumes)
		{
			if(InOptions.bRemoveExistingFoliageVolumes)
			{
				TArray<UProceduralFoliageComponent*> OutComponents;
				Volume->GetComponents<UProceduralFoliageComponent>(OutComponents, false);
				for(auto Comp : OutComponents)
				{
					Comp->RemoveProceduralContent(false);
				}
				UE_LOG(LogTemp, Warning, TEXT("Landscaping: Destroying Foliage Volume"));
				GetWorld()->DestroyActor(Volume);
			}
			else if(InOptions.bReUseExistingFoliageVolume)
			{
				NewVolumes.Add(Volume);
			}		
		}
	}
	if(InOptions.bRemoveExistingFoliageVolumes)
	{
		TArray<AInstancedFoliageActor*> InstancedActors;
		LandscapingUtils::FindAllActors(GetWorld(), InstancedActors);
		for(AInstancedFoliageActor* InstancedActor : InstancedActors)
		{
			GetWorld()->DestroyActor(InstancedActor);
		}
		return TArray<UProceduralFoliageComponent*>();
	}
	// Create ProceduralFoliageVolume
	UProceduralFoliageSpawner* FoliageSpawner = CastChecked<UProceduralFoliageSpawner>(Spawner);
	FBox Bounds;
	if(LandscapeBounds.IsValid)
	{
		UE_LOG(LogTemp, Log, TEXT("Landscaping: Create Procedural Foliage Volume - Bounds of the Landscape (or LandscapeStreamingProxy): %s"), *LandscapeBounds.ToString());
		Bounds = LandscapeBounds;
	}
	else if(LoadedLevel != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Landscaping: Bounds not valid, using Loaded Level Bounds"));
		Bounds = ALevelBounds::CalculateLevelBounds(LoadedLevel->GetLoadedLevel());
	}
	else 
	{
		UE_LOG(LogTemp, Error, TEXT("Landscaping: LandscapeBounds and Loaded Level not valid."));
		return TArray<UProceduralFoliageComponent*>();
	}
	FVector const& Center = Bounds.GetCenter();
	
	if(InOptions.bAddNewFoliageVolume)
	{
		FRotator const& Rotator = FRotator::ZeroRotator;
		int32 NewIndex = NewVolumes.Add(GetWorld()->SpawnActor<AProceduralFoliageVolume>(FoliageVolumeToSpawn, Center, Rotator));
		NewVolumes[NewIndex]->SetActorLabel(FString::Printf(TEXT("%s_Tile_%i"), *FoliageSpawner->GetName(), TileNum));
	}
	
	for(AProceduralFoliageVolume* PFV : NewVolumes)
	{	
        if(PFV != nullptr)
        {
			int32 ComponentIndex = ProceduralFoliageComponents.Add(PFV->ProceduralComponent);
			check(ProceduralFoliageComponents[ComponentIndex]);
			ProceduralFoliageComponents[ComponentIndex]->FoliageSpawner = FoliageSpawner;
			ProceduralFoliageComponents[ComponentIndex]->SetSpawningVolume(PFV);
            // Set scale
            FVector VolumeScale = Bounds.GetSize() / 200;  // 200 is the size of the Brush
			
			TArray<USceneComponent*> SceneComponents;
			PFV->GetComponents<USceneComponent>(SceneComponents);
			for (auto Comp : SceneComponents)
			{	
				Comp->SetRelativeScale3D(VolumeScale);
			}

			// Add Brush to ProceduralFoliageVolume
			PFV->BrushBuilder = NewObject<UCubeBuilder>();
			CreateBrushForVolumeActor(PFV, PFV->BrushBuilder);
        }
	}
	return ProceduralFoliageComponents;
}

bool FFoliageAutomation::GenerateFoliage(FFoliageAutomationOptions InOptions, TArray<UProceduralFoliageComponent*> ProceduralFoliageComponents) 
{
	
	if(ProceduralFoliageComponents.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Landscaping: No Procedural Foliage Component. Call AddProceduralFoliageVolume first."));
		return false;
	}
	for(UProceduralFoliageComponent* Component : ProceduralFoliageComponents)
	{
		TArray<FDesiredFoliageInstance> DesiredFoliageInstances;
		if (Component->GenerateProceduralContent(DesiredFoliageInstances))
		{
			
			if (DesiredFoliageInstances.Num() > 0)
			{
				Component->RemoveProceduralContent(false);

				FFoliageGeometryFilter OverrideGeometryFilter;
				OverrideGeometryFilter.bAllowLandscape = InOptions.FoliageGeometryFilter.bAllowLandscape;
				OverrideGeometryFilter.bAllowStaticMesh = InOptions.FoliageGeometryFilter.bAllowStaticMesh;
				OverrideGeometryFilter.bAllowBSP = InOptions.FoliageGeometryFilter.bAllowBSP;
				OverrideGeometryFilter.bAllowFoliage = InOptions.FoliageGeometryFilter.bAllowFoliage;
				OverrideGeometryFilter.bAllowTranslucent = InOptions.FoliageGeometryFilter.bAllowTranslucent;

				TMap<const UFoliageType*, TArray<FDesiredFoliageInstance>> SettingsInstancesMap;
				for (const FDesiredFoliageInstance& DesiredInst : DesiredFoliageInstances)
				{
					TArray<FDesiredFoliageInstance>& Instances = SettingsInstancesMap.FindOrAdd(DesiredInst.FoliageType);
					Instances.Add(DesiredInst);
				}

				for (auto It = SettingsInstancesMap.CreateConstIterator(); It; ++It)
				{
					const UFoliageType* Settings = It.Key();

					const TArray<FDesiredFoliageInstance> &DesiredInstances = It.Value();
				
					// FEdModeFoliage::AddInstancesImp
					{
						TArray<FPotentialInstance> PotentialInstanceBuckets[NUM_INSTANCE_BUCKETS];
						TArray<int32> ExistingInstanceBuckets = TArray<int32>();
						float Pressure = 1.0; 
					
						// FEdModeFoliage::CalculatePotentialInstances_ThreadSafe
						{
							LandscapeLayerCacheData LocalCache;
							const int32 StartIdx = 0;
							const int32 LastIdx = DesiredInstances.Num() - 1;
				
							// Reserve space in buckets for a potential instances 
							for (int32 BucketIdx = 0; BucketIdx < NUM_INSTANCE_BUCKETS; ++BucketIdx)
							{
								auto& Bucket = PotentialInstanceBuckets[BucketIdx];
								Bucket.Reserve(DesiredInstances.Num());
							}

							for (int32 InstanceIdx = StartIdx; InstanceIdx <= LastIdx; ++InstanceIdx)
							{
								const FDesiredFoliageInstance& DesiredInst = (DesiredInstances)[InstanceIdx];
								FHitResult Hit;
								static FName NAME_AddFoliageInstances = FName(TEXT("AddFoliageInstances"));

								if (AInstancedFoliageActor::FoliageTrace(Component->GetWorld(), Hit, DesiredInst, NAME_AddFoliageInstances, true, OverrideGeometryFilter))
								{
									float HitWeight = 1.f;
									const bool bValidInstance = CheckLocationForPotentialInstance_ThreadSafe(Settings, Hit.ImpactPoint, Hit.ImpactNormal)
                                        && LandscapeLayerCheck(Hit, Settings, LocalCache, HitWeight);
						
									if (bValidInstance)
									{
										const int32 BucketIndex = FMath::RoundToInt(HitWeight * (float)(NUM_INSTANCE_BUCKETS - 1));
										PotentialInstanceBuckets[BucketIndex].Add(FPotentialInstance(Hit.ImpactPoint, Hit.ImpactNormal, Hit.Component.Get(), HitWeight, DesiredInst));
									}
								}
							}
						} // END FEdModeFoliage::CalculatePotentialInstances_ThreadSafe

						// Existing foliage types in the palette  we want to override any existing mesh settings with the procedural settings.
						TMap<AInstancedFoliageActor*, TArray<const UFoliageType*>> UpdatedTypesByIFA;
						for (TArray<FPotentialInstance>& Bucket : PotentialInstanceBuckets)
						{
							for (auto& PotentialInst : Bucket)
							{
								// Get the IFA for the base component level that contains the component the instance will be placed upon
								AInstancedFoliageActor* TargetIFA = AInstancedFoliageActor::GetInstancedFoliageActorForLevel(PotentialInst.HitComponent->GetComponentLevel(), true);

								// Update the type in the IFA if needed
								TArray<const UFoliageType*>& UpdatedTypes = UpdatedTypesByIFA.FindOrAdd(TargetIFA);
								if (!UpdatedTypes.Contains(PotentialInst.DesiredInstance.FoliageType))
								{
									UpdatedTypes.Add(PotentialInst.DesiredInstance.FoliageType);
									TargetIFA->AddFoliageType(PotentialInst.DesiredInstance.FoliageType);
								}
							}
						}
				
						for (int32 BucketIdx = 0; BucketIdx < NUM_INSTANCE_BUCKETS; BucketIdx++)
						{
							TArray<FPotentialInstance>& PotentialInstances = PotentialInstanceBuckets[BucketIdx];
							float BucketFraction = (float)(BucketIdx + 1) / (float)NUM_INSTANCE_BUCKETS;
						
							// We use the number that actually succeeded in placement (due to parameters) as the target
							// for the number that should be in the brush region.
							const int32 BucketOffset = (ExistingInstanceBuckets.Num() ? ExistingInstanceBuckets[BucketIdx] : 0);
							int32 AdditionalInstances = FMath::Clamp<int32>(FMath::RoundToInt(BucketFraction * (float)(PotentialInstances.Num() - BucketOffset) * Pressure), 0, PotentialInstances.Num());

							TArray<FFoliageInstance> PlacedInstances;
							PlacedInstances.Reserve(AdditionalInstances);

							for (int32 Idx = 0; Idx < AdditionalInstances; Idx++)
							{
								FPotentialInstance& PotentialInstance = PotentialInstances[Idx];
								FFoliageInstance Inst;
								if (PotentialInstance.PlaceInstance(Component->GetWorld(), Settings, Inst))
								{
									Inst.ProceduralGuid = PotentialInstance.DesiredInstance.ProceduralGuid;
									Inst.BaseComponent = PotentialInstance.HitComponent;
									PlacedInstances.Add(MoveTemp(Inst));
								}
							}
						
							// FEdModeFoliage::SpawnFoliageInstance
							{
								TMap<ULevel*, TArray<const FFoliageInstance*>> PerLevelPlacedInstances;					
				
								for (const FFoliageInstance& PlacedInstance : PlacedInstances)
								{
									TArray<const FFoliageInstance*>& LevelInstances = PerLevelPlacedInstances.FindOrAdd(PlacedInstance.BaseComponent->GetComponentLevel());
									LevelInstances.Add(&PlacedInstance);
								}
				

								for (const auto& PlacedLevelInstances : PerLevelPlacedInstances)
								{
									ULevel* TargetLevel = PlacedLevelInstances.Key;

									CurrentFoliageTraceBrushAffectedLevels.AddUnique(TargetLevel);
									AInstancedFoliageActor* IFA = AInstancedFoliageActor::GetInstancedFoliageActorForLevel(TargetLevel, true);

									FFoliageInfo* Info = nullptr;
									UFoliageType* FoliageSettings = IFA->AddFoliageType(Settings, &Info);
#if ENGINE_MAJOR_VERSION < 5
									Info->AddInstances(IFA, FoliageSettings, PlacedLevelInstances.Value);
#else
									Info->AddInstances(FoliageSettings, PlacedLevelInstances.Value);
#endif
									Info->Refresh(true, false);
								}
							}						
						}				
					}
				}
			}
			// If no instances were spawned, inform the user
			if (!Component->HasSpawnedAnyInstances())
			{
				UE_LOG(LogTemp, Error, TEXT("Landscaping: Could not spawn Foliage - check your settings of the FoliageTypes"));
				return false;
            }
		}
	}
	
	return true;
}

/** This does not check for overlaps or density */
bool FFoliageAutomation::CheckLocationForPotentialInstance_ThreadSafe(const UFoliageType* Settings, const FVector& Location, const FVector& Normal)
{
	// Check height range
	if (!Settings->Height.Contains(Location.Z))
	{
		return false;
	}

	// Check slope
	// ImpactNormal sometimes is slightly non-normalized, so compare slope with some little deviation
	return IsWithinSlopeAngle(Normal.Z, Settings->GroundSlopeAngle.Min, Settings->GroundSlopeAngle.Max, SMALL_NUMBER);
}

bool FFoliageAutomation::IsWithinSlopeAngle(float NormalZ, float MinAngle, float MaxAngle, float Tolerance = SMALL_NUMBER)
{
	const float MaxNormalAngle = FMath::Cos(FMath::DegreesToRadians(MaxAngle));
	const float MinNormalAngle = FMath::Cos(FMath::DegreesToRadians(MinAngle));
	return !(MaxNormalAngle > (NormalZ + Tolerance) || MinNormalAngle < (NormalZ - Tolerance));
}

bool FFoliageAutomation::LandscapeLayerCheck(const FHitResult& Hit, const UFoliageType* Settings, LandscapeLayerCacheData& LandscapeLayersCache, float& OutHitWeight)
{
	OutHitWeight = 1.f;
	if (IsLandscapeLayersArrayValid(Settings->LandscapeLayers) && GetMaxHitWeight(Hit.ImpactPoint, Hit.Component.Get(), Settings->LandscapeLayers, &LandscapeLayersCache, OutHitWeight))
	{
		// Reject instance randomly in proportion to weight
		if (IsFilteredByWeight(OutHitWeight, Settings->MinimumLayerWeight, false))
		{
			return false;
		}
	}

	float HitWeightExclusion = 1.f;
	if (IsLandscapeLayersArrayValid(Settings->ExclusionLandscapeLayers) && GetMaxHitWeight(Hit.ImpactPoint, Hit.Component.Get(), Settings->ExclusionLandscapeLayers, &LandscapeLayersCache, HitWeightExclusion))
	{
		// Reject instance randomly in proportion to weight
		const bool bExclusionTest = true;
		if (IsFilteredByWeight(HitWeightExclusion, Settings->MinimumExclusionLayerWeight, bExclusionTest))
		{
			return false;
		}
	}

	return true;
}

bool FFoliageAutomation::IsLandscapeLayersArrayValid(const TArray<FName>& LandscapeLayersArray)
{
	bool bValid = false;
	for (FName LayerName : LandscapeLayersArray)
	{
		bValid |= LayerName != NAME_None;
	}

	return bValid;
}

bool FFoliageAutomation::GetMaxHitWeight(const FVector& Location, UActorComponent* ActorComponent, const TArray<FName>& LandscapeLayersArray, LandscapeLayerCacheData* LandscapeLayerCaches, float& OutMaxHitWeight)
{
	float MaxHitWeight = 0.f;
	if (ULandscapeHeightfieldCollisionComponent* HitLandscapeCollision = Cast<ULandscapeHeightfieldCollisionComponent>(ActorComponent))
	{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 1
		if (ULandscapeComponent* HitLandscape = HitLandscapeCollision->GetRenderComponent())
#else
		if (ULandscapeComponent* HitLandscape = HitLandscapeCollision->RenderComponent.Get())
#endif
		{
			for (const FName LandscapeLayerName : LandscapeLayersArray)
			{
				// Cache store mapping between component and weight data
				TMap<ULandscapeComponent*, TArray<uint8> >* LandscapeLayerCache = &LandscapeLayerCaches->FindOrAdd(LandscapeLayerName);;
				TArray<uint8>* LayerCache = &LandscapeLayerCache->FindOrAdd(HitLandscape);
				const float HitWeight = HitLandscape->GetLayerWeightAtLocation(Location, HitLandscape->GetLandscapeInfo()->GetLayerInfoByName(LandscapeLayerName), LayerCache);
				MaxHitWeight = FMath::Max(MaxHitWeight, HitWeight);
			}

			OutMaxHitWeight = MaxHitWeight;
			return true;
		}
	}

	return false;
}

bool FFoliageAutomation::IsFilteredByWeight(float Weight, float TestValue, bool bExclusionTest)
{
	if (bExclusionTest)
	{
		// Exclusion always tests 
		const float WeightNeeded = FMath::Max(SMALL_NUMBER, TestValue);
		return Weight >= WeightNeeded;
	}
	else
	{
		const float WeightNeeded = FMath::Max(SMALL_NUMBER, FMath::Max(TestValue, FMath::FRand()));
		return Weight < WeightNeeded;
	}
}

void FFoliageAutomation::CreateBrushForVolumeActor(AVolume* NewActor, UBrushBuilder* BrushBuilder)
{
	if ( NewActor != NULL )
	{
		// build a brush for the new actor
		NewActor->PreEditChange(NULL);

		NewActor->PolyFlags = 0;
		NewActor->Brush = NewObject<UModel>(NewActor, NAME_None, RF_Transactional);
		NewActor->Brush->Initialize(nullptr, true);
		NewActor->Brush->Polys = NewObject<UPolys>(NewActor->Brush, NAME_None, RF_Transactional);
		NewActor->GetBrushComponent()->Brush = NewActor->Brush;
		if(BrushBuilder != nullptr)
		{
			NewActor->BrushBuilder = DuplicateObject<UBrushBuilder>(BrushBuilder, NewActor);
		}

		BrushBuilder->Build( NewActor->GetWorld(), NewActor );

		FBSPOps::csgPrepMovingBrush( NewActor );
		if (UBrushComponent* MyBrushComponent = NewActor->GetBrushComponent())
		{
			MyBrushComponent->SetCollisionObjectType(ECC_WorldStatic);
			MyBrushComponent->SetCollisionResponseToAllChannels(ECR_Ignore);

			// This is important because the volume overlaps with all procedural foliage
			// That means during streaming we'll get a huge hitch for UpdateOverlaps
			MyBrushComponent->SetGenerateOverlapEvents(false);
		}
		NewActor->PostEditChange();
	}
}


UWorld* FFoliageAutomation::GetWorld() const
{
	return GEditor ? GEditor->GetEditorWorldContext(false).World() : nullptr;
}
