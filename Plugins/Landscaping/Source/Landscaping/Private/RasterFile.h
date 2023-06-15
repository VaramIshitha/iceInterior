// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "iostream"
#include "gdal.h"
#include "gdal_priv.h"
#include "cpl_conv.h"
#include "gdalwarper.h"
#include "gdal_utils.h"
#include "stdlib.h"
#include "RasterData.h"
#include "LandscapingStructs.h"
#include <sstream>

using namespace std;

class RasterFile
{
public:
    RasterFile(FString RasterFilename);
    RasterFile(GDALDataset* Dataset);
    RasterFile(RasterData InData);
    ~RasterFile();
    void Init();
    const char* GetProjection();
    double GetRasterScale(int RasterBand);
    double GetRasterOffset(int RasterBand);
    double* GetMinAndMaxAltitude(int RasterBand, FString &Error);
    GDALDataType GetDataType(int RasterBand);
    // [0] = Left, [1] = MeterPerPixel.X, [3] = Top, [5] = MeterPerPixel.Y
    double *GetGeoTransform();
    double GetNoDataValue();
    // [0] = Rows, [1] = Columns, [2] = Bands
    int *GetDimensions();
    FExtents GetExtents();
    // returns first 3 layers of raster interpreted as RGB FColor
    TArray<TArray<FColor>> GetColorBand();
    TArray<TArray<double>> GetRasterBand(int z);
    int GetByteSize(int LayerIndex);
    template <typename T>
    TArray<TArray<double>> GetArray2D(int LayerIndex, TArray<TArray<double>> BandLayer);
    bool Warp(FString NewFilename, FString EPSG);
    bool Mosaic(RasterFile* MosaickedFile, FString EPSG);
    void SetColorInterpretation(int LayerIndex, GDALColorInterp BandInterp);
    bool IsValid();
    FString GetInfo();

private:
    bool CreateNew(RasterData InData);
    GDALResampleAlg GetGDALResampleAlgEnum(ELandscapingResampleAlgorithm InResampleAlg);
    CPLErr GDALReprojectImageMulti( GDALDatasetH hSrcDS, const char *pszSrcWKT,
                    GDALDatasetH hDstDS, const char *pszDstWKT,
                    GDALResampleAlg eResampleAlg,
                    double dfWarpMemoryLimit,
                    double dfMaxError,
                    GDALProgressFunc pfnProgress, void *pProgressArg,
                    GDALWarpOptions *psOptions );
    FString Filename = FString();        // name of Geotiff
    GDALDataset *RasterDataset = nullptr; // Geotiff GDAL datset object.
    double GeoTransform[6];      // 6-element geotranform array
    int Dimensions[3];           // X,Y, and Z dimensions
    int NumberOfRows = 0;
    int NumberOfColumns = 0;
    int NumberOfBands = 0;     // dimensions of data in Geotiff
    double OriginX = 0;
    double OriginY = 0;
    double MeterPerPixelX = 0;
    double MeterPerPixelY = 0;
};