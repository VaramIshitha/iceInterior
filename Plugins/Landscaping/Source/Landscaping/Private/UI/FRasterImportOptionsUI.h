// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "Input/Reply.h"
#include "Widgets/Input/SHyperlink.h"
#include "GISFileManager.h"
#include "SWebBrowser.h"
#include "SWebBrowserView.h"
#include "RasterImporter.h"

// class FRasterImporterUI;
class FRasterImportOptionsUI
{

public:
    FRasterImportOptionsUI(UGISFileManager* InGisFileManager, float InLabelColumnWidth = 0.4f);
    TSharedRef<SWindow> GetDTMOptionsWindow();
    FString CalculateArea(bool bFetchFromNewExtents = false);
    RasterImportOptions ImportOptions;
    bool bRasterSelected = false;
    TSharedPtr<FString> RasterFileImportInfo;
    TSharedPtr<FString> CRSInfo;
    FText GetImportInfo() const;
    FText GetCRSInfo() const;
    void ShowCRS();

private:
    FText GetImportBBox() const;
    void ShowMap() const;
    void HandleBrowserUrlChanged(const FText& Url);
    void ShowWarning();

    UGISFileManager* GisFileManager = nullptr;
    float LabelColumnWidth = 0.4f;
    FString ImportBBox = FString();
    UGISFileManager* GetGisFileManager();
    TSharedPtr<SOverlay> BrowserContainer;
	TSharedPtr<SWebBrowserView> BrowserView;
    TSharedPtr<FBrowserContextSettings> BrowserSettings;
    bool bWarningShown = false;
};