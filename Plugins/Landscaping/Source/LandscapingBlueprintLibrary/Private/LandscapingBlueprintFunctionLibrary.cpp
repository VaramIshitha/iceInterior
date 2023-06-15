// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved


#include "LandscapingBlueprintFunctionLibrary.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Engine/Engine.h"
#include "UObject/Package.h"
#include "Misc/PackageName.h"


bool ULandscapingBlueprintFunctionLibrary::InsertDataTableRow(UDataTable* Table, FName RowName, const FGenericStruct& Value)
{
	return false;
}

bool ULandscapingBlueprintFunctionLibrary::Generic_InsertDataTableRow(UDataTable* Table, FName RowName, void* ValuePtr)
{
	bool bSuccess = false;
	void* RowPtr = Table->FindRowUnchecked(RowName);

	if (RowPtr != nullptr)
	{
		const UScriptStruct* StructType = Table->GetRowStruct();

		if (StructType != nullptr)
		{
			StructType->CopyScriptStruct(RowPtr, ValuePtr);
		}
	}
	else
	{
		FTableRowBase* RowSrc = (FTableRowBase*)ValuePtr;
		Table->AddRow(RowName, *RowSrc);
	}

	// saving the table
	Table->Modify(true);
	Table->OnDataTableChanged().Broadcast();
	// Place were we should save the file, including the filename
	UPackage* PackageToSave = Table->GetOutermost();
	// The name of the package
	const FString PackageName = PackageToSave->GetName();
	FString FinalPackageSavePath;
	FString FinalPackageFilename;
	FString BaseFilename, Extension, Directory;
	FString ExistingFilename;
	FPackageName::DoesPackageExist(PackageName, &ExistingFilename);
	// Split the path to get the filename without the directory structure
	FPaths::NormalizeFilename(ExistingFilename);
	FPaths::Split(ExistingFilename, Directory, BaseFilename, Extension);
	// The final save path is whatever the existing filename is
	FinalPackageSavePath = ExistingFilename;
	FinalPackageFilename = FString::Printf(TEXT("%s.%s"), *BaseFilename, *Extension);
	
	bSuccess = GEngine->Exec(NULL, *FString::Printf(TEXT("OBJ SAVEPACKAGE PACKAGE=\"%s\" FILE=\"%s\" SILENT=true"), *PackageName, *FinalPackageSavePath));
	
	if (!bSuccess)
	{	
		UE_LOG(LogDataTable, Error, TEXT("SaveDataTable - The asset '{0}' ({1}) failed to save."), *PackageName, *FinalPackageFilename);
	}
	else
	{
		UE_LOG(LogDataTable, Display, TEXT("SaveDataTable - The asset '{0}' ({1}) has been saved."), *PackageName, *FinalPackageFilename);
	}
	return bSuccess;
}


DEFINE_FUNCTION(ULandscapingBlueprintFunctionLibrary::execInsertDataTableRow)
{
	P_GET_OBJECT(UDataTable, Table);
	P_GET_PROPERTY(FNameProperty, RowName);

	Stack.StepCompiledIn<FStructProperty>(NULL);
	FStructProperty* Property = CastFieldChecked<FStructProperty>(Stack.MostRecentProperty);
	void* ValuePtr = Stack.MostRecentPropertyAddress;

	P_FINISH;
	bool bSuccess = false;

	if (Table == nullptr)
	{
		FBlueprintExceptionInfo ExceptionInfo(EBlueprintExceptionType::AccessViolation, FText::FromString("DataTable not valid."));
		FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
	}
	else if (Property != nullptr && ValuePtr != nullptr)
	{
		UScriptStruct* InputType = Property->Struct;
		const UScriptStruct* TableType = Table->GetRowStruct();

		if (InputType == TableType || (InputType->IsChildOf(TableType) && FStructUtils::TheSameLayout(InputType, TableType)))
		{
			P_NATIVE_BEGIN;
			bSuccess = Generic_InsertDataTableRow(Table, RowName, ValuePtr);
			P_NATIVE_END;
		}
		else
		{
			FBlueprintExceptionInfo ExceptionInfo(EBlueprintExceptionType::AccessViolation, FText::FromString("Input type does not match DataTable type"));
			FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
		}
	}
	else
	{
		FBlueprintExceptionInfo ExceptionInfo(EBlueprintExceptionType::AccessViolation, FText::FromString("Input missing"));
		FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
	}
	*(bool*)RESULT_PARAM = bSuccess;
}