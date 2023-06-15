// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#include "MapboxDataSource.h"


UMapboxDataSource::UMapboxDataSource() : Super()
{
	ULandscapingMapboxSettings* MapboxSettings = GetMutableDefault<ULandscapingMapboxSettings>();
	MapboxApiKey = MapboxSettings->ApiKey;
	RequestedZoom = MapboxSettings->Zoom;
	RequestedZoomSatellite = MapboxSettings->ZoomSatellite;
	RequestedZoomVector = MapboxSettings->ZoomVector;
	// MapboxSettings->MapboxDS = this;
	if(MapboxApiKey.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("LandscapingMapbox: Please set a Mapbox API Key in Project Settings -> Plugins -> Landscaping Mapbox"));
	}
	
}

bool UMapboxDataSource::IsValid()
{
	return !MapboxApiKey.IsEmpty();
}

void UMapboxDataSource::SetWorkingDir(FString Dir)
{
	this->CacheWorkingDir = Dir;
}

void UMapboxDataSource::SetExtents(double BottomLatitude, double LeftLongitude, double TopLatitude, double RightLongitude, int TileIndex) 
{
	UE_LOG(LogTemp, Log, TEXT("LandscapingMapbox: Set Extents for Mapbox Data Source %f,%f,%f,%f - Tile: %i"), BottomLatitude, LeftLongitude, TopLatitude, RightLongitude, TileIndex);
	this->RequestedLeftLongitude = LeftLongitude;
	this->RequestedTopLatitude = TopLatitude;
	this->RequestedRightLongitutde = RightLongitude;
	this->RequestedBottomLatitude = BottomLatitude;
	this->LandscapingTileIndex = TileIndex;
}

void UMapboxDataSource::FetchData(FLandscapingDataSourceDelegate InOnDataFetched, ELandscapingRequestDataType Type)
{
	if(!(Type == ELandscapingRequestDataType::TERRAIN || Type == ELandscapingRequestDataType::SATELLITE || Type == ELandscapingRequestDataType::VECTOR))
	{
		UE_LOG(LogTemp, Error, TEXT("LandscapingMapbox: NOT IMPLEMENTED - only TERRAIN, SATELLITE and VECTOR can be requested through Landscaping Mapbox"));
		return;
	}
	ULandscapingMapboxSettings* Settings = GetMutableDefault<ULandscapingMapboxSettings>();
	this->RequestedZoom = Settings->Zoom;
	this->RequestedZoomVector = Settings->ZoomVector;
	this->RequestedZoomSatellite = Settings->ZoomSatellite;
	this->MapboxApiKey = Settings->ApiKey;
	UE_LOG(LogTemp, Log, TEXT("LandscapingMapbox: Fetch Data from Mapbox Data Source"));
	this->OnDataFetched = InOnDataFetched;
	this->Request(this->RequestedBottomLatitude, this->RequestedLeftLongitude, this->RequestedTopLatitude, this->RequestedRightLongitutde, Type);
}

bool UMapboxDataSource::ValidateRequest(FString& OutError)
{	
	
	if (this->RequestedZoom < MB_MINZOOM || this->RequestedZoom > MB_MAXZOOM)
	{
		OutError = FString::Printf(TEXT("RequestedZoom out of valid Range (%d-%d)"), MB_MINZOOM, MB_MAXZOOM);
		return false;
	}
	
	if (this->RequestedTopLatitude > MB_MAXLATITUDE)
	{
		OutError = FString::Printf(TEXT("Top Latitude value out of range (%f-%f)"), MB_MINLATITUDE, MB_MAXLATITUDE);
		return false;
	}

	if (this->RequestedBottomLatitude < MB_MINLATITUDE)
	{
		OutError = FString::Printf(TEXT("Bottom Latitude value out of range (%f-%f)"), MB_MINLATITUDE, MB_MAXLATITUDE);
		return false;
	}
	
	if (this->RequestedRightLongitutde > MB_MAXLONGITUDE)
	{
		OutError = FString::Printf(TEXT("Right Longitude value out of range (%f-%f)"), MB_MINLONGITUDE, MB_MAXLONGITUDE);
		return false;
	}

	if (this->RequestedLeftLongitude < MB_MINLONGITUDE)
	{
		OutError = FString::Printf(TEXT("Left Longitude value out of range (%f-%f)"), MB_MINLONGITUDE, MB_MAXLONGITUDE);
		return false;
	}
	
	return true;
}

