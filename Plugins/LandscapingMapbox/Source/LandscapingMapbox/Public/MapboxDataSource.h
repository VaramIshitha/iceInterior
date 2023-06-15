// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "ImageUtils.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Interfaces/IHttpRequest.h"
#include "UObject/NoExportTypes.h"
#include "Misc/ScopedSlowTask.h"
#include "Misc/FileHelper.h"
#include "FileHelpers.h"
#include "ILandscapingDataSource.h"
#include "MapboxConverter.h"
#include "RasterData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "MapboxDataSource.generated.h"

#define MB_DIMX 256
#define MB_DIMY 256
#define MB_MINZOOM 0
#define MB_MAXZOOM 21
#define MB_MINLATITUDE -85.0511f
#define MB_MAXLATITUDE 85.0511f
#define MB_MINLONGITUDE -180.0f
#define MB_MAXLONGITUDE 180.0f

struct FMapboxRequestData
{
	ELandscapingRequestDataType DataType;
	int TileNumberX;
	int TileNumberY;
	RasterData RasterData;
};

UCLASS(BlueprintType, ClassGroup=LandscapingMapbox)
class UMapboxDataSource : public UObject, public ILandscapingDataSource
{
	GENERATED_BODY()
	
public:
	UMapboxDataSource();

	/** ILandscapingDataSource implementation */
	virtual bool LANDSCAPINGMAPBOX_API IsValid() override;
	virtual void LANDSCAPINGMAPBOX_API SetWorkingDir(FString Dir) override;
	virtual void LANDSCAPINGMAPBOX_API SetExtents(double BottomLatitude, double LeftLongitude, double TopLatitude, double RightLongitude, int TileIndex) override;
	virtual void LANDSCAPINGMAPBOX_API FetchData(FLandscapingDataSourceDelegate OnDataFetched, ELandscapingRequestDataType Type) override;

	FString MapboxApiKey = FString();
	double RequestedTopLatitude;
	double RequestedLeftLongitude;
	double RequestedBottomLatitude;
	double RequestedRightLongitutde;
	int RequestedZoom;
	int RequestedZoomSatellite;
	int RequestedZoomVector;
	FString SkuToken = FString();
	void Start(FString URL, FMapboxRequestData MapboxMetaData);
	
private:
	bool ValidateRequest(FString& OutError);
	void Request(double BottomLatitude, double LeftLongitude, double TopLatitude, double RightLongitude, ELandscapingRequestDataType Type);
	void HandleMapboxResponse(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FMapboxRequestData MapboxMetaData);
	void AbortWithError();
	void FinishResponse(RasterData Data);

	int TotalRequests = 0;
	bool bRequestFailed = false;
	int CompletedRequests = 0;
	TArray<RasterData> OutDatas = TArray<RasterData>();
	FString CacheWorkingDir;
	int64_t TokenExpiresAt = 0;
	int MapboxDimX = 256;
	int MapboxDimY = 256;
	int LandscapingTileIndex = 0;
};
