// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#include "RasterFile.h"

RasterFile::RasterFile(FString RasterFilename)
{
    Filename = RasterFilename;
    GDALAllRegister();
    // UE_LOG(LogTemp, Log, TEXT("Landscaping: Opening raster file %s"), *RasterFilename);
    RasterDataset = (GDALDataset*)GDALOpen(TCHAR_TO_ANSI(*Filename), GA_Update);
    if(IsValid())
    {
        Init();
    }
    // else
    // {
    //     UE_LOG(LogTemp, Error, TEXT("Landscaping: Opening raster file %s failed"), *RasterFilename);
    // }
}

RasterFile::RasterFile(GDALDataset* Dataset)
{
    RasterDataset = Dataset;
    if(IsValid())
    {
        Init();
    }
}

RasterFile::RasterFile(RasterData InData)
{
    CreateNew(InData);
    if(IsValid())
    {
        Init();
    }
}

RasterFile::~RasterFile()
{
    if(IsValid())
    {
        GDALClose(RasterDataset);
    }
}

void RasterFile::Init()
{
    NumberOfRows = GDALGetRasterYSize(RasterDataset);
    NumberOfColumns = GDALGetRasterXSize(RasterDataset);
    NumberOfBands = GDALGetRasterCount(RasterDataset);
    GetGeoTransform();
    OriginX = GeoTransform[0];
    OriginY = GeoTransform[3];
    MeterPerPixelX = GeoTransform[1];
    MeterPerPixelY = GeoTransform[5];
    // UE_LOG(LogTemp, Log, TEXT("Landscaping: Init RasterFile\n%s"), *GetInfo());
}

bool RasterFile::IsValid()
{
    return RasterDataset != nullptr;
}

FString RasterFile::GetInfo()
{
    return FString(GDALInfo(RasterDataset, NULL));
}

const char* RasterFile::GetProjection()
{
    if(RasterDataset == nullptr)
    {
        return "";
    }
    return RasterDataset->GetProjectionRef();
}

double* RasterFile::GetMinAndMaxAltitude(int LayerIndex, FString &Error)
{    
    double* MinMax = new double[2];
    RasterDataset->GetRasterBand(LayerIndex)->GetMetadata();
    MinMax[0] = RasterDataset->GetRasterBand(LayerIndex)->GetMinimum();
    MinMax[1] = RasterDataset->GetRasterBand(LayerIndex)->GetMaximum();
    if((MinMax[0] == 0 && MinMax[1] == 0) || MinMax[0] < MIN_ALTITUDE || MinMax[0] > MAX_ALTITUDE || MinMax[1] < MIN_ALTITUDE || MinMax[1] > MAX_ALTITUDE)
    {
        RasterDataset->GetRasterBand(LayerIndex)->ComputeStatistics(FALSE, &MinMax[0], &MinMax[1], nullptr, nullptr, nullptr, nullptr);
    }
    RasterDataset->GetRasterBand(LayerIndex)->GetMetadata();
    if(MinMax[0] < MIN_ALTITUDE || MinMax[0] > MAX_ALTITUDE || MinMax[1] < MIN_ALTITUDE || MinMax[1] > MAX_ALTITUDE)
    {
        RasterDataset->GetRasterBand(LayerIndex)->ComputeRasterMinMax(FALSE, MinMax);
    }
    if(MinMax[0] < MIN_ALTITUDE || MinMax[0] > MAX_ALTITUDE || MinMax[1] < MIN_ALTITUDE || MinMax[1] > MAX_ALTITUDE)
    {
        Error = FString::Printf(TEXT("Min (%f) and Max (%f) Altitude cannot be calculated"), MinMax[0], MinMax[1]);
        return MinMax;
    }
    return MinMax;
}

