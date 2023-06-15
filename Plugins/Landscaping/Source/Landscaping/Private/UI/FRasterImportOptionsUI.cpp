// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#include "FRasterImportOptionsUI.h"
#include "WebBrowserModule.h"
#include "GenericPlatform/GenericPlatformMemory.h"

FRasterImportOptionsUI::FRasterImportOptionsUI(UGISFileManager* InGisFileManager, float InLabelColumnWidth)
{
    GisFileManager = InGisFileManager;
    LabelColumnWidth = InLabelColumnWidth;
    if (!BrowserSettings)
	{
		IWebBrowserSingleton* WebBrowserSingleton = IWebBrowserModule::Get().GetSingleton();

		BrowserSettings = MakeShared<FBrowserContextSettings>(TEXT("LandscapingWebBrowser"));
		BrowserSettings->bPersistSessionCookies = true;
		BrowserSettings->bEnableNetSecurityExpiration = false;

		FString CachePath(FPaths::Combine(WebBrowserSingleton->ApplicationCacheDir(), TEXT("cache")));
		CachePath = FPaths::ConvertRelativePathToFull(CachePath);
		BrowserSettings->CookieStorageLocation = CachePath;
		
		WebBrowserSingleton->RegisterContext(*BrowserSettings);
	}
}

TSharedRef<SWindow> FRasterImportOptionsUI::GetDTMOptionsWindow()
{
    UGISFileManager* GisFM = GetGisFileManager();
    bWarningShown = false;
    if(GisFM != nullptr)
    {
        ImportBBox = GisFM->GetInfos()->GetCroppedExtents().ToString();
        CRSInfo = MakeShareable(new FString(GisFM->GetCRS()->GetAuthorityID() == 0 ? FString("CRS not set") : GisFM->GetCRS()->GetAuthorityIDStr()));
    }
    else 
    {
        return SNew(SWindow)
        .Type(EWindowType::Normal)
        .Title(FText::FromString("Landscaping - Options for DTM Import"))
        .SupportsMaximize(false)
        .ClientSize(FVector2D(550, 630))
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
                        .Text(FText::FromString("Landscaping - Options for DTM Import"))
                        #if ENGINE_MINOR_VERSION < 1
                        .TextStyle(FEditorStyle::Get(), "LargeText")
                        #else
                        .TextStyle(FAppStyle::Get(), "LargeText")
                        #endif
                    ]
                ]
            ]
        ];
    }
    return SNew(SWindow)
    .Type(EWindowType::Normal)
    .Title(FText::FromString("Landscaping - Options for DTM Import"))
    .SupportsMaximize(false)
    .ClientSize(FVector2D(550, 630))
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
                    .Text(FText::FromString("Landscaping - Options for DTM Import"))
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
                        FPlatformProcess::LaunchURL(TEXT("https://jorop.github.io/landscaping-docs/#/heights?id=options"), nullptr, nullptr);
                    })
                    .ToolTipText(FText::FromString("Documentation on DTM Import Options"))
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
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        .Padding(0, 5)
                        .HAlign(HAlign_Left)
                        .FillWidth(2)
                        [
                            SNew(SHyperlink)
                            .Text_Raw(this, &FRasterImportOptionsUI::GetCRSInfo)
                            .OnNavigate_Raw(this, &FRasterImportOptionsUI::ShowCRS)
                            .ToolTipText(FText::FromString("Current Level's CRS\nThe CRS can be changed in 'Project Settings -> Plugins -> Landscaping'\n\nThe link opens CRS description on epsg.io (external link)"))
                        ]
                        + SHorizontalBox::Slot()
                        .Padding(5, 5)
                        .HAlign(HAlign_Left)
                        .FillWidth(5)
                        [
                            SNew(STextBlock)
                            .Text_Raw(this, &FRasterImportOptionsUI::GetImportInfo)
                            .WrapTextAt(600.0f)
                        ]
                        + SHorizontalBox::Slot()
                        .Padding(0, 5)
                        .HAlign(HAlign_Right)
                        .FillWidth(2)
                        [
                            SNew(SHyperlink)
                            .Text(FText::FromString("Show Map"))
                            .OnNavigate_Raw(this, &FRasterImportOptionsUI::ShowMap)
                            .ToolTipText(FText::FromString("OPTIONAL: Show Area on Map (external link)"))
                        ]
                    ]
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    [
                        SAssignNew(BrowserContainer, SOverlay)
                        + SOverlay::Slot()
                        [
                            SAssignNew(BrowserView, SWebBrowserView)
                            .ShowErrorMessage(true)
                            .SupportsTransparency(true)
                            .PopupMenuMethod(EPopupMethod::UseCurrentWindow)
                            .InitialURL(FString::Printf(TEXT("https://maps.ludicdrive.com/#%s"), *ImportBBox))
                            .OnUrlChanged_Raw(this, &FRasterImportOptionsUI::HandleBrowserUrlChanged)
                            .ContextSettings(*BrowserSettings)
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
                                .FillHeight(5)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(STextBlock)
                                    .Text(FText::FromString("Corners as Bounding Box"))
                                    .ToolTipText(FText::FromString("OPTIONAL: Paste coordinates from https://maps.ludicdrive.com.\nYou can also just select the area in the map above."))
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
                                // Crop area
                                 + SVerticalBox::Slot()
                                .FillHeight(10)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(SEditableTextBox)
                                    .Text_Raw(this, &FRasterImportOptionsUI::GetImportBBox)
				                    .OnTextCommitted_Lambda([=](const FText& NewText, ETextCommit::Type InTextCommit)
                                    {
                                        ImportBBox = *NewText.ToString();
                                        FString AreaStr = CalculateArea();
                                        RasterFileImportInfo = MakeShareable(new FString(AreaStr));
                                        ShowWarning();
                                        BrowserView->LoadURL(FString::Printf(TEXT("https://maps.ludicdrive.com/#%s"), *ImportBBox));
                                    })
                                    .ToolTipText(FText::FromString("OPTIONAL: Paste coordinates from https://maps.ludicdrive.com. (coordinates after the #)\nYou can also just select the area in the map above."))
                                    .IsEnabled_Lambda([=]()
                                    {
                                        return !GetGisFileManager()->HasLandscape() || GetGisFileManager()->GetIsWorldPartition();
                                    })
                                ]
                            ]
                        ]
                    ]
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Override Z Scale of generated Landscape"))
                        #if ENGINE_MINOR_VERSION < 1
                        .TextStyle(FEditorStyle::Get(), "LargeText")
                        #else
                        .TextStyle(FAppStyle::Get(), "LargeText")
                        #endif
                    ]
                     + SVerticalBox::Slot()
                    .FillHeight(32)
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
                                    .Text(FText::FromString("High detail Z scale"))
                                ]
                                + SVerticalBox::Slot()
                                .FillHeight(5)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(STextBlock)
                                    .Text(FText::FromString("Custom Landscape Z scale"))
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
                                // Z Scale
                                + SVerticalBox::Slot()
                                .FillHeight(10)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(SCheckBox)
                                    .Type(ESlateCheckBoxType::CheckBox)
                                    .IsChecked_Lambda([=]()
                                    {
                                        if(GetGisFileManager()->HasLandscape())
                                        {
                                            ImportOptions.bHighDetailZScale = GetGisFileManager()->GetInfos()->bHighDetailZScale;
                                        }
                                        return ImportOptions.bHighDetailZScale
                                                    ? ECheckBoxState::Checked
                                                    : ECheckBoxState::Unchecked;
                                    })
                                    .OnCheckStateChanged_Lambda([=](ECheckBoxState NewState)
                                    {
                                        ImportOptions.bHighDetailZScale = (NewState == ECheckBoxState::Checked);
                                        GetGisFileManager()->GetInfos()->bHighDetailZScale = ImportOptions.bHighDetailZScale;
                                    })
                                    .ToolTipText(FText::FromString("Auto detect best z scale of Landscape\nOnly uncheck this on flat terrains or if you plan to fill areas with 'Landscape Update Options' below\nIf the heightmap range is exeeded it will fall back to 'true'"))
                                    .IsEnabled_Lambda([=]()
                                    {
                                        return bRasterSelected && !GetGisFileManager()->HasLandscape();
                                    })
                                ]
                                + SVerticalBox::Slot()
                                .FillHeight(10)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(SSpinBox<float>)
                                    .MinValue(1)
                                    .MaxValue(8192)
                                    .MinSliderValue(1)
                                    .MaxSliderValue(8192)
                                    .MinDesiredWidth(150)
                                    .Value_Lambda([=]()
                                    {
                                        if(GetGisFileManager()->HasLandscape())
                                        {
                                            ImportOptions.ZScale = GetGisFileManager()->GetInfos()->CustomZScale;
                                        }
                                        return ImportOptions.ZScale;
                                    })
                                    .OnValueChanged_Lambda([=](float NewValue)
                                    {
                                        ImportOptions.ZScale = NewValue;
                                        GetGisFileManager()->GetInfos()->CustomZScale = ImportOptions.ZScale;
                                    })
                                    .IsEnabled_Lambda([=]()
                                    {
                                        return bRasterSelected && !ImportOptions.bHighDetailZScale && !GetGisFileManager()->HasLandscape();
                                    })
                                    .ToolTipText(FText::FromString("Custom z scale for Landscape\nPlease be careful what values to choose - the data has to fit into the heightmaps range\nSee UE docs 'Landscape Technical Guide'"))
                                ]
                            ]
                        ]
                    ]
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Landscape Generation Options"))
                        #if ENGINE_MINOR_VERSION < 1
                        .TextStyle(FEditorStyle::Get(), "LargeText")
                        #else
                        .TextStyle(FAppStyle::Get(), "LargeText")
                        #endif
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
                                .FillHeight(10)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(STextBlock)
                                    .Text(FText::FromString("Import as Mesh"))
                                ]
                                + SVerticalBox::Slot()
                                .FillHeight(10)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(STextBlock)
                                    .Text(FText::FromString("Mesh Collision"))
                                ]
                                + SVerticalBox::Slot()
                                .FillHeight(10)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(STextBlock)
                                    .Text(FText::FromString("Force square Landscapes"))
                                ]
                                + SVerticalBox::Slot()
                                .FillHeight(10)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(STextBlock)
                                    .Text(FText::FromString("Desired Max Tile Size"))
                                ]
                                + SVerticalBox::Slot()
                                .FillHeight(10)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(STextBlock)
                                    .Text(FText::FromString("World Partition Grid Size"))
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
                                    .Text(FText::FromString("Use native Raster Pixel Size"))
                                ]
                                + SVerticalBox::Slot()
                                .FillHeight(10)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(STextBlock)
                                    .Text(FText::FromString("Custom Raster Pixel Size"))
                                ]
                                + SVerticalBox::Slot()
                                .FillHeight(10)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(STextBlock)
                                    .Text(FText::FromString("Resample to first Tile"))
                                ]
                                + SVerticalBox::Slot()
                                .FillHeight(10)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(STextBlock)
                                    .Text(FText::FromString("Smooth Steps"))
                                ]
                                + SVerticalBox::Slot()
                                .FillHeight(10)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(STextBlock)
                                    .Text(FText::FromString("Smooth Edges"))
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
                                .FillHeight(10)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(SCheckBox)
                                    .Type(ESlateCheckBoxType::CheckBox)
                                    .IsChecked_Lambda([=]()
                                    {
                                        return ImportOptions.bImportAsMesh
                                                    ? ECheckBoxState::Checked
                                                    : ECheckBoxState::Unchecked;
                                    })
                                    .OnCheckStateChanged_Lambda([=](ECheckBoxState NewState)
                                    {
                                        ImportOptions.bImportAsMesh = (NewState == ECheckBoxState::Checked);
                                    })
                                    .ToolTipText(FText::FromString("Import as Mesh instead of Landscape"))
                                    .IsEnabled_Lambda([=]()
                                    {
                                        return bRasterSelected;
                                    })
                                ]
                                + SVerticalBox::Slot()
                                .FillHeight(10)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(SCheckBox)
                                    .Type(ESlateCheckBoxType::CheckBox)
                                    .IsChecked_Lambda([=]()
                                    {
                                        return ImportOptions.bMeshCollision
                                                    ? ECheckBoxState::Checked
                                                    : ECheckBoxState::Unchecked;
                                    })
                                    .OnCheckStateChanged_Lambda([=](ECheckBoxState NewState)
                                    {
                                        ImportOptions.bMeshCollision = (NewState == ECheckBoxState::Checked);
                                    })
                                    .ToolTipText(FText::FromString("Add collision to Mesh"))
                                    .IsEnabled_Lambda([=]()
                                    {
                                        return bRasterSelected && ImportOptions.bImportAsMesh;
                                    })
                                ]
                                + SVerticalBox::Slot()
                                .FillHeight(10)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(SCheckBox)
                                    .Type(ESlateCheckBoxType::CheckBox)
                                    .IsChecked_Lambda([=]()
                                    {
                                        return ImportOptions.bSquareTiles
                                                    ? ECheckBoxState::Checked
                                                    : ECheckBoxState::Unchecked;
                                    })
                                    .OnCheckStateChanged_Lambda([=](ECheckBoxState NewState)
                                    {
                                        ImportOptions.bSquareTiles = (NewState == ECheckBoxState::Checked);
                                    })
                                    .ToolTipText(FText::FromString("Force square Landscapes.\n\nSquare Landscapes have the advantage, that height- and weightmaps can be exported and re-imported as PNG-files.\nThere will be steep edges in areas with no data in the source DTM."))
                                    .IsEnabled_Lambda([=]()
                                    {
                                        return bRasterSelected && !GetGisFileManager()->HasLandscape();
                                    })
                                ]
                                + SVerticalBox::Slot()
                                .FillHeight(10)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(SHorizontalBox)
                                    +SHorizontalBox::Slot()
                                    [
                                        SNew(SSpinBox<int>)
                                        .MinValue(256)
                                        .MaxValue(INT_MAX)
                                        .MinSliderValue(256)
                                        .MaxSliderValue(INT_MAX)
                                        .MinDesiredWidth(150)
                                        .Delta(256)
                                        .Value_Lambda([=]()
                                        {
                                            ImportOptions.DesiredMaxTileSize = GetGisFileManager()->GetInfos()->DesiredMaxTileSize;
                                            return ImportOptions.DesiredMaxTileSize;
                                        })
                                        .OnValueChanged_Lambda([=](int NewValue)
                                        {
                                            ImportOptions.DesiredMaxTileSize = NewValue;
                                            GetGisFileManager()->GetInfos()->DesiredMaxTileSize = ImportOptions.DesiredMaxTileSize;
                                        })
                                        .IsEnabled_Lambda([=]()
                                        {
                                            return !GetGisFileManager()->HasLandscape();
                                        })
                                        .ToolTipText(FText::FromString("Maximum Size of a single Landscape or Mesh Terrain in meter\nIn World partition the Landscape will be split in Landscape Streaming proxies controlled by World Partition Grid Size."))
                                    ]
                                ]
                                // World Partition Grid Size
                                + SVerticalBox::Slot()
                                .FillHeight(10)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(SHorizontalBox)
                                    +SHorizontalBox::Slot()
                                    [
                                        SNew(SSpinBox<int>)
                                        .MinValue(1)
                                        .MaxValue(16)
                                        .MinSliderValue(1)
                                        .MaxSliderValue(16)
                                        .MinDesiredWidth(150)
                                        .Delta(1)
                                        .Value_Lambda([=]()
                                        {
                                            ImportOptions.WorldPartitionGridSize = GetGisFileManager()->GetInfos()->WorldPartitionGridSize;
                                            return ImportOptions.WorldPartitionGridSize;
                                        })
                                        .OnValueChanged_Lambda([=](int NewValue)
                                        {
                                            ImportOptions.WorldPartitionGridSize = NewValue;
                                            GetGisFileManager()->GetInfos()->WorldPartitionGridSize = ImportOptions.WorldPartitionGridSize;
                                        })
                                        .IsEnabled_Lambda([=]()
                                        {
                                            return GetGisFileManager()->GetIsWorldPartition() && !GetGisFileManager()->HasLandscape();
                                        })
                                        .ToolTipText(FText::FromString("World Partition Grid Size\nNumber of components per landscape streaming proxies per axis"))
                                    ]
                                ]
                                + SVerticalBox::Slot()
                                .FillHeight(10)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(SSpinBox<int>)
                                    .MinValue(0)
                                    .MaxValue(14)
                                    .MinSliderValue(0)
                                    .MaxSliderValue(14)
                                    .MinDesiredWidth(150)
                                    .Value_Lambda([=]()
                                    {
                                        return ImportOptions.ZoomLevel;
                                    })
                                    .OnValueChanged_Lambda([=](int NewValue)
                                    {
                                        ImportOptions.ZoomLevel = NewValue;
                                    })
                                    .ToolTipText(FText::FromString("Mapbox Zoom Level for Terrain data (only available with Landscaping Mapbox Extension)"))
                                    .IsEnabled_Lambda([=]()
                                    {
                                        return GetGisFileManager()->HasMapboxExtension();
                                    })
                                ]
                                + SVerticalBox::Slot()
                                .FillHeight(10)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(SCheckBox)
                                    .Type(ESlateCheckBoxType::CheckBox)
                                    .IsChecked_Lambda([=]()
                                    {
                                        return ImportOptions.bNativeRasterPixelSize
                                                    ? ECheckBoxState::Checked
                                                    : ECheckBoxState::Unchecked;
                                    })
                                    .OnCheckStateChanged_Lambda([=](ECheckBoxState NewState)
                                    {
                                        ImportOptions.bNativeRasterPixelSize = (NewState == ECheckBoxState::Checked);
                                        if(!ImportOptions.bNativeRasterPixelSize)
                                        {
                                            ImportOptions.bResampleToFirstTile = false;
                                        }
                                    })
                                    .ToolTipText(FText::FromString("Use Pixel size of Digital Terrain Model Raster (Meter Per Pixel)"))
                                    .IsEnabled_Lambda([=]()
                                    {
                                        return bRasterSelected;
                                    })
                                ]
                                + SVerticalBox::Slot()
                                .FillHeight(10)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(SSpinBox<float>)
                                    .MinValue(0.25)
                                    .MaxValue(50)
                                    .MinSliderValue(0.25)
                                    .MaxSliderValue(50)
                                    .MinDesiredWidth(150)
                                    .Value_Lambda([=]()
                                    {
                                        return ImportOptions.CustomRasterPixelSize;
                                    })
                                    .OnValueChanged_Lambda([=](float NewValue)
                                    {
                                        ImportOptions.CustomRasterPixelSize = NewValue;
                                    })
                                    .IsEnabled_Lambda([=]()
                                    {
                                        return bRasterSelected && !ImportOptions.bNativeRasterPixelSize;
                                    })
                                    .ToolTipText(FText::FromString("Custom Raster Pixel Size (Meter Per Pixel)\nLower values will lead to a higher memory usage on a same sized area"))
                                ]
                                + SVerticalBox::Slot()
                                .FillHeight(10)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(SCheckBox)
                                    .Type(ESlateCheckBoxType::CheckBox)
                                    .IsChecked_Lambda([=]()
                                    {
                                        return ImportOptions.bResampleToFirstTile
                                                    ? ECheckBoxState::Checked
                                                    : ECheckBoxState::Unchecked;
                                    })
                                    .OnCheckStateChanged_Lambda([=](ECheckBoxState NewState)
                                    {
                                        ImportOptions.bResampleToFirstTile = (NewState == ECheckBoxState::Checked);
                                    })
                                    .ToolTipText(FText::FromString("Resample additional tile imports to the resolution of the first tile.\nThis will create seamless worlds but comes at the cost of possibly importing data at a higher resolution as necessary.\nAlso if the first imported tile has a low resolution, the following imports will also have the same low resolution.\nThis options is only available, if Native Raster Pixel Size is 'true'"))
                                    .IsEnabled_Lambda([=]()
                                    {
                                        return bRasterSelected && ImportOptions.bNativeRasterPixelSize;
                                    })
                                ]
                                // Smooth Landscape
                                + SVerticalBox::Slot()
                                .FillHeight(10)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(SSpinBox<int>)
                                    .MinValue(0)
                                    .MaxValue(3)
                                    .MinSliderValue(0)
                                    .MaxSliderValue(3)
                                    .MinDesiredWidth(150)
                                    .Delta(1)
                                    .Value_Lambda([=]()
                                    {
                                        return ImportOptions.SmoothSteps;
                                    })
                                    .OnValueChanged_Lambda([=](int NewValue)
                                    {
                                        ImportOptions.SmoothSteps = NewValue;
                                    })
                                    .IsEnabled_Lambda([=]()
                                    {
                                        return !ImportOptions.bImportAsMesh;
                                    })
                                    .ToolTipText(FText::FromString("Smooth Landscape\nHow many times the smooth (gaussian blur) operation is applied to the landscape.\nFor Mapbox import the recommended value is 1 on mountain terrain, and 2 on flat terrain\nFor seamless multiple Landscapes uncheck 'Smooth Edges'"))
                                ]
                                + SVerticalBox::Slot()
                                .FillHeight(10)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(SCheckBox)
                                    .Type(ESlateCheckBoxType::CheckBox)
                                    .IsChecked_Lambda([=]()
                                    {
                                        return ImportOptions.bSmoothEdges
                                                    ? ECheckBoxState::Checked
                                                    : ECheckBoxState::Unchecked;
                                    })
                                    .OnCheckStateChanged_Lambda([=](ECheckBoxState NewState)
                                    {
                                        ImportOptions.bSmoothEdges = (NewState == ECheckBoxState::Checked);
                                    })
                                    .ToolTipText(FText::FromString("When smoothing, should the edges also be smoothed?\nDisabling this will make it possible to have seamless landscapes.\nIf not using Smooth Steps on import, landscapes will align seamlessly."))
                                    .IsEnabled_Lambda([=]()
                                    {
                                        return bRasterSelected;
                                    })
                                ]
                            ]
                        ]
                    ]
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Landscape Update Options"))
                        #if ENGINE_MINOR_VERSION < 1
                        .TextStyle(FEditorStyle::Get(), "LargeText")
                        #else
                        .TextStyle(FAppStyle::Get(), "LargeText")
                        #endif
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
                                .FillHeight(10)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(STextBlock)
                                    .Text(FText::FromString("Update Landscape"))
                                ]
                                + SVerticalBox::Slot()
                                .FillHeight(10)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(STextBlock)
                                    .Text(FText::FromString("Only fill missing"))
                                ]
                                + SVerticalBox::Slot()
                                .FillHeight(10)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(STextBlock)
                                    .Text(FText::FromString("Minimum height tolerance"))
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
                                .FillHeight(10)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(SCheckBox)
                                    .Type(ESlateCheckBoxType::CheckBox)
                                    .IsChecked_Lambda([=]()
                                    {
                                        return ImportOptions.bUpdateLandscape
                                                    ? ECheckBoxState::Checked
                                                    : ECheckBoxState::Unchecked;
                                    })
                                    .OnCheckStateChanged_Lambda([=](ECheckBoxState NewState)
                                    {
                                        ImportOptions.bUpdateLandscape = (NewState == ECheckBoxState::Checked);
                                    })
                                    .ToolTipText(FText::FromString("Will update the Landscape with the new height data from the selected DTM files or Mapbox\nOnly available in World Partition worlds\nMake sure the original heightmap was imported with 'High detail Z scale' unchecked"))
                                    .IsEnabled_Lambda([=]()
                                    {
                                        return bRasterSelected && GetGisFileManager()->HasLandscape();
                                    })
                                ]
                                + SVerticalBox::Slot()
                                .FillHeight(10)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(SCheckBox)
                                    .Type(ESlateCheckBoxType::CheckBox)
                                    .IsChecked_Lambda([=]()
                                    {
                                        return ImportOptions.bFillMissing
                                                    ? ECheckBoxState::Checked
                                                    : ECheckBoxState::Unchecked;
                                    })
                                    .OnCheckStateChanged_Lambda([=](ECheckBoxState NewState)
                                    {
                                        ImportOptions.bFillMissing = (NewState == ECheckBoxState::Checked);
                                    })
                                    .ToolTipText(FText::FromString("Only fill areas with missing height data\nChecked means it will fill only areas with minimum altitude with the new data\nUnchecked will update the whole Landscape heightmap\nThere will be seams on the edges of the original and new data which can be corrected with the Sculpt tools in Landscape Mode"))
                                    .IsEnabled_Lambda([=]()
                                    {
                                        return ImportOptions.bUpdateLandscape;
                                    })
                                ]
                                 + SVerticalBox::Slot()
                                .FillHeight(10)
                                .VAlign(VAlign_Center)
                                .Padding(5)
                                [
                                    SNew(SSpinBox<int>)
                                    .MinValue(0)
                                    .MaxValue(100000)
                                    .MinSliderValue(0)
                                    .MaxSliderValue(100000)
                                    .MinDesiredWidth(150)
                                    .Delta(1)
                                    .Value_Lambda([=]()
                                    {
                                        return ImportOptions.MinHeightTolerance;
                                    })
                                    .OnValueChanged_Lambda([=](int NewValue)
                                    {
                                        ImportOptions.MinHeightTolerance = NewValue;
                                    })
                                    .IsEnabled_Lambda([=]()
                                    {
                                        return ImportOptions.bFillMissing;
                                    })
                                    .ToolTipText(FText::FromString("Minimum-height tolerance in centimeter\nIncreasing this value will reduce the seams between old and new data"))
                                ]
                            ]
                        ]
                    ]
                ]
            ]
        ]
    ];
}

