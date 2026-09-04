#include "ZLSocialPersonaSettings.h"

#include "DataRegistry.h"

bool UZLSocialPersonaSettings::IsConfiguredRegistry(const UDataRegistry* Registry) const
{
	return Registry && PersonaRegistries.ContainsByPredicate([Registry](const TSoftObjectPtr<UDataRegistry>& Candidate)
	{
		return Candidate.Get() == Registry || Candidate.ToSoftObjectPath().ToString() == Registry->GetPathName();
	});
}
