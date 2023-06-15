// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "LandscapingStyle.h"

class FLandscapingCommands : public TCommands<FLandscapingCommands>
{
public:

	FLandscapingCommands()
		: TCommands<FLandscapingCommands>(TEXT("Landscaping"), NSLOCTEXT("Contexts", "Landscaping", "Landscaping Plugin"), NAME_None, FLandscapingStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};