// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved


#include "LandscapingTileLoader.h"
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
#include "WorldPartition/LoaderAdapter/LoaderAdapterShape.h"
#endif


FLandscapingTileLoader::FLandscapingTileLoader(UGISFileManager* InGisFileManager)
{
	GisFM = InGisFileManager;
}

FLandscapingTileLoader::~FLandscapingTileLoader()
{
	GisFM = nullptr;
}


bool FLandscapingTileLoader::LoadWorldPartition(int TileIndex)
{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 0
	ULandscapingSettings* Settings = GetMutableDefault<ULandscapingSettings>();
	if(!Settings->bEnableLargeWorlds || !GisFM->GetInfos()->bWorldPartition)
	{
		return true;
	}
	if(GetWorld() != nullptr && GetWorld()->GetWorldPartition() != nullptr && GisFM->GetInfos()->Tiles.Num() > TileIndex)
	{
		GetWorld()->GetWorldSettings()->GetWorldPartition()->LoadEditorCells(GisFM->GetInfos()->Tiles[TileIndex].Bounds, true);
		GEditor->RedrawLevelEditingViewports();
		TWeakPtr<class ILevelEditor> LevelEditor = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor")).GetLevelEditorInstance();
		if (LevelEditor.IsValid())
		{
			TSharedPtr<class ISceneOutliner> SceneOutlinerPtr = LevelEditor.Pin()->GetSceneOutliner();
			if (SceneOutlinerPtr)
			{
				SceneOutlinerPtr->FullRefresh();
			}
		}
		return true;
	}
	return false;
#else
	return true;
#endif
}

bool FLandscapingTileLoader::UnloadWorldPartition(int TileIndex)
{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 0
	ULandscapingSettings* Settings = GetMutableDefault<ULandscapingSettings>();
	if(!Settings->bEnableLargeWorlds || !GisFM->GetInfos()->bWorldPartition)
	{
		return true;
	}
	if(GisFM->GetInfos()->Tiles.Num() <= TileIndex)
	{
		UE_LOG(LogTemp, Error, TEXT("Landscaping: TileIndex > Number of Tiles - cannot unload World Partition Cells"));
		return false;
	}
	if(GetWorld()->GetWorldPartition() != nullptr && GisFM->GetInfos()->Tiles.Num() > TileIndex)
	{
		if(!UEditorLoadingAndSavingUtils::SaveCurrentLevel())
		{
			UE_LOG(LogTemp, Error, TEXT("Landscaping: Could not save current level - Please check Landscaping Settings! Enable Large World is true, therefore a level must be saved before unload."));
			return false;
		}
		FBox Box = GisFM->GetInfos()->Tiles[TileIndex].Bounds;
		auto GetCellsToUnload = [this, Box](TArray<UWorldPartitionEditorCell*>& CellsToUnload)
		{
			return GetWorld()->GetWorldSettings()->GetWorldPartition()->EditorHash->GetIntersectingCells(Box, CellsToUnload) > 0;
		};

		TArray<UWorldPartitionEditorCell*> CellsToProcess;
		if (!GetCellsToUnload(CellsToProcess))
		{
			return true;
		}

		TSet<UPackage*> ModifiedPackages;
		TMap<FWorldPartitionActorDesc*, int32> UnloadCount;

		for (UWorldPartitionEditorCell* Cell : CellsToProcess)
		{
			for (const UWorldPartitionEditorCell::FActorReference& ActorDesc : Cell->LoadedActors)
			{
				if (ActorDesc.IsValid())
				{
					UnloadCount.FindOrAdd(*ActorDesc, 0)++;
				}
			}
		}
		bool bIsUpdateUnloadingActors = false;
		for (const TPair<FWorldPartitionActorDesc*, int32> Pair : UnloadCount)
		{
			FWorldPartitionActorDesc* ActorDesc = Pair.Key;

			// Only prompt if the actor will get unloaded by the unloading cells
			if (ActorDesc->GetHardRefCount() == Pair.Value)
			{
				if (AActor* LoadedActor = ActorDesc->GetActor())
				{
					UPackage* ActorPackage = LoadedActor->GetExternalPackage();
					if (ActorPackage && ActorPackage->IsDirty())
					{
						ModifiedPackages.Add(ActorPackage);
					}
					bIsUpdateUnloadingActors = true;
				}
			}
		}
		// Make sure we save modified actor packages before unloading
		FEditorFileUtils::EPromptReturnCode RetCode = FEditorFileUtils::PR_Success;
		// Skip this when running commandlet because MarkPackageDirty() is allowed to dirty packages at loading when running a commandlet.
		// Usually, the main reason actor packages are dirtied is when RerunConstructionScripts is called on the actor when it is added to the level.
		if (ModifiedPackages.Num())
		{
			const bool bCheckDirty = true;
			const bool bAlreadyCheckedOut = true;
			const bool bCanBeDeclined = false;
			const bool bPromptToSave = false;
			const FText Title = FText::FromString("Save Actor(s)");
			const FText Message = FText::FromString("Save Actor(s) before unloading them.");

			RetCode = FEditorFileUtils::PromptForCheckoutAndSave(ModifiedPackages.Array(), bCheckDirty, bPromptToSave, Title, Message, nullptr, bAlreadyCheckedOut, bCanBeDeclined);
			UE_LOG(LogTemp, Log, TEXT("Landscaping: Saving Editor Cells - Modified Packages %i"), ModifiedPackages.Num());
			check(RetCode != FEditorFileUtils::PR_Failure);
		}
		if (bIsUpdateUnloadingActors)
		{
			GEditor->SelectNone(true, true);
		}

		GetWorld()->GetWorldSettings()->GetWorldPartition()->UnloadEditorCells(Box, true);
		GEditor->RedrawLevelEditingViewports();
		TWeakPtr<class ILevelEditor> LevelEditor = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor")).GetLevelEditorInstance();
		if (LevelEditor.IsValid())
		{
			TSharedPtr<class ISceneOutliner> SceneOutlinerPtr = LevelEditor.Pin()->GetSceneOutliner();
			if (SceneOutlinerPtr)
			{
				SceneOutlinerPtr->FullRefresh();
			}
		}
		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
		GEngine->ForceGarbageCollection(true);
		return true;
	}
	UE_LOG(LogTemp, Log, TEXT("Landscaping: Unlaod world partition failed"));
	return false;
#else
	return true;
#endif
}

