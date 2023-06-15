// Copyright Lost in Game Studio. All Rights Reserved

#include "TouchInterface.h"

#include "TouchInterfaceStyle.h"

#define LOCTEXT_NAMESPACE "FVirtualControlDesignerModule"

void FTouchInterfaceModule::StartupModule()
{
	FTouchInterfaceStyle::RegisterStyle();
	FTouchInterfaceStyle::ReloadTextures();
}

void FTouchInterfaceModule::ShutdownModule()
{
	FTouchInterfaceStyle::UnregisterStyle();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FTouchInterfaceModule, TouchInterface)