void FRasterImportOptionsUI::ShowMap() const
{
    FString Url = FString::Printf(TEXT("https://maps.ludicdrive.com/#%s"), *ImportBBox);
    FPlatformProcess::LaunchURL(*Url, nullptr, nullptr);
}

void FRasterImportOptionsUI::ShowCRS()
{
    FString Url = FString("https://epsg.io/");
    if(GetGisFileManager()->GetCRS()->GetAuthorityID() > 0)
    {
        Url.AppendInt(GetGisFileManager()->GetCRS()->GetAuthorityID());
    }
    FPlatformProcess::LaunchURL(*Url, nullptr, nullptr);
}

FText FRasterImportOptionsUI::GetImportBBox() const
{
    return FText::FromString(ImportBBox);
}

void FRasterImportOptionsUI::HandleBrowserUrlChanged(const FText& Url)
{
    TArray<FString> Out;
    Url.ToString().ParseIntoArray(Out,TEXT("#"), true);
    if(Out.Num() == 2 && !Out[1].Equals(ImportBBox))
    {
        ImportBBox = Out[1];
    }
    else 
    {
        if(GetGisFileManager()->HasMapboxExtension() && !GetGisFileManager()->HasRasterData() && GetGisFileManager()->GetCRS()->GetAuthorityID() != 0)
        {
            bRasterSelected = true;
            ImportOptions.bMapboxImport = true;
        }
        return;
    }
    FString AreaStr = CalculateArea();
    RasterFileImportInfo = MakeShareable(new FString(AreaStr));
    ShowWarning();
}

