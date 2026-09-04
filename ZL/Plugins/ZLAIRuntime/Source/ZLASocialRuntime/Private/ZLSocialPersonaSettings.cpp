#include "ZLSocialPersonaSettings.h"

#include "DataRegistry.h"
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
		const UDataRegistry* Registry = RegistryReference.Get();
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
