// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved



#include "FVectorImporterUI.h"

FVectorImporterUI::FVectorImporterUI(UGISFileManager* InGisFileManager)
{
    GisFileManager = InGisFileManager;
    SplinesOptionsUI = new FLandscapeSplinesOptionsUI();
    Options = new VectorGeometrySpawnOptions();
}

FVectorImporterUI::~FVectorImporterUI()
{

}


TSharedRef<SVerticalBox> FVectorImporterUI::ImportVectorFilesUI()
{
    TypeOptions.Empty();
    TypeOptions.Add(MakeShareable(new FString("Spline Mesh")));
    TypeOptions.Add(MakeShareable(new FString("Actor / Blueprint")));
    TypeOptions.Add(MakeShareable(new FString("Paint Layer / Deform Landscape")));
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
    TypeOptions.Add(MakeShareable(new FString("Landscape Spline")));
#endif
    SelectedTypeOption = TypeOptions[0];

    SplinePointTypes.Empty();
    SplinePointTypes.Add(MakeShareable(new FString("Linear")));
    SplinePointTypes.Add(MakeShareable(new FString("Curve")));
    SplinePointTypes.Add(MakeShareable(new FString("Constant")));
    SplinePointTypes.Add(MakeShareable(new FString("CurveClamped")));
    SplinePointTypes.Add(MakeShareable(new FString("CurveCustomTangent")));
    Options->SplinePointTypeStr = SplinePointTypes[1];
    FUserInterfaceParts* UserInterfaceParts = new FUserInterfaceParts();
    return SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            UserInterfaceParts->SHeader1("Import Roads, Railtracks, Rivers, etc.", "")
        ]
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .Padding(0, 5)
            .HAlign(HAlign_Left)
            [
                SNew(STextBlock)
                .Text_Raw(this, &FVectorImporterUI::GetImportInfoVectorData)
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
                    FPlatformProcess::LaunchURL(TEXT("https://jorop.github.io/landscaping-docs/#/props?id=props"), nullptr, nullptr);
                })
                .ToolTipText(FText::FromString("Documentation on how to generate props from GIS vector data (shapefiles, etc.)"))
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
                      .FillHeight(10)
                      .VAlign(VAlign_Center)
                      .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Zoom Level"))
                        .ToolTipText(FText::FromString("Mapbox Zoom Level for Vector data"))
                    ]
                    + SVerticalBox::Slot()
                      .FillHeight(10)
                      .VAlign(VAlign_Center)
                      .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Select Files"))
                        .ToolTipText(FText::FromString("Select file(s)"))
                    ]
                     + SVerticalBox::Slot()
                      .FillHeight(10)
                      .VAlign(VAlign_Center)
                      .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("What should be created"))
                        .ToolTipText(FText::FromString("Instantiate Spline Mesh or Actor or paint Landscape Material Layer"))
                    ]
                    + SVerticalBox::Slot()
                      .FillHeight(10)
                      .VAlign(VAlign_Center)
                      .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Options"))
                        .ToolTipText(FText::FromString("Options"))
                    ]
                    + SVerticalBox::Slot()
                      .FillHeight(10)
                      .VAlign(VAlign_Center)
                      .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Offset from Ground (centimeter)"))
                        .ToolTipText(FText::FromString("How many cm should the spline point be instantiated above the ground"))
                    ]
                    + SVerticalBox::Slot()
                      .FillHeight(10)
                      .VAlign(VAlign_Center)
                      .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Revert Spline Direction"))
                        .ToolTipText(FText::FromString("Useful for rivers, if the geometry of the vector data is wrong"))
                    ]
                    + SVerticalBox::Slot()
                      .FillHeight(10)
                      .VAlign(VAlign_Center)
                      .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Crop to Bounds"))
                        .ToolTipText(FText::FromString("Crop vector data to bounds of the landscape"))
                    ]
                    + SVerticalBox::Slot()
                      .FillHeight(10)
                      .VAlign(VAlign_Center)
                      .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("OSM Feature Class"))
                        .ToolTipText(FText::FromString("Select the OSM Feature Class for which the splines or Actors should be created\nor the Landscape Layer should be painted"))
                    ]
                    + SVerticalBox::Slot()
                      .FillHeight(10)
                      .VAlign(VAlign_Center)
                      .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Spline Point Type"))
                        .ToolTipText(FText::FromString("Default is Curve. Choose Linear here for procedural spline based buildings"))
                    ]
                    + SVerticalBox::Slot()
                      .FillHeight(10)
                      .VAlign(VAlign_Center)
                      .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Align points horizontally"))
                        .ToolTipText(FText::FromString("This will make sure, spline points align horizontally. E.g. for buildings"))
                    ]
                    + SVerticalBox::Slot()
                      .FillHeight(10)
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
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(SButton)
                        .Text_Raw(this, &FVectorImporterUI::SelectVectorDataText)
                        .HAlign(HAlign_Center)
                        .OnClicked_Raw(this, &FVectorImporterUI::SelectVectorDataClicked)
                        .ToolTipText(FText::FromString("Select file(s)"))
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(SComboBox<TSharedPtr<FString>>)
                        .OptionsSource(&TypeOptions)
                        .InitiallySelectedItem(SelectedTypeOption)
                        [
                            SNew(STextBlock)
                            .Text_Lambda([=]()
                            {
                                if(SelectedTypeOption.IsValid())
                                {
                                    return FText::FromString(*SelectedTypeOption);
                                }
                                return FText::FromString("Select File first");
                            })
                        ]
                        .OnSelectionChanged_Lambda([=](TSharedPtr<FString> NewValue, ESelectInfo::Type SelectionType)
                        {
                            SelectedTypeOption = NewValue;
                            Options->LandscapeSplineOptions.bPaintMaterialLayer = SelectedTypeOption.Get()->Equals("Paint Layer / Deform Landscape");
                        })
                        .OnGenerateWidget_Lambda([=](TSharedPtr<FString> InOption)
                        {
                            return SNew(STextBlock).Text(FText::FromString(*InOption));
                        })
                        .ToolTipText(FText::FromString("Instantiate Spline Mesh or Actor or paint Landscape Material Layer"))
                        .IsEnabled_Lambda([=]()
                        {
                            return GetGisFileManager()->HasLandscape();
                        })
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(SButton)
                        .Text_Raw(this, &FVectorImporterUI::SplineOrActorOptionsText)
                        .HAlign(HAlign_Center)
                        .OnClicked_Raw(this, &FVectorImporterUI::OpenOptionsWindow)
                        .ToolTipText(FText::FromString("Options"))
                    ]
                    + SVerticalBox::Slot()
                      .FillHeight(10)
                      .VAlign(VAlign_Center)
                      .Padding(5)
                    [
                        SNew(SSpinBox<float>)
                        .MinValue(-10000)
                        .MaxValue(10000)
                        .MinSliderValue(-10000)
                        .MaxSliderValue(10000)
                        .MinDesiredWidth(150)
                        .Value_Lambda([this]()
                        {
                            return GetGisFileManager()->GetOffsetFromGround();
                        })
                        .OnValueChanged_Lambda([this](float NewValue)
                        {
                            GetGisFileManager()->SetOffsetFromGround(NewValue);
                        })
                        .ToolTipText(FText::FromString("How many cm should the spline point be instantiated above the ground"))
                        .IsEnabled_Lambda([=]()
                        {
                            return GetGisFileManager()->HasLandscape();
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
                            return Options->bRevertSplineDirection
                                        ? ECheckBoxState::Checked
                                        : ECheckBoxState::Unchecked;
                        })
                        .OnCheckStateChanged_Lambda([=](ECheckBoxState NewState)
                        {
                            Options->bRevertSplineDirection = (NewState == ECheckBoxState::Checked);
                        })
                        .ToolTipText(FText::FromString("Useful for rivers, if the geometry of the vector data is wrong"))
                        .IsEnabled_Lambda([=]()
                        {
                            return GetGisFileManager()->HasLandscape();
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
                            return Options->bCropToBounds
                                        ? ECheckBoxState::Checked
                                        : ECheckBoxState::Unchecked;
                        })
                        .OnCheckStateChanged_Lambda([=](ECheckBoxState NewState)
                        {
                            Options->bCropToBounds = (NewState == ECheckBoxState::Checked);
                        })
                        .ToolTipText(FText::FromString("Crop vector data to bounds of the landscape"))
                        .IsEnabled_Lambda([=]()
                        {
                            return GetGisFileManager()->HasLandscape() && !GetGisFileManager()->IsImportedThroughLandscaping();
                        })
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(SComboBox<TSharedPtr<FString>>)
                        .OptionsSource(&ShapeFClasses)
                        .InitiallySelectedItem(Options->ShapeFClass)
                        [
                            SNew(STextBlock)
                            .Text_Lambda([=]()
                            {
                                if(Options->ShapeFClass.IsValid())
                                {
                                    return FText::FromString(*Options->ShapeFClass);
                                }
                                return FText::FromString("Select File first");
                            })
                        ]
                        .OnSelectionChanged_Lambda([=](TSharedPtr<FString> NewValue, ESelectInfo::Type SelectionType)
                        {
                            Options->ShapeFClass = NewValue;
                            if(NewValue.IsValid() && GetGisFileManager())
                            {
                                GetGisFileManager()->SetShapeFClass(FString(*NewValue.Get()));
                            }
                        })
                        .OnGenerateWidget_Lambda([=](TSharedPtr<FString> InOption)
                        {
                            return SNew(STextBlock).Text(FText::FromString(*InOption));
                        })
                        .ToolTipText(FText::FromString("Select the OSM feature class for which the splines or blueprints should be created"))
                        .IsEnabled_Lambda([=]()
                        {
                            return GetGisFileManager()->HasLandscape();
                        })
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(SComboBox<TSharedPtr<FString>>)
                        .OptionsSource(&SplinePointTypes)
                        .InitiallySelectedItem(Options->SplinePointTypeStr)
                        [
                            SNew(STextBlock)
                            .Text_Lambda([=]()
                            {
                                if(Options->SplinePointTypeStr.IsValid())
                                {
                                    return FText::FromString(*Options->SplinePointTypeStr);
                                }
                                return FText::FromString("Select File first");
                            })
                        ]
                        .OnSelectionChanged_Lambda([=](TSharedPtr<FString> NewValue, ESelectInfo::Type SelectionType)
                        {
                            Options->SplinePointTypeStr = NewValue;
                            Options->SplinePointType = (ESplinePointType::Type)SplinePointTypes.Find(NewValue);
                        })
                        .OnGenerateWidget_Lambda([=](TSharedPtr<FString> InOption)
                        {
                            return SNew(STextBlock).Text(FText::FromString(*InOption));
                        })
                        .ToolTipText(FText::FromString("Spline Point Type.\nChoose 'Linear' here for procedural spline based buildings"))
                        .IsEnabled_Lambda([=]()
                        {
                            return GetGisFileManager()->HasLandscape();
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
                            return Options->bIsBuilding
                                        ? ECheckBoxState::Checked
                                        : ECheckBoxState::Unchecked;
                        })
                        .OnCheckStateChanged_Lambda([=](ECheckBoxState NewState)
                        {
                            Options->bIsBuilding = (NewState == ECheckBoxState::Checked);
                        })
                        .ToolTipText(FText::FromString("This will make sure, spline points align horizontally. E.g. for procedural spline based buildings\nThe first point will snap to the ground with offset from ground taken into account."))
                        .IsEnabled_Lambda([=]()
                        {
                            return GetGisFileManager()->HasLandscape();
                        })
                    ]
                    + SVerticalBox::Slot()
                      .FillHeight(10)
                      .VAlign(VAlign_Center)
                      .Padding(5)
                    [
                        SNew(SButton)
                        .Text(FText::FromString("Import"))
                        .HAlign(HAlign_Center)
                        .OnClicked_Raw(this, &FVectorImporterUI::ImportVectorDataClicked)
                        .IsEnabled_Lambda([=]()
                        {
                            return GetGisFileManager()->HasVectorData();
                        })
                    ]
                ]
            ]
        ];
}

