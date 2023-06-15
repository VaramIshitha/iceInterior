// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/Level.h"
#include "Engine/LevelStreamingDynamic.h"
#include "LandscapeInfo.h"
#include "LevelUtils.h"
#include "Landscape.h"
#include "EditorLevelUtils.h"
#include "ALandscapingInfos.h"
#include "FileHelpers.h"
#include "WorldPartition/WorldPartition.h"
#if ENGINE_MINOR_VERSION < 1
#include "WorldPartition/WorldPartitionEditorCell.h"
#endif
#include "WorldPartition/WorldPartitionEditorHash.h"
#include "LevelEditor.h"
#include "SceneOutliner/Public/ISceneOutliner.h"
#include "GISFileManager.h"

class UGISFileManager;

class FLandscapingTileLoader
{
    public:
    FLandscapingTileLoader(UGISFileManager* InGisFileManager);
    ~FLandscapingTileLoader();
    bool LoadWorldPartition(int TileIndex = 0);
    bool UnloadWorldPartition(int TileIndex = 0);
    bool RefreshTileList();

    private:
    UWorld* GetWorld();
    UGISFileManager* GisFM = nullptr;

};