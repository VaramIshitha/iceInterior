// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "RasterData.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FLandscapingDataSourceDelegate, TArray<RasterData>&, int);

enum class ELandscapingRequestDataType
{
	SATELLITE,
	TERRAIN,
    VECTOR
};

class ILandscapingDataSource
{
public:
    virtual ~ILandscapingDataSource()
    {

    }

    virtual bool IsValid()
    {
        return false;
    }

    virtual void SetWorkingDir(FString Directory)
    {
        
    }

    virtual void SetExtents(double LeftLongitude, double TopLatitude, double RightLongitude, double BottomLatitude, int TileIndex = 0) 
    {

    }
    
    virtual void FetchData(FLandscapingDataSourceDelegate InOnDataFetched, ELandscapingRequestDataType Type)
    {

    }

    FLandscapingDataSourceDelegate OnDataFetched;
};