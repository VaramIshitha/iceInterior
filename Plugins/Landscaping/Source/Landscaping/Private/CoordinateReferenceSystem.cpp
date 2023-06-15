// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved


#include "CoordinateReferenceSystem.h"

UCoordinateReferenceSystem::UCoordinateReferenceSystem(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    GDALAllRegister();
}

bool UCoordinateReferenceSystem::IsValid()
{
    return IsAuthorityIDValid() && IsOriginValid();
}

void UCoordinateReferenceSystem::FetchAuthorityIDFromSettings()
{
    // don't fetch if we already got an valid AuhorityID
    if(IsOriginValid() && IsEpsgIDValid(AuthorityID))
    {
        return;
    }
    ULandscapingSettings* Settings = GetMutableDefault<ULandscapingSettings>();
    if(IsModeCustomCRS())
    {
        SetAuthorityID(Settings->Projection);
    }
}

int UCoordinateReferenceSystem::GetAuthorityID()
{
    FetchAuthorityIDFromSettings();
    return AuthorityID;
}

FString UCoordinateReferenceSystem::GetAuthorityIDStr()
{
    FetchAuthorityIDFromSettings();
    return FString::Printf(TEXT("EPSG:%i"), AuthorityID);
}

FVector UCoordinateReferenceSystem::GetOrigin() const
{
    return Origin;
}

void UCoordinateReferenceSystem::SetOrigin(FVector InOrigin)
{
    Origin = InOrigin;
}

bool UCoordinateReferenceSystem::IsOriginValid() const
{
    return !Origin.IsZero();
}

// wkt of target crs
FString UCoordinateReferenceSystem::GetWkt() const
{
    return Wkt;
}

void UCoordinateReferenceSystem::SetAuthorityID(int Epsg, FString InWkt)
{
    bool bIsEPSGValid = IsEpsgIDValid(Epsg);
    if(!bIsEPSGValid && InWkt.IsEmpty())
    {
        if(Epsg == 0)
        {
            return;
        }
        UE_LOG(LogTemp, Error, TEXT("Landscaping: Please use only valid CRS (Projected Coordinate Reference Systems). You can look it up at https://epsg.io/"));
        if(IsModeUseSourceCRS() || IsModeCustomCRS())
        {
            UE_LOG(LogTemp, Error, TEXT("Landscaping: You might still import the file when switching to 'Autoreproject to UTM' in 'Project Settings -> Plugins -> Landscaping -> Projection Mode'"));
        }
        return;
    }
    
    if(InWkt.IsEmpty() && bIsEPSGValid)
    {
        AuthorityID = Epsg;
        UE_LOG(LogTemp, Log, TEXT("Landscaping: Try setting Projection WKT of EPSG:%i"), Epsg);
        OGRSpatialReference SpaRef;
        OGRErr Error = SpaRef.importFromEPSG(Epsg);
        if(Error == OGRERR_NONE)
        {
            char* OutWkt = nullptr;
            Error = SpaRef.exportToWkt(&OutWkt);
            if(Error == OGRERR_NONE && IsWktValid(FString(OutWkt), false))
            {
                Wkt = FString(OutWkt);
                UE_LOG(LogTemp, Log, TEXT("Landscaping: Successfully determined Projection WKT to %s"), *Wkt);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Landscaping: Export to WKT from EPSG:%i failed %s"), Epsg, *GetOGRErrStr(Error));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Landscaping: Load spatial ref from EPSG:%i failed %s"), Epsg, *GetOGRErrStr(Error));
        }
    }
    else if(IsWktValid(InWkt))
    {
        Wkt = InWkt;
        AuthorityID = Epsg; // we can rely on Wkt even if EPSG is not projected
    }
}

bool UCoordinateReferenceSystem::IsEpsgIDValid(int Epsg, bool bCheckIfProjected) const
{
    if(Epsg == 0)
    {
        return false;
    }
    if(Epsg > MIN_EPSG && Epsg < MAX_EPSG)
    {
        // we do not make other checks of the epsg code here because they are not reliable in any way
        return true;
    }
    UE_LOG(LogTemp, Warning, TEXT("Landscaping: EPSG:%i out of valid range"), Epsg);
    return false;
}

