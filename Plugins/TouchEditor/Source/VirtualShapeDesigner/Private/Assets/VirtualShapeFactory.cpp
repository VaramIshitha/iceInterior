// Copyright Lost in Game Studio. All Rights Reserved.


#include "VirtualShapeFactory.h"
#include "Classes/VirtualShape.h"


UVirtualShapeFactory::UVirtualShapeFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UVirtualShape::StaticClass();
}

UObject* UVirtualShapeFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UVirtualShape>(InParent, InClass, InName, Flags);
}