double* RasterFile::GetGeoTransform()
{
    // In a north up image, GeoTransform[1] is the pixel width, and GeoTransform[5] is the pixel height.
    // The upper left corner of the upper left pixel is at position (GeoTransform[0],GeoTransform[3]).
    // The default transform is (0,1,0,0,0,1) and should be returned even when a CE_Failure error is returned,
    // such as for formats that don’t support transformation to projection coordinates.
    RasterDataset->GetGeoTransform(GeoTransform);
    return GeoTransform;
}

FExtents RasterFile::GetExtents()
{
    Init();
    double Left = OriginX;
    double Top = OriginY;
    double Right = OriginX + MeterPerPixelX * NumberOfColumns;
    double Bottom = OriginY + MeterPerPixelY * NumberOfRows;
    return FExtents(Bottom, Left, Top, Right);
}

double RasterFile::GetNoDataValue()
{
    int Success = 0;
    double NoDataValueFetched = (double)RasterDataset->GetRasterBand(1)->GetNoDataValue(&Success);
    return Success ? NoDataValueFetched : 0;
}

int* RasterFile::GetDimensions()
{
    Dimensions[0] = NumberOfRows;
    Dimensions[1] = NumberOfColumns;
    Dimensions[2] = NumberOfBands;
    return Dimensions;
}

void RasterFile::SetColorInterpretation(int LayerIndex, GDALColorInterp BandInterp)
{
    RasterDataset->GetRasterBand(LayerIndex)->SetColorInterpretation(BandInterp);
}

TArray<TArray<FColor>> RasterFile::GetColorBand()
{
    TArray<TArray<double>> R = GetRasterBand(1);
    TArray<TArray<double>> G = GetRasterBand(2);
    TArray<TArray<double>> B = GetRasterBand(3);
    TArray<TArray<FColor>> RasterColorBand;
    for(int y = 0; y < NumberOfRows; y++)
    {
        RasterColorBand.Add(TArray<FColor>());
        for(int x = 0; x < NumberOfColumns; x++)
        {
            FColor Color = FColor(R[y][x], G[y][x], B[y][x]);
            RasterColorBand[y].Add(Color);
        }
    }
    return RasterColorBand;
}

TArray<TArray<double>> RasterFile::GetRasterBand(int LayerIndex)
{
    TArray<TArray<double>> BandLayer = TArray<TArray<double>>();
    if(RasterDataset == nullptr)
    {
        return BandLayer;
    }
    GDALRasterBandH RasterBand = RasterDataset->GetRasterBand(LayerIndex);
    if(RasterBand == nullptr)
    {
        return BandLayer;
    }
    int DataType = GDALGetRasterDataType(RasterBand);
    switch (DataType)
    {
    case 0:
        // GDT_Unknown, or unknown data type.
        return BandLayer;
    case 1:
        // GDAL GDT_Byte (-128 to 127) - unsigned  char
        return GetArray2D<unsigned char>(LayerIndex, BandLayer);
    case 2:
        // GDAL GDT_UInt16 - short
        return GetArray2D<unsigned short>(LayerIndex, BandLayer);
    case 3:
        // GDT_Int16
        return GetArray2D<short>(LayerIndex, BandLayer);
    case 4:
        // GDT_UInt32
        return GetArray2D<unsigned int>(LayerIndex, BandLayer);
    case 5:
        // GDT_Int32
        return GetArray2D<int>(LayerIndex, BandLayer);
    case 6:
        // GDT_Float32
        return GetArray2D<float>(LayerIndex, BandLayer);
    case 7:
        // GDT_Float64
        return GetArray2D<double>(LayerIndex, BandLayer);
    default:
        break;
    }
    return BandLayer;
}

int RasterFile::GetByteSize(int LayerIndex)
{
    GDALDataType BandType = GDALGetRasterDataType(RasterDataset->GetRasterBand(LayerIndex));
    int SizeBytes = GDALGetDataTypeSizeBytes(BandType);
    return SizeBytes;
}

GDALDataType RasterFile::GetDataType(int LayerIndex)
{
    return GDALGetRasterDataType(RasterDataset->GetRasterBand(LayerIndex));
}

