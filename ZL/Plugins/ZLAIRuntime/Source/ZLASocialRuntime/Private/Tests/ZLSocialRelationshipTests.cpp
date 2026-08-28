#include "Misc/AutomationTest.h"
#include "ZLSocialEventRouter.h"
#include "ZLSocialRelationship.h"
#include "ZLSocialTags.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FZLSocialRelationshipTest, "ZL.Social.Relationship.SparseBoundsAndAuthority", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FZLSocialRelationshipTest::RunTest(const FString& Parameters)
{
	FZLSocialRelationshipStore Store;
	TArray<FZLSocialAgentProfile> Population;
	Population.SetNum(125);
	for (int32 Index = 0; Index < Population.Num(); ++Index)
	{
		Population[Index].AgentId = FName(*FString::Printf(TEXT("agent_%03d"), Index));
	}
	Population[0].AgentId = TEXT("victim");
	TestEqual(TEXT("Population does not prebuild relationship graph"), Store.GetRelationshipEdgeCount(), 0);

	FZLSocialEventRouter Router;
	FZLSocialEvent Punch;
	TestTrue(TEXT("Punch is created"), Router.CreateEvent(ZLSocialTags::Event_Punch, TEXT("player"), TEXT("victim"), FVector::ZeroVector, 1.0, Punch));
	FZLSocialPerceptionResult Direct;
	Direct.bPerceived = true;
	Direct.Channel = EZLSocialPerceptionChannel::Direct;
	Direct.EffectiveIntensity = 1.0f;
	FZLSocialRelationshipDelta DirectDelta;
	TestTrue(TEXT("Direct victim relationship updates"), Store.ApplyPersonalEvent(Punch, Population[0], Direct, 1.0, &DirectDelta));
	TestEqual(TEXT("One relevant interaction creates one sparse edge"), Store.GetRelationshipEdgeCount(), 1);
	TestTrue(TEXT("Punch reduces trust"), DirectDelta.Trust < 0.0f);
	TestFalse(TEXT("Same root does not apply twice"), Store.ApplyPersonalEvent(Punch, Population[0], Direct, 1.1));

	FZLSocialEvent VisualPunch;
	TestTrue(TEXT("Second punch is created"), Router.CreateEvent(ZLSocialTags::Event_Punch, TEXT("player"), NAME_None, FVector::ZeroVector, 2.0, VisualPunch));
	FZLSocialPerceptionResult Visual = Direct;
	Visual.Channel = EZLSocialPerceptionChannel::Visual;
	Visual.EffectiveIntensity = 0.8f;
	FZLSocialRelationshipDelta VisualDelta;
	TestTrue(TEXT("Visual witness relationship updates"), Store.ApplyPersonalEvent(VisualPunch, Population[1], Visual, 2.0, &VisualDelta));
	TestTrue(TEXT("Direct personal impact exceeds visual witness impact"), FMath::Abs(DirectDelta.Trust) > FMath::Abs(VisualDelta.Trust));

	FZLSocialAgentProfile Ordinary = Population[2];
	Ordinary.FactionId = TEXT("guard");
	FZLSocialPerceptionResult Social = Direct;
	Social.Channel = EZLSocialPerceptionChannel::Social;
	Social.EffectiveIntensity = 0.9f;
	TestFalse(TEXT("Ordinary witness cannot update faction standing"), Store.ConfirmFactionStanding(Punch, Ordinary, Social, 1.0));

	FZLSocialAgentProfile Authority = Population[3];
	Authority.AgentLevel = EZLSocialAgentLevel::Important;
	Authority.FactionId = TEXT("guard");
	Authority.bHasFactionAuthority = true;
	FZLSocialPerceptionResult LowConfidence = Social;
	LowConfidence.EffectiveIntensity = 0.59f;
	TestFalse(TEXT("Low confidence authority confirmation is rejected"), Store.ConfirmFactionStanding(Punch, Authority, LowConfidence, 1.0));
	TestTrue(TEXT("Qualified authority updates faction standing"), Store.ConfirmFactionStanding(Punch, Authority, Social, 1.0));
	const FZLSocialFactionStandingState* Standing = Store.FindFactionStanding(TEXT("guard"), TEXT("player"));
	TestNotNull(TEXT("Sparse faction standing is created"), Standing);
	TestTrue(TEXT("Punch lowers faction standing"), Standing != nullptr && Standing->Standing < 0.0f);
	const float StandingAfterFirstUpdate = Standing != nullptr ? Standing->Standing : 0.0f;
	TestFalse(TEXT("Same root cannot update faction twice"), Store.ConfirmFactionStanding(Punch, Authority, Social, 1.1));
	Standing = Store.FindFactionStanding(TEXT("guard"), TEXT("player"));
	TestTrue(TEXT("Duplicate root leaves standing unchanged"), Standing != nullptr && FMath::IsNearlyEqual(Standing->Standing, StandingAfterFirstUpdate));

	FZLSocialEvent Help;
	TestTrue(TEXT("Help is created"), Router.CreateEvent(ZLSocialTags::Event_Help, TEXT("player"), Authority.AgentId, FVector::ZeroVector, 3.0, Help));
	TestTrue(TEXT("Authority can confirm positive standing"), Store.ConfirmFactionStanding(Help, Authority, Direct, 3.0));
	Store.DecayTowardsNeutral(10.0f);
	const FZLSocialRelationshipState* Relationship = Store.FindRelationship(Population[0].AgentId, TEXT("player"));
	TestTrue(TEXT("Long-term values stay bounded"), Relationship != nullptr && Relationship->Trust >= -1.0f && Relationship->Trust <= 1.0f && Relationship->Fear >= 0.0f && Relationship->Fear <= 1.0f);
	return true;
}

#endif
