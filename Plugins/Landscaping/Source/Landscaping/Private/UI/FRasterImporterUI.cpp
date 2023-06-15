// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved



#include "FRasterImporterUI.h"

FRasterImporterUI::FRasterImporterUI(UGISFileManager* InGisFileManager)
{
    GisFileManager = InGisFileManager;
    DTMOptions = new FRasterImportOptionsUI(GisFileManager, LabelColumnWidth);
    if(GetGisFileManager()->HasLandscape())
    {
        UpdateAreaLabel();
    }
    else 
    {
        GetGisFileManager()->GetInfos()->ResetCRS();
    }
    Material = GetGisFileManager()->GetLandscapeMaterial();
    // DTMOptions->RasterFileImportInfo = MakeShareable(new FString("Import Digital Terrain Models"));
    DTMOptions->CRSInfo = MakeShareable(new FString(GetGisFileManager()->GetCRS()->GetAuthorityID() == 0 ? FString("CRS not set") : GetGisFileManager()->GetCRS()->GetAuthorityIDStr()));
    ColorChannels.Empty();
    ColorChannels.Add(MakeShareable(new FString("A")));
    ColorChannels.Add(MakeShareable(new FString("R")));
    ColorChannels.Add(MakeShareable(new FString("B")));
    ColorChannels.Add(MakeShareable(new FString("G")));
    if(GetGisFileManager()->HasLandscape())
    {
        FLandscapingInfo TileInfo = GetGisFileManager()->GetInfos()->Tiles[0];
        // Get Current Tile Size and add some slack
        int CurrentTileSize = (int)FMath::Max(TileInfo.LandscapeResolution.X * TileInfo.LandscapeScale.X, TileInfo.LandscapeResolution.Y * TileInfo.LandscapeScale.Y) + 10;
        DTMOptions->ImportOptions.DesiredMaxTileSize = FMath::Max(GetGisFileManager()->GetInfos()->DesiredMaxTileSize, CurrentTileSize);
    }
}

void FRasterImporterUI::UpdateAreaLabel(bool bFetchFromNewExtents)
{
    FString AreaStr = DTMOptions->CalculateArea(bFetchFromNewExtents);
    DTMOptions->RasterFileImportInfo = MakeShareable(new FString(AreaStr));
}

TSharedRef<SVerticalBox> FRasterImporterUI::ImportRasterFilesUI()
{    
    ThumbnailPool = MakeShareable(new FAssetThumbnailPool(16, false));
    FUserInterfaceParts* UserInterfaceParts = new FUserInterfaceParts();
    return SNew(SVerticalBox)
    + SVerticalBox::Slot()
    .AutoHeight()
    [
        UserInterfaceParts->SHeader1("Import Landscape", "")
    ]
    + SVerticalBox::Slot()
    .AutoHeight()
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .Padding(0, 5)
        .HAlign(HAlign_Left)
        .FillWidth(2)
        [
            SNew(SHyperlink)
            .Text_Raw(DTMOptions, &FRasterImportOptionsUI::GetCRSInfo)
            .OnNavigate_Raw(DTMOptions, &FRasterImportOptionsUI::ShowCRS)
            .ToolTipText(FText::FromString("Current Level's CRS\nIt will be detected from input file or can be changed in 'Project Settings -> Plugins -> Landscaping'\n\nThe link opens the CRS' description on epsg.io (external link)"))
        ]
        + SHorizontalBox::Slot()
        .Padding(0, 5)
        .HAlign(HAlign_Left)
        .FillWidth(5)
        [
            SNew(STextBlock)
            .Text_Raw(DTMOptions, &FRasterImportOptionsUI::GetImportInfo)
            .WrapTextAt(600.0f)
        ]
        + SHorizontalBox::Slot()
        .Padding(0, 5)
        .HAlign(HAlign_Right)
        .FillWidth(2)
        [
            SNew(SHyperlink)
            .Text(FText::FromString("Documentation"))
            .OnNavigate_Lambda([=]()
            {
                FPlatformProcess::LaunchURL(TEXT("https://jorop.github.io/landscaping-docs/#/get-started?id=import-heightmap"), nullptr, nullptr);
            })
            .ToolTipText(FText::FromString("Documentation on how to import a DTM file as Unreal Engine Landscape."))
        ]
    ]
    + SVerticalBox::Slot()
    .AutoHeight()
    [
        SNew(SSplitter)
        .Orientation(Orient_Horizontal)
        + SSplitter::Slot()
        .Value(LabelColumnWidth)
        [
            SNew(SBorder)
            #if ENGINE_MINOR_VERSION < 1
            .BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
            #else
            .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
            #endif
            .BorderBackgroundColor(FLinearColor::Gray)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .FillHeight(12)
                .VAlign(VAlign_Center)
                .Padding(5)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString("Select DTM Files"))
                ]
                + SVerticalBox::Slot()
                .FillHeight(12)
                .VAlign(VAlign_Center)
                .Padding(5)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString("DTM Import Options"))
                ]
                + SVerticalBox::Slot()
                .FillHeight(35)
                .VAlign(VAlign_Center)
                .Padding(5)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString("Landscape Material"))
                    .ToolTipText(FText::FromString("The Landscape Material to use for the (tiled) Landscape.\nUse Landscape Material Settings below to change after import of heightmaps."))
                ]
                + SVerticalBox::Slot()
                .FillHeight(12)
                .VAlign(VAlign_Center)
                .Padding(5)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString("Default Layer"))
                    .ToolTipText(FText::FromString("Default layer which is used if there is no specific paint layer in the area."))
                ]
#if ENGINE_MAJOR_VERSION < 5
                + SVerticalBox::Slot()
                .FillHeight(12)
                .VAlign(VAlign_Center)
                .Padding(5)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString("Only convert to PNGs"))
                ]
