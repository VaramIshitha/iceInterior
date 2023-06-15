// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once
#include "CoreMinimal.h"
#include "LandscapeProxy.h"
#include "Landscape.h"
#include "LandscapeStreamingProxy.h"
#include "EngineUtils.h"
#include "Engine/Selection.h"
#include <math.h>
#include <chrono>
#include <string>

using namespace std;
using namespace std::chrono;

class ALandscapingProcMeshLandscape;

class LandscapingUtils
{
public:
    
	static FBox GetBounds(TArray<FVector> Points)
	{
		FBox Bounds = FBox(ForceInit);
		for(FVector Point : Points)
		{
			Bounds += Point;
		}
		return Bounds;
	}

	template<typename T>
	static TArray<T*> GetSelectedActors()
	{
		TArray<T*> SelectedTypedActors;
		TArray<AActor*> SelectedActors;
		GEditor->GetSelectedActors()->GetSelectedObjects(SelectedActors);
		for(int i = 0; i < SelectedActors.Num(); i++)
		{
			if(T* TypedActor = (T*)SelectedActors[i])
			{
				SelectedTypedActors.Add(TypedActor);
			}
		}
		return SelectedTypedActors;
	}

	// get all landscapes or landscape proxies
	static TArray<ALandscapeProxy*> GetLandscapeCells(UWorld* World)
	{
		TArray<ALandscapeProxy*> LandscapeProxy;
		if(World == nullptr)
		{
			World = GEditor ? GEditor->GetEditorWorldContext(false).World() : nullptr;
		}
		if(World == nullptr)
		{
			return LandscapeProxy;
		}
		LandscapingUtils::FindAllActors<ALandscapeProxy>(World, LandscapeProxy);
		return LandscapeProxy;
	}

	static FString GetUniqueId()
	{
		int64_t timestamp = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
		return FString::Printf(TEXT("%s"), *FString(to_string(timestamp).c_str()));
	}

	static FString GetUniqueFilename(FString FilePath)
	{
		return FString::Printf(TEXT("%s_Landscaping_%s"), *FPaths::GetBaseFilename(FilePath), *GetUniqueId());
	}

	template<typename T>
	static void FindAllActors(UWorld* World, TArray<T*>& Out)
	{
		for (TActorIterator<T> It(World); It; ++It)
		{
			Out.Add(*It);
		}
	}

	static FVector SnapToGround(FVector Location, float OffsetFromGround, UWorld* InWorld = nullptr, int Tries = 3)
	{
		FVector HitPoint;
		FVector Start = FVector(Location.X, Location.Y, 900000);
		FVector End = Start + FVector::DownVector * 10000000;
		for(int i = 0; i < Tries; i++)
		{
			if(GetSurfacePoint(Start, End, HitPoint, InWorld))
			{
				return FVector(HitPoint.X, HitPoint.Y, HitPoint.Z + OffsetFromGround);
			}
			int sign = i%2 == 0 ? -1 : 1;
			// if we are on the edge of a landscape, try a slight angle
			End = End + FVector(i*15*sign, i*15*sign, 0);
		}
		return Location;
	}

	static bool GetSurfacePoint(FVector Start, FVector End, FVector& HitPoint, UWorld* InWorld = nullptr)
	{
		if(InWorld == nullptr)
		{
			return false;
		}
		FHitResult HitResult = FHitResult(ForceInitToZero);
		bool DidHit = InWorld->LineTraceSingleByChannel(
			HitResult,
			Start,
			End,
			ECC_Visibility
		);
		
		HitPoint = HitResult.ImpactPoint;
		return DidHit;
	}

	static TArray<FColor> ResampleFColor(const TArray<FColor>& Data, int32 OldWidth, int32 OldHeight, int32 NewWidth, int32 NewHeight)
	{
		TArray<uint8> R;
		TArray<uint8> G;
		TArray<uint8> B;
		TArray<uint8> A;
		for(FColor Color : Data)
		{
			R.Add(Color.R);
			G.Add(Color.G);
			B.Add(Color.B);
			// A.Add(Color.A);
		}
		TArray<uint8> ResampledR = ResampleData<uint8>(R, OldWidth, OldHeight, NewWidth, NewHeight);
		TArray<uint8> ResampledG = ResampleData<uint8>(G, OldWidth, OldHeight, NewWidth, NewHeight);
		TArray<uint8> ResampledB = ResampleData<uint8>(B, OldWidth, OldHeight, NewWidth, NewHeight);
		// TArray<uint8> ResampledA = ResampleData<uint8>(A, OldWidth, OldHeight, NewWidth, NewHeight);

		TArray<FColor> Result;
		for(int i = 0; i < ResampledR.Num(); i++)
		{
			Result.Add(FColor(ResampledR[i], ResampledG[i], ResampledB[i], 255U)); //ResampledA[i]));
		}
		return Result;
	}

	template<typename T>
	static TArray<T> ResampleData(const TArray<T>& Data, int32 OldWidth, int32 OldHeight, int32 NewWidth, int32 NewHeight)
	{
		TArray<T> Result;
		Result.Empty(NewWidth * NewHeight);
		Result.AddUninitialized(NewWidth * NewHeight);

		const float XScale = (float)(OldWidth - 1) / (NewWidth - 1);
		const float YScale = (float)(OldHeight - 1) / (NewHeight - 1);
		for (int32 Y = 0; Y < NewHeight; ++Y)
		{
			for (int32 X = 0; X < NewWidth; ++X)
			{
				const float OldY = Y * YScale;
				const float OldX = X * XScale;
				const int32 X0 = FMath::FloorToInt(OldX);
				const int32 X1 = FMath::Min(FMath::FloorToInt(OldX) + 1, OldWidth - 1);
				const int32 Y0 = FMath::FloorToInt(OldY);
				const int32 Y1 = FMath::Min(FMath::FloorToInt(OldY) + 1, OldHeight - 1);
				const T& Original00 = Data[Y0 * OldWidth + X0];
				const T& Original10 = Data[Y0 * OldWidth + X1];
				const T& Original01 = Data[Y1 * OldWidth + X0];
				const T& Original11 = Data[Y1 * OldWidth + X1];
				Result[Y * NewWidth + X] = FMath::BiLerp(Original00, Original10, Original01, Original11, FMath::Fractional(OldX), FMath::Fractional(OldY));
			}
		}

		return Result;
	}

	//TEMPLATE Load Obj From Path
	template <typename ObjClass>
	static FORCEINLINE ObjClass* LoadObjFromPath(const FName& Path) 
	{
		if (Path == NAME_None) return nullptr;

		return Cast<ObjClass>(StaticLoadObject(ObjClass::StaticClass(), NULL, *Path.ToString()));
	}

	// Load Material From Path 
	static FORCEINLINE UMaterialInterface* LoadMatFromPath(const FName& Path) 
	{
		if (Path == NAME_None) return nullptr;

		return LoadObjFromPath<UMaterialInterface>(Path);
	}

	// mapbox tile to lon lat
	static double TileXToLon(int X, int Z)
	{
		return X / (double)(1 << Z) * 360.0 - 180;
	}
	
	static double TileYToLat(int Y, int Z)
	{
		double Pi = 2 * acos(0.0);
		double N = Pi - 2.0 * Pi * Y / (double)(1 << Z);
		return 180.0 / Pi * atan(0.5 * (exp(N) - exp(-N)));
	}

};