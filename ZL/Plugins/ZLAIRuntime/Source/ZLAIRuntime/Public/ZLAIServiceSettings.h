#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"

#include "ZLAIServiceSettings.generated.h"

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "ZL AI Service"))
class ZLAIRUNTIME_API UZLAIServiceSettings final : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category = "Service")
	FString ServiceBaseUrl = TEXT("http://127.0.0.1:8000");

	UPROPERTY(Config, EditAnywhere, Category = "Service", meta = (ClampMin = "0.1", UIMin = "0.1", Units = "s"))
	float RequestTimeoutSeconds = 30.0f;
};