#endif
                + SVerticalBox::Slot()
                .FillHeight(12)
                .VAlign(VAlign_Center)
                .Padding(5)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString("Import selected"))
                ]
            ]
        ]
        + SSplitter::Slot()
        .Value(1.f - LabelColumnWidth)
        [
            SNew(SBorder)
            #if ENGINE_MINOR_VERSION < 1
            .BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
            #else
            .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
            #endif
            .BorderBackgroundColor(FLinearColor::Gray)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .FillHeight(12)
                .VAlign(VAlign_Center)
                .Padding(5)
                [
                    SNew(SButton)
                    .Text(FText::FromString("Select"))
                    .HAlign(HAlign_Center)
                    .OnClicked_Raw(this, &FRasterImporterUI::SelectRasterFilesClicked)
                    .IsEnabled_Lambda([=]()
                    {
                        return !DTMOptions->bRasterSelected;
                    })
                    .ToolTipText(FText::FromString("Select Digital Terrain Models.\n\nPlease make sure you select Digital Terrain Models (DTM) or Digital Elevation Models (DEM) and not Digital Surface Models (DSM)."))
                ]
                // Crop import Area
                + SVerticalBox::Slot()
                .FillHeight(12)
                .VAlign(VAlign_Center)
                .Padding(5)
                [
                    SNew(SButton)
                    .Text(FText::FromString("Options"))
                    .HAlign(HAlign_Center)
                    .OnClicked_Raw(this, &FRasterImporterUI::OpenDTMOptionsWindow)
                    .IsEnabled_Lambda([=]()
                    {
                        return DTMOptions->bRasterSelected || GetGisFileManager()->HasLandscape() || GetGisFileManager()->HasMapboxExtension();
                    })
                    .ToolTipText(FText::FromString("Options for DTM Import."))
                ]
                // Material
                + SVerticalBox::Slot()
                .FillHeight(35)
                .VAlign(VAlign_Center)
                .Padding(5)
                [
                    SNew(SObjectPropertyEntryBox)
                    .AllowedClass(UMaterialInterface::StaticClass())
                    .ObjectPath_Lambda([=]()
                    {
                        if(Material != nullptr) 
                        {
                            return Material->GetPathName();
                        }
                        if(GisFileManager != nullptr && GetGisFileManager()->GetLandscapeMaterial() != nullptr)
                        {
                            return GetGisFileManager()->GetLandscapeMaterial()->GetPathName();
                        }
                        return FString();
                    })
                    .OnObjectChanged_Lambda([=](FAssetData Asset)
                    {
                        UObject* NewMaterial = Asset.GetAsset();
                        Material = (UMaterialInterface*)NewMaterial;
                        UpdateLayerList();
                        RefreshLayerLandcoverMapping();
                    })
                    .ThumbnailPool(ThumbnailPool)
                    .ToolTipText(FText::FromString("The Landscape Material to use for the (tiled) Landscape.\nOpen Landscape Material Settings below to change after import of heightmaps."))
                    .IsEnabled_Lambda([=]()
                    {
                        return DTMOptions->bRasterSelected && !GetGisFileManager()->HasLandscape();
                    })
                ]
                + SVerticalBox::Slot()
                .FillHeight(10)
                .VAlign(VAlign_Center)
                .Padding(5)
                [
                    SNew(SComboBox<TSharedPtr<FString>>)
                    .OptionsSource(&MaterialLayers)
                    .InitiallySelectedItem(DefaultLayer)
                    [
                        SNew(STextBlock)
                        .Text_Lambda([=]()
                        {
                            if(DefaultLayer.IsValid())
                            {
                                return FText::FromString(*DefaultLayer);
                            }
                            return FText::FromString("Select Landscape Material first");
                        })
                    ]
                    .OnSelectionChanged_Lambda([=](TSharedPtr<FString> NewValue, ESelectInfo::Type SelectionType)
                    {
                        DefaultLayer = NewValue;
                        RefreshLayerLandcoverMapping();
                    })
                    .OnGenerateWidget_Lambda([=](TSharedPtr<FString> InOption)
                    {
                        return SNew(STextBlock).Text(FText::FromString(*InOption));
                    })
                    .IsEnabled_Lambda([=]()
                    {
                        return DefaultLayer.IsValid();
                    })
                    .ToolTipText(FText::FromString("Default layer which is used if there is no specific paint layer in the area."))
                ]
                + SVerticalBox::Slot()
                .FillHeight(12)
                .VAlign(VAlign_Center)
                .Padding(5)
                [
                    SNew(SButton)
                    .Text_Raw(this, &FRasterImporterUI::GetImportButtonText)
                    .HAlign(HAlign_Center)
                    .OnClicked_Raw(this, &FRasterImporterUI::ImportRasterFilesClicked)
                    .IsEnabled_Lambda([=]()
                    {
                        return DTMOptions->bRasterSelected;
                    })
                    .ToolTipText(FText::FromString("Create Unreal Landscape(s) with current Settings."))
                ]
            ]
        ]
    ]
    + SVerticalBox::Slot()
    .AutoHeight()
    .Padding(FMargin(0, 10, 0, 0))
    [
        SNew(SSplitter)
        .Orientation(Orient_Horizontal)
        + SSplitter::Slot()
        .Value(LabelColumnWidth)
        [
            SNew(SBorder)
            #if ENGINE_MINOR_VERSION < 1
            .BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
            #else
            .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
            #endif
            .BorderBackgroundColor(FLinearColor::Gray)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .FillHeight(10)
                .VAlign(VAlign_Center)
                .Padding(5)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString("Landscape Material Settings"))
                    .ToolTipText(FText::FromString("Open Landscape Material Dialog."))
                ]
             ]
        ]
        + SSplitter::Slot()
        .Value(1.f - LabelColumnWidth)
        [
            SNew(SBorder)
            #if ENGINE_MINOR_VERSION < 1
            .BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
            #else
            .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
            #endif
            .BorderBackgroundColor(FLinearColor::Gray)
            [
                SNew(SVerticalBox)
                // Paint Layers
                + SVerticalBox::Slot()
                .FillHeight(10)
                .VAlign(VAlign_Center)
                .Padding(5)
                [
                    SNew(SButton)
                    .Text(FText::FromString("Open"))
                    .HAlign(HAlign_Center)
                    .OnClicked_Raw(this, &FRasterImporterUI::OpenMaterialAndWeightmapsWindow)
                    .IsEnabled_Lambda([=]()
                    {
                        return GetGisFileManager()->HasLandscape();
                    })
                    .ToolTipText(FText::FromString("Open Landscape Material Dialog"))
                ]
            ]
        ]
    ]
    + SVerticalBox::Slot()
    .AutoHeight()
    .Padding(FMargin(0, 10, 0, 0))
    [
        SNew(SSplitter)
        .Orientation(Orient_Horizontal)
        + SSplitter::Slot()
        .Value(LabelColumnWidth)
        [
            SNew(SBorder)
            #if ENGINE_MINOR_VERSION < 1
            .BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
            #else
            .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
            #endif
            .BorderBackgroundColor(FLinearColor::Gray)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .FillHeight(10)
                .VAlign(VAlign_Center)
                .Padding(5)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString("Satellite Image as Decal"))
                    .ToolTipText(FText::FromString("Import Satellite image as decal instead as Landscape Material"))
                ]
                + SVerticalBox::Slot()
                .FillHeight(10)
                .VAlign(VAlign_Center)
                .Padding(5)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString("Satellite Image as Vertex Color"))
                    .ToolTipText(FText::FromString("Import Satellite image as Vertex Color instead as Landscape Material\nNOTE: only works for Mesh Tiles, and will fall back to decal on Landscapes"))
                ]
                + SVerticalBox::Slot()
                .FillHeight(10)
                .VAlign(VAlign_Center)
                .Padding(5)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString("Mapbox Zoom Level"))
                ]
                + SVerticalBox::Slot()
                .FillHeight(10)
                .VAlign(VAlign_Center)
                .Padding(5)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString("Satellite"))
                    .ToolTipText(FText::FromString("Add Satellite Image ontop of the Landscape"))
                ]
             ]
        ]
        + SSplitter::Slot()
        .Value(1.f - LabelColumnWidth)
        [
            SNew(SBorder)
            #if ENGINE_MINOR_VERSION < 1
            .BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
            #else
            .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
            #endif
            .BorderBackgroundColor(FLinearColor::Gray)
            [
                SNew(SVerticalBox)
                // choose import as decal or landscape mat
                + SVerticalBox::Slot()
                .FillHeight(10)
                .VAlign(VAlign_Center)
                .Padding(5)
                [
                    SNew(SCheckBox)
                    .Type(ESlateCheckBoxType::CheckBox)
                    .IsChecked_Lambda([=]()
                    {
                        return DTMOptions->ImportOptions.bImportSatImgAsDecal
                                    ? ECheckBoxState::Checked
                                    : ECheckBoxState::Unchecked;
                    })
                    .OnCheckStateChanged_Lambda([=](ECheckBoxState NewState)
                    {
                        DTMOptions->ImportOptions.bImportSatImgAsDecal = (NewState == ECheckBoxState::Checked);
                        if(DTMOptions->ImportOptions.bImportSatImgAsDecal)
                        {
                            DTMOptions->ImportOptions.bImportSatImgAsVertexColor = false;
                        }
                    })
                    .IsEnabled_Lambda([=]()
                    {
                        return GetGisFileManager()->HasLandscape();
                    })
                    .ToolTipText(FText::FromString("Import Satellite image as decal instead as Landscape or Mesh Material"))
                ]
                // choose import as decal or landscape mat
                + SVerticalBox::Slot()
                .FillHeight(10)
                .VAlign(VAlign_Center)
                .Padding(5)
                [
                    SNew(SCheckBox)
                    .Type(ESlateCheckBoxType::CheckBox)
                    .IsChecked_Lambda([=]()
                    {
                        return DTMOptions->ImportOptions.bImportSatImgAsVertexColor
                                    ? ECheckBoxState::Checked
                                    : ECheckBoxState::Unchecked;
                    })
                    .OnCheckStateChanged_Lambda([=](ECheckBoxState NewState)
                    {
                        DTMOptions->ImportOptions.bImportSatImgAsVertexColor = (NewState == ECheckBoxState::Checked);
                        if(DTMOptions->ImportOptions.bImportSatImgAsVertexColor)
                        {
                            DTMOptions->ImportOptions.bImportSatImgAsDecal = false;
                        }
                    })
                    .IsEnabled_Lambda([=]()
                    {
                        return GetGisFileManager()->HasLandscape();
                    })
                    .ToolTipText(FText::FromString("Import Satellite image as Vertex Color instead as Landscape Material\nNOTE: only works for Procedural Mesh Tiles (not Landscapes or Nanite Mesh), and will fall back to decal on Landscapes or Nanite Mesh"))
                ]
                + SVerticalBox::Slot()
                .FillHeight(10)
                .VAlign(VAlign_Center)
                .Padding(5)
                [
                    SNew(SSpinBox<int>)
                    .MinValue(0)
                    .MaxValue(21)
                    .MinSliderValue(0)
                    .MaxSliderValue(21)
                    .MinDesiredWidth(150)
                    .Value_Lambda([=]()
                    {
                        return DTMOptions->ImportOptions.ZoomSatellite;
                    })
                    .OnValueChanged_Lambda([=](int NewValue)
                    {
                        DTMOptions->ImportOptions.ZoomSatellite = NewValue;
                    })
                    .ToolTipText(FText::FromString("Mapbox Zoom Level for Satellite data (only available with Landscaping Mapbox Extension)"))
                    .IsEnabled_Lambda([=]()
                    {
                        return GetGisFileManager()->HasLandscape() && GetGisFileManager()->HasMapboxExtension();
                    })
                ]
                // Satellite import
                + SVerticalBox::Slot()
                .FillHeight(10)
                .VAlign(VAlign_Center)
                .Padding(5)
                [
                    SNew(SButton)
                    .Text(FText::FromString("Add Satellite Image"))
                    .HAlign(HAlign_Center)
                    .OnClicked_Raw(this, &FRasterImporterUI::AddSatelliteImageClicked)
                    .IsEnabled_Lambda([=]()
                    {
                        return GetGisFileManager()->HasLandscape();
                    })
                    .ToolTipText(FText::FromString("Add Satellite Image ontop of the Landscape"))
                ]
            ]
        ]
    ]
    + SVerticalBox::Slot()
    .AutoHeight()
    .Padding(FMargin(0, 10, 0, 0))
    [
        SNew(SSplitter)
        .Orientation(Orient_Horizontal)
        + SSplitter::Slot()
        .Value(LabelColumnWidth)
        [
            SNew(SBorder)
            #if ENGINE_MINOR_VERSION < 1
            .BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
            #else
            .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
            #endif
            .BorderBackgroundColor(FLinearColor::Gray)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .FillHeight(12)
                .VAlign(VAlign_Center)
                .Padding(5)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString("Nanite Mesh"))
                ]
             ]
        ]
        + SSplitter::Slot()
        .Value(1.f - LabelColumnWidth)
        [
            SNew(SBorder)
            #if ENGINE_MINOR_VERSION < 1
            .BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
            #else
            .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
            #endif
            .BorderBackgroundColor(FLinearColor::Gray)
            [
                SNew(SVerticalBox)
                // Replace with Nanite mesh
                + SVerticalBox::Slot()
                .FillHeight(12)
                .VAlign(VAlign_Center)
                .Padding(5)
                [
                    SNew(SButton)
                    .Text(FText::FromString("Replace with Nanite Mesh"))
                    .HAlign(HAlign_Center)
                    .OnClicked_Raw(this, &FRasterImporterUI::SaveAndReplaceLandscapeSMClicked)
                    .IsEnabled_Lambda([=]()
                    {
                        return GetGisFileManager()->HasLandscapeSM();
                    })
                    .ToolTipText(FText::FromString("Save Procedural Mesh Landscapes as Nanite Mesh and replace Procedural Mesh Actor in Level with Nanite Mesh Actor"))
                ]
            ]
        ]
    ];
}