bool FLandscapingTileLoader::RefreshTileList()
{
	if(GetWorld() == nullptr)
	{
		return true;
	}
	bool bAlteredTileList = false;
	TArray<ALandscape*> Landscapes;
	LandscapingUtils::FindAllActors<ALandscape>(GetWorld(), Landscapes);
	
	// check if a landscape got deleted and delete the FLandscapingInfo (Tile) in ALandscapingInfos
	int TileToRemove = -1;
	int NumberOfLandscapes = 0;
	do 
	{
		TileToRemove = -1;
		NumberOfLandscapes = 0;
		for(int TileIndex = 0; TileIndex < GisFM->GetInfos()->Tiles.Num(); TileIndex++)
		{
			if(GisFM->GetInfos()->Tiles[TileIndex].Landscape == nullptr && GisFM->GetInfos()->Tiles[TileIndex].LandscapeSM == nullptr && GisFM->GetInfos()->Tiles[TileIndex].LandscapeNaniteMesh == nullptr)
			{
				TileToRemove = TileIndex;
			}
			if(GisFM->GetInfos()->Tiles[TileIndex].Landscape != nullptr)
			{
				NumberOfLandscapes++;
			}
		}
		
		if(TileToRemove > -1)
		{
			UE_LOG(LogTemp, Log, TEXT("Landscaping: Remove Tile Info at Index %i"), TileToRemove);
			GisFM->GetInfos()->Tiles.RemoveAt(TileToRemove);
		}
	}
	while(TileToRemove > -1);
	
	// add landscapes, which are not in the Tile list of ALandscapingInfos
	if(Landscapes.Num() != NumberOfLandscapes)
	{
		for(int Idx = 0; Idx < Landscapes.Num(); Idx++)
		{
			bool bIsLandscapeInList = false;
			for(int TileIndex = 0; TileIndex < GisFM->GetInfos()->Tiles.Num(); TileIndex++)
			{
				if(Landscapes[Idx] == GisFM->GetInfos()->Tiles[TileIndex].Landscape)
				{
					bIsLandscapeInList = true;
					break;
				}
			}
			if(bIsLandscapeInList)
			{
				continue;
			}
			ALandscape* LandscapeToAdd = Landscapes[Idx];
			UE_LOG(LogTemp, Log, TEXT("Landscaping: Found Landscape which is not in the Tiles List - Adding to Landscaping Tiles"));
			FVector Translation = LandscapeToAdd->LandscapeActorToWorld().GetTranslation();
			FLandscapingInfo Tile = FLandscapingInfo();
			Tile.LocationX = Translation.X / GisFM->GetCRS()->GetLandscapeScaleFactor();
			Tile.LocationY = Translation.Y / GisFM->GetCRS()->GetLandscapeScaleFactor();
			Tile.LocationZ = Translation.Z / GisFM->GetCRS()->GetLandscapeScaleFactor();
			Tile.LandscapeScale = LandscapeToAdd->LandscapeActorToWorld().GetScale3D() / GisFM->GetCRS()->GetLandscapeScaleFactor();;
			Tile.MeterPerPixelX = 1;
			Tile.MeterPerPixelY = -1;
			ULandscapeInfo* LandscapeInfo = LandscapeToAdd->GetLandscapeInfo();
			FIntRect Rect = LandscapeToAdd->GetBoundingRect();
			LandscapeInfo->GetLandscapeExtent(Rect.Min.X, Rect.Min.Y, Rect.Max.X, Rect.Max.Y);
			FIntPoint Resolution = Rect.Size();
			Tile.LandscapeResolution = FIntVector(Resolution.X + 1, Resolution.Y + 1, 0);
			Tile.ImportResolution = Tile.LandscapeResolution;
			Tile.Bounds = LandscapeToAdd->GetComponentsBoundingBox(false, true);
			Tile.Landscape = LandscapeToAdd;
			GisFM->GetInfos()->Tiles.Add(Tile);
		}
	}
	if(bAlteredTileList && GisFM->GetInfos()->Tiles.Num() == 0)
	{
		return false;
	}
	return true;
}

UWorld* FLandscapingTileLoader::GetWorld()
{
	return GEditor ? GEditor->GetEditorWorldContext(false).World() : nullptr;
}
