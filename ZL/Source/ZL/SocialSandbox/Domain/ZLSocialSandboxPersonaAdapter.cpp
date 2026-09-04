#include "SocialSandbox/Domain/ZLSocialSandboxPersonaAdapter.h"

bool FZLSocialSandboxPersonaAdapter::ToNpcProfile(const FZLSocialPersonaData& Persona, const FLinearColor& BodyColor, FZLSocialSandboxNpcProfile& OutProfile, FString& OutError)
{
	if (!Persona.IsValid(&OutError))
	{
		return false;
	}

	FZLSocialSandboxNpcProfile Candidate;
	Candidate.StableId = Persona.StableId;
	Candidate.DisplayName = FText::FromString(Persona.DisplayName);
	Candidate.Role = Persona.Role;
	Candidate.Personality = Persona.Personality;
	Candidate.SpeakingStyle = Persona.SpeakingStyle;
	Candidate.Goals = Persona.Goals;
	Candidate.Trust = Persona.InitialRelationship.Trust;
	Candidate.Affinity = Persona.InitialRelationship.Affinity;
	Candidate.RelationshipFear = Persona.InitialRelationship.Fear;
	Candidate.Familiarity = Persona.InitialRelationship.Familiarity;
	Candidate.Fear = Persona.InitialInstantState.Fear;
	Candidate.Anger = Persona.InitialInstantState.Anger;
	Candidate.Curiosity = Persona.InitialInstantState.Curiosity;
	Candidate.Alert = Persona.InitialInstantState.Alert;
	Candidate.BodyColor = BodyColor;
	if (!Candidate.IsValid())
	{
		OutError = TEXT("Persona cannot produce a valid sandbox NPC profile");
		return false;
	}
	OutProfile = MoveTemp(Candidate);
	OutError.Reset();
	return true;
}
