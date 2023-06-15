// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SHyperlink.h"
#include "PropertyCustomizationHelpers.h"
#include "Input/Reply.h"
#include "Templates/SharedPointer.h"
#include <regex>
#include "GISFileManager.h"
#include "RasterImporter.h"
#include "FUserInterfaceParts.h"
#include "FRasterImportOptionsUI.h"

class FRasterImporterUI
{
    
public:
    FRasterImporterUI(UGISFileManager* InGisFileManager);
    TSharedRef<SVerticalBox> ImportRasterFilesUI();
    void UpdateAreaLabel(bool bFetchFromNewExtents = false);
    
private:
    TSharedRef<ITableRow> OnGenerateWidgetForLayerDataListView(TSharedPtr<MaterialLayerSettings> InLayerData, const TSharedRef<STableViewBase>& OwnerTable);
    TSharedRef<ITableRow> OnGenerateWidgetForLanduseListView(TSharedPtr<Landuse> InLanduseData, const TSharedRef<STableViewBase>& OwnerTable);
    TSharedRef<ITableRow> OnGenerateWidgetForLanduseNamesListView(TSharedPtr<Landuse> InLanduseData, const TSharedRef<STableViewBase>& OwnerTable);
    FReply SelectRasterFilesClicked();
    FReply SelectWeightmapsClicked();
    FReply ImportRasterFilesClicked();
    FReply SaveAndReplaceLandscapeSMClicked();
    FReply UpdateWeightmapsClicked();
    FReply AddSatelliteImageClicked();
    
    void UpdateProj();
    void UpdateLandcoverTypes();
    void UpdateOnLanduseSelectionChanged();
    void RefreshLayerLandcoverMapping();
    void UpdateLayerList();
    FText GetImportButtonText() const;
    FText GetImportInfoRasterFile() const;
    FReply OpenMaterialAndWeightmapsWindow();
    FReply OpenDTMOptionsWindow();
    UGISFileManager* GetGisFileManager();
    
    UGISFileManager* GisFileManager = nullptr;
    UMaterialInterface* Material = nullptr;
    TArray<FString> AvailableLandcoverTypes = TArray<FString>();
    TSharedPtr<FAssetThumbnailPool> ThumbnailPool = TSharedPtr<FAssetThumbnailPool>();
    TSharedPtr<SListView<TSharedPtr<MaterialLayerSettings>>> LayerDataListView = TSharedPtr<SListView<TSharedPtr<MaterialLayerSettings>>>();
    TArray<TSharedPtr<MaterialLayerSettings>> LayerDataList = TArray<TSharedPtr<MaterialLayerSettings>>();
    TSharedPtr<STileView<TSharedPtr<Landuse>>> DummyLanduseListView = TSharedPtr<STileView<TSharedPtr<Landuse>>>();
    TArray<TSharedPtr<Landuse>> DummyLanduseList = TArray<TSharedPtr<Landuse>>();
    TArray<TSharedPtr<FString>> MaterialLayers = TArray<TSharedPtr<FString>>();
    TArray<TSharedPtr<FString>> ColorChannels = TArray<TSharedPtr<FString>>();
    
    SWindow* CachedWeightmapWindow;
    SWindow* CachedDTMOptionsWindow;
    float LabelColumnWidth = 0.4f;
    float MaterialLayerColumnWidth = 0.5f;
    TSharedPtr<FString> DefaultLayer = TSharedPtr<FString>();
    FRasterImportOptionsUI* DTMOptions = nullptr;
    FLandscapingMapboxFinishedDelegate MapboxFinishedDelegateHandle;
};