FReply FRasterImporterUI::AddSatelliteImageClicked()
{
    GetGisFileManager()->CreateSatelliteImagery(Material, DTMOptions->ImportOptions);
    return FReply::Handled();
}

FReply FRasterImporterUI::OpenDTMOptionsWindow()
{
    TSharedRef<SWindow> AreaWindow = DTMOptions->GetDTMOptionsWindow();
    
    TSharedPtr<SWindow> TopWindow = FSlateApplication::Get().GetActiveTopLevelWindow();
	if(TopWindow.IsValid())
	{
		//Add as Native
		FSlateApplication::Get().AddWindowAsNativeChild(AreaWindow, TopWindow.ToSharedRef(), true);
	}
	else
	{
		//Default in case no top window
		FSlateApplication::Get().AddWindow(AreaWindow);
	}
    CachedDTMOptionsWindow = &AreaWindow.Get();
    return FReply::Handled();
}

FReply FRasterImporterUI::OpenMaterialAndWeightmapsWindow()
{
    TSharedRef<SWindow> WeightmapWindow = SNew(SWindow)
    .Type(EWindowType::Normal)
    .Title(FText::FromString("Landscaping - Landscape Material and Weightmaps"))
    .SupportsMaximize(false)
    .ClientSize(FVector2D(750, 600))
    [
        SNew(SBox)
        .Padding(FMargin(15, 10, 15, 0))
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .HAlign(HAlign_Left)
                .FillWidth(8)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString("Landscaping - Landscape Material and Weightmaps"))
                    #if ENGINE_MINOR_VERSION < 1
                    .TextStyle(FEditorStyle::Get(), "LargeText")
                    #else
                    .TextStyle(FAppStyle::Get(), "LargeText")
                    #endif
                ]
                + SHorizontalBox::Slot()
                .HAlign(HAlign_Right)
                .FillWidth(2)
                [
                    SNew(SHyperlink)
                    .Text(FText::FromString("Documentation"))
                    .OnNavigate_Lambda([=]()
                    {
                        FPlatformProcess::LaunchURL(TEXT("https://jorop.github.io/landscaping-docs/#/landcover?id=landcover"), nullptr, nullptr);
                    })
                    .ToolTipText(FText::FromString("Documentation on Landscape Material and Weightmaps import options."))
                ]
            ]
            + SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(SSeparator)
                #if ENGINE_MINOR_VERSION < 1
                .SeparatorImage(FEditorStyle::GetBrush("Menu.Separator"))
                #else
                .SeparatorImage(FAppStyle::GetBrush("Menu.Separator"))
                #endif
                .Orientation(Orient_Horizontal)
            ]
            + SVerticalBox::Slot()
            [
                SNew(SScrollBox)
                + SScrollBox::Slot()
                .Padding(0.6f)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    [
                        SNew(SSplitter)
                        .Orientation(Orient_Horizontal)
                        + SSplitter::Slot()
                        .Value(LabelColumnWidth)
                        [
                            SNew(SBorder)
                            #if ENGINE_MINOR_VERSION < 1
                            .BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
                            #else
                            .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
                            #endif
                            .BorderBackgroundColor(FLinearColor::Gray)
                            [
                                SNew(SVerticalBox)
                                + SVerticalBox::Slot()
                                .FillHeight(5)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(STextBlock)
                                    .Text(FText::FromString("Select Vector Files"))
                                    .ToolTipText(FText::FromString("Select one or multiple Vector files to generate weightmaps."))
                                ]
                                + SVerticalBox::Slot()
                                .FillHeight(10)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(STextBlock)
                                    .Text(FText::FromString("Landscape Material"))
                                    .ToolTipText(FText::FromString("The Landscape Material to use for the (tiled) Landscape."))
                                ]
                                + SVerticalBox::Slot()
                                .FillHeight(5)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(STextBlock)
                                    .Text(FText::FromString("Default Layer"))
                                    .ToolTipText(FText::FromString("Default layer which is used if there is no specific paint layer in the area."))
                                ]
                                + SVerticalBox::Slot()
                                .FillHeight(5)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(STextBlock)
                                    .Text_Lambda([=]()
                                    {
                                        return GetGisFileManager()->HasLandscape() ? FText::FromString("Replace Weightmaps") : FText::FromString("Close Dialog");
                                    })
                                    .ToolTipText(FText::FromString("Replace weightmaps of existing (tiled) Landscape."))
                                ]
                            ]
                        ]
                        + SSplitter::Slot()
                        .Value(1.f - LabelColumnWidth)
                        [
                            SNew(SBorder)
                            #if ENGINE_MINOR_VERSION < 1
                            .BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
                            #else
                            .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
                            #endif
                            .BorderBackgroundColor(FLinearColor::Gray)
                            [
                                SNew(SVerticalBox)
                                // Layers
                                + SVerticalBox::Slot()
                                .FillHeight(10)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(SButton)
                                    .Text(FText::FromString("Select"))
                                    .HAlign(HAlign_Center)
                                    .OnClicked_Raw(this, &FRasterImporterUI::SelectWeightmapsClicked)
                                    .ToolTipText(FText::FromString("Select one or multiple Vector files to generate the weightmaps."))
                                ]
                                + SVerticalBox::Slot()
                                .FillHeight(35)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(SObjectPropertyEntryBox)
                                    .AllowedClass(UMaterialInterface::StaticClass())
                                    .ObjectPath_Lambda([=]()
                                    {
                                        return Material != nullptr
                                                ? Material->GetPathName()
                                                : FString();
                                    })
                                    .OnObjectChanged_Lambda([=](FAssetData Asset)
                                    {
                                        UObject* NewMaterial = Asset.GetAsset();
                                        Material = (UMaterialInterface*)NewMaterial;
                                        UpdateLayerList();
                                        RefreshLayerLandcoverMapping();
                                    })
                                    .ThumbnailPool(ThumbnailPool)
                                    .ToolTipText(FText::FromString("The Landscape Material to use for the (tiled) Landscape."))
                                ]
                                + SVerticalBox::Slot()
                                .FillHeight(10)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(SComboBox<TSharedPtr<FString>>)
                                    .OptionsSource(&MaterialLayers)
                                    .InitiallySelectedItem(DefaultLayer)
                                    [
                                        SNew(STextBlock)
                                        .Text_Lambda([=]()
                                        {
                                            if(DefaultLayer.IsValid())
                                            {
                                                return FText::FromString(*DefaultLayer);
                                            }
                                            return FText::FromString("Select Landscape Material first");
                                        })
                                    ]
                                    .OnSelectionChanged_Lambda([=](TSharedPtr<FString> NewValue, ESelectInfo::Type SelectionType)
                                    {
                                        DefaultLayer = NewValue;
                                        RefreshLayerLandcoverMapping();
                                    })
                                    .OnGenerateWidget_Lambda([=](TSharedPtr<FString> InOption)
                                    {
                                        return SNew(STextBlock).Text(FText::FromString(*InOption));
                                    })
                                    .IsEnabled_Lambda([=]()
                                    {
                                        return DefaultLayer.IsValid();
                                    })
                                    .ToolTipText(FText::FromString("Default layer which is used if there is no specific paint layer in the area."))
                                ]
                                + SVerticalBox::Slot()
                                .FillHeight(10)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(SButton)
                                    .Text_Lambda([=]()
                                    {
                                        return GetGisFileManager()->HasLandscape() ? FText::FromString("Replace") : FText::FromString("Close");
                                    })
                                    .HAlign(HAlign_Center)
                                    .OnClicked_Raw(this, &FRasterImporterUI::UpdateWeightmapsClicked)
                                    .ToolTipText(FText::FromString("Generate new or replace weightmaps of existing (tiled) Landscape."))
                                    .IsEnabled_Lambda([=]()
                                    {
                                        return DefaultLayer.IsValid();
                                    })
                                ]
                            ]
                        ]
                    ]
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    [
                        SNew(SSeparator)
                        #if ENGINE_MINOR_VERSION < 1
                        .SeparatorImage(FEditorStyle::GetBrush("Menu.Separator"))
                        #else
                        .SeparatorImage(FAppStyle::GetBrush("Menu.Separator"))
                        #endif
                        .Orientation(Orient_Horizontal)
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(70)                    
                    [
                        SNew(SSplitter)
                        .Orientation(Orient_Horizontal)
                        + SSplitter::Slot()
                        .OnSlotResized(SSplitter::FOnSlotResized::CreateLambda([this] (float InWidth) { MaterialLayerColumnWidth = InWidth; }))
                        .Value(MaterialLayerColumnWidth)
                        [
                            SNew(SVerticalBox)
                            + SVerticalBox::Slot()
                            .FillHeight(20)
                            .VAlign(VAlign_Center)
                            .Padding(5)
                            [
                                SNew(STextBlock)
                                .Text(FText::FromString("Landuse Types ->"))
                            ]
                        ]
                        + SSplitter::Slot()
                        .Value(1 - MaterialLayerColumnWidth)
                        [
                            SAssignNew(DummyLanduseListView, STileView<TSharedPtr<Landuse>>)
                            .ListItemsSource(&DummyLanduseList)
                            .ItemHeight(128)
		                    .ItemWidth(25)
                            .Orientation(EOrientation::Orient_Horizontal)
                            .OnGenerateTile_Raw(this, &FRasterImporterUI::OnGenerateWidgetForLanduseNamesListView)
                            .SelectionMode(ESelectionMode::None)
                        ]
                    ]
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    [
                        SNew(SSeparator)
                        #if ENGINE_MINOR_VERSION < 1
                        .SeparatorImage(FEditorStyle::GetBrush("Menu.Separator"))
                        #else
                        .SeparatorImage(FAppStyle::GetBrush("Menu.Separator"))
                        #endif
                        .Orientation(Orient_Horizontal)
                    ]
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    [
                        SAssignNew(LayerDataListView, SListView<TSharedPtr<MaterialLayerSettings>>)
                        .ListItemsSource(&LayerDataList)
                        .OnGenerateRow_Raw(this, &FRasterImporterUI::OnGenerateWidgetForLayerDataListView)
                        .SelectionMode(ESelectionMode::None)
                    ]
                ]
            ]
        ]
    ];
    TSharedPtr<SWindow> TopWindow = FSlateApplication::Get().GetActiveTopLevelWindow();
	if(TopWindow.IsValid())
	{
		//Add as Native
		FSlateApplication::Get().AddWindowAsNativeChild(WeightmapWindow, TopWindow.ToSharedRef(), true);
	}
	else
	{
		//Default in case no top window
		FSlateApplication::Get().AddWindow(WeightmapWindow);
	}
    CachedWeightmapWindow = &WeightmapWindow.Get();
    return FReply::Handled();
}


