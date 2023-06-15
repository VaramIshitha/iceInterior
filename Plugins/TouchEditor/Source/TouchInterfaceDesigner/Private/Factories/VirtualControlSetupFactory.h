// Copyright Lost in Game Studio. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "VirtualControlSetupFactory.generated.h"

UCLASS()
class TOUCHINTERFACEDESIGNER_API UVirtualControlSetupFactory : public UFactory
{
	GENERATED_BODY()

	UVirtualControlSetupFactory();

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};
