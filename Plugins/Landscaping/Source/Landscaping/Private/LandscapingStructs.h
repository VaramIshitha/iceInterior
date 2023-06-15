// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "LandscapeProxy.h"
#include "Math/Box2D.h"
#include "ILandscapingDataSource.h"
#include "LandscapingStructs.generated.h"

struct Landuse
{
    FString LanduseType = FString();
    bool bActive = false;
    bool bIsInteractable = true;
};

struct MaterialLayerSettings
{
    FName Name = FName();
    TArray<uint8> Data = TArray<uint8>();
    bool bWeightBlended = false;
    TSharedPtr<SListView<TSharedPtr<Landuse>>> LanduseListView = TSharedPtr<SListView<TSharedPtr<Landuse>>>();
    TArray<TSharedPtr<Landuse>> LanduseList = TArray<TSharedPtr<Landuse>>();
    TSharedPtr<FString> ColorChannel = TSharedPtr<FString>();
    int Tiling = 1;
    UTexture2D* NoiseTexture = nullptr;
    uint8 MinWeight = 64;
};

struct FTileImportConfiguration // from STiledLandscapeImportDlg.h
{
    int32 SizeX = 0;
    int32 NumComponents = 0;
    int32 NumSectionsPerComponent = 0;
    int32 NumQuadsPerSection = 0;
};

USTRUCT()
struct FExtents
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category=Landscaping)
    double Bottom = 0;
    UPROPERTY(EditAnywhere, Category=Landscaping)
    double Left = 0;
    UPROPERTY(EditAnywhere, Category=Landscaping)
    double Top = 0;
    UPROPERTY(EditAnywhere, Category=Landscaping)
    double Right = 0;

    FExtents() {}

    FExtents(double Bottom, double Left, double Top, double Right)
    {
        this->Bottom = Bottom;
        this->Left = Left;
        this->Top = Top;
        this->Right = Right;
    }

    bool IsEmpty()
    {
        return Bottom == 0.0 && Left == 0.0 && Top == 0.0 && Right == 0.0;
    }

    void Reset()
    {
        Bottom = 0;
        Left = 0;
        Top = 0;
        Right = 0;
    }

    bool Equals(const FExtents& Other) const
    {
        return FMath::Abs(Bottom - Other.Bottom) < 0.1 && FMath::Abs(Left - Other.Left) < 0.1 && FMath::Abs(Top - Other.Top) < 0.1 && FMath::Abs(Right - Other.Right) < 0.1;
    }

    // overlaps and touches are counted as overlaps
    bool IsOverlapping(const FExtents& Other) const
    {
        if(Left > Other.Right || Other.Left > Right)
        {
            return false;
        }
        if(Top < Other.Bottom || Other.Top < Bottom)
        {
            return false;
        }
        return true;
    }

    // only real overlaps are counted, touches (equal edge) are not counted as overlap
    bool IsOverlappingStrict(const FExtents& Other) const
    {
        if(Left >= Other.Right || Other.Left >= Right)
        {
            return false;
        }
        if(Top <= Other.Bottom || Other.Top <= Bottom)
        {
            return false;
        }
        return true;
    }

    bool IsInside(const FExtents& Other) const
    {
        return Left > Other.Left && Top < Other.Top && Right < Other.Right && Bottom > Other.Bottom;
    }

    // returns area in km2 - only valid if extents is in level crs (meter)
    double GetArea()
    {
        return FMath::Abs(Left - Right) / 1000.0 * FMath::Abs(Top - Bottom) / 1000.0;
    }

    FString ToString() const
    {
        FString ExtentsStr = FString::SanitizeFloat(Bottom);
        ExtentsStr.Append(",");
        ExtentsStr.Append(FString::SanitizeFloat(Left));
        ExtentsStr.Append(",");
        ExtentsStr.Append(FString::SanitizeFloat(Top));
        ExtentsStr.Append(",");
        ExtentsStr.Append(FString::SanitizeFloat(Right));
        return ExtentsStr;
    }

    bool operator==(const FExtents& Other) const
    {
        return Equals(Other);
    }
};

