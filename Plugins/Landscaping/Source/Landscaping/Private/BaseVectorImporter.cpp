// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#include "BaseVectorImporter.h"

TArray<FString> BaseVectorImporter::GetFeatureClasses(bool bSort)
{
    if(bSort)
    {
        FeatureClasses.Sort();
    }
    return FeatureClasses;
}