FText FVectorImporterUI::SelectVectorDataText() const
{
    return FText::FromString("Select");
}

FText FVectorImporterUI::SplineOrActorOptionsText() const
{
    FString SelectedOption = *SelectedTypeOption.Get();
    return SelectedTypeOption.IsValid() ? FText::FromString(FString::Printf(TEXT("%s Options"), *SelectedOption)) : FText::FromString("Options");
}


FReply FVectorImporterUI::ImportVectorDataClicked()
{
    if (GisFileManager == nullptr)
    {
        FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("Please select file(s) first."));
        return FReply::Handled();
    }
    if (Options->SplineMesh == nullptr && Options->ActorOrBlueprintClass == nullptr && !Options->LandscapeSplineOptions.bPaintMaterialLayer)
    {
        FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("Please select a mesh or a blueprint first"));
        return FReply::Handled();
    }
    
    GetGisFileManager()->CreateBlueprints(*Options);
    return FReply::Handled();
}

FReply FVectorImporterUI::SelectVectorDataClicked()
{
    if(!GetGisFileManager()->HasLandscape())
    {
        GetGisFileManager()->CreateDummyLandscape();
    }
    // TODO replace this with possibility to select area for vector data only import
    if(GetGisFileManager()->GetCRS()->GetAuthorityID() == 0)
    {
        return OpenSetAnchorWindow();
    }
    else
    {
        return HandleSelectVectorData();
    }
}

FReply FVectorImporterUI::HandleSelectVectorData()
{
    MapboxFinishedDelegateHandle.Clear();
    MapboxFinishedDelegateHandle.AddLambda([=](FDataLoadResult &ResultInfo)
    {
        if(ResultInfo.ErrorMsg.IsEmpty())
        {
            ShapeFClasses.Empty();
            ShapeFClasses.Add(MakeShareable(new FString("ALL")));
            for(FString FeatureClass : ResultInfo.VectorFeatureClasses)
            {
                ShapeFClasses.Add(MakeShareable(new FString(FeatureClass)));
            }
            Options->ShapeFClass = ShapeFClasses[0];
            VectorImportInfo = FString("Ready to create");
        }
    });
    
    TArray<FString> OutputFiles;
    FDataLoadResult ResultInfo = GetGisFileManager()->OpenFileDialog(FString("Open Vector File for Splines and/or Blueprint Actors"), FString(""), VECTOR_FILE, OutputFiles, MapboxFinishedDelegateHandle, Options->ZoomLevel);
    
    if(ResultInfo.StatusMsg.Contains("Mapbox"))
    {
        return FReply::Handled();
    }
    
    if(ResultInfo.ErrorMsg.IsEmpty())
    {
        ShapeFClasses.Empty();
        ShapeFClasses.Add(MakeShareable(new FString("ALL")));
        for(FString FeatureClass : ResultInfo.VectorFeatureClasses)
        {
            ShapeFClasses.Add(MakeShareable(new FString(FeatureClass)));
        }
        Options->ShapeFClass = ShapeFClasses[0];
        VectorImportInfo = FString("Ready to create");
        return FReply::Handled();
    } 

    FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(ResultInfo.ErrorMsg));
    return FReply::Handled();
}

FText FVectorImporterUI::GetImportInfoVectorData() const
{
    return FText::FromString(*VectorImportInfo);
}

FText FVectorImporterUI::GetAnchorButtonText() const
{
    return FText::FromString("Autodetect (Please read tooltip)");
}

