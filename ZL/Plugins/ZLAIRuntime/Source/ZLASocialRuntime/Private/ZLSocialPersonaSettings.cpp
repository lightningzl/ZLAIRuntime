#include "ZLSocialPersonaSettings.h"

#include "DataRegistry.h"
#include "DataRegistryId.h"
#include "ZLSocialPersona.h"

bool UZLSocialPersonaSettings::IsConfiguredRegistry(const UDataRegistry* Registry) const
{
	return Registry && PersonaRegistries.ContainsByPredicate([Registry](const TSoftObjectPtr<UDataRegistry>& Candidate)
	{
		return Candidate.Get() == Registry || Candidate.ToSoftObjectPath().ToString() == Registry->GetPathName();
	});
}

void UZLSocialPersonaSettings::GetConfiguredPersonaIds(TArray<FName>& OutPersonaIds) const
{
	OutPersonaIds.Reset();
	for (const TSoftObjectPtr<UDataRegistry>& RegistryReference : PersonaRegistries)
	{
		// Editor option lists need the explicitly configured asset available even when it has not been opened yet.
		const UDataRegistry* Registry = RegistryReference.LoadSynchronous();
		if (!Registry || !Registry->DoesItemStructMatchFilter(FZLSocialPersonaRow::StaticStruct()->GetFName()))
		{
			continue;
		}

		TArray<FName> RegistryIds;
		Registry->GetItemNames(RegistryIds);
		OutPersonaIds.Append(RegistryIds);
	}
	OutPersonaIds.Sort(FNameLexicalLess());
	for (int32 Index = OutPersonaIds.Num() - 1; Index > 0; --Index)
	{
		if (OutPersonaIds[Index] == OutPersonaIds[Index - 1])
		{
			OutPersonaIds.RemoveAt(Index, 1, EAllowShrinking::No);
		}
	}
}

bool UZLSocialPersonaSettings::ResolveConfiguredPersona(const FName StableId, FZLSocialPersonaData& OutPersona, FString& OutError) const
{
	if (StableId.IsNone())
	{
		OutError = TEXT("Persona ID is required");
		return false;
	}
	const FZLSocialPersonaData* FoundPersona = nullptr;
	for (const TSoftObjectPtr<UDataRegistry>& RegistryReference : PersonaRegistries)
	{
		const UDataRegistry* Registry = RegistryReference.Get();
		if (!Registry || !Registry->DoesItemStructMatchFilter(FZLSocialPersonaRow::StaticStruct()->GetFName()))
		{
			continue;
		}
		const FZLSocialPersonaRow* Row = Registry->GetCachedItem<FZLSocialPersonaRow>(FDataRegistryId(Registry->GetRegistryType(), StableId));
		if (!Row)
		{
			continue;
		}
		if (FoundPersona)
		{
			OutError = TEXT("Persona ID resolves from more than one configured Registry");
			return false;
		}
		FoundPersona = &Row->Persona;
	}
	if (!FoundPersona || !FoundPersona->IsValid(&OutError))
	{
		if (OutError.IsEmpty()) { OutError = TEXT("Persona ID is not available from configured Registries"); }
		return false;
	}
	OutPersona = *FoundPersona;
	OutError.Reset();
	return true;
}
