// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#include "LandscapingCommands.h"

#define LOCTEXT_NAMESPACE "FLandscapingModule"

void FLandscapingCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "Landscaping", "Bring up Landscaping window", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