FReply FVectorImporterUI::OpenSetAnchorWindow()
{
    TSharedRef<SWindow> AnchorWindow = SNew(SWindow)
    .Type(EWindowType::Normal)
    .Title(FText::FromString("Landscaping - Set Anchor for VectorFile Import"))
    .SupportsMaximize(false)
    .ClientSize(FVector2D(300, 200))
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
                    .Text(FText::FromString("Anchor for Landscape"))
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
                        FPlatformProcess::LaunchURL(TEXT("https://jorop.github.io/landscaping-docs/#/props?id=props"), nullptr, nullptr);
                    })
                    .ToolTipText(FText::FromString("Documentation on VectorFile Import"))
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
                    .FillHeight(5)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Set from Geo Referencing System"))
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(SButton)
                        .Text(FText::FromString("Apply"))
                        .HAlign(HAlign_Center)
                        .OnClicked_Raw(this, &FVectorImporterUI::SetFromGeoReferencingSystemClicked)
                        .ToolTipText(FText::FromString("Set the CRS from the Geo Referencing System in the Level.\nA Geo Referencing System Actor has to be present and a valid projected CRS and Easting/Northing has to be set in the Geo Referencing System Actor"))
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(5)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Or try autodetect CRS"))
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(SButton)
                        .Text_Raw(this, &FVectorImporterUI::GetAnchorButtonText)
                        .HAlign(HAlign_Center)
                        .OnClicked_Raw(this, &FVectorImporterUI::CloseAnchorWindowClicked)
                        .ToolTipText(FText::FromString("Autodetect CRS.\nThis will take the origin from the location of the first point in the Vector Data and the CRS according to 'Project Settings -> Plugins -> Landscaping -> Projection Mode' and might not match your pre-existing Landscape's projection!\nThe origin can be moved with the LandscapingInfos Actor afterwards."))
                    ]
                ]
            ]
        ]
    ];
    
    TSharedPtr<SWindow> TopWindow = FSlateApplication::Get().GetActiveTopLevelWindow();
	if(TopWindow.IsValid())
	{
		//Add as Native
		FSlateApplication::Get().AddWindowAsNativeChild(AnchorWindow, TopWindow.ToSharedRef(), true);
	}
	else
	{
		//Default in case no top window
		FSlateApplication::Get().AddWindow(AnchorWindow);
	}
    CachedAnchorWindow = &AnchorWindow.Get();
    return FReply::Handled();
}

FReply FVectorImporterUI::CloseAnchorWindowClicked()
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
    CachedAnchorWindow->RequestDestroyWindow();
    return HandleSelectVectorData();
}

FReply FVectorImporterUI::SetFromGeoReferencingSystemClicked()
{
    bool bResult = GetGisFileManager()->ReadGeoReferencingSystem();
    if(!bResult)
    {
        return FReply::Handled();
    }
    CachedAnchorWindow->RequestDestroyWindow();
    return HandleSelectVectorData();
}


FReply FVectorImporterUI::OpenOptionsWindow()
{
    if(SelectedTypeOption.IsValid())
    {
        UpdateLayerList();
        if(SelectedTypeOption.Get()->Equals("Spline Mesh"))
        {
            Options->bLandscapeSplines = false;
            if(CachedOptionsActorWindow != nullptr)
            {
                CachedOptionsActorWindow->RequestDestroyWindow();
                CachedOptionsActorWindow = nullptr;
            }
            if(CachedOptionsPaintLayerWindow != nullptr)
            {
                CachedOptionsPaintLayerWindow->RequestDestroyWindow();
                CachedOptionsPaintLayerWindow = nullptr;
            }
            if(CachedOptionsLandscapeSplineWindow != nullptr)
            {
                CachedOptionsLandscapeSplineWindow->RequestDestroyWindow();
                CachedOptionsLandscapeSplineWindow = nullptr;
            }
            if(CachedOptionsSplineMeshWindow == nullptr)
            {
                return OpenOptionsSplineMeshWindow();
            }
        }
        else if(SelectedTypeOption.Get()->Equals("Actor / Blueprint"))
        {
            Options->bLandscapeSplines = false;
            if(CachedOptionsSplineMeshWindow != nullptr)
            {
                CachedOptionsSplineMeshWindow->RequestDestroyWindow();
                CachedOptionsSplineMeshWindow = nullptr;
            }
            if(CachedOptionsPaintLayerWindow != nullptr)
            {
                CachedOptionsPaintLayerWindow->RequestDestroyWindow();
                CachedOptionsPaintLayerWindow = nullptr;
            }
            if(CachedOptionsLandscapeSplineWindow != nullptr)
            {
                CachedOptionsLandscapeSplineWindow->RequestDestroyWindow();
                CachedOptionsLandscapeSplineWindow = nullptr;
            }
            if(CachedOptionsActorWindow == nullptr)
            {
                return OpenOptionsActorWindow();
            }
        }
        else if(SelectedTypeOption.Get()->Equals("Paint Layer / Deform Landscape"))
        {
            Options->bLandscapeSplines = false;
            if(CachedOptionsSplineMeshWindow != nullptr)
            {
                CachedOptionsSplineMeshWindow->RequestDestroyWindow();
                CachedOptionsSplineMeshWindow = nullptr;
            }
            if(CachedOptionsActorWindow != nullptr)
            {
                CachedOptionsActorWindow->RequestDestroyWindow();
                CachedOptionsActorWindow = nullptr;
            }
            if(CachedOptionsLandscapeSplineWindow != nullptr)
            {
                CachedOptionsLandscapeSplineWindow->RequestDestroyWindow();
                CachedOptionsLandscapeSplineWindow = nullptr;
            }
            if(CachedOptionsPaintLayerWindow == nullptr)
            {
                return OpenOptionsPaintLayerWindow();
            }
        }
        else if(SelectedTypeOption.Get()->Equals("Landscape Spline"))
        {
            Options->bLandscapeSplines = true;
            if(CachedOptionsSplineMeshWindow != nullptr)
            {
                CachedOptionsSplineMeshWindow->RequestDestroyWindow();
                CachedOptionsSplineMeshWindow = nullptr;
            }
            if(CachedOptionsActorWindow != nullptr)
            {
                CachedOptionsActorWindow->RequestDestroyWindow();
                CachedOptionsActorWindow = nullptr;
            }
            if(CachedOptionsPaintLayerWindow != nullptr)
            {
                CachedOptionsPaintLayerWindow->RequestDestroyWindow();
                CachedOptionsPaintLayerWindow = nullptr;
            }
            if(CachedOptionsLandscapeSplineWindow == nullptr)
            {
                return OpenOptionsLandscapeSplineWindow();
            }
        }
    }
    if(CachedOptionsSplineMeshWindow == nullptr && CachedOptionsActorWindow == nullptr && CachedOptionsPaintLayerWindow == nullptr && CachedOptionsLandscapeSplineWindow == nullptr)
    {
        FMessageDialog::Open(
            EAppMsgType::Ok, 
            FText::FromString("Initialization failed. Please close Landscaping tab and open it again!")
        );
    }
    return FReply::Handled();
}

