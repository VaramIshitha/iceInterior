// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "VirtualControlEventFactory.generated.h"

UCLASS()
class UVirtualControlEventFactory : public UFactory
{
	GENERATED_BODY()
	
public:
	UVirtualControlEventFactory();

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;	
};
