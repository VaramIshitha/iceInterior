// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

// epsg validation
#define MIN_EPSG 1000
#define MAX_EPSG 32768
#define EPSG_REGEX "^\\s*([1-9]\\d{0,5}|0)\\s*$"
// Raster Altitude Constraints
#define MIN_ALTITUDE -12000
#define MAX_ALTITUDE 9000
// Vector file types
#define VECTOR_FILE "Vectorfile|*.shp;*.json;*.geojson;*.geojsonl;*.geojsons;*.osm;*.osm.pbf;*.gpkg;"
#define LANDUSE_FILE "Landuse File|*.shp;*.json;*.geojson;*.geojsonl;*.geojsons*.osm;*.osm.pbf;*.gpkg;"