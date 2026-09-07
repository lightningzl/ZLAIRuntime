#pragma once

#include "CoreMinimal.h"

class AZLSocialSandboxNpc;
struct FZLSocialSandboxDecisionDebug;
struct FZLDecisionV2SocialFact;
struct FZLSocialKnowledgeItem;

class ZL_API FZLSocialSandboxInspectorPresenter
{
public:
	static FText Build(
		const AZLSocialSandboxNpc* Npc,
		const FZLSocialSandboxDecisionDebug* DecisionDebug,
		const TArray<FZLDecisionV2SocialFact>& SocialFacts,
		const TArray<FZLSocialKnowledgeItem>& Knowledge);
};
