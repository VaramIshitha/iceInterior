// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "CoreMinimal.h"

class BaseVectorImporter
{
public:
    TArray<FString> GetFeatureClasses(bool bSort = true);
protected:
    TArray<FString> FeatureClasses = TArray<FString>();
};