#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "ZLSocialWorldContext.generated.h"

namespace ZLSocialWorldContextLimits
{
	inline constexpr int32 MaxStableIdLength = 64;
	inline constexpr int32 MaxDisplayNameLength = 96;
	inline constexpr int32 MaxSummaryLength = 512;
	inline constexpr int32 MaxItems = 16;
}

/** A static rule that applies to the scene. It is not a runtime fact or NPC memory. */
USTRUCT(BlueprintType)
struct ZLASOCIALRUNTIME_API FZLSocialWorldRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Context", meta = (DisplayName = "规则 ID", ToolTip = "规则的稳定唯一标识；用于配置引用，最长 64 个字符。"))
	FName StableId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Context", meta = (DisplayName = "规则说明", ToolTip = "场景的静态公开规则。不能写入运行中的事件或 NPC 私有记忆。", MultiLine = true))
	FString Summary;

	bool IsValid() const;
};

/** Public background for a named faction at the start of a scene. */
USTRUCT(BlueprintType)
struct ZLASOCIALRUNTIME_API FZLSocialFactionBackground
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Context", meta = (DisplayName = "势力 ID", ToolTip = "势力的稳定唯一标识；最长 64 个字符。"))
	FName FactionId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Context", meta = (DisplayName = "势力显示名", ToolTip = "在编辑器和调试信息中显示的势力名称，最长 96 个字符。"))
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Context", meta = (DisplayName = "势力背景摘要", ToolTip = "场景开始时公开的势力背景；不能表示动态立场或个人知识。", MultiLine = true))
	FString BackgroundSummary;

	bool IsValid() const;
};

/** Static common knowledge available as a scene starting condition, never a dynamic broadcast. */
USTRUCT(BlueprintType)
struct ZLASOCIALRUNTIME_API FZLSocialPublicKnowledgeSeed
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Context", meta = (DisplayName = "常识 ID", ToolTip = "公开常识的稳定唯一标识；最长 64 个字符。"))
	FName StableId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Context", meta = (DisplayName = "常识说明", ToolTip = "场景初始的公开常识，不会自动广播为动态知识。", MultiLine = true))
	FString Summary;

	bool IsValid() const;
};

USTRUCT(BlueprintType)
struct ZLASOCIALRUNTIME_API FZLSocialWorldContextData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity", meta = (DisplayName = "世界背景 ID", ToolTip = "World Context 的稳定唯一标识；最长 64 个字符。"))
	FName StableId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity", meta = (DisplayName = "世界背景显示名", ToolTip = "在编辑器中显示的场景世界背景名称，最长 96 个字符。"))
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Context", meta = (DisplayName = "世界规则", ToolTip = "最多 16 条静态公开规则。"))
	TArray<FZLSocialWorldRule> Rules;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Context", meta = (DisplayName = "势力背景", ToolTip = "最多 16 组势力背景。"))
	TArray<FZLSocialFactionBackground> Factions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Context", meta = (DisplayName = "初始公开常识", ToolTip = "最多 16 条场景开始时公开的常识。"))
	TArray<FZLSocialPublicKnowledgeSeed> PublicKnowledge;

	bool IsValid(FString* OutError = nullptr) const;
};

/** Editable static scene background. Dynamic events and NPC knowledge are deliberately excluded. */
UCLASS(BlueprintType)
class ZLASOCIALRUNTIME_API UZLSocialWorldContextAsset final : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Context", meta = (DisplayName = "世界背景配置", ToolTip = "静态公开世界背景；不会包含动态事件或 NPC 的个人知识。"))
	FZLSocialWorldContextData WorldContext;

	UPROPERTY(EditAnywhere, Category = "JSON", meta = (DisplayName = "导入 JSON 文件", ToolTip = "可选 JSON 文件路径；也可使用下方的粘贴 JSON 导入按钮。"))
	FFilePath ImportFile;

	UPROPERTY(EditAnywhere, Category = "JSON", meta = (DisplayName = "导出 JSON 文件", ToolTip = "导出严格校验后的完整 JSON 的目标文件路径。"))
	FFilePath ExportFile;

	UPROPERTY(VisibleAnywhere, Transient, Category = "JSON", meta = (DisplayName = "最近 JSON 操作结果", ToolTip = "最近一次导入或导出的成功结果或拒绝原因。"))
	FString LastJsonOperationResult;

	UFUNCTION(CallInEditor, Category = "JSON")
	void ImportWorldContextJson();

	UFUNCTION(CallInEditor, Category = "JSON")
	void ExportWorldContextJson();

	bool ImportWorldContextJsonText(const FString& Json);
	bool IsValid(FString* OutError = nullptr) const { return WorldContext.IsValid(OutError); }
};