FReply FVectorImporterUI::OpenOptionsSplineMeshWindow()
{
    TSharedRef<SWindow> SplineMeshWindow = SNew(SWindow)
    .Type(EWindowType::Normal)
    .Title(FText::FromString("Landscaping - Options for Spline Mesh"))
    .SupportsMaximize(false)
    .ClientSize(FVector2D(550, 650))
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
                    .Text(FText::FromString("Options for Spline Mesh"))
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
                        FPlatformProcess::LaunchURL(TEXT("https://jorop.github.io/landscaping-docs/#/props?id=props"), nullptr, nullptr);
                    })
                    .ToolTipText(FText::FromString("Documentation for Spline Mesh Creation"))
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
                        .FillHeight(10)
                        .VAlign(VAlign_Center)
                        .Padding(5)
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString("Spline Segment Mesh"))
                            .ToolTipText(FText::FromString("This mesh will be repeated along the spline"))
                        ]
                        + SVerticalBox::Slot()
                        .FillHeight(10)
                        .VAlign(VAlign_Center)
                        .Padding(5)
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString("Start Segment Mesh Scale"))
                            .ToolTipText(FText::FromString("At which scale a segment should start with"))
                        ]
                        + SVerticalBox::Slot()
                        .FillHeight(10)
                        .VAlign(VAlign_Center)
                        .Padding(5)
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString("End Segment Mesh Scale"))
                            .ToolTipText(FText::FromString("At which scale a segment should end with"))
                        ]
                        + SVerticalBox::Slot()
                        .FillHeight(10)
                        .VAlign(VAlign_Center)
                        .Padding(5)
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString("Start Index"))
                            .ToolTipText(FText::FromString("Start from this shape to create Splines.\n\nThe process can be repeated multiple times creating Splines in chunks.\nUse this, if the number of Splines slows down your machine."))
                        ]
                        + SVerticalBox::Slot()
                        .FillHeight(10)
                        .VAlign(VAlign_Center)
                        .Padding(5)
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString("Max Entities"))
                            .ToolTipText(FText::FromString("Number of Splines which will be created.\n\nOnly so much shapes will be processed beginning with Start Index.\nThe process can be repeated multiple times creating Splines in chunks.\nUse this, if the number of Splines slows down your machine.\n'0' means all beginning with Start Index."))
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
                            SNew(SSpinBox<int>)
                            .MinValue(0)
                            .MaxValue(30)
                            .MinSliderValue(0)
                            .MaxSliderValue(30)
                            .MinDesiredWidth(150)
                            .Value_Lambda([=]()
                            {
                                return Options->ZoomLevel;
                            })
                            .OnValueChanged_Lambda([=](int NewValue)
                            {
                                Options->ZoomLevel = NewValue;
                            })
                            .ToolTipText(FText::FromString("Mapbox Zoom Level for Vector data (only available with Landscaping Mapbox Extension)"))
                            .IsEnabled_Lambda([=]()
                            {
                                return GetGisFileManager()->HasLandscape() && GetGisFileManager()->HasMapboxExtension();
                            })
                        ]
                        + SVerticalBox::Slot()
                        .FillHeight(10)
                        .VAlign(VAlign_Center)
                        .Padding(5)
                        [
                            SNew(SObjectPropertyEntryBox)
                            .AllowedClass(UStaticMesh::StaticClass())
                            .ObjectPath_Lambda([this]()
                            {
                                return this->Options->SplineMesh != nullptr
                                        ? this->Options->SplineMesh->
                                                GetPathName()
                                        : FString();
                            })
                            .OnObjectChanged_Lambda([this](FAssetData Asset)
                            {
                                if(Asset.IsValid()) 
                                {
                                    this->Options->SplineMesh = Asset.GetAsset();
                                    this->Options->ActorOrBlueprintClass = nullptr;
                                }
                                else 
                                {
                                    this->Options->SplineMesh = nullptr;
                                }
                            })
                            .ToolTipText(FText::FromString("This mesh will be repeated along the spline"))
                            .IsEnabled_Lambda([=]()
                            {
                                return GetGisFileManager()->HasLandscape();
                            })
                        ]
                        + SVerticalBox::Slot()
                        .FillHeight(10)
                        .VAlign(VAlign_Center)
                        .Padding(5)
                        [
                            SNew(SSpinBox<float>)
                            .MinValue(1)
                            .MaxValue(10000)
                            .MinSliderValue(1)
                            .MaxSliderValue(10000)
                            .MinDesiredWidth(150)
                            .Value_Lambda([this]()
                            {
                                return this->Options->StartWidth;
                            })
                            .OnValueChanged_Lambda([this](float NewValue)
                            {
                                this->Options->StartWidth = NewValue;
                            })
                            .ToolTipText(FText::FromString("At which scale a segment should start with"))
                            .IsEnabled_Lambda([=]()
                            {
                                return Options->ActorOrBlueprintClass == nullptr && GetGisFileManager()->HasLandscape();
                            })
                        ]
                        + SVerticalBox::Slot()
                        .FillHeight(10)
                        .VAlign(VAlign_Center)
                        .Padding(5)
                        [
                            SNew(SSpinBox<float>)
                            .MinValue(1)
                            .MaxValue(10000)
                            .MinSliderValue(1)
                            .MaxSliderValue(10000)
                            .MinDesiredWidth(150)
                            .Value_Lambda([this]()
                            {
                                return this->Options->EndWidth;
                            })
                            .OnValueChanged_Lambda([this](float NewValue)
                            {
                                this->Options->EndWidth = NewValue;
                            })
                            .ToolTipText(FText::FromString("At which scale a segment should end with"))
                            .IsEnabled_Lambda([=]()
                            {
                                return Options->ActorOrBlueprintClass == nullptr && GetGisFileManager()->HasLandscape();
                            })
                        ]
                        + SVerticalBox::Slot()
                        .FillHeight(10)
                        .VAlign(VAlign_Center)
                        .Padding(5)
                        [
                            SNew(SSpinBox<int>)
                            .MinValue(0)
                            .MaxValue(1000)
                            .MinSliderValue(0)
                            .MaxSliderValue(1000)
                            .MinDesiredWidth(150)
                            .Value_Lambda([=]()
                            {
                                return Options->StartEntityIndex;
                            })
                            .OnValueChanged_Lambda([=](int NewValue)
                            {
                                Options->StartEntityIndex = NewValue;
                            })
                            .ToolTipText(FText::FromString("Start from this shape to create Splines.\n\nThe process can be repeated multiple times creating Splines in chunks.\nUse this, if the number of Splines slows down your machine."))
                            .IsEnabled_Lambda([=]()
                            {
                                return GetGisFileManager()->HasLandscape();
                            })
                        ]
                        + SVerticalBox::Slot()
                        .FillHeight(10)
                        .VAlign(VAlign_Center)
                        .Padding(5)
                        [
                            SNew(SSpinBox<int>)
                            .MinValue(0)
                            .MaxValue(1000)
                            .MinSliderValue(0)
                            .MaxSliderValue(1000)
                            .MinDesiredWidth(150)
                            .Value_Lambda([=]()
                            {
                                return Options->MaxEntities;
                            })
                            .OnValueChanged_Lambda([=](int NewValue)
                            {
                                Options->MaxEntities = NewValue;
                            })
                            .ToolTipText(FText::FromString("Number of Splines which will be created.\n\nOnly so much shapes will be processed beginning with Start Index.\nThe process can be repeated multiple times creating Splines in chunks.\nUse this, if the number of Splines slows down your machine.\n'0' means all beginning with Start Index."))
                            .IsEnabled_Lambda([=]()
                            {
                                return GetGisFileManager()->HasLandscape();
                            })
                        ]
                    ]
                ]
            ]
             + SVerticalBox::Slot()
            .AutoHeight()
            [
                PaintLayerOptions()
            ]
        ]
    ];
    
    TSharedPtr<SWindow> TopWindow = FSlateApplication::Get().GetActiveTopLevelWindow();
	if(TopWindow.IsValid())
	{
		//Add as Native
		FSlateApplication::Get().AddWindowAsNativeChild(SplineMeshWindow, TopWindow.ToSharedRef(), true);
	}
	else
	{
		//Default in case no top window
		FSlateApplication::Get().AddWindow(SplineMeshWindow);
	}
    SplineMeshWindow->SetOnWindowClosed(FOnWindowClosed::CreateRaw(this, &FVectorImporterUI::OnWindowClosed));
    CachedOptionsSplineMeshWindow = &SplineMeshWindow.Get();
    return FReply::Handled();
}


