// Copyright Lost in Game Studio. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "TouchInterfacePresetFactory.generated.h"

UCLASS()
class TOUCHINTERFACEDESIGNER_API UTouchInterfacePresetFactory : public UFactory
{
	GENERATED_BODY()

	UTouchInterfacePresetFactory();

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};
