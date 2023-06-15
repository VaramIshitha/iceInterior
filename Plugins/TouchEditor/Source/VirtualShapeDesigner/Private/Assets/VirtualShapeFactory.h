// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

#include "Factories/Factory.h"
#include "VirtualShapeFactory.generated.h"

UCLASS()
class VIRTUALSHAPEDESIGNER_API UVirtualShapeFactory : public UFactory
{
	GENERATED_BODY()

	UVirtualShapeFactory();

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};
