// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include <math.h>

class FMapboxConverter
{
public:
    static int LonToTileX(double Lon, int z)
	{
		return (int)(floor((Lon + 180.0) / 360.0 * (1 << z))); 
	}
	
	static int LatToTileY(double Lat, int z)
	{
		double Pi = 2 * acos(0.0);
		double LatRad = Lat * Pi/180.0;
		return (int)(floor((1.0 - asinh(tan(LatRad)) / Pi) / 2.0 * (1 << z)));
	}
	
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