void UMapboxDataSource::Request(double Bottom, double Left, double Top, double Right, ELandscapingRequestDataType Type)
{
	this->bRequestFailed = false;
	this->TotalRequests = 0;
	this->CompletedRequests = 0;

	FString ValidationError;
	if (!this->ValidateRequest(ValidationError))
	{
		RasterData Data = RasterData();
		Data.Error = ValidationError;
		this->OutDatas.Add(Data);
		this->OnDataFetched.Broadcast(OutDatas, this->LandscapingTileIndex);
		return;
	}
	
	UMapboxDataSource* RequestTask = this;
	
	FString Tileset = TEXT("mapbox.terrain-rgb");
	FString Format = TEXT(".pngraw");
	MapboxDimX = MB_DIMX;
	MapboxDimY = MB_DIMY;
	ULandscapingMapboxSettings* Settings = GetMutableDefault<ULandscapingMapboxSettings>();
	if(Settings->HeightDataAPI == EMapboxHeightData::MapboxTerrainDEMv1)
	{
		Tileset = TEXT("mapbox.mapbox-terrain-dem-v1");
		Format = TEXT("@2x.pngraw");
		RequestTask->RequestedZoom = this->RequestedZoom > 14 ? 14 : this->RequestedZoom;
		MapboxDimX = 2 * MB_DIMX;
		MapboxDimY = 2 * MB_DIMY;
	}

	switch(Type)
	{
		case ELandscapingRequestDataType::SATELLITE:
		{
			Tileset = TEXT("mapbox.satellite");
			Format = TEXT(".jpg90");
			RequestTask->RequestedZoom = this->RequestedZoomSatellite;
			MapboxDimX = MB_DIMX;
			MapboxDimY = MB_DIMY;
			break;
		}
		case ELandscapingRequestDataType::VECTOR:
		{
			Tileset = FString("mapbox.mapbox-streets-v8");
			Format = TEXT(".vector.pbf");
			RequestTask->RequestedZoom = this->RequestedZoomVector;
			MapboxDimX = MB_DIMX;
			MapboxDimY = MB_DIMY;
			break;
		}
		default:
		{
			break;
		}
	}

	int maxy = FMapboxConverter::LatToTileY(Bottom, RequestTask->RequestedZoom);
	int minx = FMapboxConverter::LonToTileX(Left, RequestTask->RequestedZoom);
	int miny = FMapboxConverter::LatToTileY(Top, RequestTask->RequestedZoom);
	int maxx = FMapboxConverter::LonToTileX(Right, RequestTask->RequestedZoom);
	if(Type == ELandscapingRequestDataType::SATELLITE || Type == ELandscapingRequestDataType::VECTOR)
	{
		maxy++;
		minx = FMath::Max(0, minx - 1);
		miny = FMath::Max(0, miny - 1);
		maxx++;
	}
	double BottomR = FMapboxConverter::TileYToLat(maxy, RequestTask->RequestedZoom);
	double LeftR = FMapboxConverter::TileXToLon(minx, RequestTask->RequestedZoom);
	double RightR = FMapboxConverter::TileXToLon(maxx, RequestTask->RequestedZoom);
	double TopR = FMapboxConverter::TileYToLat(miny, RequestTask->RequestedZoom);

	//UE_LOG(LogTemp, Log, TEXT("LandscapingMapbox: Slippy tiles extents from %f,%f,%f,%f got us %f,%f,%f,%f"), Bottom, Left, Top, Right, BottomR, LeftR, TopR, RightR);
	//UE_LOG(LogTemp, Log, TEXT("LandscapingMapbox: minx: %i maxx: %i miny: %i maxy: %i zoom: %i"), minx, maxx, miny, maxy, RequestTask->RequestedZoom);

	this->OutDatas.Empty();
	this->TotalRequests = (maxx + 1 - minx) * (maxy + 1 - miny);
	if(this->TotalRequests > 0)
	{
		FScopedSlowTask MapboxSlowTask(this->TotalRequests, FText::FromString("Requesting Mapbox Tiles"));
		MapboxSlowTask.MakeDialog();
		if(Settings->TileDownloadWarnLimit > 0 && this->TotalRequests > Settings->TileDownloadWarnLimit)
		{
			FString InfoMsg = FString::Printf(TEXT("With zoom-level %i you are going to import %i tiles from Mapbox.\n\nAre you sure?"), RequestTask->RequestedZoom, this->TotalRequests);
			EAppReturnType::Type Answer = FMessageDialog::Open(EAppMsgType::OkCancel, FText::FromString(*InfoMsg));
			if(Answer == EAppReturnType::Cancel)
			{
				MapboxSlowTask.Destroy();
				return;
			}
		}
		for (int x = minx; x <= maxx; x++)
		{
			for (int y = miny; y <= maxy; y++)
			{
				MapboxSlowTask.EnterProgressFrame(1.0, FText::FromString(FString::Printf(TEXT("Mapbox Requests %i / %i"), this->CompletedRequests + 1, this->TotalRequests)));
				RasterData Data = RasterData();
				if(Type == ELandscapingRequestDataType::VECTOR)
				{
					Data.Filename = FString::Printf(TEXT("%s/x%iy%iz%i.pbf"), *this->CacheWorkingDir, x, y, RequestTask->RequestedZoom);
				}
				else
				{
					FString Postfix = Type == ELandscapingRequestDataType::SATELLITE ? "_sat" : "";
					Data.Filename = FString::Printf(TEXT("%s/x%iy%iz%i%s.tif"), *this->CacheWorkingDir, x, y, RequestTask->RequestedZoom, *Postfix);
				}
				Data.ImportResolution.X = MapboxDimX;
				Data.ImportResolution.Y = MapboxDimY;
				Data.Projection = "EPSG:4326";
				Data.Bottom = FMapboxConverter::TileYToLat(y + 1, RequestTask->RequestedZoom);
				Data.Left = FMapboxConverter::TileXToLon(x, RequestTask->RequestedZoom);
				Data.Top = FMapboxConverter::TileYToLat(y, RequestTask->RequestedZoom);
				Data.Right = FMapboxConverter::TileXToLon(x + 1, RequestTask->RequestedZoom);
				Data.MeterPerPixel.X = (Data.Right - Data.Left) / Data.ImportResolution.X;
				Data.MeterPerPixel.Y = (Data.Bottom - Data.Top) / Data.ImportResolution.Y;
				Data.bMapboxImport = true;
				Data.BandCount = Type == ELandscapingRequestDataType::SATELLITE ? 3 : 1;
				FMapboxRequestData RequestData = FMapboxRequestData();
				RequestData.DataType = Type;
				RequestData.TileNumberX = x;
				RequestData.TileNumberY = y;
				RequestData.RasterData = Data;
				if(!FPaths::FileExists(Data.Filename))
				{
					FString RequestURL = FString::Printf(TEXT("https://api.mapbox.com/v4/%s/%d/%d/%d%s?access_token=%s"), 
						*Tileset, RequestTask->RequestedZoom, x, y, *Format, *this->MapboxApiKey);
					RequestTask->Start(RequestURL, RequestData);
				}
				else
				{
					FinishResponse(Data);
				}
			}
		}
	}
	else
	{
		RasterData Data = RasterData();
		Data.Error = "No Requests launched - Please check your Map Bounding Box!";
		this->OutDatas.Add(Data);
		this->OnDataFetched.Broadcast(OutDatas, this->LandscapingTileIndex);
	}
}