bool UCoordinateReferenceSystem::IsWktValid(FString InWkt, bool bCheckIfProjected) const
{
    if(!InWkt.IsEmpty())
    {
        OGRSpatialReference SpaRef;
        OGRErr Error = SpaRef.importFromWkt(TCHAR_TO_ANSI(*InWkt));
        if(Error != OGRERR_NONE)
        {
            UE_LOG(LogTemp, Error, TEXT("Landscaping: Wkt: %s not valid: %s"), *InWkt, *GetOGRErrStr(Error));
            return false;
        }
        if(!bCheckIfProjected)
        {
            return true;
        }
        if(SpaRef.IsProjected())
        {
            return true;
        }
        UE_LOG(LogTemp, Error, TEXT("Landscaping: Wkt: %s is not projected"), *InWkt);
        return false;
    }
    UE_LOG(LogTemp, Warning, TEXT("Landscaping: Wkt empty"));
    return false;
}

bool UCoordinateReferenceSystem::IsAuthorityIDValid()
{
    if(AuthorityID < MIN_EPSG || AuthorityID > MAX_EPSG)
    {
        // not a valid authority id - see if we have set a custom crs in settings
        FetchAuthorityIDFromSettings();
    }
    return IsEpsgIDValid(AuthorityID);
}

void UCoordinateReferenceSystem::FetchModeFromSetting()
{
    ULandscapingSettings* Settings = GetMutableDefault<ULandscapingSettings>();
    ProjectionMode = Settings->ProjectionMode;
}

bool UCoordinateReferenceSystem::IsModeAutoReprojectToUTM()
{
    FetchModeFromSetting();
    return ProjectionMode == ELandscapingProjectionMode::AutoReprojectToUTM;
}

bool UCoordinateReferenceSystem::IsModeUseSourceCRS()
{
    FetchModeFromSetting();
    return ProjectionMode == ELandscapingProjectionMode::UseSourceCRS;
}

bool UCoordinateReferenceSystem::IsModeCustomCRS()
{
    FetchModeFromSetting();
    return ProjectionMode == ELandscapingProjectionMode::CustomCRS;
}

void UCoordinateReferenceSystem::SetCroppedExtents(FExtents InExtents)
{
    CurrentCroppedExtents = InExtents;
}

bool UCoordinateReferenceSystem::SetCroppedExtents(FString ImportBBox)
{
    TArray<FString> Out;
    ImportBBox.ParseIntoArray(Out,TEXT(","), true);
    bool bIsValid = Out.Num() == 4;
    for(int i = 0; i < Out.Num(); i++)
    {
        bIsValid = bIsValid && regex_match (string(TCHAR_TO_ANSI(*Out[i])), regex("^(-?)(0|([1-9][0-9]*))(\\.[0-9]+)?$"));
    }
    if(bIsValid)
    {   
        CurrentCroppedExtents = FExtents(stod(*Out[0]), stod(*Out[1]), stod(*Out[2]), stod(*Out[3]));
    }
    return bIsValid;
}

FExtents UCoordinateReferenceSystem::GetCroppedExtents() const
{
    return CurrentCroppedExtents;
}

bool UCoordinateReferenceSystem::GetUsePreciseScale() const
{
    return bUsePreciseScale;
}

void UCoordinateReferenceSystem::SetUsePreciseScale(bool InPreciseScale)
{
    bUsePreciseScale = InPreciseScale;
}

bool UCoordinateReferenceSystem::IsSameAsLevelCRS(const char* Proj) const
{
    OGRSpatialReference SpaRef;
    OGRErr Error = SpaRef.importFromWkt(TCHAR_TO_ANSI(*Wkt));
    if(Error != OGRERR_NONE)
    {
        UE_LOG(LogTemp, Error, TEXT("Landscaping: Import Level WKT failed (%s)"), *GetOGRErrStr(Error));
    }
    OGRSpatialReference FileSpaRef;
    Error = FileSpaRef.importFromWkt(Proj);
    if(Error != OGRERR_NONE)
    {
        UE_LOG(LogTemp, Error, TEXT("Landscaping: Import file WKT failed"), *GetOGRErrStr(Error));
    }
    return FileSpaRef.IsSame(&SpaRef) == 1;
}

