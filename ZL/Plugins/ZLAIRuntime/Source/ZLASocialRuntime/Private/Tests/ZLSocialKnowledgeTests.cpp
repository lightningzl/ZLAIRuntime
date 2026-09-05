#include "Misc/AutomationTest.h"
#include "ZLSocialKnowledge.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FZLSocialKnowledgeIsolationTest, "ZL.Social.Knowledge.Isolation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FZLSocialKnowledgeIsolationTest::RunTest(const FString&)
{
	const double Now = 10.0;
	FZLSocialKnowledgeStore Store;
	FZLSocialWorldFact Fact; Fact.FactId = TEXT("night_curfew"); Fact.Summary = TEXT("A curfew has been confirmed."); Fact.CauseEventId = FGuid::NewGuid(); Fact.ConfirmedAtSeconds = Now; Fact.ExpiresAtSeconds = Now + 60.0;
	TestTrue(TEXT("UE authority records a valid world fact"), Store.RecordWorldFact(Fact, Now));
	FZLSocialKnowledgeItem Item; Item.FactId = Fact.FactId; Item.Summary = Fact.Summary; Item.Source = EZLSocialKnowledgeSource::DirectPerception; Item.SourceAgentId = TEXT("player"); Item.Confidence = 1.0f; Item.LearnedAtSeconds = Now; Item.ExpiresAtSeconds = Now + 30.0; Item.UpdateReason = TEXT("personally witnessed");
	TestTrue(TEXT("Only the witnessing NPC learns the fact"), Store.Learn(TEXT("npc_guard"), Item, Now));
	TestNotNull(TEXT("Witness knowledge is retained"), Store.Find(TEXT("npc_guard"), Fact.FactId));
	TestNull(TEXT("Uninformed NPC cannot read another NPC knowledge"), Store.Find(TEXT("npc_civilian"), Fact.FactId));
	TestTrue(TEXT("A valid rebuttal updates the witness belief"), Store.Rebut(TEXT("npc_guard"), Fact.FactId, TEXT("npc_merchant"), TEXT("merchant disputes the scope"), Now));
	TestEqual(TEXT("Rebuttal is visible only to its owner"), Store.Find(TEXT("npc_guard"), Fact.FactId)->Source, EZLSocialKnowledgeSource::Rebuttal);
	FZLSocialKnowledgeItem Report = Item;
	Report.Source = EZLSocialKnowledgeSource::ConfirmedReport;
	Report.SourceAgentId = TEXT("npc_guard");
	Report.Confidence = 0.8f;
	Report.UpdateReason = TEXT("confirmed report");
	TestTrue(TEXT("A named receiver can learn from a confirmed report"), Store.Learn(TEXT("npc_merchant"), Report, Now));
	TestNull(TEXT("A report is not implicitly broadcast to other NPCs"), Store.Find(TEXT("npc_civilian"), Fact.FactId));
	FZLSocialKnowledgeItem Rumor = Report;
	Rumor.Source = EZLSocialKnowledgeSource::BoundedPropagation;
	Rumor.SourceAgentId = TEXT("npc_merchant");
	Rumor.Confidence = 0.48f;
	Rumor.UpdateReason = TEXT("single-hop rumor");
	TestTrue(TEXT("A bounded rumor can be recorded for one selected NPC"), Store.Learn(TEXT("npc_civilian"), Rumor, Now));
	TestEqual(TEXT("Rumor keeps a distinct lower-confidence source"), Store.Find(TEXT("npc_civilian"), Fact.FactId)->Source, EZLSocialKnowledgeSource::BoundedPropagation);
	Store.ForgetExpired(Now + 61.0);
	TestNull(TEXT("Expired knowledge is forgotten"), Store.Find(TEXT("npc_guard"), Fact.FactId));
	return true;
}
#endif
