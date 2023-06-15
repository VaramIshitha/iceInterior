// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "Defines.h"
#include "CoreMinimal.h"
#include "GISFileManager.h"
#include "FUserInterfaceParts.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SVectorInputBox.h"
#include "Widgets/Input/SHyperlink.h"
#include "PropertyCustomizationHelpers.h"
#include "EditorWidgets/Public/SEnumCombo.h"
#include "Input/Reply.h"
#include "FLandscapeSplinesOptionsUI.h"

class FVectorImporterUI
{
public:
    FVectorImporterUI(UGISFileManager* InGisFileManager);
    ~FVectorImporterUI();
    TSharedRef<SVerticalBox> ImportVectorFilesUI();
    
private:
    FReply ImportVectorDataClicked();
    FReply SelectVectorDataClicked();
    FReply SetFromGeoReferencingSystemClicked();
    FReply CloseAnchorWindowClicked();
    FReply OpenSetAnchorWindow();
    FReply OpenOptionsWindow();
    FReply OpenOptionsSplineMeshWindow();
    FReply OpenOptionsLandscapeSplineWindow();
    FReply OpenOptionsActorWindow();
    FReply OpenOptionsPaintLayerWindow();
    void OnWindowClosed(const TSharedRef<SWindow>& InWindow);
    FReply HandleSelectVectorData();
    FText SelectVectorDataText() const;
    FText SplineOrActorOptionsText() const;
    FText GetImportInfoVectorData() const;
    FText GetAnchorButtonText() const;
    TSharedRef<SVerticalBox> PaintLayerOptions();
    
    void UpdateLayerList();
    UGISFileManager* GetGisFileManager();

    UGISFileManager* GisFileManager = nullptr;
    FLandscapeSplinesOptionsUI* SplinesOptionsUI = nullptr;
    VectorGeometrySpawnOptions* Options = nullptr;
    FString VectorImportInfo = FString("Import Vector data");
    TArray<TSharedPtr<FString>> ShapeFClasses = TArray<TSharedPtr<FString>>();
    TArray<TSharedPtr<FString>> TypeOptions = TArray<TSharedPtr<FString>>();
    TArray<TSharedPtr<FString>> SplinePointTypes = TArray<TSharedPtr<FString>>();
    TSharedPtr<FString> SelectedTypeOption = nullptr;
    TArray<TSharedPtr<FString>> MaterialLayers = TArray<TSharedPtr<FString>>();
    
    // UI
    float LabelColumnWidth = 0.4f;
    FString AnchorCorner = FString();
    SWindow* CachedAnchorWindow = nullptr;
    SWindow* CachedOptionsSplineMeshWindow = nullptr;
    SWindow* CachedOptionsActorWindow = nullptr;
    SWindow* CachedOptionsPaintLayerWindow = nullptr;
    SWindow* CachedOptionsLandscapeSplineWindow = nullptr;

    bool bTextCommitted = false;
    FLandscapingMapboxFinishedDelegate MapboxFinishedDelegateHandle;
};