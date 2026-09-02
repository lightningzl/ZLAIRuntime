#include "SocialSandbox/Domain/ZLSocialSandboxNpcProfile.h"

bool FZLSocialSandboxNpcProfile::IsValid() const
{
	return !StableId.IsNone()
		&& !DisplayName.IsEmptyOrWhitespace()
		&& !Role.TrimStartAndEnd().IsEmpty()
		&& !SpeakingStyle.TrimStartAndEnd().IsEmpty()
		&& !Personality.IsEmpty()
		&& FMath::IsWithinInclusive(Trust, -1.0f, 1.0f)
		&& FMath::IsWithinInclusive(Affinity, -1.0f, 1.0f)
		&& FMath::IsWithinInclusive(RelationshipFear, 0.0f, 1.0f)
		&& FMath::IsWithinInclusive(Familiarity, 0.0f, 1.0f);
}

FZLSocialSandboxNpcProfile FZLSocialSandboxNpcProfile::Create(const FName StableId)
{
	FZLSocialSandboxNpcProfile Profile;
	Profile.StableId = StableId;
	if (StableId == TEXT("npc_guard"))
	{
		Profile.DisplayName = FText::FromString(TEXT("守卫"));
		Profile.Role = TEXT("order-focused town guard");
		Profile.Personality = {TEXT("cautious"), TEXT("dutiful"), TEXT("protective")};
		Profile.SpeakingStyle = TEXT("brief, formal, and commanding");
		Profile.Goals = {TEXT("keep public order"), TEXT("protect nearby civilians")};
		Profile.Trust = 0.0f;
		Profile.Familiarity = 0.35f;
		Profile.Alert = 0.45f;
		Profile.BodyColor = FLinearColor(0.12f, 0.32f, 0.82f);
	}
	else if (StableId == TEXT("npc_merchant"))
	{
		Profile.DisplayName = FText::FromString(TEXT("商人"));
		Profile.Role = TEXT("pragmatic market merchant");
		Profile.Personality = {TEXT("sociable"), TEXT("calculating"), TEXT("risk-averse")};
		Profile.SpeakingStyle = TEXT("courteous, observant, and transactional");
		Profile.Goals = {TEXT("protect the stall"), TEXT("avoid costly conflict")};
		Profile.Trust = 0.15f;
		Profile.Affinity = 0.1f;
		Profile.Familiarity = 0.5f;
		Profile.Curiosity = 0.35f;
		Profile.BodyColor = FLinearColor(0.92f, 0.5f, 0.08f);
	}
	else if (StableId == TEXT("npc_rival"))
	{
		Profile.DisplayName = FText::FromString(TEXT("旧敌"));
		Profile.Role = TEXT("proud rival with a prior grievance");
		Profile.Personality = {TEXT("proud"), TEXT("bold in public"), TEXT("resentful")};
		Profile.SpeakingStyle = TEXT("sharp, personal, and unwilling to look weak");
		Profile.Goals = {TEXT("preserve reputation"), TEXT("settle the old grievance safely")};
		Profile.Trust = -0.65f;
		Profile.Affinity = -0.55f;
		Profile.RelationshipFear = 0.2f;
		Profile.Familiarity = 0.9f;
		Profile.Anger = 0.45f;
		Profile.Alert = 0.35f;
		Profile.BodyColor = FLinearColor(0.72f, 0.12f, 0.18f);
	}
	else
	{
		Profile.StableId = TEXT("npc_civilian");
		Profile.DisplayName = FText::FromString(TEXT("居民"));
		Profile.Role = TEXT("uninvolved local civilian");
		Profile.Personality = {TEXT("reserved"), TEXT("empathetic"), TEXT("conflict-averse")};
		Profile.SpeakingStyle = TEXT("plain, hesitant, and conciliatory");
		Profile.Goals = {TEXT("stay safe"), TEXT("avoid escalation")};
		Profile.Trust = 0.0f;
		Profile.Affinity = 0.05f;
		Profile.RelationshipFear = 0.1f;
		Profile.Familiarity = 0.1f;
		Profile.Fear = 0.2f;
		Profile.BodyColor = FLinearColor(0.18f, 0.68f, 0.28f);
	}
	return Profile;
}