template <typename T>
TArray<TArray<double>> RasterFile::GetArray2D(int LayerIndex, TArray<TArray<double>> BandLayer)
{
    GDALDataType BandType = GDALGetRasterDataType(RasterDataset->GetRasterBand(LayerIndex));
    int SizeBytes = GDALGetDataTypeSizeBytes(BandType);
    T* RowBuff = (T*)CPLMalloc(SizeBytes * NumberOfColumns);
    for (int Row = 0; Row < NumberOfRows; Row++)
    {
        CPLErr e = RasterDataset->GetRasterBand(LayerIndex)->RasterIO(
            GF_Read, 0, Row, NumberOfColumns, 1, RowBuff, NumberOfColumns, 1, BandType, 0, 0);
        if (e != 0)
        {
            return BandLayer;
        }
        BandLayer.Add(TArray<double>());
        for (int Col = 0; Col < NumberOfColumns; Col++)
        {
            BandLayer[Row].Add((double)RowBuff[Col]);
        }
    }
    CPLFree(RowBuff);
    return BandLayer;
}

bool RasterFile::Warp(FString NewFilename, FString EPSG)
{
    ULandscapingSettings* Settings = GetMutableDefault<ULandscapingSettings>();
    GDALResampleAlg ResampleAlg = GetGDALResampleAlgEnum(Settings->ResampleAlgorithm);
    GDALDatasetH WarpedDS = GDALAutoCreateWarpedVRT(
        RasterDataset,
        nullptr,
        TCHAR_TO_ANSI(*EPSG),
        ResampleAlg,
        0.0,
        nullptr
    );
    bool Success = false;
    if(WarpedDS != nullptr)
    {
        GDALDatasetH CopyDS = GDALCreateCopy(GDALGetDriverByName("GTiff"), TCHAR_TO_ANSI(*NewFilename), WarpedDS, false, nullptr, nullptr, nullptr);
        Success = CopyDS != nullptr;
        GDALClose(CopyDS);
        GDALClose(WarpedDS);
    }
    return Success;
}