FExtents UCoordinateReferenceSystem::GetValidExtents(FVector2D LeftBottom, FVector2D RightTop) const
{
    if(LeftBottom.IsZero() || RightTop.IsZero())
    {
        return FExtents();
    }
    return FExtents(LeftBottom.Y, LeftBottom.X, RightTop.Y, RightTop.X);
}
FExtents UCoordinateReferenceSystem::ConvertFromTo(FExtents InExtents, FString InWktFrom, FString InWktTo) const
{
    FVector2D LeftBottom = ConvertPointToGeogCS(InExtents.Left, InExtents.Bottom, InWktFrom, InWktTo);
    FVector2D RightTop = ConvertPointToGeogCS(InExtents.Right, InExtents.Top, InWktFrom, InWktTo);
    return GetValidExtents(LeftBottom, RightTop);
}

FExtents UCoordinateReferenceSystem::ConvertToGeogCS(FExtents InExtents, FString InFromWkt) const
{
    FVector2D LeftBottom = ConvertPointToGeogCS(InExtents.Left, InExtents.Bottom, InFromWkt, FString());
    FVector2D RightTop = ConvertPointToGeogCS(InExtents.Right, InExtents.Top, InFromWkt, FString());
    return GetValidExtents(LeftBottom, RightTop);
}

FExtents UCoordinateReferenceSystem::ConvertFromGeogCS(FExtents InExtents, FString InWkt) const
{
    FVector2D LeftBottom = ConvertPointToGeogCS(InExtents.Left, InExtents.Bottom, InWkt, FString(), true);
    FVector2D RightTop = ConvertPointToGeogCS(InExtents.Right, InExtents.Top, InWkt, FString(), true);
    return GetValidExtents(LeftBottom, RightTop);
}

FExtents UCoordinateReferenceSystem::ConvertToGeogCS(FExtents InExtents) const
{
    FVector2D LeftBottom = ConvertPointToGeogCS(InExtents.Left, InExtents.Bottom, Wkt, FString());
    FVector2D RightTop = ConvertPointToGeogCS(InExtents.Right, InExtents.Top, Wkt, FString());
    return GetValidExtents(LeftBottom, RightTop);
}

FExtents UCoordinateReferenceSystem::ConvertFromGeogCS(FExtents InExtents) const
{
    FVector2D LeftBottom = ConvertPointToGeogCS(InExtents.Left, InExtents.Bottom, Wkt, FString(), true);
    FVector2D RightTop = ConvertPointToGeogCS(InExtents.Right, InExtents.Top, Wkt, FString(), true);
    return GetValidExtents(LeftBottom, RightTop);
}

