// Copyright Lost in Game Studio. All Rights Reserved

#include "VirtualControlSetupFactory.h"
#include "VirtualControlSetup.h"

UVirtualControlSetupFactory::UVirtualControlSetupFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UVirtualControlSetup::StaticClass();
}

UObject* UVirtualControlSetupFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,UObject* Context, FFeedbackContext* Warn)
{
	UVirtualControlSetup* SetupInstance = NewObject<UVirtualControlSetup>(InParent, InClass, InName, Flags);
	return SetupInstance;
}
