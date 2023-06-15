// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SBoxPanel.h"
#if ENGINE_MAJOR_VERSION < 5
#include "EditorWidgets/Public/SEnumCombobox.h"
#else
#include "EditorWidgets/Public/SEnumCombo.h"
#endif
#include "Widgets/Input/SHyperlink.h"
#include "PropertyCustomizationHelpers.h"
#include "Input/Reply.h"
#include "GISFileManager.h"
#include "FUserInterfaceParts.h"

class FFoliageAutomationUI
{
    
public:
    FFoliageAutomationUI(UGISFileManager* FileManager);
    TSharedRef<SVerticalBox> FoliageAutomationUI();
    TSharedRef<SWidget> MaterialSettings(FMaterialProxySettings* Settings);
    FReply GenerateFoliageClicked();

private:
    UGISFileManager* GetGisFileManager();
    FFoliageAutomation* AutomaticTiledLandscape = nullptr;
    TSharedPtr<FAssetThumbnailPool> ThumbnailPool;
    UProceduralFoliageSpawner* Spawner = nullptr;
    FFoliageAutomationOptions Options;
    UGISFileManager* GisFileManager;
    float LabelColumnWidth = 0.4f;
};
