#include "ZLSocialActionParser.h"

FZLSocialActionParseResult FZLSocialActionParser::Parse(const FString& Input)
{
	FString Normalized = Input.TrimStartAndEnd().ToLower();
	Normalized.ReplaceInline(TEXT("_"), TEXT(" "));
	while (Normalized.Contains(TEXT("  ")))
	{
		Normalized.ReplaceInline(TEXT("  "), TEXT(" "));
	}

	struct FAlias
	{
		const TCHAR* Text;
		EZLSocialActionType Action;
		const TCHAR* Id;
	};
	static const FAlias Aliases[] = {
		{ TEXT("face"), EZLSocialActionType::Face, TEXT("face") },
		{ TEXT("face target"), EZLSocialActionType::Face, TEXT("face_target") },
		{ TEXT("面向"), EZLSocialActionType::Face, TEXT("face_zh") },
		{ TEXT("面对"), EZLSocialActionType::Face, TEXT("face_zh") },
		{ TEXT("转向"), EZLSocialActionType::Face, TEXT("face_zh") },
		{ TEXT("approach"), EZLSocialActionType::Approach, TEXT("approach") },
		{ TEXT("move toward"), EZLSocialActionType::Approach, TEXT("move_toward") },
		{ TEXT("靠近"), EZLSocialActionType::Approach, TEXT("approach_zh") },
		{ TEXT("走近"), EZLSocialActionType::Approach, TEXT("approach_zh") },
		{ TEXT("move away"), EZLSocialActionType::MoveAway, TEXT("move_away") },
		{ TEXT("back away"), EZLSocialActionType::MoveAway, TEXT("back_away") },
		{ TEXT("远离"), EZLSocialActionType::MoveAway, TEXT("move_away_zh") },
		{ TEXT("退开"), EZLSocialActionType::MoveAway, TEXT("move_away_zh") },
		{ TEXT("attack"), EZLSocialActionType::Attack, TEXT("attack") },
		{ TEXT("strike"), EZLSocialActionType::Attack, TEXT("strike") },
		{ TEXT("攻击"), EZLSocialActionType::Attack, TEXT("attack_zh") },
		{ TEXT("打击"), EZLSocialActionType::Attack, TEXT("attack_zh") },
		{ TEXT("stop"), EZLSocialActionType::Stop, TEXT("stop") },
		{ TEXT("停止"), EZLSocialActionType::Stop, TEXT("stop_zh") },
		{ TEXT("停下"), EZLSocialActionType::Stop, TEXT("stop_zh") }
	};

	for (const FAlias& Alias : Aliases)
	{
		if (Normalized == Alias.Text)
		{
			FZLSocialActionParseResult Result;
			Result.bMatched = true;
			Result.Action = Alias.Action;
			Result.AliasId = Alias.Id;
			return Result;
		}
	}
	return FZLSocialActionParseResult();
}