FString FRasterImportOptionsUI::CalculateArea(bool bFetchFromNewExtents)
{
    // bounding box from maps.ludicdrive.com:
    // eg: 23.241346,1.054688,27.059126,6.800537
    // a,b,c,d
    // a: bottom(lat)
    // b: left(lon)
    // c: top(lat)
    // d: right(lon)

    // ----------------c,d
    // |				 |
    // |				 |
    // |				 |
    // |				 |
    // a,b----------------
    
    if(bFetchFromNewExtents)
    {
        ImportBBox = FString();
    }

    if(!ImportBBox.IsEmpty())
    {
        if(ImportBBox.Equals("0.000000,0.000000,0.000000,0.000000"))
        {
            return FString("Select import area on the map");
        }
        bool bIsValid = GetGisFileManager()->GetInfos()->SetCroppedExtents(ImportBBox);
        if(bIsValid)
        {
            if(GetGisFileManager()->HasMapboxExtension() && !GetGisFileManager()->HasRasterData())
            {
                bRasterSelected = true;
                ImportOptions.bMapboxImport = true;
                if(!GetGisFileManager()->GetCRS()->IsAuthorityIDValid())
                {
                    if(GetGisFileManager()->GetCRS()->IsModeUseSourceCRS())
                    {
                        GetGisFileManager()->GetCRS()->SetAuthorityID(3857);
                    }
                    else if(GetGisFileManager()->GetCRS()->IsModeAutoReprojectToUTM())
                    {
                        FExtents Ext = GetGisFileManager()->GetInfos()->GetCroppedExtents();
                        int AuthorityID = GetGisFileManager()->GetCRS()->GetUTMAuthorityID(Ext.Left, Ext.Top);
                        GetGisFileManager()->GetCRS()->SetAuthorityID(AuthorityID);
                    }
                }
                CRSInfo = MakeShareable(new FString(GetGisFileManager()->GetCRS()->GetAuthorityID() == 0 ? FString("CRS not set") : GetGisFileManager()->GetCRS()->GetAuthorityIDStr()));
            }
        }
        else
        {
            ImportBBox.Empty();
            FMessageDialog::Open(
                EAppMsgType::Ok, 
                FText::FromString("Invalid Bounding Box.\nPlease only copy the part after the # in the URL of https://maps.ludicdrive.com")
            );
            return FString("Select import area on the map");
        }
    }
    else
    {
        ImportBBox = GetGisFileManager()->GetInfos()->GetCroppedExtents().ToString();
    }
    
    double OverallArea = GetGisFileManager()->GetInfos()->GetArea();
    FVector AreaDimension = GetGisFileManager()->GetInfos()->GetAreaDimension();
    FString AreaLabelStr = FString("Overall area");
    if(OverallArea < 0 || OverallArea > 1000000)
    {
        double AreaTwoDigits = (double)((int)(OverallArea*100.0))/100.0;
        FString Msg = FString::Printf(TEXT("Input file and CRS mismatch\n\nPlease check\n- 'Settings -> Plugins -> Landscaping' and choose an approriate CRS\n- Input file is inside of valid area of CRS"), AreaTwoDigits);
        UE_LOG(LogTemp, Error, TEXT("Landscaping: %s"), *Msg);
        FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(*Msg));
        return FString("Invalid input");
    }
    if(OverallArea == 0 || bRasterSelected)
    {
        OverallArea = AreaDimension.X / 1000.0 * AreaDimension.Y / 1000.0;
        AreaLabelStr = FString("Last selected area");
    }
    double AreaTwoDigits = (double)((int)(OverallArea*100.0))/100.0;
    double WidthTwoDigits = (double)((int)(AreaDimension.X*0.1))/100.0;
    double HeightTwoDigits = (double)((int)(AreaDimension.Y*0.1))/100.0;
    FString ImportInfo = FString::Printf(TEXT("%s: %s km² (%s x %s km)"), *AreaLabelStr, *FString::SanitizeFloat(AreaTwoDigits, 2), *FString::SanitizeFloat(WidthTwoDigits, 2), *FString::SanitizeFloat(HeightTwoDigits, 2));
    return ImportInfo;
}

