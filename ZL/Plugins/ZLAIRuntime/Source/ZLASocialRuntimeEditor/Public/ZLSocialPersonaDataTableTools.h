#pragma once

#include "CoreMinimal.h"

class UDataTable;

class ZLASOCIALRUNTIMEEDITOR_API FZLSocialPersonaDataTableTools
{
public:
	static bool ImportBatchJson(UDataTable& DataTable, const FString& Json, FString& OutResult);
	static bool ExportBatchJson(const UDataTable& DataTable, FString& OutJson, FString& OutResult);
};
