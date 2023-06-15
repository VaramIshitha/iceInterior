// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/ScriptMacros.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/DataTable.h"
#include "Kismet/KismetSystemLibrary.h"
#include "LandscapingBlueprintFunctionLibrary.generated.h"

UCLASS()
class LANDSCAPINGBLUEPRINTLIBRARY_API ULandscapingBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

    UFUNCTION(BlueprintCallable, CustomThunk, Category = "DataTable", meta=(CustomStructureParam = "Value", AutoCreateRefTerm = "Value"))
	static bool InsertDataTableRow(UDataTable* Table, FName RowName, const FGenericStruct& Value);

    static bool Generic_InsertDataTableRow(UDataTable* Table, FName RowName, void* ValuePtr);

	DECLARE_FUNCTION(execInsertDataTableRow);
};