void FRasterImportOptionsUI::ShowWarning()
{
    FVector AreaDimension = GetGisFileManager()->GetInfos()->GetAreaDimension();
    double NewArea = AreaDimension.X / 1000.0 * AreaDimension.Y / 1000.0;
    uint32 RamInGB = FPlatformMemory::GetPhysicalGBRam();
    double MeterPerPixel = GetGisFileManager()->GetAvgMeterPerPixel();
    double WarnLimit = (double)RamInGB / NewArea * MeterPerPixel;
    double ImportArea = (double)((int)(NewArea*100.0))/100.0;
    UE_LOG(LogTemp, Log, TEXT("Landscaping: Selected %s km² with %i GB RAM and avg meter per pixel of %f. MemoryGB/Area*MeterPerPixel: %f (above 0.1 is fine - this applies for Landscapes and not Meshes)"), *FString::SanitizeFloat(ImportArea, 2), RamInGB, MeterPerPixel, WarnLimit);
    if(bWarningShown || !bRasterSelected)
    {
        return;
    }
    if(WarnLimit < 0.1)
    {
        FString Msg = FString::Printf(TEXT("You are about to import %s km² with %i GB RAM.\nYou might run out of memory.\nSee docs for recommendations!"), *FString::SanitizeFloat(ImportArea, 2), RamInGB);
        FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(*Msg));
        bWarningShown = true;
    }
}

FText FRasterImportOptionsUI::GetImportInfo() const
{
    return RasterFileImportInfo.IsValid() ? FText::FromString(*RasterFileImportInfo) : FText::FromString("Import DTM");
}

FText FRasterImportOptionsUI::GetCRSInfo() const
{
    return CRSInfo.IsValid() ? FText::FromString(*CRSInfo) : FText::FromString("CRS not set");
}

UGISFileManager* FRasterImportOptionsUI::GetGisFileManager()
{
    if(GisFileManager == nullptr)
    {
        GisFileManager = GEditor->GetEditorSubsystem<UGISFileManager>();
    }
    return GisFileManager;
}