struct RasterImportOptions
{
    int DesiredMaxTileSize = 8192;
    FString DefaultLayer = FString();
    int bResampleToFirstTile = true;
    int SmoothSteps = 0;
    int bSmoothEdges = false;
    int WorldPartitionGridSize = 16;
    bool bMapboxImport = false;
    ILandscapingDataSource* DataSource;
    bool bHighDetailZScale = true;
    double ZScale = 100;
    bool bSquareTiles = false;
    bool bNativeRasterPixelSize = true;
    float CustomRasterPixelSize = 1.0;
    bool bImportAsMesh = false;
    bool bImportSatImgAsDecal = true;
    bool bImportSatImgAsVertexColor = false;
    bool bMeshCollision = false;
    bool bUpdateLandscape = false;
    bool bFillMissing = false;
    int MinHeightTolerance = 200;
    int ZoomLevel = 14;
    int ZoomSatellite = 16;
};

struct TileConfig 
{
    int NumberTilesX = 1;
    int NumberTilesY = 1;
    TArray<UTexture2D*> Textures;
};

USTRUCT()
struct FDataLoadResult
{
    GENERATED_BODY()

    FDataLoadResult(FString Error) { ErrorMsg = Error; }
    FDataLoadResult() {}
    TArray<FString> VectorFeatureClasses = TArray<FString>();
    FString ErrorMsg = FString();
    FString StatusMsg = FString();
};

USTRUCT(BlueprintType)
struct FVectorData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Landscaping)
    FString Id = FString();
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Landscaping)
    bool bShow = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Landscaping)
    TArray<FVector> PointsOriginal = TArray<FVector>();
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Landscaping)
    TArray<FVector> Points = TArray<FVector>();
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Landscaping)
    FString FeatureClass = FString();
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Landscaping)
    FString Name = FString();
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Landscaping)
    FString GeometryName = FString();
    UPROPERTY(VisibleAnyWhere, BlueprintReadWrite, Category=Landscaping)
    FBox Bounds = FBox(ForceInit);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Landscaping)
    TMap<FString, FString> StringAttributes = TMap<FString, FString>();
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Landscaping)
    TMap<FString, int> IntegerAttributes = TMap<FString, int>();
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Landscaping)
    TMap<FString, int64> Integer64Attributes = TMap<FString, int64>();
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Landscaping)
    TMap<FString, double> DoubleAttributes = TMap<FString, double>();
};

UENUM()
enum class ELandscapingProjectionMode : uint8
{
	AutoReprojectToUTM UMETA(DisplayName = "Automatically reproject to appropriate UTM CRS"),
	UseSourceCRS UMETA(DisplayName = "Use CRS of import file or EPSG:3857 on Mapbox import"),
	CustomCRS UMETA(DisplayName = "Use custom CRS specified below")
};

UENUM()
enum class ELandscapingResampleAlgorithm : uint8
{
    ResampleAlgNearestNeighbour UMETA(DisplayName = "Nearest neighbour (select on one input pixel)"),
    ResampleAlgBilinear UMETA(DisplayName = "Bilinear (2x2 kernel)"),
    ResampleAlgCubic UMETA(DisplayName = "Cubic Convolution Approximation (4x4 kernel)"),
    ResampleAlgCubicSpline UMETA(DisplayName = "Cubic B-Spline Approximation (4x4 kernel)"),
    ResampleAlgLanczos UMETA(DisplayName = "Lanczos windowed sinc interpolation (6x6 kernel)"),
    ResampleAlgAverage UMETA(DisplayName = "Average (computes the weighted average of all non-NODATA contributing pixels)"),
    ResampleAlgMode UMETA(DisplayName = "Mode (selects the value which appears most often of all the sampled points)"),
    ResampleAlgMax UMETA(DisplayName = "Max (selects maximum of all non-NODATA contributing pixels)"),
    ResampleAlgMin UMETA(DisplayName = "Min (selects minimum of all non-NODATA contributing pixels)"),
    ResampleAlgMed UMETA(DisplayName = "Med (selects median of all non-NODATA contributing pixels)"),
    ResampleAlgQ1 UMETA(DisplayName = "Q1 (selects first quartile of all non-NODATA contributing pixels)"),
    ResampleAlgQ3 UMETA(DisplayName = "Q3 (selects third quartile of all non-NODATA contributing pixels)"),
    ResampleAlgSum UMETA(DisplayName = "Sum (weighed sum of all non-NODATA contributing pixels)"),
    ResampleAlgRMS UMETA(DisplayName = "RMS (weighted root mean square (quadratic mean) of all non-NODATA contributing pixels)")
};