void UMapboxDataSource::Start(FString URL, FMapboxRequestData MapboxMetaData)
{
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION == 25
	TSharedRef<IHttpRequest, ESPMode::NotThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
#else 
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
#endif
	// UE_LOG(LogTemp, Log, TEXT("LandscapingMapbox: Sent Request: %s"), *URL);
	
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UMapboxDataSource::HandleMapboxResponse, MapboxMetaData);
	HttpRequest->SetURL(URL);
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->ProcessRequest();
}

void UMapboxDataSource::HandleMapboxResponse(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FMapboxRequestData MapboxMetaData)
{
	if (bSucceeded && HttpResponse.IsValid() && HttpResponse->GetContentLength() > 0)
	{
		FScopedSlowTask MapboxSlowTask(2.0, FText::FromString(FString::Printf(TEXT("Finishing Request #%i"), this->CompletedRequests + 1)));
		MapboxSlowTask.MakeDialog();
		MapboxSlowTask.EnterProgressFrame(1.0);
		FString ContentType = HttpResponse->GetContentType();
		if(ContentType.Contains("application/json"))
		{
			UE_LOG(LogTemp, Error, TEXT("LandscapingMapbox: Could not retrieve expected data from Mapbox: %s"), *HttpResponse->GetContentAsString());
			MapboxSlowTask.EnterProgressFrame(1.0);
			return AbortWithError();
		}

		const TArray<uint8>& Content = HttpResponse->GetContent();
		if(MapboxMetaData.DataType == ELandscapingRequestDataType::VECTOR)
		{
			if(FFileHelper::SaveArrayToFile(Content, *MapboxMetaData.RasterData.Filename))
			{
				MapboxSlowTask.EnterProgressFrame(1.0);
				return FinishResponse(MapboxMetaData.RasterData);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("LandscapingMapbox: Could not save vector file from Mapbox: %s"), *MapboxMetaData.RasterData.Filename);
				MapboxSlowTask.EnterProgressFrame(1.0);
				return AbortWithError();
			}
		}

		static const FName MODULE_IMAGE_WRAPPER("ImageWrapper");
		IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(MODULE_IMAGE_WRAPPER);

		EImageFormat ImageFormat = ImageWrapperModule.DetectImageFormat(Content.GetData(), Content.Num());
		bool bIsValid = true;
		if (ImageFormat == EImageFormat::Invalid)
		{
			
			UE_LOG(LogTemp, Error, TEXT("LandscapingMapbox: Image Download: Could not recognize file type of image downloaded from Mapbox, server-reported content type: %s"), *ContentType);
			MapboxSlowTask.EnterProgressFrame(1.0);
			return AbortWithError();
		}

		TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);
		if (!ImageWrapper.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("LandscapingMapbox: Image Download: Unable to make image wrapper for image format %d"), (int32)ImageFormat);
			MapboxSlowTask.EnterProgressFrame(1.0);
			return AbortWithError();
		}

		if (!ImageWrapper->SetCompressed(Content.GetData(), Content.Num()))
		{
			UE_LOG(LogTemp, Error, TEXT("LandscapingMapbox: Image Download: Unable to parse image format %d from Mapbox"), (int32)ImageFormat);
			MapboxSlowTask.EnterProgressFrame(1.0);
			return AbortWithError();
		}

		// UE_LOG(LogTemp, Log, TEXT("LandscapingMapbox: Image Download of image format %d"), (int32)ImageFormat);

		TArray<uint8> RawImageData;
		if (!ImageWrapper->GetRaw(ERGBFormat::RGBA, 8, RawImageData))
		{
			UE_LOG(LogTemp, Error, TEXT("LandscapingMapbox: Image Download: Unable to convert image format %d to BGRA 8"), (int32)ImageFormat);
			MapboxSlowTask.EnterProgressFrame(1.0);
			return AbortWithError();
		}
		
		// UE_LOG(LogTemp, Log, TEXT("LandscapingMapbox: Completed Request, total resolved: %d"), this->CompletedRequests);

		MapboxMetaData.RasterData.NoDataValue = -99999;
		uint64 DataIndex = 0;
		uint64 DataNum = RawImageData.Num();
		if(DataNum < MapboxDimX * MapboxDimY)
		{
			UE_LOG(LogTemp, Error, TEXT("LandscapingMapbox: Data does not match (%i), (Rows: %i Columns: %i)"), DataNum, MapboxDimY, MapboxDimX);
			MapboxSlowTask.EnterProgressFrame(1.0);
			return AbortWithError();
		}
		TArray<FColor> ColorBand;
		for(int Row = 0; Row < MapboxDimX; Row++)
		{
			if(MapboxMetaData.DataType == ELandscapingRequestDataType::TERRAIN)
			{
				MapboxMetaData.RasterData.RasterBandData.Add(TArray<double>());
			}
			else if(MapboxMetaData.DataType == ELandscapingRequestDataType::SATELLITE)
			{
				MapboxMetaData.RasterData.ColorBandData.Add(TArray<FColor>());
			}
			for(int Column = 0; Column < MapboxDimY; Column++)
			{
				const FColor* Source = &((FColor*)(RawImageData.GetData()))[DataIndex];
				if(MapboxMetaData.DataType == ELandscapingRequestDataType::TERRAIN)
				{	
					// according to https://docs.mapbox.com/MapboxMetaData/tilesets/guides/access-elevation-MapboxMetaData/
					// elevation = -10000 + (({R} * 256 * 256 + {G} * 256 + {B}) * 0.1)
					// note: in the FColor we deal with BGR (so we have to swap R and B)
					double Height = DataIndex < DataNum ? -10000.0 + ((Source->B * 256 * 256 + Source->G * 256 + Source->R) * 0.1) : MapboxMetaData.RasterData.NoDataValue;
					MapboxMetaData.RasterData.RasterBandData[Row].Add(Height);
				}
				else if(MapboxMetaData.DataType == ELandscapingRequestDataType::SATELLITE)
				{
					MapboxMetaData.RasterData.ColorBandData[Row].Add(FColor(Source->B, Source->G, Source->R, Source->A));
					ColorBand.Add(FColor(Source->B, Source->G, Source->R, Source->A));
				}
				DataIndex++;
			}
		}
		
		MapboxSlowTask.EnterProgressFrame(1.0);
		FinishResponse(MapboxMetaData.RasterData);
		return;
	}
	
	if (!bSucceeded)
	{
		AbortWithError();
	}
}

void UMapboxDataSource::AbortWithError()
{
	this->CompletedRequests++;
	if (this->CompletedRequests >= this->TotalRequests)
	{
		this->OnDataFetched.Broadcast(OutDatas, this->LandscapingTileIndex);
	}
}

void UMapboxDataSource::FinishResponse(RasterData Data)
{
	this->CompletedRequests++;
	this->OutDatas.Add(Data);
	if (this->CompletedRequests >= this->TotalRequests)
	{
		UE_LOG(LogTemp, Log, TEXT("LandscapingMapbox: Requests done"));
		this->OnDataFetched.Broadcast(OutDatas, this->LandscapingTileIndex);
	}
}
