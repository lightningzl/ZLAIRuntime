#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "UObject/SoftObjectPath.h"

#include "ZLSocialPersona.generated.h"

namespace ZLSocialPersonaLimits
{
	inline constexpr int32 MaxStableIdLength = 64;
	inline constexpr int32 MaxDisplayNameLength = 64;
	inline constexpr int32 MaxBackgroundSummaryLength = 512;
	inline constexpr int32 MaxRoleLength = 128;
	inline constexpr int32 MaxListItems = 8;
	inline constexpr int32 MaxListItemLength = 128;
	inline constexpr int32 MaxSpeakingStyleLength = 256;
}

USTRUCT(BlueprintType)
struct ZLASOCIALRUNTIME_API FZLSocialPersonaRelationship
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relationship", meta = (DisplayName = "信任", ToolTip = "NPC 对初始交互对象的信任程度，范围 -1 到 1。", ClampMin = "-1.0", ClampMax = "1.0"))
	float Trust = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relationship", meta = (DisplayName = "好感", ToolTip = "NPC 对初始交互对象的亲近或排斥程度，范围 -1 到 1。", ClampMin = "-1.0", ClampMax = "1.0"))
	float Affinity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relationship", meta = (DisplayName = "关系恐惧", ToolTip = "NPC 对初始交互对象的恐惧程度，范围 0 到 1。", ClampMin = "0.0", ClampMax = "1.0"))
	float Fear = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relationship", meta = (DisplayName = "熟悉度", ToolTip = "NPC 对初始交互对象的熟悉程度，范围 0 到 1。", ClampMin = "0.0", ClampMax = "1.0"))
	float Familiarity = 0.0f;

	bool IsValid() const;
};

USTRUCT(BlueprintType)
struct ZLASOCIALRUNTIME_API FZLSocialPersonaInstantState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Instant State", meta = (DisplayName = "恐惧", ToolTip = "NPC 进入场景时的恐惧值，范围 0 到 1。", ClampMin = "0.0", ClampMax = "1.0"))
	float Fear = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Instant State", meta = (DisplayName = "愤怒", ToolTip = "NPC 进入场景时的愤怒值，范围 0 到 1。", ClampMin = "0.0", ClampMax = "1.0"))
	float Anger = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Instant State", meta = (DisplayName = "好奇", ToolTip = "NPC 进入场景时的好奇值，范围 0 到 1。", ClampMin = "0.0", ClampMax = "1.0"))
	float Curiosity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Instant State", meta = (DisplayName = "警觉", ToolTip = "NPC 进入场景时的警觉值，范围 0 到 1。", ClampMin = "0.0", ClampMax = "1.0"))
	float Alert = 0.0f;

	bool IsValid() const;
};

USTRUCT(BlueprintType)
struct ZLASOCIALRUNTIME_API FZLSocialPersonaData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity", meta = (DisplayName = "稳定 ID", ToolTip = "Persona 的稳定唯一标识；DataTable 行名与 DataRegistry ID 应使用相同值，最长 64 个字符。"))
	FName StableId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity", meta = (DisplayName = "显示名称", ToolTip = "场景和 Inspector 中显示的 NPC 名称，最长 64 个字符。"))
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity", meta = (DisplayName = "背景摘要", ToolTip = "NPC 的静态公开背景摘要，供现有 Decision Context 使用；不保存运行时事实。", MultiLine = true))
	FString BackgroundSummary;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity", meta = (DisplayName = "身份角色", ToolTip = "NPC 在场景中的身份或职业描述，最长 128 个字符。"))
	FString Role;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Personality", meta = (DisplayName = "性格特征", ToolTip = "1 至 8 条性格描述，每条最长 128 个字符。"))
	TArray<FString> Personality;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Personality", meta = (DisplayName = "表达风格", ToolTip = "NPC 的语言表达倾向，最长 256 个字符。", MultiLine = true))
	FString SpeakingStyle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Personality", meta = (DisplayName = "初始目标", ToolTip = "1 至 8 条初始目标，每条最长 128 个字符。"))
	TArray<FString> Goals;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Initial State", meta = (DisplayName = "初始关系", ToolTip = "NPC 的初始关系数值；运行中会由 UE 社会系统独立维护。"))
	FZLSocialPersonaRelationship InitialRelationship;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Initial State", meta = (DisplayName = "初始即时状态", ToolTip = "NPC 生成时的情绪起点；运行中会随事件变化。"))
	FZLSocialPersonaInstantState InitialInstantState;

	bool IsValid(FString* OutError = nullptr) const;
};

USTRUCT(BlueprintType)
struct ZLASOCIALRUNTIME_API FZLSocialPersonaRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Persona", meta = (DisplayName = "Persona 配置", ToolTip = "NPC 的静态 Persona 起点，不包含运行时个人事实、历史或服务响应。"))
	FZLSocialPersonaData Persona;

	bool IsValid(FString* OutError = nullptr) const { return Persona.IsValid(OutError); }
};

UCLASS(BlueprintType)
class ZLASOCIALRUNTIME_API UZLSocialPersonaAsset final : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Persona")
	FZLSocialPersonaData Persona;

	UPROPERTY(EditAnywhere, Category = "JSON", meta = (DisplayName = "导入 JSON 文件", ToolTip = "可选 Persona JSON 文件路径；也可使用粘贴 JSON 导入。"))
	FFilePath ImportFile;

	UPROPERTY(EditAnywhere, Category = "JSON", meta = (DisplayName = "导出 JSON 文件", ToolTip = "导出严格验证的完整 Persona JSON 的目标路径。"))
	FFilePath ExportFile;

	UPROPERTY(VisibleAnywhere, Transient, Category = "JSON", meta = (DisplayName = "最近 JSON 操作结果", ToolTip = "最近一次 Persona JSON 导入或导出的成功结果或拒绝原因。"))
	FString LastJsonOperationResult;

	UFUNCTION(CallInEditor, Category = "JSON")
	void ImportPersonaJson();

	UFUNCTION(CallInEditor, Category = "JSON")
	void ExportPersonaJson();

	/** Validates and applies a pasted single-Persona JSON document. */
	bool ImportPersonaJsonText(const FString& Json);

	bool IsValid(FString* OutError = nullptr) const { return Persona.IsValid(OutError); }
};
