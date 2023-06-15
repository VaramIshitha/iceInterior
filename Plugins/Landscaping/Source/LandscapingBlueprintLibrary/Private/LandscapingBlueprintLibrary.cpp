// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#include "LandscapingBlueprintLibrary.h"

#define LOCTEXT_NAMESPACE "FLandscapingBlueprintLibraryModule"

void FLandscapingBlueprintLibraryModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
}

void FLandscapingBlueprintLibraryModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLandscapingBlueprintLibraryModule, LandscapingBlueprintLibrary)