FReply FVectorImporterUI::OpenOptionsLandscapeSplineWindow()
{
    TSharedRef<SWindow> LandscapeSplineWindow = SNew(SWindow)
    .Type(EWindowType::Normal)
    .Title(FText::FromString("Landscaping - Options for Landscape Spline"))
    .SupportsMaximize(false)
    .ClientSize(FVector2D(550, 650))
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
                    .Text(FText::FromString("Options for Landscape Spline"))
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
                        FPlatformProcess::LaunchURL(TEXT("https://jorop.github.io/landscaping-docs/#/props?id=landscape-splines"), nullptr, nullptr);
                    })
                    .ToolTipText(FText::FromString("Documentation for Spline Mesh Creation"))
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
                        .FillHeight(10)
                        .VAlign(VAlign_Center)
                        .Padding(5)
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString("Spline Segment Mesh"))
                            .ToolTipText(FText::FromString("This mesh will be repeated along the spline"))
                        ]
                        + SVerticalBox::Slot()
                        .FillHeight(10)
                        .VAlign(VAlign_Center)
                        .Padding(5)
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString("Spline Mesh Scale"))
                            .ToolTipText(FText::FromString("Scale of Landscape Spline Mesh"))
                        ]
                        + SVerticalBox::Slot()
                        .FillHeight(10)
                        .VAlign(VAlign_Center)
                        .Padding(5)
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString("Start Index"))
                            .ToolTipText(FText::FromString("Start from this shape to create Splines.\n\nThe process can be repeated multiple times creating Splines in chunks.\nUse this, if the number of Splines slows down your machine."))
                        ]
                        + SVerticalBox::Slot()
                        .FillHeight(10)
                        .VAlign(VAlign_Center)
                        .Padding(5)
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString("Max Entities"))
                            .ToolTipText(FText::FromString("Number of Splines which will be created.\n\nOnly so much shapes will be processed beginning with Start Index.\nThe process can be repeated multiple times creating Splines in chunks.\nUse this, if the number of Splines slows down your machine.\n'0' means all beginning with Start Index."))
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
                            SNew(SObjectPropertyEntryBox)
                            .AllowedClass(UStaticMesh::StaticClass())
                            .ObjectPath_Lambda([this]()
                            {
                                return this->Options->SplineMesh != nullptr
                                        ? this->Options->SplineMesh->
                                                GetPathName()
                                        : FString();
                            })
                            .OnObjectChanged_Lambda([this](FAssetData Asset)
                            {
                                if(Asset.IsValid()) 
                                {
                                    this->Options->SplineMesh = Asset.GetAsset();
                                    this->Options->ActorOrBlueprintClass = nullptr;
                                }
                                else 
                                {
                                    this->Options->SplineMesh = nullptr;
                                }
                            })
                            .ToolTipText(FText::FromString("This mesh will be repeated along the spline"))
                            .IsEnabled_Lambda([=]()
                            {
                                return GetGisFileManager()->HasLandscape();
                            })
                        ]
                        // xyz scale
                        + SVerticalBox::Slot()
                        .FillHeight(10)
                        .VAlign(VAlign_Center)
                        .Padding(5)
                        [
                            SNew(SHorizontalBox)
                            + SHorizontalBox::Slot()
                            [
                                SNew(SSpinBox<float>)
                                .MinValue(0)
                                .MaxValue(100000)
                                .MinSliderValue(0)
                                .MaxSliderValue(100000)
                                .MinDesiredWidth(150)
                                .Value_Lambda([=]()
                                {
                                    return this->Options->Scale.X;
                                })
                                .OnValueChanged_Lambda([=](float NewValue)
                                {
                                    this->Options->Scale = FVector(NewValue, this->Options->Scale.Y, this->Options->Scale.Z);
                                })
                                .ToolTipText(FText::FromString("X Scale of Landscape Spline Mesh"))
                                .IsEnabled_Lambda([=]()
                                {
                                    return GetGisFileManager()->HasLandscape();
                                })
                            ]
                            + SHorizontalBox::Slot()
                            [
                                SNew(SSpinBox<float>)
                                .MinValue(0)
                                .MaxValue(100000)
                                .MinSliderValue(0)
                                .MaxSliderValue(100000)
                                .MinDesiredWidth(150)
                                .Value_Lambda([=]()
                                {
                                    return this->Options->Scale.Y;
                                })
                                .OnValueChanged_Lambda([=](float NewValue)
                                {
                                    this->Options->Scale = FVector(this->Options->Scale.X, NewValue, this->Options->Scale.Z);
                                })
                                .ToolTipText(FText::FromString("Y Scale of Landscape Spline Mesh"))
                                .IsEnabled_Lambda([=]()
                                {
                                    return GetGisFileManager()->HasLandscape();
                                })
                            ]
                            + SHorizontalBox::Slot()
                            [
                                SNew(SSpinBox<float>)
                                .MinValue(0)
                                .MaxValue(100000)
                                .MinSliderValue(0)
                                .MaxSliderValue(100000)
                                .MinDesiredWidth(150)
                                .Value_Lambda([=]()
                                {
                                    return this->Options->Scale.Z;
                                })
                                .OnValueChanged_Lambda([=](float NewValue)
                                {
                                    this->Options->Scale = FVector(this->Options->Scale.X, this->Options->Scale.Y, NewValue);
                                })
                                .ToolTipText(FText::FromString("Z Scale of Landscape Spline Mesh"))
                                .IsEnabled_Lambda([=]()
                                {
                                    return GetGisFileManager()->HasLandscape();
                                })
                            ]
                        ]
                        + SVerticalBox::Slot()
                        .FillHeight(10)
                        .VAlign(VAlign_Center)
                        .Padding(5)
                        [
                            SNew(SSpinBox<int>)
                            .MinValue(0)
                            .MaxValue(1000)
                            .MinSliderValue(0)
                            .MaxSliderValue(1000)
                            .MinDesiredWidth(150)
                            .Value_Lambda([=]()
                            {
                                return Options->StartEntityIndex;
                            })
                            .OnValueChanged_Lambda([=](int NewValue)
                            {
                                Options->StartEntityIndex = NewValue;
                            })
                            .ToolTipText(FText::FromString("Start from this shape to create Splines.\n\nThe process can be repeated multiple times creating Splines in chunks.\nUse this, if the number of Splines slows down your machine."))
                            .IsEnabled_Lambda([=]()
                            {
                                return GetGisFileManager()->HasLandscape();
                            })
                        ]
                        + SVerticalBox::Slot()
                        .FillHeight(10)
                        .VAlign(VAlign_Center)
                        .Padding(5)
                        [
                            SNew(SSpinBox<int>)
                            .MinValue(0)
                            .MaxValue(1000)
                            .MinSliderValue(0)
                            .MaxSliderValue(1000)
                            .MinDesiredWidth(150)
                            .Value_Lambda([=]()
                            {
                                return Options->MaxEntities;
                            })
                            .OnValueChanged_Lambda([=](int NewValue)
                            {
                                Options->MaxEntities = NewValue;
                            })
                            .ToolTipText(FText::FromString("Number of Splines which will be created.\n\nOnly so much shapes will be processed beginning with Start Index.\nThe process can be repeated multiple times creating Splines in chunks.\nUse this, if the number of Splines slows down your machine.\n'0' means all beginning with Start Index."))
                            .IsEnabled_Lambda([=]()
                            {
                                return GetGisFileManager()->HasLandscape();
                            })
                        ]
                    ]
                ]
            ]
             + SVerticalBox::Slot()
            .AutoHeight()
            [
                SplinesOptionsUI->LandscapeSplineOptions(Options, LabelColumnWidth, MaterialLayers)
            ]
        ]
    ];
    
    TSharedPtr<SWindow> TopWindow = FSlateApplication::Get().GetActiveTopLevelWindow();
	if(TopWindow.IsValid())
	{
		//Add as Native
		FSlateApplication::Get().AddWindowAsNativeChild(LandscapeSplineWindow, TopWindow.ToSharedRef(), true);
	}
	else
	{
		//Default in case no top window
		FSlateApplication::Get().AddWindow(LandscapeSplineWindow);
	}
    LandscapeSplineWindow->SetOnWindowClosed(FOnWindowClosed::CreateRaw(this, &FVectorImporterUI::OnWindowClosed));
    CachedOptionsLandscapeSplineWindow = &LandscapeSplineWindow.Get();
    return FReply::Handled();
}

