#include "Misc/AutomationTest.h"
#include "ZLSocialToolRegistry.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	FZLSocialToolCall MakeMoveAwayCall(const TCHAR* CallId = TEXT("call-1"))
	{
		FZLSocialToolCall Call;
		Call.CallId = CallId;
		Call.Name = TEXT("move_away");
		Call.TargetId = TEXT("player");
		Call.StateVersion = 7;
		return Call;
	}

	FZLSocialToolValidationContext MakeValidContext()
	{
		FZLSocialToolValidationContext Context;
		Context.CurrentStateVersion = 7;
		Context.NowSeconds = 10.0;
		Context.DistanceToTarget = 500.0f;
		Context.Capabilities.Add(TEXT("Tool.MoveAway"));
		return Context;
	}

	FName ValidateOnce(
		const FZLSocialToolCall& Call,
		const FZLSocialToolValidationContext& Context)
	{
		FZLSocialToolRegistry Registry;
		Registry.RegisterMilestone8Defaults();
		return Registry.ValidateAndCommit(Call, Context).ReasonCode;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FZLSocialToolRegistryValidationTest,
	"ZL.Social.ToolRegistry.Validation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FZLSocialToolRegistryValidationTest::RunTest(const FString& Parameters)
{
	FZLSocialToolRegistry Registry;
	Registry.RegisterMilestone8Defaults();
	TestEqual(TEXT("Milestone 8 exposes only its four allowed tools"), Registry.GetDefinitionCount(), 4);

	const FZLSocialToolCall ValidCall = MakeMoveAwayCall();
	const FZLSocialToolValidationContext ValidContext = MakeValidContext();
	const FZLSocialToolValidationResult Accepted = Registry.ValidateAndCommit(ValidCall, ValidContext);
	TestTrue(TEXT("A valid allowlisted call is accepted"), Accepted.bAccepted);
	TestEqual(TEXT("Accepted call has a stable reason"), Accepted.ReasonCode, ZLSocialToolReason::Accepted);
	TestEqual(TEXT("Accepted call is remembered"), Registry.GetRememberedCallCount(), 1);

	const FZLSocialToolValidationResult Duplicate = Registry.ValidateAndCommit(ValidCall, ValidContext);
	TestFalse(TEXT("A duplicate call is rejected"), Duplicate.bAccepted);
	TestEqual(TEXT("Duplicate rejection is explicit"), Duplicate.ReasonCode, ZLSocialToolReason::DuplicateCall);
	TestEqual(TEXT("A duplicate does not grow idempotency state"), Registry.GetRememberedCallCount(), 1);

	FZLSocialToolCall Call = ValidCall;
	FZLSocialToolValidationContext Context = ValidContext;
	Call.Name = TEXT("unregistered_tool");
	TestEqual(TEXT("Unknown tools are rejected"), ValidateOnce(Call, Context), ZLSocialToolReason::UnknownTool);

	Call = ValidCall;
	Call.CallId.Reset();
	TestEqual(TEXT("Blank call ids are rejected"), ValidateOnce(Call, Context), ZLSocialToolReason::InvalidCallId);

	Call = ValidCall;
	Context = ValidContext;
	Context.Capabilities.Reset();
	TestEqual(TEXT("Missing capabilities are rejected"), ValidateOnce(Call, Context), ZLSocialToolReason::MissingCapability);

	Call = ValidCall;
	Context = ValidContext;
	Call.TargetId = NAME_None;
	TestEqual(TEXT("Targeted tools require a valid target"), ValidateOnce(Call, Context), ZLSocialToolReason::InvalidTarget);

	Call = ValidCall;
	Context = ValidContext;
	Call.StateVersion = 6;
	TestEqual(TEXT("Stale state versions are rejected"), ValidateOnce(Call, Context), ZLSocialToolReason::StateVersionMismatch);

	Call = ValidCall;
	Context = ValidContext;
	Context.bRequestFresh = false;
	TestEqual(TEXT("Expired requests are rejected"), ValidateOnce(Call, Context), ZLSocialToolReason::Expired);

	Context = ValidContext;
	Context.DistanceToTarget = 3001.0f;
	TestEqual(TEXT("Out-of-range targets are rejected"), ValidateOnce(ValidCall, Context), ZLSocialToolReason::DistanceExceeded);

	Context = ValidContext;
	Context.bNavigationReachable = false;
	TestEqual(TEXT("Unreachable navigation is rejected"), ValidateOnce(ValidCall, Context), ZLSocialToolReason::NavigationUnavailable);

	Context = ValidContext;
	Context.bExecutable = false;
	TestEqual(TEXT("Invalid execution state is rejected"), ValidateOnce(ValidCall, Context), ZLSocialToolReason::InvalidState);

	Context = ValidContext;
	Context.ExecutionsInWindow = FZLSocialToolRegistry::MaxExecutionsPerWindow;
	TestEqual(TEXT("The execution window is rate limited"), ValidateOnce(ValidCall, Context), ZLSocialToolReason::RateLimited);

	FZLSocialToolRegistry CooldownRegistry;
	CooldownRegistry.RegisterMilestone8Defaults();
	CooldownRegistry.ValidateAndCommit(ValidCall, ValidContext);
	Call = MakeMoveAwayCall(TEXT("call-2"));
	Context = ValidContext;
	Context.NowSeconds += 0.1;
	TestEqual(TEXT("Repeated tool use observes its cooldown"), CooldownRegistry.ValidateAndCommit(Call, Context).ReasonCode, ZLSocialToolReason::Cooldown);

	FZLSocialToolRegistry RejectionRegistry;
	RejectionRegistry.RegisterMilestone8Defaults();
	Context = ValidContext;
	Context.Capabilities.Reset();
	TestFalse(TEXT("Rejected calls do not commit"), RejectionRegistry.ValidateAndCommit(ValidCall, Context).bAccepted);
	TestEqual(TEXT("Rejected calls leave idempotency state empty"), RejectionRegistry.GetRememberedCallCount(), 0);
	TestTrue(TEXT("The same call id can succeed after its validation issue is fixed"), RejectionRegistry.ValidateAndCommit(ValidCall, ValidContext).bAccepted);

	FZLSocialToolRegistry BoundedRegistry;
	BoundedRegistry.RegisterMilestone8Defaults();
	FZLSocialToolCall StopCall;
	StopCall.Name = TEXT("stop");
	StopCall.StateVersion = 7;
	Context = ValidContext;
	Context.Capabilities.Reset();
	Context.Capabilities.Add(TEXT("Tool.Stop"));
	for (int32 Index = 0; Index < FZLSocialToolRegistry::MaxRememberedCalls + 1; ++Index)
	{
		StopCall.CallId = FString::Printf(TEXT("bounded-%d"), Index);
		Context.NowSeconds = 10.0 + Index;
		TestTrue(TEXT("Bounded history setup call is accepted"), BoundedRegistry.ValidateAndCommit(StopCall, Context).bAccepted);
	}
	TestEqual(
		TEXT("Idempotency history stays bounded"),
		BoundedRegistry.GetRememberedCallCount(),
		FZLSocialToolRegistry::MaxRememberedCalls);

	return true;
}

#endif
