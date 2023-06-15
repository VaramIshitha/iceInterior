// Copyright Lost in Game Studio. All Rights Reserved.

#include "VirtualControlEventFactory.h"

#include "Classes/VirtualControlEvent.h"
#include "Factories/BlueprintFactory.h"

UVirtualControlEventFactory::UVirtualControlEventFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UVirtualControlEvent::StaticClass();
}

UObject* UVirtualControlEventFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	//Use Blueprint Factory to create blueprint child of c++ class
	UBlueprintFactory* Factory = NewObject<UBlueprintFactory>();

	//Set parent c++ class
	Factory->ParentClass = SupportedClass;

	//Return blueprint object based on c++ class
	return Factory->FactoryCreateNew(UBlueprint::StaticClass(), InParent, InName, Flags, Context, Warn);
}
