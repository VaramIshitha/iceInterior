// Copyright Lost in Game Studio. All Rights Reserved

#include "TouchInterfacePresetFactory.h"
#include "Presets/TouchInterfacePreset.h"

UTouchInterfacePresetFactory::UTouchInterfacePresetFactory()
{
	bCreateNew = true;
	bEditAfterNew = false;
	SupportedClass = UTouchInterfacePreset::StaticClass();
}

UObject* UTouchInterfacePresetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UTouchInterfacePreset* TouchInterfacePresetInstance = NewObject<UTouchInterfacePreset>(InParent, InClass, InName, Flags);
	return TouchInterfacePresetInstance;
}