FVector2D UCoordinateReferenceSystem::ConvertPointToGeogCS(double X, double Y, FString From, FString To, bool SwitchSourceTarget) const
{
    if(From.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Landscaping: Convert Point to failed - no Wkt"));
        return FVector2D(0);
    }
    
    OGRSpatialReference source;
    if(From.StartsWith("EPSG:"))
    {
        From.RemoveFromStart("EPSG:");
        int EPSG = FCString::Atoi(*From);
        OGRErr Error = source.importFromEPSG(EPSG);
        if(Error != OGRERR_NONE)
        {
            UE_LOG(LogTemp, Error, TEXT("Landscaping: Convert Point to failed on import source EPSG - Error: %s - %s"), *GetOGRErrStr(Error), *From);
            return FVector2D(0);
        }
    }
    else
    {
        OGRErr Error = source.importFromWkt(TCHAR_TO_ANSI(*From));
        if(Error != OGRERR_NONE)
        {
            UE_LOG(LogTemp, Error, TEXT("Landscaping: Convert Point to failed on import - Error: %s - Wkt: %s"), *GetOGRErrStr(Error), *From);
            return FVector2D(0);
        }
    }
    source.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
    OGRSpatialReference target;
    if(To.IsEmpty() && !From.StartsWith("EPSG:") && From.Contains("GEOGCS"))
    {
        target = *source.CloneGeogCS();
    }
    else if(!To.IsEmpty())
    {
        if(To.StartsWith("EPSG:"))
        {
            To.RemoveFromStart("EPSG:");
            int EPSG = FCString::Atoi(*To);
            OGRErr Error = target.importFromEPSG(EPSG);
            if(Error != OGRERR_NONE)
            {
                UE_LOG(LogTemp, Error, TEXT("Landscaping: Convert Point to failed on import target EPSG - Error: %s - %s"), *GetOGRErrStr(Error), *To);
                return FVector2D(0);
            }
        }
        else 
        {
            target.importFromWkt(TCHAR_TO_ANSI(*To));
        }
    }
    else
    {
        target.importFromEPSG(4326);
    }
    if(target.Validate() != OGRERR_NONE)
    {
        UE_LOG(LogTemp, Error, TEXT("Landscaping: Convert Point to failed on target Spatial Ref - Wkt: %s"), *From);
        return FVector2D(0);
    }
    target.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
    OGRPoint p(X, Y);
    OGRErr Error = OGRERR_NONE;
    if(SwitchSourceTarget)
    {
        p.assignSpatialReference(&target);
        Error = p.transformTo(&source);
    }
    else
    {
        p.assignSpatialReference(&source);
        Error = p.transformTo(&target);
    }
    if(Error != OGRERR_NONE)
    {
        UE_LOG(LogTemp, Error, TEXT("Landscaping: Convert Point to failed on transform to - Error: %s - Wkt: %s"), *GetOGRErrStr(Error), *From);
        return FVector2D(0);
    }

    return FVector2D(p.getX(), p.getY());
}

// Proj can be epsg, wkt, projstring, ...
int UCoordinateReferenceSystem::FindAuthorityID(const char* Proj) const
{
    OGRSpatialReference SpaRef(Proj);
    SpaRef.AutoIdentifyEPSG();
    const char* EpsgStr = SpaRef.GetAttrValue("AUTHORITY", 1);
    return EpsgStr == nullptr ? 0 : atoi(EpsgStr);
}

void UCoordinateReferenceSystem::DumpProjectionToLog(const char* Proj, FString Filename) const
{
    OGRSpatialReference SpaRef(Proj);
    char* PrettyWkt;
    OGRErr E = SpaRef.exportToPrettyWkt(&PrettyWkt);
    FString PrettyWktStr = FString(PrettyWkt);
    if(E == OGRERR_NONE)
    {
        UE_LOG(LogTemp, Log, TEXT("Landscaping: %s:\n%s"), *Filename, *PrettyWktStr);
        if(!PrettyWktStr.Contains("meter") || !PrettyWktStr.Contains("metre"))
        {
            UE_LOG(LogTemp, Warning, TEXT("Landscaping: File's unit does not appear to be meter - please use Projection Mode 'Automatically reproject to appropriate UTM CRS' (recommended) or 'Use custom CRS specified below' in 'Project Settings -> Plugins -> Landscaping'"));
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("Landscaping: File's unit appears to be meter and can be imported with Projection Mode 'Use CRS of import file or EPSG:3857 on Mapbox import' in 'Project Settings -> Plugins -> Landscaping'"));
        }
    }
    else 
    {
        UE_LOG(LogTemp, Error, TEXT("Landscaping: %s: %s"), *Filename, *GetOGRErrStr(E));
        UE_LOG(LogTemp, Warning, TEXT("Landscaping: File's unit unclear or projection missing - please use Projection Mode 'Use custom CRS specified below' and specify the file's CRS in 'Project Settings -> Plugins -> Landscaping'"));
    }
}