TSharedRef<ITableRow> FRasterImporterUI::OnGenerateWidgetForLanduseNamesListView(
    TSharedPtr<Landuse> InLanduseData,
    const TSharedRef<STableViewBase>& OwnerTable)
{
    return SNew(STableRow<TSharedRef<Landuse>>, OwnerTable)
    [
        SNew(SVerticalBox)
        // Landusenames for weightmaps
        + SVerticalBox::Slot()
        .Padding(2)
        .FillHeight(128)
        [
            SNew(STextBlock)
            .Text(FText::FromString(InLanduseData->LanduseType))
            .Clipping(EWidgetClipping::Inherit)
            .RenderTransform_Lambda([=]()
            {
                return TransformCast<FSlateRenderTransform>(Concatenate(FVector2D(-118, 0), FQuat2D(FMath::DegreesToRadians(-90.0f))));
            })
        ]
    ];
}

TSharedRef<ITableRow> FRasterImporterUI::OnGenerateWidgetForLanduseListView(
    TSharedPtr<Landuse> InLanduseData,
    const TSharedRef<STableViewBase>& OwnerTable)
{
    return SNew(STableRow<TSharedRef<Landuse>>, OwnerTable)
    [
        SNew(SVerticalBox)
        // Landuse for weightmaps
        + SVerticalBox::Slot()
        .VAlign(VAlign_Center)
        .HAlign(HAlign_Center)
        .Padding(3)
        [
            SNew(SCheckBox)
            .Type(ESlateCheckBoxType::CheckBox)
            .IsChecked_Lambda([=]()
            {
                return InLanduseData->bActive
                            ? ECheckBoxState::Checked
                            : ECheckBoxState::Unchecked;
            })
            .OnCheckStateChanged_Lambda([=](ECheckBoxState NewState)
            {
                InLanduseData->bActive = (NewState == ECheckBoxState::Checked);
                RefreshLayerLandcoverMapping();
            })
            .ToolTipText(FText::FromString(InLanduseData->LanduseType))
            .IsEnabled_Lambda([=]()
            {
                return InLanduseData->bIsInteractable;
            })
        ]
    ];
}