// creates a new RasterFile file with the given extents and all other data from this RasterFile struct
bool RasterFile::CreateNew(RasterData InData)
{
    //UE_LOG(LogTemp, Log, TEXT("Landscaping: Create new raster with %s"), *InData.ToString());
    int SizeBytes = GDALGetDataTypeSizeBytes(GDT_Float32);

    // Args are used for color images
    char* Args[] = {
        "PHOTOMETRIC=RGB",
		"PROFILE=GeoTIFF",
		NULL
	};

    RasterDataset = GetGDALDriverManager()->GetDriverByName("GTiff")->Create(
        TCHAR_TO_ANSI(*InData.Filename), 
        InData.ImportResolution.X, 
        InData.ImportResolution.Y, 
        InData.BandCount, 
        GDT_Float32,
        InData.BandCount >= 3 ? Args : NULL);
    if(RasterDataset == nullptr && (!FPaths::FileExists(InData.Filename) || InData.Filename.StartsWith("/vsimem/")))
    {
        UE_LOG(LogTemp, Error, TEXT("Landscaping: Could not create Raster %s"), *InData.Filename);
        return false;
    }
    if(RasterDataset == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("Landscaping: File created but dataset is null %s"), *InData.Filename);
        return false;
    }
    
    double adfGeoTransform[6] = { InData.Left, InData.MeterPerPixel.X, 0, InData.Top, 0, InData.MeterPerPixel.Y };
    RasterDataset->SetGeoTransform(adfGeoTransform);
    if(InData.ProjectionWkt.IsEmpty())
    {
        RasterDataset->SetProjection(TCHAR_TO_ANSI(*InData.Projection));
    }
    else
    {
        RasterDataset->SetProjection(TCHAR_TO_ANSI(*InData.ProjectionWkt));
    }
    // write data if there is any
    if(!InData.RasterBandData.IsEmpty() || !InData.ColorBandData.IsEmpty())
    {
        for(int BandNr = 1; BandNr <= InData.BandCount; BandNr++)
        {
            GDALRasterBand* poBand = RasterDataset->GetRasterBand(BandNr);
            poBand->SetNoDataValue(InData.NoDataValue);
            for (int Row = 0; Row < InData.ImportResolution.Y; Row++)
            {
                float* pOutputLine = (float*) CPLMalloc(SizeBytes * InData.ImportResolution.X);
                for (int Column = 0; Column < InData.ImportResolution.X; Column++)
                {
                    if(InData.RasterBandData.IsEmpty())
                    {
                        // satellite
                        if(BandNr == 1)
                        {
                            pOutputLine[Column] = (float)InData.ColorBandData[Row][Column].R;
                        }
                        else if(BandNr == 2)
                        {
                            pOutputLine[Column] = (float)InData.ColorBandData[Row][Column].G;
                        }
                        else if(BandNr == 3)
                        {
                            pOutputLine[Column] = (float)InData.ColorBandData[Row][Column].B;
                        }
                    }
                    else
                    {
                        // heightdata
                        pOutputLine[Column] = (float)InData.RasterBandData[Row][Column];
                    }
                }
                CPLErr e = poBand->RasterIO(
                    GF_Write, 
                    0, 
                    Row, 
                    InData.ImportResolution.X, 
                    1, 
                    pOutputLine, 
                    InData.ImportResolution.X, 
                    1, 
                    GDT_Float32, 
                    0, 
                    0);
                CPLFree(pOutputLine);
            }
        }
    }
    // satellite
	if(InData.BandCount >= 3)
	{
        RasterDataset->GetRasterBand(1)->SetColorInterpretation(GDALColorInterp::GCI_RedBand);
		RasterDataset->GetRasterBand(2)->SetColorInterpretation(GDALColorInterp::GCI_GreenBand);
		RasterDataset->GetRasterBand(3)->SetColorInterpretation(GDALColorInterp::GCI_BlueBand);
	}
    return true;
}

bool RasterFile::Mosaic(RasterFile* MosaickedFile, FString EPSG)
{
    ULandscapingSettings* Settings = GetMutableDefault<ULandscapingSettings>();
    GDALResampleAlg ResampleAlg = GetGDALResampleAlgEnum(Settings->ResampleAlgorithm);
    if(GDALReprojectImageMulti(
        RasterDataset, 
        TCHAR_TO_ANSI(*EPSG),
        MosaickedFile->RasterDataset, 
        TCHAR_TO_ANSI(*EPSG),
        ResampleAlg,
        Settings->ResampleMemoryGigaByte * 1024 *1024,
        0.0,
        nullptr, 
        nullptr, 
        nullptr
        ) != 0)
    {
        return false;
    }
    return true;
}