// value to multiply by linear distances to transform them to meters
double UCoordinateReferenceSystem::GetLinearUnits() const
{
    OGRSpatialReference SpaRef;
    SpaRef.importFromEPSG(AuthorityID);
    return SpaRef.GetLinearUnits();
}

// this should automaticly determine the right utm
// utm will have the hightest accuracy
int UCoordinateReferenceSystem::GetUTMAuthorityID(double Lon, double Lat) const
{
    int UTM = (int)(floor((Lon + 180) / 6 )) % 60 + 1;
    FString Code = Lat >= 0 ? FString("326") : FString("327");
    if(UTM < 10)
    {
        Code.Append(FString("0")).AppendInt(UTM);
    }
    else
    { 
        Code.AppendInt(UTM);
    }
    return FCString::Atoi(*Code);
}

FVector UCoordinateReferenceSystem::GetLandscapeScale(FLandscapingInfo InInfo) const
{
    return bUsePreciseScale ? InInfo.LandscapeScale * LandscapeScaleFactor : FVector((int)InInfo.LandscapeScale.X * LandscapeScaleFactor, (int)InInfo.LandscapeScale.Y * LandscapeScaleFactor, InInfo.LandscapeScale.Z * LandscapeScaleFactor);
}

FVector UCoordinateReferenceSystem::GetLandscapeLocation(FLandscapingInfo InInfo) const
{
    // bUsePreciseScale is not relevant here
    return InInfo.GetLocation() * LandscapeScaleFactor;
}

void UCoordinateReferenceSystem::SetLandscapeScale(FVector InLandscapeScale)
{
    LandscapeScale = InLandscapeScale;
}

void UCoordinateReferenceSystem::SetLandscapeScaleFactor(double InLandscapeScaleFactor)
{
    LandscapeScaleFactor = InLandscapeScaleFactor;
}

double UCoordinateReferenceSystem::GetLandscapeScaleFactor() const
{
    return LandscapeScaleFactor;
}

void UCoordinateReferenceSystem::SetVectorDataScale(FVector InVectorScale)
{
    VectorScale = InVectorScale;
}

// only for vector data which is already converted to same CRS as raster data (using UCoordinateReferenceSystem::AuthorityID)
FVector UCoordinateReferenceSystem::ConvertCRSPointToUnreal(FVector Point) const
{
    FVector CurrentOrigin = GetOrigin();
    FVector Scale = VectorScale * LandscapeScaleFactor;
    FVector ConvertedPoint = FVector(0);
    ConvertedPoint.X = (Point.X - CurrentOrigin.X) * FMath::Abs(Scale.X) + XYOffset.X;
    ConvertedPoint.Y = (Point.Y - CurrentOrigin.Y) * FMath::Abs(Scale.Y) * -1.0 + XYOffset.Y;
    return ConvertedPoint;
}

void UCoordinateReferenceSystem::SetXYOffset(FVector InXYOffset)
{
    XYOffset = InXYOffset;
}

FString UCoordinateReferenceSystem::GetOGRErrStr(OGRErr InError) const
{
    TMap<OGRErr, FString> Errors = TMap<OGRErr, FString>();
    Errors.Add(OGRERR_NONE, "Success");
    Errors.Add(OGRERR_NOT_ENOUGH_DATA, "Not enough data to deserialize");
    Errors.Add(OGRERR_NOT_ENOUGH_MEMORY, "Not enough memory");
    Errors.Add(OGRERR_UNSUPPORTED_GEOMETRY_TYPE, "Unsupported geometry type");
    Errors.Add(OGRERR_UNSUPPORTED_OPERATION, "Unsupported operation");
    Errors.Add(OGRERR_CORRUPT_DATA, "Corrupt data");
    Errors.Add(OGRERR_FAILURE, "Failure");
    Errors.Add(OGRERR_UNSUPPORTED_SRS, "Unsupported SRS");
    Errors.Add(OGRERR_INVALID_HANDLE, "Invalid handle");
    Errors.Add(OGRERR_NON_EXISTING_FEATURE, "Non existing feature");
    return Errors.Contains(InError) ? Errors[InError] : "Unknown Error";
    
}