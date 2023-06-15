// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#include "DefaultdataSource.h"



UDefaultDataSource::UDefaultDataSource() : Super()
{

}

bool UDefaultDataSource::IsValid()
{
    return false;
}

void UDefaultDataSource::SetWorkingDir(FString Directory)
{
    UE_LOG(LogTemp, Log, TEXT("Landscaping: Set Working Directory for Default Data Source..."));
}

void UDefaultDataSource::SetExtents(double BottomLatitude, double LeftLongitude, double TopLatitude, double RightLongitude, int TileIndex) 
{ 
    UE_LOG(LogTemp, Log, TEXT("Landscaping: Set Extents for Default Data Source..."));
}

void UDefaultDataSource::FetchData(FLandscapingDataSourceDelegate InOnDataFetched, ELandscapingRequestDataType Type, int ZoomLevel = -1)
{
    UE_LOG(LogTemp, Log, TEXT("Landscaping: Fetch Data from Default Data Source..."));
}