GDALResampleAlg RasterFile::GetGDALResampleAlgEnum(ELandscapingResampleAlgorithm InResampleAlg)
{
    GDALResampleAlg ResampleAlg = GRA_Bilinear;
    switch (InResampleAlg)
    {
        case ELandscapingResampleAlgorithm::ResampleAlgNearestNeighbour:
            ResampleAlg = GRA_NearestNeighbour;
            break;
        case ELandscapingResampleAlgorithm::ResampleAlgBilinear:
            ResampleAlg = GRA_Bilinear;
            break;
        case ELandscapingResampleAlgorithm::ResampleAlgCubic:
            ResampleAlg = GRA_Cubic;
            break;
        case ELandscapingResampleAlgorithm::ResampleAlgCubicSpline:
            ResampleAlg = GRA_CubicSpline;
            break;
        case ELandscapingResampleAlgorithm::ResampleAlgLanczos:
            ResampleAlg = GRA_Lanczos;
            break;
        case ELandscapingResampleAlgorithm::ResampleAlgAverage:
            ResampleAlg = GRA_Average;
            break;
        case ELandscapingResampleAlgorithm::ResampleAlgMode:
            ResampleAlg = GRA_Mode;
            break;
        case ELandscapingResampleAlgorithm::ResampleAlgMax:
            ResampleAlg = GRA_Max;
            break;
        case ELandscapingResampleAlgorithm::ResampleAlgMin:
            ResampleAlg = GRA_Min;
            break;
        case ELandscapingResampleAlgorithm::ResampleAlgMed:
            ResampleAlg = GRA_Med;
            break;
        case ELandscapingResampleAlgorithm::ResampleAlgQ1:
            ResampleAlg = GRA_Q1;
            break;
        case ELandscapingResampleAlgorithm::ResampleAlgQ3:
            ResampleAlg = GRA_Q3;
            break;
        case ELandscapingResampleAlgorithm::ResampleAlgSum:
            ResampleAlg = GRA_Sum;
            break;
        case ELandscapingResampleAlgorithm::ResampleAlgRMS:
            ResampleAlg = GRA_RMS;
            break;
        default:
            ResampleAlg = GRA_Bilinear;
    }
    return ResampleAlg;
}

/**
 * GDALReprojectImage() method with a ChunkAndWarpImage replaced with ChunkAndWarpMulti
 */
CPLErr RasterFile::GDALReprojectImageMulti( GDALDatasetH hSrcDS, const char *pszSrcWKT,
                    GDALDatasetH hDstDS, const char *pszDstWKT,
                    GDALResampleAlg eResampleAlg,
                    double dfWarpMemoryLimit,
                    double dfMaxError,
                    GDALProgressFunc pfnProgress, void *pProgressArg,
                    GDALWarpOptions *psOptions )