TSharedRef<ITableRow> FRasterImporterUI::OnGenerateWidgetForLayerDataListView(
    TSharedPtr<MaterialLayerSettings> InLayerData,
    const TSharedRef<STableViewBase>& OwnerTable)
{
    return SNew(STableRow<TSharedRef<MaterialLayerSettings>>, OwnerTable)
    [
        SNew(SBorder)
        [
            SNew(SSplitter)
            .Orientation(Orient_Horizontal)
            + SSplitter::Slot()
            .OnSlotResized(SSplitter::FOnSlotResized::CreateLambda([this] (float InWidth) { MaterialLayerColumnWidth = InWidth; }))
            .Value(MaterialLayerColumnWidth)
            [
                SNew(SHorizontalBox)
                //Blend option
                + SHorizontalBox::Slot()
                .VAlign(VAlign_Center)
                .HAlign(HAlign_Center)
                .Padding(2)
                .AutoWidth()
                [
                    SNew(SCheckBox)
                    .Type(ESlateCheckBoxType::CheckBox)
                    .IsChecked_Lambda([=]()
                    {
                        return InLayerData->bWeightBlended
                                    ? ECheckBoxState::Checked
                                    : ECheckBoxState::Unchecked;
                    })
                    .OnCheckStateChanged_Lambda([=](ECheckBoxState NewState)
                    {
                        InLayerData->bWeightBlended = (NewState == ECheckBoxState::Checked);
                    })
                    .ToolTipText(FText::FromString("Weight-Blended Layer"))
                    .IsEnabled_Lambda([=]()
                    {
                        return false; // DefaultLayer.IsValid() && !InLayerData->Name.ToString().Equals(*DefaultLayer.Get());
                    })
                ]
                // Layer name
                + SHorizontalBox::Slot()
                .VAlign(VAlign_Center)
                .HAlign(HAlign_Left)
                .FillWidth(0.35)
                .MaxWidth(100)
                .Padding(5, 0, 0, 0)
                [
                    SNew(STextBlock)
                    .Text(FText::FromName(InLayerData->Name))
                    .ToolTipText(FText::FromName(InLayerData->Name))
                ]
                + SHorizontalBox::Slot()
                .VAlign(VAlign_Center)
                .HAlign(HAlign_Left)
                .AutoWidth()
                .Padding(5, 0, 0, 0)
                [
                    // Noise Texture
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot()
                    .VAlign(VAlign_Center)
                    .HAlign(HAlign_Left)
                    .FillWidth(0.35)
                    .Padding(5, 0, 0, 0)
                    [
                        SNew(SObjectPropertyEntryBox)
                        .AllowedClass(UTexture2D::StaticClass())
                        .ObjectPath_Lambda([=]()
                        {
                            return InLayerData->NoiseTexture != nullptr
                                    ? InLayerData->NoiseTexture->GetPathName()
                                    : FString();
                        })
                        .OnObjectChanged_Lambda([=](FAssetData Asset)
                        {
                            UObject* NewTexture = Asset.GetAsset();
                            InLayerData->NoiseTexture = (UTexture2D*)NewTexture;
                        })
                        .IsEnabled_Lambda([=]()
                        {
                            return DefaultLayer.IsValid() && !InLayerData->Name.ToString().Equals(*DefaultLayer.Get()) && InLayerData->bWeightBlended;
                        })
                        .ThumbnailPool(ThumbnailPool)
                        .ToolTipText(FText::FromString("Noise Texture to to create variation.\nThe selected color channel will be blended with the default layer."))
                    ]
                    // Color Channel
                    + SHorizontalBox::Slot()
                    .VAlign(VAlign_Center)
                    .HAlign(HAlign_Left)
                    .FillWidth(0.2)
                    .Padding(5, 0, 0, 0)
                    [
                        SNew(SComboBox<TSharedPtr<FString>>)
                        .OptionsSource(&ColorChannels)
                        .InitiallySelectedItem(InLayerData->ColorChannel)
                        [
                            SNew(STextBlock)
                            .Text_Lambda([=]()
                            {
                                if(InLayerData->ColorChannel.IsValid())
                                {
                                    return FText::FromString(*InLayerData->ColorChannel);
                                }
                                return FText::FromString("-");
                            })
                        ]
                        .OnSelectionChanged_Lambda([=](TSharedPtr<FString> NewValue, ESelectInfo::Type SelectionType)
                        {
                            InLayerData->ColorChannel = NewValue;
                            RefreshLayerLandcoverMapping();
                        })
                        .OnGenerateWidget_Lambda([=](TSharedPtr<FString> InOption)
                        {
                            return SNew(STextBlock).Text(FText::FromString(*InOption));
                        })
                        .IsEnabled_Lambda([=]()
                        {
                            return InLayerData->NoiseTexture != nullptr;
                        })
                        .ToolTipText(FText::FromString("Which color channel of the noise texture will be used."))
                    ]
                    // min weight
                    + SHorizontalBox::Slot()
                    .VAlign(VAlign_Center)
                    .HAlign(HAlign_Left)
                    .FillWidth(0.15)
                    .Padding(5, 0, 0, 0)
                    [
                        SNew(SSpinBox<uint8>)
                        .MinValue(0)
                        .MaxValue(255)
                        .MinSliderValue(0)
                        .MaxSliderValue(255)
                        .Delta(1)
                        .Value_Lambda([=]()
                        {
                            return InLayerData->MinWeight;
                        })
                        .OnValueChanged_Lambda([=](uint8 NewValue)
                        {
                            InLayerData->MinWeight = NewValue;
                        })
                        .IsEnabled_Lambda([=]()
                        {
                            return InLayerData->NoiseTexture != nullptr;
                        })
                        .ToolTipText(FText::FromString("Minimum weight of the painted layer when using noise textures.\nIf not using noise textures, this will be 255 on all covered areas.\n\nNOTE: if this value is 0 and the noise texture has 0 value at the according area,\nthe default layer will be painted there."))
                    ]
                    // Noise Texture Tiling
                    + SHorizontalBox::Slot()
                    .VAlign(VAlign_Center)
                    .HAlign(HAlign_Left)
                    .FillWidth(0.15)
                    .Padding(5, 0, 0, 0)
                    [
                        SNew(SSpinBox<int>)
                        .MinValue(1)
                        .MaxValue(16)
                        .MinSliderValue(1)
                        .MaxSliderValue(16)
                        .Delta(1)
                        .Value_Lambda([=]()
                        {
                            return InLayerData->Tiling;
                        })
                        .OnValueChanged_Lambda([=](int NewValue)
                        {
                            InLayerData->Tiling = NewValue;
                        })
                        .IsEnabled_Lambda([=]()
                        {
                            return InLayerData->NoiseTexture != nullptr;
                        })
                        .ToolTipText(FText::FromString("Tiling of the noise texture."))
                    ]
                ]
            ]
            + SSplitter::Slot()
            .Value(1 - MaterialLayerColumnWidth)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                [
                    SAssignNew(InLayerData->LanduseListView, SListView<TSharedPtr<Landuse>>)
                    .ListItemsSource(&InLayerData->LanduseList)
                    .Orientation(EOrientation::Orient_Horizontal)
                    .OnGenerateRow_Raw(this, &FRasterImporterUI::OnGenerateWidgetForLanduseListView)
                    .SelectionMode(ESelectionMode::None)
                ]
            ]
        ]
    ];
}

