#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "SocialSandbox/ZLSocialSandboxDecisionContext.h"
#include "ZLAIServiceProtocol.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FZLSocialSandboxDecisionContextTest,
	"ZL.Social.Sandbox.PersonalDecisionContext",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FZLSocialSandboxDecisionContextTest::RunTest(const FString& Parameters)
{
	FZLSocialSandboxDecisionContextInput Input;
	Input.NpcId = TEXT("npc_guard");
	Input.DisplayName = FText::FromString(TEXT("Guard"));
	Input.StateVersion = 7;
	Input.TriggerSpeechContent = TEXT("Please keep your distance.");
	Input.TriggerObservation.EventId = FGuid::NewGuid();
	Input.TriggerObservation.ObserverId = TEXT("npc_guard");
	Input.TriggerObservation.SourceId = TEXT("player");
	Input.TriggerObservation.Source = EZLSocialObservationSource::Speech;
	Input.TriggerObservation.ExplicitTargetId = TEXT("npc_guard");
	Input.TriggerObservation.TargetJudgment = EZLSocialTargetJudgment::ExplicitSelf;
	Input.TriggerObservation.bHeard = true;
	Input.TriggerObservation.bHeardClearly = true;
	Input.TriggerObservation.ObservedAtSeconds = 12.0;

	FZLSocialObservation PersonalHistory;
	PersonalHistory.EventId = FGuid::NewGuid();
	PersonalHistory.ObserverId = TEXT("npc_guard");
	PersonalHistory.SourceId = TEXT("player");
	PersonalHistory.Source = EZLSocialObservationSource::Action;
	PersonalHistory.Action = EZLSocialActionType::Stop;
	PersonalHistory.ActionPhase = EZLSocialActionPhase::Completed;
	PersonalHistory.bSaw = true;
	PersonalHistory.ObservedAtSeconds = 10.0;
	Input.PersonalHistory.Add(PersonalHistory);
	FZLSocialObservation OtherNpcHistory = PersonalHistory;
	OtherNpcHistory.EventId = FGuid::NewGuid();
	OtherNpcHistory.ObserverId = TEXT("npc_merchant");
	Input.PersonalHistory.Add(OtherNpcHistory);
	FZLSocialSandboxPublicHistoryFact GuardSpeech;
	GuardSpeech.Kind = TEXT("speech");
	GuardSpeech.SourceId = TEXT("npc_guard");
	GuardSpeech.TargetId = TEXT("player");
	GuardSpeech.Summary = TEXT("The guard publicly warned the player to stop.");
	GuardSpeech.OccurredAtSeconds = 11.0;
	Input.PublicHistory.Add(GuardSpeech);

	FZLDecisionRequest Request;
	FString Error;
	TestTrue(TEXT("Selected NPC context builds"), FZLSocialSandboxDecisionContextBuilder::Build(Input, Request, Error));
	TestEqual(TEXT("Selected stable ID is preserved"), Request.NpcId, FString(TEXT("npc_guard")));
	TestEqual(TEXT("State version is preserved"), Request.StateVersion, static_cast<int64>(7));
	TestEqual(TEXT("Selected observations and public NPC facts survive"), Request.Context.RecentHistory.Num(), 2);
	TestEqual(TEXT("Public NPC speech preserves its source"), Request.Context.RecentHistory.Last().SourceId, FString(TEXT("npc_guard")));
	TestEqual(TEXT("Speech content comes from perceived trigger"), Request.Trigger.Content, Input.TriggerSpeechContent);
	TestTrue(TEXT("Direct channel is explicit"), Request.Trigger.Channels.Contains(TEXT("direct")));
	TestTrue(TEXT("Built context satisfies protocol"), ZLAIServiceProtocol::ValidateDecisionRequest(Request, Error));

	Input.TriggerObservation.bHeard = false;
	TestFalse(TEXT("Unheard speech cannot build Decision context"), FZLSocialSandboxDecisionContextBuilder::Build(Input, Request, Error));
	return true;
}

#endif