{
    GDALWarpOptions *psWOptions;

/* -------------------------------------------------------------------- */
/*      Setup a reprojection based transformer.                         */
/* -------------------------------------------------------------------- */
    void *hTransformArg;

    hTransformArg = 
        GDALCreateGenImgProjTransformer( hSrcDS, pszSrcWKT, hDstDS, pszDstWKT, 
                                         TRUE, 1000.0, 0 );

    if( hTransformArg == NULL )
        return CE_Failure;

/* -------------------------------------------------------------------- */
/*      Create a copy of the user provided options, or a defaulted      */
/*      options structure.                                              */
/* -------------------------------------------------------------------- */
    if( psOptions == NULL )
        psWOptions = GDALCreateWarpOptions();
    else
        psWOptions = GDALCloneWarpOptions( psOptions );

    psWOptions->eResampleAlg = eResampleAlg;

/* -------------------------------------------------------------------- */
/*      Set transform.                                                  */
/* -------------------------------------------------------------------- */
    if( dfMaxError > 0.0 )
    {
        psWOptions->pTransformerArg = 
            GDALCreateApproxTransformer( GDALGenImgProjTransform, 
                                         hTransformArg, dfMaxError );

        psWOptions->pfnTransformer = GDALApproxTransform;
    }
    else
    {
        psWOptions->pfnTransformer = GDALGenImgProjTransform;
        psWOptions->pTransformerArg = hTransformArg;
    }

/* -------------------------------------------------------------------- */
/*      Set file and band mapping.                                      */
/* -------------------------------------------------------------------- */
    int  iBand;

    psWOptions->hSrcDS = hSrcDS;
    psWOptions->hDstDS = hDstDS;

    if( psWOptions->nBandCount == 0 )
    {
        psWOptions->nBandCount = MIN(GDALGetRasterCount(hSrcDS),
                                     GDALGetRasterCount(hDstDS));
        
        psWOptions->panSrcBands = (int *) 
            CPLMalloc(sizeof(int) * psWOptions->nBandCount);
        psWOptions->panDstBands = (int *) 
            CPLMalloc(sizeof(int) * psWOptions->nBandCount);

        for( iBand = 0; iBand < psWOptions->nBandCount; iBand++ )
        {
            psWOptions->panSrcBands[iBand] = iBand+1;
            psWOptions->panDstBands[iBand] = iBand+1;
        }
    }

/* -------------------------------------------------------------------- */
/*      Set source nodata values if the source dataset seems to have    */
/*      any. Same for target nodata values                              */
/* -------------------------------------------------------------------- */
    for( iBand = 0; iBand < psWOptions->nBandCount; iBand++ )
    {
        GDALRasterBandH hBand = GDALGetRasterBand( hSrcDS, iBand+1 );
        int             bGotNoData = FALSE;
        double          dfNoDataValue;

        if (GDALGetRasterColorInterpretation(hBand) == GCI_AlphaBand)
        {
            psWOptions->nSrcAlphaBand = iBand + 1;
        }

        dfNoDataValue = GDALGetRasterNoDataValue( hBand, &bGotNoData );
        if( bGotNoData )
        {
            if( psWOptions->padfSrcNoDataReal == NULL )
            {
                int  ii;

                psWOptions->padfSrcNoDataReal = (double *) 
                    CPLMalloc(sizeof(double) * psWOptions->nBandCount);
                psWOptions->padfSrcNoDataImag = (double *) 
                    CPLMalloc(sizeof(double) * psWOptions->nBandCount);

                for( ii = 0; ii < psWOptions->nBandCount; ii++ )
                {
                    psWOptions->padfSrcNoDataReal[ii] = -1.1e20;
                    psWOptions->padfSrcNoDataImag[ii] = 0.0;
                }
            }

            psWOptions->padfSrcNoDataReal[iBand] = dfNoDataValue;
        }

        // Deal with target band
        hBand = GDALGetRasterBand( hDstDS, iBand+1 );
        if (hBand && GDALGetRasterColorInterpretation(hBand) == GCI_AlphaBand)
        {
            psWOptions->nDstAlphaBand = iBand + 1;
        }

        dfNoDataValue = GDALGetRasterNoDataValue( hBand, &bGotNoData );
        if( bGotNoData )
        {
            if( psWOptions->padfDstNoDataReal == NULL )
            {
                int  ii;

                psWOptions->padfDstNoDataReal = (double *) 
                    CPLMalloc(sizeof(double) * psWOptions->nBandCount);
                psWOptions->padfDstNoDataImag = (double *) 
                    CPLMalloc(sizeof(double) * psWOptions->nBandCount);

                for( ii = 0; ii < psWOptions->nBandCount; ii++ )
                {
                    psWOptions->padfDstNoDataReal[ii] = -1.1e20;
                    psWOptions->padfDstNoDataImag[ii] = 0.0;
                }
            }

            psWOptions->padfDstNoDataReal[iBand] = dfNoDataValue;
        }
    }

/* -------------------------------------------------------------------- */
/*      Set the progress function.                                      */
/* -------------------------------------------------------------------- */
    if( pfnProgress != NULL )
    {
        psWOptions->pfnProgress = pfnProgress;
        psWOptions->pProgressArg = pProgressArg;
    }

/* -------------------------------------------------------------------- */
/*      Create a warp options based on the options.                     */
/* -------------------------------------------------------------------- */
    GDALWarpOperation  oWarper;
    CPLErr eErr;

    eErr = oWarper.Initialize( psWOptions );

    if( eErr == CE_None )
        eErr = oWarper.ChunkAndWarpMulti( 0, 0, 
                                          GDALGetRasterXSize(hDstDS),
                                          GDALGetRasterYSize(hDstDS) );

/* -------------------------------------------------------------------- */
/*      Cleanup.                                                        */
/* -------------------------------------------------------------------- */
    GDALDestroyGenImgProjTransformer( hTransformArg );

    if( dfMaxError > 0.0 )
        GDALDestroyApproxTransformer( psWOptions->pTransformerArg );
        
    GDALDestroyWarpOptions( psWOptions );

    return eErr;
}
