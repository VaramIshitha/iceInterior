// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#include "FLandscapeSplinesOptionsUI.h"


TSharedRef<SVerticalBox> FLandscapeSplinesOptionsUI::LandscapeSplineOptions(VectorGeometrySpawnOptions* Options, float LabelColumnWidth, TArray<TSharedPtr<FString>> InMaterialLayers)
{
    MaterialLayers = InMaterialLayers;
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
                .Text(FText::FromString("Additional Options for Landscape Splines"))
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
                .ToolTipText(FText::FromString("Documentation for Landscape Splines"))
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
                        .Text(FText::FromString("Half Width"))
                        .ToolTipText(FText::FromString("Half-Width of the spline at a control point"))
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Layer Width Ratio"))
                        .ToolTipText(FText::FromString("Layer Width ratio of the spline at a control point"))
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Side Falloff"))
                        .ToolTipText(FText::FromString("Falloff at the sides of the spline at a control point"))
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Left Side Falloff Factor"))
                        .ToolTipText(FText::FromString("Falloff factor of the height on the left side of the spline"))
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Right Side Falloff Factor"))
                        .ToolTipText(FText::FromString("Falloff factor of the height on the right side of the spline"))
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Left Side Layer Falloff Factor"))
                        .ToolTipText(FText::FromString("Falloff factor of the paint layer on the left side of the spline"))
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Right Side Layer Falloff Factor"))
                        .ToolTipText(FText::FromString("Falloff factor of the paint layer on the right side of the spline"))
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("End Falloff"))
                        .ToolTipText(FText::FromString("Falloff at the start/end of the spline (if this point is a start or end point, otherwise ignored)"))
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Raise Heights"))
                        .ToolTipText(FText::FromString("Allow the landscape to be raised up to the level of the spline.\nIf both  Raise Heights and Lower Heights are false, no height modification of the landscape will be performed"))
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Lower Heights"))
                        .ToolTipText(FText::FromString("Allow the landscape to be lowered down to the level of the spline.\nIf both Raise Heights and Lower Heights are false, no height modification of the landscape will be performed"))
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(10)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Segment Mesh Offset"))
                        .ToolTipText(FText::FromString("Vertical offset of the spline segment mesh. Useful for a river's surface, among other things"))
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
                        SNew(SSpinBox<float>)
                        .MinValue(0)
                        .MaxValue(10000)
                        .MinSliderValue(0)
                        .MaxSliderValue(10000)
                        .MinDesiredWidth(150)
                        .Value_Lambda([=]()
                        {
                            return Options->LandscapeSplineOptions.ControlPointOptions.Width;
                        })
                        .OnValueChanged_Lambda([=](float NewValue)
                        {
                            Options->LandscapeSplineOptions.ControlPointOptions.Width = NewValue;
                        })
                        .ToolTipText(FText::FromString("Start Width of the Area to be lowered or raised or painted"))
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
                        .MinValue(0)
                        .MaxValue(10000)
                        .MinSliderValue(0)
                        .MaxSliderValue(10000)
                        .MinDesiredWidth(150)
                        .Value_Lambda([=]()
                        {
                            return Options->LandscapeSplineOptions.ControlPointOptions.LayerWidthRatio;
                        })
                        .OnValueChanged_Lambda([=](float NewValue)
                        {
                            Options->LandscapeSplineOptions.ControlPointOptions.LayerWidthRatio = NewValue;
                        })
                        .ToolTipText(FText::FromString("Layer Width ratio of the spline at a control point"))
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
                        .MinValue(0)
                        .MaxValue(10000)
                        .MinSliderValue(0)
                        .MaxSliderValue(10000)
                        .MinDesiredWidth(150)
                        .Value_Lambda([=]()
                        {
                            return Options->LandscapeSplineOptions.ControlPointOptions.SideFalloff;
                        })
                        .OnValueChanged_Lambda([=](float NewValue)
                        {
                            Options->LandscapeSplineOptions.ControlPointOptions.SideFalloff = NewValue;
                        })
                        .ToolTipText(FText::FromString("Falloff at the sides of the spline at a control point"))
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
                        .MinValue(0)
                        .MaxValue(1)
                        .MinSliderValue(0)
                        .MaxSliderValue(1)
                        .MinDesiredWidth(150)
                        .Value_Lambda([=]()
                        {
                            return Options->LandscapeSplineOptions.ControlPointOptions.LeftSideFalloffFactor;
                        })
                        .OnValueChanged_Lambda([=](float NewValue)
                        {
                            Options->LandscapeSplineOptions.ControlPointOptions.LeftSideFalloffFactor = NewValue;
                        })
                        .ToolTipText(FText::FromString("Falloff factor of the height on the left side of the spline"))
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
                        .MinValue(0)
                        .MaxValue(1)
                        .MinSliderValue(0)
                        .MaxSliderValue(1)
                        .MinDesiredWidth(150)
                        .Value_Lambda([=]()
                        {
                            return Options->LandscapeSplineOptions.ControlPointOptions.RightSideFalloffFactor;
                        })
                        .OnValueChanged_Lambda([=](float NewValue)
                        {
                            Options->LandscapeSplineOptions.ControlPointOptions.RightSideFalloffFactor = NewValue;
                        })
                        .ToolTipText(FText::FromString("Falloff factor of the height on the right side of the spline"))
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
                        .MinValue(0)
                        .MaxValue(1)
                        .MinSliderValue(0)
                        .MaxSliderValue(1)
                        .MinDesiredWidth(150)
                        .Value_Lambda([=]()
                        {
                            return Options->LandscapeSplineOptions.ControlPointOptions.LeftSideLayerFalloffFactor;
                        })
                        .OnValueChanged_Lambda([=](float NewValue)
                        {
                            Options->LandscapeSplineOptions.ControlPointOptions.LeftSideLayerFalloffFactor = NewValue;
                        })
                        .ToolTipText(FText::FromString("Falloff factor of the paint layer on the left side of the spline"))
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
                        .MinValue(0)
                        .MaxValue(1)
                        .MinSliderValue(0)
                        .MaxSliderValue(1)
                        .MinDesiredWidth(150)
                        .Value_Lambda([=]()
                        {
                            return Options->LandscapeSplineOptions.ControlPointOptions.RightSideLayerFalloffFactor;
                        })
                        .OnValueChanged_Lambda([=](float NewValue)
                        {
                            Options->LandscapeSplineOptions.ControlPointOptions.RightSideLayerFalloffFactor = NewValue;
                        })
                        .ToolTipText(FText::FromString("Falloff factor of the paint layer on the right side of the spline"))
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
                        .MinValue(0)
                        .MaxValue(1000)
                        .MinSliderValue(0)
                        .MaxSliderValue(1000)
                        .MinDesiredWidth(150)
                        .Value_Lambda([=]()
                        {
                            return Options->LandscapeSplineOptions.ControlPointOptions.EndFalloff;
                        })
                        .OnValueChanged_Lambda([=](float NewValue)
                        {
                            Options->LandscapeSplineOptions.ControlPointOptions.EndFalloff = NewValue;
                        })
                        .ToolTipText(FText::FromString("Falloff at the start/end of the spline (if this point is a start or end point, otherwise ignored)"))
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
                            return Options->LandscapeSplineOptions.RaiseHeights
                                        ? ECheckBoxState::Checked
                                        : ECheckBoxState::Unchecked;
                        })
                        .OnCheckStateChanged_Lambda([=](ECheckBoxState NewState)
                        {
                            Options->LandscapeSplineOptions.RaiseHeights = (NewState == ECheckBoxState::Checked);
                        })
                        .ToolTipText(FText::FromString("Whether to raise the Area along the spline or not"))
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
                            return Options->LandscapeSplineOptions.LowerHeights
                                        ? ECheckBoxState::Checked
                                        : ECheckBoxState::Unchecked;
                        })
                        .OnCheckStateChanged_Lambda([=](ECheckBoxState NewState)
                        {
                            Options->LandscapeSplineOptions.LowerHeights = (NewState == ECheckBoxState::Checked);
                        })
                        .ToolTipText(FText::FromString("Whether to lower the Area along the spline or not"))
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
                        .MinValue(0)
                        .MaxValue(10000)
                        .MinSliderValue(0)
                        .MaxSliderValue(10000)
                        .MinDesiredWidth(150)
                        .Value_Lambda([=]()
                        {
                            return Options->LandscapeSplineOptions.ControlPointOptions.SegmentMeshOffset;
                        })
                        .OnValueChanged_Lambda([=](float NewValue)
                        {
                            Options->LandscapeSplineOptions.ControlPointOptions.SegmentMeshOffset = NewValue;
                        })
                        .ToolTipText(FText::FromString("Vertical offset of the spline segment mesh. Useful for a river's surface, among other things"))
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
                            return GetGisFileManager()->HasLandscape();
                        })
                    ]
                ]
            ]
        ];
}

UGISFileManager* FLandscapeSplinesOptionsUI::GetGisFileManager()
{
    if(GisFileManager == nullptr)
    {
        GisFileManager = GEditor->GetEditorSubsystem<UGISFileManager>();
    }
    return GisFileManager;
}