FReply FVectorImporterUI::OpenOptionsActorWindow()
{
    TSharedRef<SWindow> ActorWindow = SNew(SWindow)
    .Type(EWindowType::Normal)
    .Title(FText::FromString("Landscaping - Options for Actor / Blueprint"))
    .SupportsMaximize(false)
    .ClientSize(FVector2D(550, 550))
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
                    .Text(FText::FromString("Options for Actor / Blueprint"))
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
                        FPlatformProcess::LaunchURL(TEXT("https://jorop.github.io/landscaping-docs/#/props?id=props"), nullptr, nullptr);
                    })
                    .ToolTipText(FText::FromString("Documentation for Actor Creation"))
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
                        .FillHeight(10)
                        .VAlign(VAlign_Center)
                        .Padding(5)
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString("Actor / Blueprint"))
                            .ToolTipText(FText::FromString("Actor / Blueprint for instantiating. For lines it has to have a spline component attached.\nFor points every Actor will do."))
                        ]
                        + SVerticalBox::Slot()
                        .FillHeight(10)
                        .VAlign(VAlign_Center)
                        .Padding(5)
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString("Scale"))
                            .ToolTipText(FText::FromString("Actor / Blueprint Scale"))
                        ]
                        + SVerticalBox::Slot()
                        .FillHeight(10)
                        .VAlign(VAlign_Center)
                        .Padding(5)
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString("Create Auxiliary Actor"))
                            .ToolTipText(FText::FromString("Create an Auxiliary Actor for every spawned Actor / Blueprint with Landscape manipulation options"))
                        ]
                        + SVerticalBox::Slot()
                        .FillHeight(10)
                        .VAlign(VAlign_Center)
                        .Padding(5)
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString("Start Index"))
                            .ToolTipText(FText::FromString("Start from this shape to create Actors/Blueprints.\n\nThe process can be repeated multiple times creating Actors/Blueprints in chunks.\nUse this, if the number of Actors/Blueprints slows down your machine."))
                        ]
                        + SVerticalBox::Slot()
                        .FillHeight(10)
                        .VAlign(VAlign_Center)
                        .Padding(5)
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString("Max Entities"))
                            .ToolTipText(FText::FromString("Number of Actors/Blueprints which will be created.\n\nOnly so much shapes will be processed beginning with Start Index.\nThe process can be repeated multiple times creating Actors/Blueprints in chunks.\nUse this, if the number of Actors/Blueprints slows down your machine.\n'0' means all beginning with Start Index."))
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
                            SNew(SClassPropertyEntryBox)
                            .MetaClass(AActor::StaticClass())
                            .SelectedClass_Lambda([this]()
                            {
                                return this->Options->ActorOrBlueprintClass;
                            })
                            .OnSetClass_Lambda([this](const UClass* NewClass)
                            {
                                if(NewClass != nullptr)
                                {
                                    UClass* ClassToAssign = StaticLoadClass(
                                        AActor::StaticClass(), nullptr,
                                        *NewClass->GetPathName());
                                    this->Options->ActorOrBlueprintClass = ClassToAssign;
                                    this->Options->SplineMesh = nullptr;
                                }
                                else
                                {
                                    this->Options->ActorOrBlueprintClass = nullptr;    
                                }
                            })
                            .ToolTipText(FText::FromString("Actor for instantiating. For lines it has to have a spline component attached.\nFor points every Actor will do."))
                            .IsEnabled_Lambda([=]()
                            {
                                return GetGisFileManager()->HasLandscape();
                            })
                        ]
                        + SVerticalBox::Slot()
                        .FillHeight(10)
                        .VAlign(VAlign_Center)
                        .Padding(5)
                        [
                            SNew(SHorizontalBox)
                            + SHorizontalBox::Slot()
                            [
                                SNew(SSpinBox<float>)
                                .MinValue(0)
                                .MaxValue(100000)
                                .MinSliderValue(0)
                                .MaxSliderValue(100000)
                                .MinDesiredWidth(150)
                                .Value_Lambda([=]()
                                {
                                    return this->Options->Scale.X;
                                })
                                .OnValueChanged_Lambda([=](float NewValue)
                                {
                                    this->Options->Scale = FVector(NewValue, this->Options->Scale.Y, this->Options->Scale.Z);
                                })
                                .ToolTipText(FText::FromString("X Scale of Actor / Blueprint"))
                                .IsEnabled_Lambda([=]()
                                {
                                    return GetGisFileManager()->HasLandscape();
                                })
                            ]
                            + SHorizontalBox::Slot()
                            [
                                SNew(SSpinBox<float>)
                                .MinValue(0)
                                .MaxValue(100000)
                                .MinSliderValue(0)
                                .MaxSliderValue(100000)
                                .MinDesiredWidth(150)
                                .Value_Lambda([=]()
                                {
                                    return this->Options->Scale.Y;
                                })
                                .OnValueChanged_Lambda([=](float NewValue)
                                {
                                    this->Options->Scale = FVector(this->Options->Scale.X, NewValue, this->Options->Scale.Z);
                                })
                                .ToolTipText(FText::FromString("Y Scale of Actor / Blueprint"))
                                .IsEnabled_Lambda([=]()
                                {
                                    return GetGisFileManager()->HasLandscape();
                                })
                            ]
                            + SHorizontalBox::Slot()
                            [
                                SNew(SSpinBox<float>)
                                .MinValue(0)
                                .MaxValue(100000)
                                .MinSliderValue(0)
                                .MaxSliderValue(100000)
                                .MinDesiredWidth(150)
                                .Value_Lambda([=]()
                                {
                                    return this->Options->Scale.Z;
                                })
                                .OnValueChanged_Lambda([=](float NewValue)
                                {
                                    this->Options->Scale = FVector(this->Options->Scale.X, this->Options->Scale.Y, NewValue);
                                })
                                .ToolTipText(FText::FromString("Z Scale of Actor / Blueprint"))
                                .IsEnabled_Lambda([=]()
                                {
                                    return GetGisFileManager()->HasLandscape();
                                })
                            ]
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
                                return Options->bSpawnAux
                                            ? ECheckBoxState::Checked
                                            : ECheckBoxState::Unchecked;
                            })
                            .OnCheckStateChanged_Lambda([=](ECheckBoxState NewState)
                            {
                                Options->bSpawnAux = (NewState == ECheckBoxState::Checked);
                            })
                            .ToolTipText(FText::FromString("Create an Auxiliary Actor for every spawned Actor / Blueprint with Landscape manipulation options"))
                            .IsEnabled_Lambda([=]()
                            {
                                return GetGisFileManager()->HasLandscape();
                            })
                        ]
                        + SVerticalBox::Slot()
                        .FillHeight(10)
                        .VAlign(VAlign_Center)
                        .Padding(5)
                        [
                            SNew(SSpinBox<int>)
                            .MinValue(0)
                            .MaxValue(1000)
                            .MinSliderValue(0)
                            .MaxSliderValue(1000)
                            .MinDesiredWidth(150)
                            .Value_Lambda([=]()
                            {
                                return Options->StartEntityIndex;
                            })
                            .OnValueChanged_Lambda([=](int NewValue)
                            {
                                Options->StartEntityIndex = NewValue;
                            })
                            .ToolTipText(FText::FromString("Start from this shape to create Actors/Blueprints.\n\nThe process can be repeated multiple times creating Actors/Blueprints in chunks.\nUse this, if the number of Actors/Blueprints slows down your machine."))
                            .IsEnabled_Lambda([=]()
                            {
                                return GetGisFileManager()->HasLandscape();
                            })
                        ]
                        + SVerticalBox::Slot()
                        .FillHeight(10)
                        .VAlign(VAlign_Center)
                        .Padding(5)
                        [
                            SNew(SSpinBox<int>)
                            .MinValue(0)
                            .MaxValue(1000)
                            .MinSliderValue(0)
                            .MaxSliderValue(1000)
                            .MinDesiredWidth(150)
                            .Value_Lambda([=]()
                            {
                                return Options->MaxEntities;
                            })
                            .OnValueChanged_Lambda([=](int NewValue)
                            {
                                Options->MaxEntities = NewValue;
                            })
                            .ToolTipText(FText::FromString("Number of Actors/Blueprints which will be created.\n\nOnly so much shapes will be processed beginning with Start Index.\nThe process can be repeated multiple times creating Actors/Blueprints in chunks.\nUse this, if the number of Actors/Blueprints slows down your machine.\n'0' means all beginning with Start Index."))
                            .IsEnabled_Lambda([=]()
                            {
                                return GetGisFileManager()->HasLandscape();
                            })
                        ]
                    ]
                ]
            ]
            + SVerticalBox::Slot()
            .AutoHeight()
            [
                PaintLayerOptions()
            ]
        ]
    ];
    
    TSharedPtr<SWindow> TopWindow = FSlateApplication::Get().GetActiveTopLevelWindow();
	if(TopWindow.IsValid())
	{
		//Add as Native
		FSlateApplication::Get().AddWindowAsNativeChild(ActorWindow, TopWindow.ToSharedRef(), true);
	}
	else
	{
		//Default in case no top window
		FSlateApplication::Get().AddWindow(ActorWindow);
	}
    ActorWindow->SetOnWindowClosed(FOnWindowClosed::CreateRaw(this, &FVectorImporterUI::OnWindowClosed));
    CachedOptionsActorWindow = &ActorWindow.Get();
    return FReply::Handled();
}