FReply FRasterImporterUI::ImportRasterFilesClicked()
{
    DTMOptions->ImportOptions.DefaultLayer = DefaultLayer.IsValid() ? *DefaultLayer.Get() : "";
    if(GetGisFileManager()->GetIsWorldPartition())
    {
        DTMOptions->ImportOptions.WorldPartitionGridSize = GetGisFileManager()->GetInfos()->WorldPartitionGridSize > 0 ? GetGisFileManager()->GetInfos()->WorldPartitionGridSize : DTMOptions->ImportOptions.WorldPartitionGridSize;
        GetGisFileManager()->GetInfos()->WorldPartitionGridSize = DTMOptions->ImportOptions.WorldPartitionGridSize;
    }
    DTMOptions->ImportOptions.DesiredMaxTileSize = GetGisFileManager()->GetInfos()->DesiredMaxTileSize > 0 ? GetGisFileManager()->GetInfos()->DesiredMaxTileSize : DTMOptions->ImportOptions.DesiredMaxTileSize;
    GetGisFileManager()->GetInfos()->DesiredMaxTileSize = DTMOptions->ImportOptions.DesiredMaxTileSize;
    if(GetGisFileManager()->HasMapboxExtension() && !GetGisFileManager()->HasRasterData() && DTMOptions->ImportOptions.SmoothSteps == 0)
    {
        EAppReturnType::Type Answer = FMessageDialog::Open(
            EAppMsgType::YesNo, 
            FText::FromString("Smooth Landscape on import?\n\nSmooth steps is now set to 0.\nFor Mapbox imports 1 smooth step is recommended.")
        );
        if(Answer == EAppReturnType::Yes)
        {
            DTMOptions->ImportOptions.SmoothSteps = 1;
        }
    }
    MapboxFinishedDelegateHandle.Clear();
    MapboxFinishedDelegateHandle.AddLambda([=](FDataLoadResult &InResult)
    {
        DTMOptions->bRasterSelected = true;
        UpdateAreaLabel(true);
        DTMOptions->bRasterSelected = false;
        DTMOptions->ImportOptions.bMapboxImport = false;
    });
    GetGisFileManager()->CreateTerrains(Material, LayerDataList, DTMOptions->ImportOptions, MapboxFinishedDelegateHandle);
    DTMOptions->bRasterSelected = false;
    return FReply::Handled();
}

