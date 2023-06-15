// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Slate.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"

class FUserInterfaceParts
{
public:
    FUserInterfaceParts(){};
    TSharedRef<SVerticalBox> SHeader1(FString InHeader, FString InHeaderLabel);
};
