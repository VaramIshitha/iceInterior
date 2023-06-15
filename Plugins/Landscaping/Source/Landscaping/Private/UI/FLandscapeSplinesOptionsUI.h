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

class FLandscapeSplinesOptionsUI
{

public:
    TSharedRef<SVerticalBox> LandscapeSplineOptions(VectorGeometrySpawnOptions* Options, float LabelColumnWidth, TArray<TSharedPtr<FString>> InMaterialLayers);
private:
    UGISFileManager* GisFileManager = nullptr;
    UGISFileManager* GetGisFileManager();
    TArray<TSharedPtr<FString>> MaterialLayers = TArray<TSharedPtr<FString>>();

};