FReply FRasterImporterUI::SaveAndReplaceLandscapeSMClicked()
{
    GetGisFileManager()->GetInfos()->SaveStaticMeshAndReplace();
    return FReply::Handled();
}

FReply FRasterImporterUI::UpdateWeightmapsClicked()
{
    if (GisFileManager == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("Landscaping: Initialization failed. Close Landscaping window and open it again"));
        FMessageDialog::Open(
            EAppMsgType::Ok, 
            FText::FromString("Initialization failed. Please close Landscaping tab and open it again!")
        );
        return FReply::Handled();
    }
    if(GetGisFileManager()->HasLandscape())
    {
        FString DefaultLayerName = DefaultLayer.IsValid() ? *DefaultLayer.Get() : "";
        GetGisFileManager()->CreateWeightmaps(Material, LayerDataList, DefaultLayerName);
    }
    CachedWeightmapWindow->RequestDestroyWindow();
    
    return FReply::Handled();
}

FReply FRasterImporterUI::SelectRasterFilesClicked()
{
    TArray<FString> OutputFiles;
    const FString FileTypes = FString("DTM |").Append(GetGisFileManager()->GetAllowedDTMFileTypes());
    FDataLoadResult ResultInfo = GetGisFileManager()->OpenFileDialog(FString("Open Digital Terrain Model Files"), FString(""), FileTypes, OutputFiles, MapboxFinishedDelegateHandle);
    if(ResultInfo.ErrorMsg.IsEmpty())
    {
        DTMOptions->RasterFileImportInfo = MakeShareable(new FString(ResultInfo.StatusMsg));
        DTMOptions->CRSInfo = MakeShareable(new FString(GetGisFileManager()->GetCRS()->GetAuthorityID() == 0 ? FString("CRS not set") : GetGisFileManager()->GetCRS()->GetAuthorityIDStr()));
        DTMOptions->bRasterSelected = true;
        UpdateAreaLabel(true);
    }
    else 
    {
        FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(ResultInfo.ErrorMsg));
        UE_LOG(LogTemp, Error, TEXT("Landscaping: %s"), *ResultInfo.ErrorMsg);
    }
    return FReply::Handled();
}

FReply FRasterImporterUI::SelectWeightmapsClicked()
{
    MapboxFinishedDelegateHandle.Clear();
    MapboxFinishedDelegateHandle.AddLambda([=](FDataLoadResult &ResultInfo)
    {
        if(ResultInfo.ErrorMsg.IsEmpty())
        {
            AvailableLandcoverTypes = ResultInfo.VectorFeatureClasses;
            DummyLanduseList.Empty();
            UpdateLayerList();
            RefreshLayerLandcoverMapping();
        }
        else
        {
            FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString::Printf(TEXT("%s\nConsider a lower zoom level"), *ResultInfo.ErrorMsg)));
        }
    });

    TArray<FString> OutputFiles;
    FDataLoadResult ResultInfo = GetGisFileManager()->OpenFileDialog(FString("Open Landuse Files for Weightmaps"), FString(""), LANDUSE_FILE, OutputFiles, MapboxFinishedDelegateHandle);
    
    if (ResultInfo.StatusMsg.Contains("Mapbox"))
    {
        return FReply::Handled();
    }

    if(ResultInfo.ErrorMsg.IsEmpty())
    {
        AvailableLandcoverTypes = ResultInfo.VectorFeatureClasses;
        DummyLanduseList.Empty();
        UpdateLayerList();
        RefreshLayerLandcoverMapping();
        return FReply::Handled();
    } 

    AvailableLandcoverTypes = TArray<FString>();
    FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(ResultInfo.ErrorMsg));
    return FReply::Handled();
}

