// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "ILandscapingDataSource.h"
#include "DefaultDataSource.generated.h"


UCLASS()
class UDefaultDataSource : public UObject, public ILandscapingDataSource
{
    GENERATED_BODY()
public:
    UDefaultDataSource();
    virtual bool IsValid() override;
    virtual void SetWorkingDir(FString Directory) override;
    virtual void SetExtents(double BottomLatitude, double LeftLongitude, double TopLatitude, double RightLongitude, int TileIndex = 0) override;
    virtual void FetchData(FLandscapingDataSourceDelegate InOnDataFetched, ELandscapingRequestDataType Type, int ZoomLevel) override;
};