FReply FVectorImporterUI::OpenOptionsPaintLayerWindow()
{
    TSharedRef<SWindow> PaintWindow = SNew(SWindow)
    .Type(EWindowType::Normal)
    .Title(FText::FromString("Landscaping - Options for Actor / Blueprint"))
    .SupportsMaximize(false)
    .ClientSize(FVector2D(550, 400))
    [
        SNew(SBox)
        .Padding(FMargin(15, 10, 15, 0))
        [
            PaintLayerOptions()
        ]
    ];
    
    TSharedPtr<SWindow> TopWindow = FSlateApplication::Get().GetActiveTopLevelWindow();
	if(TopWindow.IsValid())
	{
		//Add as Native
		FSlateApplication::Get().AddWindowAsNativeChild(PaintWindow, TopWindow.ToSharedRef(), true);
	}
	else
	{
		//Default in case no top window
		FSlateApplication::Get().AddWindow(PaintWindow);
	}
    PaintWindow->SetOnWindowClosed(FOnWindowClosed::CreateRaw(this, &FVectorImporterUI::OnWindowClosed));
    CachedOptionsPaintLayerWindow = &PaintWindow.Get();
    return FReply::Handled();
}

TSharedRef<SVerticalBox> FVectorImporterUI::PaintLayerOptions()
{
    return SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .HAlign(HAlign_Left)
            .FillWidth(8)
            [
                SNew(STextBlock)
                .Text(FText::FromString("Options for Paint Layer / Deform Landscape"))
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
                    FPlatformProcess::LaunchURL(TEXT("https://jorop.github.io/landscaping-docs/#/props?id=props"), nullptr, nullptr);
                })
                .ToolTipText(FText::FromString("Documentation for Paint Layer / Deform Landscape Options"))
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
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Use Paint Layer / Deform Landscape Options"))
                        .ToolTipText(FText::FromString("Wheter to use Paint Layer / Deform Landscape Options or not"))
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Start Width"))
                        .ToolTipText(FText::FromString("Width of the spline at the start node, in Spline Component local space"))
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("End Width"))
                        .ToolTipText(FText::FromString("Width of the spline at the end node, in Spline Component local space"))
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Start Side Falloff"))
                        .ToolTipText(FText::FromString("Width of the falloff at either side of the spline at the start node, in Spline Component local space"))
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("End Side Falloff"))
                        .ToolTipText(FText::FromString("Width of the falloff at either side of the spline at the end node, in Spline Component local space"))
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Start Roll"))
                        .ToolTipText(FText::FromString("Roll applied to the spline at the start node, in degrees. 0 is flat"))
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("End Roll"))
                        .ToolTipText(FText::FromString("Roll applied to the spline at the end node, in degrees. 0 is flat"))
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Subdivisions"))
                        .ToolTipText(FText::FromString("Number of triangles to place along the spline when applying it to the landscape.\nHigher numbers give better results, but setting it too high will be slow and may cause artifacts"))
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Raise Heights"))
                        .ToolTipText(FText::FromString("Allow the landscape to be raised up to the level of the spline.\nIf both bRaiseHeights and bLowerHeights are false, no height modification of the landscape will be performed"))
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Lower Heights"))
                        .ToolTipText(FText::FromString("Allow the landscape to be lowered down to the level of the spline.\nIf both bRaiseHeights and bLowerHeights are false, no height modification of the landscape will be performed"))
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Paint Layer Name"))
                        .ToolTipText(FText::FromString("Name of the Paint Layer of the current Material which should be painted along the Spline.\nIf the LayerInfo does not exist, it will be created."))
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Edit Layer Name"))
                        .ToolTipText(FText::FromString("Name of the Landscape Edit Layer"))
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
                            return Options->LandscapeSplineOptions.bPaintMaterialLayer
                                        ? ECheckBoxState::Checked
                                        : ECheckBoxState::Unchecked;
                        })
                        .OnCheckStateChanged_Lambda([=](ECheckBoxState NewState)
                        {
                            Options->LandscapeSplineOptions.bPaintMaterialLayer = (NewState == ECheckBoxState::Checked);
                        })
                        .ToolTipText(FText::FromString("Wheter to apply Paint Layer / Deform Landscape Options below or not"))
                        .IsEnabled_Lambda([=]()
                        {
                            return !SelectedTypeOption.Get()->Equals("Paint Layer / Deform Landscape");
                        })
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(SSpinBox<float>)
                        .MinValue(1)
                        .MaxValue(10000)
                        .MinSliderValue(1)
                        .MaxSliderValue(10000)
                        .MinDesiredWidth(150)
                        .Value_Lambda([=]()
                        {
                            return Options->LandscapeSplineOptions.StartWidth;
                        })
                        .OnValueChanged_Lambda([=](float NewValue)
                        {
                            Options->LandscapeSplineOptions.StartWidth = NewValue;
                        })
                        .ToolTipText(FText::FromString("Start Width of the Area to be lowered or raised or painted"))
                        .IsEnabled_Lambda([=]()
                        {
                            return GetGisFileManager()->HasLandscape() && Options->LandscapeSplineOptions.bPaintMaterialLayer;
                        })
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(SSpinBox<float>)
                        .MinValue(1)
                        .MaxValue(10000)
                        .MinSliderValue(1)
                        .MaxSliderValue(10000)
                        .MinDesiredWidth(150)
                        .Value_Lambda([=]()
                        {
                            return Options->LandscapeSplineOptions.EndWidth;
                        })
                        .OnValueChanged_Lambda([=](float NewValue)
                        {
                            Options->LandscapeSplineOptions.EndWidth = NewValue;
                        })
                        .ToolTipText(FText::FromString("End Width of the Area to be lowered or raised or painted"))
                        .IsEnabled_Lambda([=]()
                        {
                            return GetGisFileManager()->HasLandscape() && Options->LandscapeSplineOptions.bPaintMaterialLayer;
                        })
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(SSpinBox<float>)
                        .MinValue(1)
                        .MaxValue(10000)
                        .MinSliderValue(1)
                        .MaxSliderValue(10000)
                        .MinDesiredWidth(150)
                        .Value_Lambda([=]()
                        {
                            return Options->LandscapeSplineOptions.StartSideFalloff;
                        })
                        .OnValueChanged_Lambda([=](float NewValue)
                        {
                            Options->LandscapeSplineOptions.StartSideFalloff = NewValue;
                        })
                        .ToolTipText(FText::FromString("Start Side Falloff of the Area to be lowered or raised or painted"))
                        .IsEnabled_Lambda([=]()
                        {
                            return GetGisFileManager()->HasLandscape() && Options->LandscapeSplineOptions.bPaintMaterialLayer;
                        })
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(SSpinBox<float>)
                        .MinValue(1)
                        .MaxValue(10000)
                        .MinSliderValue(1)
                        .MaxSliderValue(10000)
                        .MinDesiredWidth(150)
                        .Value_Lambda([=]()
                        {
                            return Options->LandscapeSplineOptions.EndSideFalloff;
                        })
                        .OnValueChanged_Lambda([=](float NewValue)
                        {
                            Options->LandscapeSplineOptions.EndSideFalloff = NewValue;
                        })
                        .ToolTipText(FText::FromString("End Side Falloff of the Area to be lowered or raised or painted"))
                        .IsEnabled_Lambda([=]()
                        {
                            return GetGisFileManager()->HasLandscape() && Options->LandscapeSplineOptions.bPaintMaterialLayer;
                        })
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(SSpinBox<float>)
                        .MinValue(1)
                        .MaxValue(10000)
                        .MinSliderValue(1)
                        .MaxSliderValue(10000)
                        .MinDesiredWidth(150)
                        .Value_Lambda([=]()
                        {
                            return Options->LandscapeSplineOptions.StartRoll;
                        })
                        .OnValueChanged_Lambda([=](float NewValue)
                        {
                            Options->LandscapeSplineOptions.StartRoll = NewValue;
                        })
                        .ToolTipText(FText::FromString("Start Roll of the Area to be lowered or raised or painted"))
                        .IsEnabled_Lambda([=]()
                        {
                            return GetGisFileManager()->HasLandscape() && Options->LandscapeSplineOptions.bPaintMaterialLayer;
                        })
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(SSpinBox<float>)
                        .MinValue(1)
                        .MaxValue(10000)
                        .MinSliderValue(1)
                        .MaxSliderValue(10000)
                        .MinDesiredWidth(150)
                        .Value_Lambda([=]()
                        {
                            return Options->LandscapeSplineOptions.EndRoll;
                        })
                        .OnValueChanged_Lambda([=](float NewValue)
                        {
                            Options->LandscapeSplineOptions.EndRoll = NewValue;
                        })
                        .ToolTipText(FText::FromString("End Roll of the Area to be lowered or raised or painted"))
                        .IsEnabled_Lambda([=]()
                        {
                            return GetGisFileManager()->HasLandscape() && Options->LandscapeSplineOptions.bPaintMaterialLayer;
                        })
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(SSpinBox<int>)
                        .MinValue(0)
                        .MaxValue(100)
                        .MinSliderValue(0)
                        .MaxSliderValue(100)
                        .MinDesiredWidth(150)
                        .Value_Lambda([=]()
                        {
                            return Options->LandscapeSplineOptions.NumSubdivisions;
                        })
                        .OnValueChanged_Lambda([=](int NewValue)
                        {
                            Options->LandscapeSplineOptions.NumSubdivisions = NewValue;
                        })
                        .ToolTipText(FText::FromString("Number of Subdivisions of the Area to be lowered or raised or painted"))
                        .IsEnabled_Lambda([=]()
                        {
                            return GetGisFileManager()->HasLandscape() && Options->LandscapeSplineOptions.bPaintMaterialLayer;
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
                            return Options->LandscapeSplineOptions.RaiseHeights
                                        ? ECheckBoxState::Checked
                                        : ECheckBoxState::Unchecked;
                        })
                        .OnCheckStateChanged_Lambda([=](ECheckBoxState NewState)
                        {
                            Options->LandscapeSplineOptions.RaiseHeights = (NewState == ECheckBoxState::Checked);
                        })
                        .ToolTipText(FText::FromString("Whether to raise the Area or not"))
                        .IsEnabled_Lambda([=]()
                        {
                            return GetGisFileManager()->HasLandscape() && Options->LandscapeSplineOptions.bPaintMaterialLayer;
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
                            return Options->LandscapeSplineOptions.LowerHeights
                                        ? ECheckBoxState::Checked
                                        : ECheckBoxState::Unchecked;
                        })
                        .OnCheckStateChanged_Lambda([=](ECheckBoxState NewState)
                        {
                            Options->LandscapeSplineOptions.LowerHeights = (NewState == ECheckBoxState::Checked);
                        })
                        .ToolTipText(FText::FromString("Whether to lower the Area or not"))
                        .IsEnabled_Lambda([=]()
                        {
                            return GetGisFileManager()->HasLandscape() && Options->LandscapeSplineOptions.bPaintMaterialLayer;
                        })
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(SComboBox<TSharedPtr<FString>>)
                        .OptionsSource(&MaterialLayers)
                        .InitiallySelectedItem(Options->PaintLayer)
                        [
                            SNew(STextBlock)
                            .Text_Lambda([=]()
                            {
                                if(Options->PaintLayer.IsValid())
                                {
                                    return FText::FromString(*Options->PaintLayer);
                                }
                                return FText::FromString("Landscape has no Material assigned");
                            })
                        ]
                        .OnSelectionChanged_Lambda([=](TSharedPtr<FString> NewValue, ESelectInfo::Type SelectionType)
                        {
                            Options->PaintLayer = NewValue;
                        })
                        .OnGenerateWidget_Lambda([=](TSharedPtr<FString> InOption)
                        {
                            return SNew(STextBlock).Text(FText::FromString(*InOption));
                        })
                        .ToolTipText(FText::FromString("Which Landscape Material Layer should be painted"))
                        .IsEnabled_Lambda([=]()
                        {
                            return GetGisFileManager()->HasLandscape() && Options->LandscapeSplineOptions.bPaintMaterialLayer;
                        })
                    ]
                     + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(SEditableTextBox)
                        .Text_Lambda([=]()
                        {
                            return FText::FromString(Options->LandscapeSplineOptions.EditLayerName.ToString());
                        })
                        .OnTextCommitted_Lambda([=](const FText& NewText, ETextCommit::Type InTextCommit)
                        {
                            Options->LandscapeSplineOptions.EditLayerName = FName(NewText.ToString());
                        })
                        .ToolTipText(FText::FromString("Edit Layer Name to be affected. Only relevant when Edit Layers are enabled."))
                        .IsEnabled_Lambda([=]()
                        {
                            return GetGisFileManager()->HasLandscape();
                        })
                    ]
                ]
            ]
        ];
}

