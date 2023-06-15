// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#include "FFoliageAutomationUI.h"


FFoliageAutomationUI::FFoliageAutomationUI(UGISFileManager* InFileManager)
{
    GisFileManager = InFileManager;
}

TSharedRef<SVerticalBox> FFoliageAutomationUI::FoliageAutomationUI()
{
    ThumbnailPool = MakeShareable(new FAssetThumbnailPool(16, false));
    FUserInterfaceParts* UserInterfaceParts = new FUserInterfaceParts();
    return SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            UserInterfaceParts->SHeader1("Create Foliage Spawners", "")
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
                .Text(FText::FromString("Automate Procedural Foliage Spawner"))
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
                    FPlatformProcess::LaunchURL(TEXT("https://jorop.github.io/landscaping-docs/#/vegetation?id=vegetation"), nullptr, nullptr);
                })
                .ToolTipText(FText::FromString("Documentation on how to automate Unreal\nProcedural Foliage Spawner for tiled Landscapes."))
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
                      .FillHeight(35)
                      .VAlign(VAlign_Center)
                      .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Foliage Spawner"))
                        .ToolTipText(FText::FromString("Select configured Procedural Foliage Spawner."))
                    ]
                    + SVerticalBox::Slot()
                      .FillHeight(10)
                      .VAlign(VAlign_Center)
                      .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Remove Existing Foliage Volumes"))
                        .ToolTipText(FText::FromString("Will remove existing Procedural Foliage Volumes and all Foliage with it"))
                    ]
                    + SVerticalBox::Slot()
                      .FillHeight(10)
                      .VAlign(VAlign_Center)
                      .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Update Existing Foliage Volumes"))
                        .ToolTipText(FText::FromString("Only update existing Procedural Foliage Volumes"))
                    ]
                    + SVerticalBox::Slot()
                      .FillHeight(10)
                      .VAlign(VAlign_Center)
                      .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Add New Foliage Volumes"))
                        .ToolTipText(FText::FromString("Add new Procedural Foliage Volumes"))
                    ]
                    + SVerticalBox::Slot()
                      .FillHeight(10)
                      .VAlign(VAlign_Center)
                      .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Use Streaming Proxy Bounds"))
                        .ToolTipText(FText::FromString("Use Streaming Proxy bounds when possible\nCould lead to unprecise bounds but allows for faster generation\nUse this on large Landscapes"))
                    ]
                    + SVerticalBox::Slot()
                      .FillHeight(10)
                      .VAlign(VAlign_Center)
                      .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Spawn Foliage"))
                        .ToolTipText(FText::FromString("Spawns foliage right after creating the Volume"))
                    ]
                    + SVerticalBox::Slot()
                      .FillHeight(10)
                      .VAlign(VAlign_Center)
                      .Padding(5)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Create Procedural Foliage Spawner"))
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
                    .FillHeight(35)
                    .VAlign(VAlign_Center)
                    .Padding(5)
                    [
                        SNew(SObjectPropertyEntryBox)
                        .AllowedClass(UProceduralFoliageSpawner::StaticClass())
                        .ObjectPath_Lambda([=]()
                        {
                            return Spawner != nullptr
                                    ? Spawner->GetPathName()
                                    : FString();
                        })
                        .OnObjectChanged_Lambda([=](FAssetData Asset)
                        {
                            UObject* NewSpawner = Asset.GetAsset();
                            Spawner = (UProceduralFoliageSpawner*)
                                NewSpawner;
                        })
                        .ThumbnailPool(ThumbnailPool)
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
                            return Options.bRemoveExistingFoliageVolumes
                                        ? ECheckBoxState::Checked
                                        : ECheckBoxState::Unchecked;
                        })
                        .OnCheckStateChanged_Lambda([=](ECheckBoxState NewState)
                        {
                            Options.bRemoveExistingFoliageVolumes = (NewState ==
                                ECheckBoxState::Checked);
                            if(Options.bRemoveExistingFoliageVolumes)
                            {
                                Options.bAddNewFoliageVolume = false;
                                Options.bReUseExistingFoliageVolume = false;
                                Options.bSpawnFoliage = false;
                            }
                        })
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
                            return Options.bReUseExistingFoliageVolume
                                        ? ECheckBoxState::Checked
                                        : ECheckBoxState::Unchecked;
                        })
                        .OnCheckStateChanged_Lambda([=](ECheckBoxState NewState)
                        {
                            Options.bReUseExistingFoliageVolume = (NewState ==
                                ECheckBoxState::Checked);
                            if(Options.bReUseExistingFoliageVolume)
                            {
                                Options.bRemoveExistingFoliageVolumes = false;
                            }
                        })
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
                            return Options.bAddNewFoliageVolume
                                        ? ECheckBoxState::Checked
                                        : ECheckBoxState::Unchecked;
                        })
                        .OnCheckStateChanged_Lambda([=](ECheckBoxState NewState)
                        {
                            Options.bAddNewFoliageVolume = (NewState == ECheckBoxState::Checked);
                            if(Options.bAddNewFoliageVolume)
                            {
                                Options.bRemoveExistingFoliageVolumes = false;
                            }
                        })
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
                            return Options.bUseLandscapeProxyBounds
                                        ? ECheckBoxState::Checked
                                        : ECheckBoxState::Unchecked;
                        })
                        .OnCheckStateChanged_Lambda([=](ECheckBoxState NewState)
                        {
                            Options.bUseLandscapeProxyBounds = (NewState == ECheckBoxState::Checked);
                        })
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
                            return Options.bSpawnFoliage
                                        ? ECheckBoxState::Checked
                                        : ECheckBoxState::Unchecked;
                        })
                        .OnCheckStateChanged_Lambda([=](ECheckBoxState NewState)
                        {
                            if(!Options.bRemoveExistingFoliageVolumes)
                            {
                                Options.bSpawnFoliage = (NewState == ECheckBoxState::Checked);
                            }
                        })
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
                        .HAlign(HAlign_Center)
                        .Text(FText::FromString("Create / Spawn"))
                        .OnClicked_Raw(this, &FFoliageAutomationUI::GenerateFoliageClicked)
                        .IsEnabled_Lambda([=]()
                        {
                            return GetGisFileManager()->HasLandscape() &&
                                (Options.bRemoveExistingFoliageVolumes || Options.bAddNewFoliageVolume || Options.bReUseExistingFoliageVolume);
                        })
                    ]
                ]
            ]
        ];
}

FReply FFoliageAutomationUI::GenerateFoliageClicked()
{
    if (Spawner == nullptr && Options.bAddNewFoliageVolume)
    {
        FMessageDialog::Open(EAppMsgType::Ok,
                             FText::FromString("Please assign a Procedural Foliage Spawner."));
        return FReply::Handled();
    }
    GetGisFileManager()->CreateFoliage(Spawner, Options);
    return FReply::Handled();
}

UGISFileManager* FFoliageAutomationUI::GetGisFileManager()
{
    if(GisFileManager == nullptr)
    {
        GisFileManager = GEditor->GetEditorSubsystem<UGISFileManager>();
    }
    return GisFileManager;
}
