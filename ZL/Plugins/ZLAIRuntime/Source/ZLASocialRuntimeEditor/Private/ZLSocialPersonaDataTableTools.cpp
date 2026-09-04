#include "ZLSocialPersonaDataTableTools.h"

#include "Engine/DataTable.h"
#include "ScopedTransaction.h"
#include "ZLSocialPersona.h"
#include "ZLSocialPersonaJson.h"

bool FZLSocialPersonaDataTableTools::ImportBatchJson(UDataTable& DataTable, const FString& Json, FString& OutResult)
{
	if (DataTable.GetRowStruct() != FZLSocialPersonaRow::StaticStruct())
	{
		OutResult = TEXT("DataTable must use FZLSocialPersonaRow");
		return false;
	}
	TArray<FZLSocialPersonaData> Personas;
	if (!FZLSocialPersonaJsonCodec::DeserializeBatch(Json, Personas, OutResult))
	{
		return false;
	}
	const FScopedTransaction Transaction(NSLOCTEXT("ZLSocialPersona", "ImportBatch", "Import Persona JSON batch"));
	DataTable.Modify();
	for (const FZLSocialPersonaData& Persona : Personas)
	{
		FZLSocialPersonaRow Row;
		Row.Persona = Persona;
		DataTable.AddRow(Persona.StableId, Row);
	}
	DataTable.HandleDataTableChanged();
	DataTable.MarkPackageDirty();
	OutResult = FString::Printf(TEXT("Imported %d Persona rows"), Personas.Num());
	return true;
}

bool FZLSocialPersonaDataTableTools::ExportBatchJson(const UDataTable& DataTable, FString& OutJson, FString& OutResult)
{
	if (DataTable.GetRowStruct() != FZLSocialPersonaRow::StaticStruct())
	{
		OutResult = TEXT("DataTable must use FZLSocialPersonaRow");
		return false;
	}
	TArray<FZLSocialPersonaData> Personas;
	DataTable.ForeachRow<FZLSocialPersonaRow>(TEXT("ExportPersonaBatch"), [&Personas](const FName&, const FZLSocialPersonaRow& Row)
	{
		Personas.Add(Row.Persona);
	});
	if (!FZLSocialPersonaJsonCodec::SerializeBatch(Personas, OutJson, OutResult))
	{
		return false;
	}
	OutResult = FString::Printf(TEXT("Exported %d Persona rows"), Personas.Num());
	return true;
}
