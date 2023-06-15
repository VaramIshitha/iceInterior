// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "CoreTypes.h"
#include "ILandscapingDataSource.h"

class ILandscapingModuleInterface : public IModuleInterface
{

public:

    virtual ~ILandscapingModuleInterface()
    {

    }

    virtual ILandscapingDataSource* GetDataSource()
    {
        return nullptr;
    }

    virtual void ResetDataSource()
    {

    }
};