// this does reset all layer/landuse mappings
void FVectorImporterUI::UpdateLayerList()
{
    TArray<FName> LayerNamesList = ALandscapeProxy::GetLayersFromMaterial(GetGisFileManager()->GetInfos()->Material);
    if(MaterialLayers.Num() > 0)
    {
        MaterialLayers.Empty();
    }
    for (const FName LayerName : LayerNamesList)
    {
        MaterialLayers.Add(MakeShareable(new FString(LayerName.ToString())));
    }
    if(MaterialLayers.Num() > 0)
    {
        Options->PaintLayer = MaterialLayers[0];
        GetGisFileManager()->MaterialLayers = MaterialLayers;
    }
}

void FVectorImporterUI::OnWindowClosed(const TSharedRef<SWindow>& InWindow)
{
    FString WindowTitle = InWindow->GetTitle().ToString();
    if(CachedOptionsSplineMeshWindow != nullptr && WindowTitle.Equals(CachedOptionsSplineMeshWindow->GetTitle().ToString()))
    {
        CachedOptionsSplineMeshWindow = nullptr;
    }
    else if(CachedOptionsActorWindow != nullptr && WindowTitle.Equals(CachedOptionsActorWindow->GetTitle().ToString()))
    {
        CachedOptionsActorWindow = nullptr;
    }
    else if(CachedOptionsPaintLayerWindow != nullptr && WindowTitle.Equals(CachedOptionsPaintLayerWindow->GetTitle().ToString()))
    {
        CachedOptionsPaintLayerWindow = nullptr;
    }
    else if(CachedOptionsLandscapeSplineWindow != nullptr && WindowTitle.Equals(CachedOptionsLandscapeSplineWindow->GetTitle().ToString()))
    {
        CachedOptionsLandscapeSplineWindow = nullptr;
    }
}

UGISFileManager* FVectorImporterUI::GetGisFileManager()
{
    if(GisFileManager == nullptr)
    {
        GisFileManager = GEditor->GetEditorSubsystem<UGISFileManager>();
    }
    return GisFileManager;
}