void FRasterImporterUI::RefreshLayerLandcoverMapping()
{
    UpdateLandcoverTypes();
    UpdateOnLanduseSelectionChanged();
    if(DummyLanduseListView.IsValid())
    {
        DummyLanduseListView.Get()->RequestListRefresh();
    }
    if(LayerDataListView.IsValid())
    {
        LayerDataListView.Get()->RequestListRefresh();
    }
}

// this does reset all layer/landuse mappings
void FRasterImporterUI::UpdateLayerList()
{
    TArray<FName> LayerNamesList = ALandscapeProxy::GetLayersFromMaterial(Material);
    if(LayerDataList.Num() > 0)
    {
        LayerDataList.Empty();
    }
    if(MaterialLayers.Num() > 0)
    {
        MaterialLayers.Empty();
    }
    for (const FName LayerName : LayerNamesList)
    {
        // List view data source
        TSharedPtr<MaterialLayerSettings> LayerData = MakeShareable(new MaterialLayerSettings());
        LayerData->Name = LayerName;
        LayerDataList.Add(LayerData);
        MaterialLayers.Add(MakeShareable(new FString(LayerName.ToString())));
    }
    if(MaterialLayers.Num() > 0)
    {
        DefaultLayer = MaterialLayers[0];
        GetGisFileManager()->MaterialLayers = MaterialLayers;
    }
}

void FRasterImporterUI::UpdateLandcoverTypes()
{
    for(FString Landcover : AvailableLandcoverTypes)
    {
        if(Landcover.IsEmpty())
        {
            continue;
        }
        bool bAddedToLanduseList = false;
        for(TSharedPtr<Landuse> DummyLanduse : DummyLanduseList)
        {
            bAddedToLanduseList = DummyLanduse.Get()->LanduseType.Equals(Landcover);
            if(bAddedToLanduseList)
            {
                break;
            }
        }
        if(!bAddedToLanduseList)
        {
            TSharedPtr<Landuse> NewLanduse = MakeShared<Landuse>();
            NewLanduse.Get()->LanduseType = Landcover;
            DummyLanduseList.Add(NewLanduse);
        }
    }
    for(TSharedPtr<MaterialLayerSettings> Settings : LayerDataList)
    {
        FString DefaultLayerName = DefaultLayer.IsValid() ? *DefaultLayer.Get() : "";
        if(Settings.Get()->Name.ToString().Equals(DefaultLayerName))
        {
            Settings.Get()->bWeightBlended = true;
        }
        else
        {
            Settings.Get()->bWeightBlended = false;
        }
        for(FString Landcover : AvailableLandcoverTypes)
        {
            bool bAddedToLanduseList = false;
            for(TSharedPtr<Landuse> LanduseItem : Settings.Get()->LanduseList)
            {
                bAddedToLanduseList = LanduseItem.Get()->LanduseType.Equals(Landcover);
                if(bAddedToLanduseList)
                {
                    break;
                }
            }
            if(!bAddedToLanduseList)
            {
                TSharedPtr<Landuse> NewLanduse = MakeShared<Landuse>();
                NewLanduse.Get()->bActive = Settings.Get()->Name.ToString().ToLower().Equals(Landcover.ToLower());
                if(NewLanduse.Get()->bActive)
                {
                    Settings.Get()->bWeightBlended = true;
                }
                NewLanduse.Get()->LanduseType = Landcover;
                Settings.Get()->LanduseList.Add(NewLanduse);
            }
        }
        
    }
    if(LayerDataList.Num() > 0 && LayerDataList[0].Get()->LanduseListView.IsValid())
    {
        LayerDataList[0].Get()->LanduseListView.Get()->RequestListRefresh();
    }
}

void FRasterImporterUI::UpdateOnLanduseSelectionChanged()
{
    TMap<FString, bool> HasInteractable;
    for(TSharedPtr<MaterialLayerSettings> Settings : LayerDataList)
    {
        for(TSharedPtr<Landuse> CurrentLanduse : Settings.Get()->LanduseList)
        {
            if(CurrentLanduse.Get()->bActive)
            {
                HasInteractable.Add(CurrentLanduse.Get()->LanduseType, true);
            }
        }
    }
    for(int LayerIndex = 0; LayerIndex < LayerDataList.Num(); LayerIndex++)
    {
        for(int LanduseIndex = 0; LanduseIndex < LayerDataList[LayerIndex].Get()->LanduseList.Num(); LanduseIndex++)
        {
            FString LanduseItem = LayerDataList[LayerIndex].Get()->LanduseList[LanduseIndex].Get()->LanduseType;
            if(HasInteractable.Contains(LanduseItem) && !LayerDataList[LayerIndex].Get()->LanduseList[LanduseIndex].Get()->bActive)
            {
                LayerDataList[LayerIndex].Get()->LanduseList[LanduseIndex].Get()->bIsInteractable = false;
            }
            else
            {
                LayerDataList[LayerIndex].Get()->LanduseList[LanduseIndex].Get()->bIsInteractable = true;
            }
            if(LayerDataList[LayerIndex].Get()->LanduseList[LanduseIndex].Get()->bActive)
            {
                LayerDataList[LayerIndex].Get()->bWeightBlended = true;
            }
        }
    }
    GetGisFileManager()->GetInfos()->SetLayerDataList(LayerDataList);
}

FText FRasterImporterUI::GetImportInfoRasterFile() const
{
    return FText::FromString(*DTMOptions->RasterFileImportInfo);
}

FText FRasterImporterUI::GetImportButtonText() const
{
    FString ButtonText = FString("Select file(s) first");
    if(DTMOptions != nullptr)
    {
        ButtonText = FString::Printf(TEXT("%s%s"), (DTMOptions->bRasterSelected ? *FString("Import") : *FString("Select file(s) first")), (DTMOptions->ImportOptions.bMapboxImport ? *FString(" from Mapbox") : *FString("")));
    }
    return FText::FromString(*ButtonText);
}

UGISFileManager* FRasterImporterUI::GetGisFileManager()
{
    if(GisFileManager == nullptr)
    {
        GisFileManager = GEditor->GetEditorSubsystem<UGISFileManager>();
    }
    return GisFileManager;
}