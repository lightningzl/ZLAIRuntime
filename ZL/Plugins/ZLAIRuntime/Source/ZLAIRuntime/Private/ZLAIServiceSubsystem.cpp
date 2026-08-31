#include "ZLAIServiceSubsystem.h"

#include "Async/Async.h"
#include "HttpModule.h"
#include "HAL/PlatformTime.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "ZLAIServiceProtocol.h"
#include "ZLAIServiceSettings.h"
#include "ZLAIRuntimeModule.h"

namespace
{
	FString BuildDialogueEndpoint(FString ServiceBaseUrl)
	{
		ServiceBaseUrl.TrimStartAndEndInline();
		while (ServiceBaseUrl.RemoveFromEnd(TEXT("/")))
		{
		}

		return ServiceBaseUrl + TEXT("/v1/dialogue");
	}

	FString BuildDecisionEndpoint(FString ServiceBaseUrl)
	{
		ServiceBaseUrl.TrimStartAndEndInline();
		while (ServiceBaseUrl.RemoveFromEnd(TEXT("/")))
		{
		}
		return ServiceBaseUrl + TEXT("/v1/decision");
	}

	const TCHAR* ErrorCategoryToString(const EZLServiceErrorCategory Category)
	{
		switch (Category)
		{
		case EZLServiceErrorCategory::Client: return TEXT("client");
		case EZLServiceErrorCategory::Network: return TEXT("network");
		case EZLServiceErrorCategory::Timeout: return TEXT("timeout");
		case EZLServiceErrorCategory::Http: return TEXT("http");
		case EZLServiceErrorCategory::Parse: return TEXT("parse");
		case EZLServiceErrorCategory::Stale: return TEXT("stale");
		default: return TEXT("unknown");
		}
	}

	void LogFailure(const FZLServiceError& Error)
	{
		UE_LOG(
			LogZLAIRuntime,
			Warning,
			TEXT("Dialogue request failed request_id=%s category=%s code=%s http_status=%d"),
			*Error.RequestId,
			ErrorCategoryToString(Error.Category),
			*Error.Code,
			Error.HttpStatusCode);
	}

	void DispatchFailure(FZLServiceError Error, FZLDialogueFailureDelegate OnFailure)
	{
		AsyncTask(ENamedThreads::GameThread, [Error = MoveTemp(Error), OnFailure = MoveTemp(OnFailure)]() mutable
		{
			LogFailure(Error);
			OnFailure.ExecuteIfBound(Error);
		});
	}

	void LogDecisionFailure(const FZLServiceError& Error)
	{
		UE_LOG(
			LogZLAIRuntime,
			Warning,
			TEXT("Decision request failed request_id=%s category=%s code=%s http_status=%d"),
			*Error.RequestId,
			ErrorCategoryToString(Error.Category),
			*Error.Code,
			Error.HttpStatusCode);
	}

	void DispatchDecisionFailure(FZLServiceError Error, FZLDecisionFailureDelegate OnFailure)
	{
		AsyncTask(ENamedThreads::GameThread, [Error = MoveTemp(Error), OnFailure = MoveTemp(OnFailure)]() mutable
		{
			LogDecisionFailure(Error);
			OnFailure.ExecuteIfBound(Error);
		});
	}
}

FString UZLAIServiceSubsystem::SendDecisionRequest(
	FZLDecisionRequest DecisionRequest,
	FZLDecisionSuccessDelegate OnSuccess,
	FZLDecisionFailureDelegate OnFailure)
{
	if (DecisionRequest.RequestId.IsEmpty())
	{
		DecisionRequest.RequestId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);
	}
	const UZLAIServiceSettings* Settings = GetDefault<UZLAIServiceSettings>();
	const FString ServiceBaseUrl = Settings->ServiceBaseUrl;
	const float RequestTimeoutSeconds = FMath::Max(Settings->RequestTimeoutSeconds, 0.1f);
	if (ServiceBaseUrl.TrimStartAndEnd().IsEmpty())
	{
		FZLServiceError Error;
		Error.Category = EZLServiceErrorCategory::Client;
		Error.RequestId = DecisionRequest.RequestId;
		Error.Code = TEXT("client_error");
		Error.Message = TEXT("Service base URL must not be empty");
		DispatchDecisionFailure(MoveTemp(Error), MoveTemp(OnFailure));
		return DecisionRequest.RequestId;
	}

	FString RequestBody;
	FString ValidationError;
	if (!ZLAIServiceProtocol::ValidateDecisionRequest(DecisionRequest, ValidationError)
		|| !ZLAIServiceProtocol::SerializeDecisionRequest(DecisionRequest, RequestBody))
	{
		FZLServiceError Error;
		Error.Category = EZLServiceErrorCategory::Client;
		Error.RequestId = DecisionRequest.RequestId;
		Error.Code = TEXT("client_error");
		Error.Message = ValidationError.IsEmpty()
			? TEXT("Failed to serialize Decision request")
			: MoveTemp(ValidationError);
		DispatchDecisionFailure(MoveTemp(Error), MoveTemp(OnFailure));
		return DecisionRequest.RequestId;
	}

	const double SentAtSeconds = FPlatformTime::Seconds();
	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL(BuildDecisionEndpoint(ServiceBaseUrl));
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));
	HttpRequest->SetContentAsString(RequestBody);
	HttpRequest->SetTimeout(RequestTimeoutSeconds);

	const TWeakObjectPtr<UZLAIServiceSubsystem> WeakThis(this);
	HttpRequest->OnProcessRequestComplete().BindLambda(
		[WeakThis,
			ExpectedRequestId = DecisionRequest.RequestId,
			ExpectedNpcId = DecisionRequest.NpcId,
			ExpectedStateVersion = DecisionRequest.StateVersion,
			SentAtSeconds,
			TtlMs = DecisionRequest.TtlMs,
			OnSuccess,
			OnFailure](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded) mutable
		{
			const int32 StatusCode = Response.IsValid() ? Response->GetResponseCode() : 0;
			const FString ResponseBody = Response.IsValid() ? Response->GetContentAsString() : FString();
			const bool bTimedOut = Request.IsValid()
				&& Request->GetFailureReason() == EHttpFailureReason::TimedOut;
			AsyncTask(ENamedThreads::GameThread,
				[WeakThis,
					ExpectedRequestId = MoveTemp(ExpectedRequestId),
					ExpectedNpcId = MoveTemp(ExpectedNpcId),
					ExpectedStateVersion,
					SentAtSeconds,
					TtlMs,
					bSucceeded,
					bTimedOut,
					StatusCode,
					ResponseBody,
					OnSuccess = MoveTemp(OnSuccess),
					OnFailure = MoveTemp(OnFailure)]() mutable
				{
					if (WeakThis.IsValid())
					{
						WeakThis->CompleteDecisionRequest(
							ExpectedRequestId,
							ExpectedNpcId,
							ExpectedStateVersion,
							SentAtSeconds,
							TtlMs,
							bSucceeded,
							bTimedOut,
							StatusCode,
							ResponseBody,
							MoveTemp(OnSuccess),
							MoveTemp(OnFailure));
					}
				});
		});

	if (!HttpRequest->ProcessRequest())
	{
		HttpRequest->OnProcessRequestComplete().Unbind();
		FZLServiceError Error;
		Error.Category = EZLServiceErrorCategory::Client;
		Error.RequestId = DecisionRequest.RequestId;
		Error.Code = TEXT("client_error");
		Error.Message = TEXT("Failed to start Decision request");
		DispatchDecisionFailure(MoveTemp(Error), MoveTemp(OnFailure));
	}
	return DecisionRequest.RequestId;
}

FString UZLAIServiceSubsystem::SendDialogueRequest(
	const FString& NpcId,
	const FString& PlayerInput,
	FZLDialogueSuccessDelegate OnSuccess,
	FZLDialogueFailureDelegate OnFailure)
{
	FZLDialogueRequest DialogueRequest;
	DialogueRequest.RequestId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);
	DialogueRequest.NpcId = NpcId;
	DialogueRequest.PlayerInput = PlayerInput;
	return SendDialogueRequest(MoveTemp(DialogueRequest), MoveTemp(OnSuccess), MoveTemp(OnFailure));
}

FString UZLAIServiceSubsystem::SendDialogueRequest(
	const FString& NpcId,
	const FString& PlayerInput,
	const FZLDialogueContext& Context,
	FZLDialogueSuccessDelegate OnSuccess,
	FZLDialogueFailureDelegate OnFailure)
{
	FZLDialogueRequest DialogueRequest;
	DialogueRequest.RequestId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);
	DialogueRequest.NpcId = NpcId;
	DialogueRequest.PlayerInput = PlayerInput;
	DialogueRequest.bHasContext = true;
	DialogueRequest.Context = Context;
	return SendDialogueRequest(MoveTemp(DialogueRequest), MoveTemp(OnSuccess), MoveTemp(OnFailure));
}

FString UZLAIServiceSubsystem::SendDialogueRequest(
	const FString& NpcId,
	const FString& PlayerInput,
	const FZLDialogueMemory& Memory,
	FZLDialogueSuccessDelegate OnSuccess,
	FZLDialogueFailureDelegate OnFailure)
{
	FZLDialogueRequest DialogueRequest;
	DialogueRequest.RequestId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);
	DialogueRequest.NpcId = NpcId;
	DialogueRequest.PlayerInput = PlayerInput;
	DialogueRequest.bHasMemory = true;
	DialogueRequest.Memory = Memory;
	return SendDialogueRequest(MoveTemp(DialogueRequest), MoveTemp(OnSuccess), MoveTemp(OnFailure));
}

FString UZLAIServiceSubsystem::SendDialogueRequest(
	const FString& NpcId,
	const FString& PlayerInput,
	const FZLDialogueContext& Context,
	const FZLDialogueMemory& Memory,
	FZLDialogueSuccessDelegate OnSuccess,
	FZLDialogueFailureDelegate OnFailure)
{
	FZLDialogueRequest DialogueRequest;
	DialogueRequest.RequestId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);
	DialogueRequest.NpcId = NpcId;
	DialogueRequest.PlayerInput = PlayerInput;
	DialogueRequest.bHasContext = true;
	DialogueRequest.Context = Context;
	DialogueRequest.bHasMemory = true;
	DialogueRequest.Memory = Memory;
	return SendDialogueRequest(MoveTemp(DialogueRequest), MoveTemp(OnSuccess), MoveTemp(OnFailure));
}

FString UZLAIServiceSubsystem::SendDialogueRequest(
	FZLDialogueRequest DialogueRequest,
	FZLDialogueSuccessDelegate OnSuccess,
	FZLDialogueFailureDelegate OnFailure)
{
	const UZLAIServiceSettings* Settings = GetDefault<UZLAIServiceSettings>();
	const FString ServiceBaseUrl = Settings->ServiceBaseUrl;
	const float RequestTimeoutSeconds = FMath::Max(Settings->RequestTimeoutSeconds, 0.1f);

	const FString Endpoint = BuildDialogueEndpoint(ServiceBaseUrl);
	if (ServiceBaseUrl.TrimStartAndEnd().IsEmpty())
	{
		FZLServiceError Error;
		Error.Category = EZLServiceErrorCategory::Client;
		Error.RequestId = DialogueRequest.RequestId;
		Error.Code = TEXT("client_error");
		Error.Message = TEXT("Service base URL must not be empty");
		DispatchFailure(MoveTemp(Error), MoveTemp(OnFailure));
		return DialogueRequest.RequestId;
	}

	FString RequestBody;
	FString ValidationError;
	if (!ZLAIServiceProtocol::ValidateDialogueRequest(DialogueRequest, ValidationError))
	{
		FZLServiceError Error;
		Error.Category = EZLServiceErrorCategory::Client;
		Error.RequestId = DialogueRequest.RequestId;
		Error.Code = TEXT("client_error");
		Error.Message = MoveTemp(ValidationError);
		DispatchFailure(MoveTemp(Error), MoveTemp(OnFailure));
		return DialogueRequest.RequestId;
	}

	if (!ZLAIServiceProtocol::SerializeDialogueRequest(DialogueRequest, RequestBody))
	{
		FZLServiceError Error;
		Error.Category = EZLServiceErrorCategory::Client;
		Error.RequestId = DialogueRequest.RequestId;
		Error.Code = TEXT("client_error");
		Error.Message = TEXT("Failed to serialize dialogue request");
		DispatchFailure(MoveTemp(Error), MoveTemp(OnFailure));
		return DialogueRequest.RequestId;
	}

	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL(Endpoint);
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));
	HttpRequest->SetContentAsString(RequestBody);
	HttpRequest->SetTimeout(RequestTimeoutSeconds);

	const TWeakObjectPtr<UZLAIServiceSubsystem> WeakThis(this);
	HttpRequest->OnProcessRequestComplete().BindLambda(
		[WeakThis,
			ExpectedRequestId = DialogueRequest.RequestId,
			ExpectedNpcId = DialogueRequest.NpcId,
			OnSuccess,
			OnFailure](
			FHttpRequestPtr Request,
			FHttpResponsePtr Response,
			bool bSucceeded) mutable
		{
			const int32 StatusCode = Response.IsValid() ? Response->GetResponseCode() : 0;
			const FString ResponseBody = Response.IsValid() ? Response->GetContentAsString() : FString();
			const bool bTimedOut = Request.IsValid()
				&& Request->GetFailureReason() == EHttpFailureReason::TimedOut;

			AsyncTask(ENamedThreads::GameThread,
				[WeakThis,
					ExpectedRequestId = MoveTemp(ExpectedRequestId),
					ExpectedNpcId = MoveTemp(ExpectedNpcId),
					bSucceeded,
					bTimedOut,
					StatusCode,
					ResponseBody,
					OnSuccess = MoveTemp(OnSuccess),
					OnFailure = MoveTemp(OnFailure)]() mutable
				{
					if (WeakThis.IsValid())
					{
						WeakThis->CompleteRequest(
							ExpectedRequestId,
							ExpectedNpcId,
							bSucceeded,
							bTimedOut,
							StatusCode,
							ResponseBody,
							MoveTemp(OnSuccess),
							MoveTemp(OnFailure));
					}
				});
		});

	if (!HttpRequest->ProcessRequest())
	{
		HttpRequest->OnProcessRequestComplete().Unbind();

		FZLServiceError Error;
		Error.Category = EZLServiceErrorCategory::Client;
		Error.RequestId = DialogueRequest.RequestId;
		Error.Code = TEXT("client_error");
		Error.Message = TEXT("Failed to start dialogue request");
		DispatchFailure(MoveTemp(Error), MoveTemp(OnFailure));
	}

	return DialogueRequest.RequestId;
}

void UZLAIServiceSubsystem::CompleteRequest(
	const FString& ExpectedRequestId,
	const FString& ExpectedNpcId,
	const bool bTransportSucceeded,
	const bool bTimedOut,
	const int32 HttpStatusCode,
	const FString& ResponseBody,
	FZLDialogueSuccessDelegate OnSuccess,
	FZLDialogueFailureDelegate OnFailure)
{
	check(IsInGameThread());

	if (!bTransportSucceeded)
	{
		FZLServiceError Error;
		Error.RequestId = ExpectedRequestId;
		Error.Category = bTimedOut ? EZLServiceErrorCategory::Timeout : EZLServiceErrorCategory::Network;
		Error.Code = bTimedOut ? TEXT("timeout") : TEXT("network_error");
		Error.Message = bTimedOut
			? TEXT("Dialogue request timed out")
			: TEXT("Dialogue request did not complete");
		LogFailure(Error);
		OnFailure.ExecuteIfBound(Error);
		return;
	}

	if (!EHttpResponseCodes::IsOk(HttpStatusCode))
	{
		FZLServiceError Error;
		Error.Category = EZLServiceErrorCategory::Http;
		if (!ZLAIServiceProtocol::TryParseServiceError(ResponseBody, Error))
		{
			Error.RequestId = ExpectedRequestId;
			Error.Code = TEXT("http_error");
			Error.Message = TEXT("Service returned a non-success status");
		}

		Error.RequestId = ExpectedRequestId;
		Error.Category = EZLServiceErrorCategory::Http;
		Error.HttpStatusCode = HttpStatusCode;
		LogFailure(Error);
		OnFailure.ExecuteIfBound(Error);
		return;
	}

	FZLDialogueResponse DialogueResponse;
	if (!ZLAIServiceProtocol::TryParseDialogueResponse(ResponseBody, DialogueResponse)
		|| DialogueResponse.RequestId != ExpectedRequestId
		|| DialogueResponse.NpcId != ExpectedNpcId)
	{
		FZLServiceError Error;
		Error.RequestId = ExpectedRequestId;
		Error.Category = EZLServiceErrorCategory::Parse;
		Error.Code = TEXT("parse_error");
		Error.Message = TEXT("Service returned an invalid dialogue response");
		Error.HttpStatusCode = HttpStatusCode;
		LogFailure(Error);
		OnFailure.ExecuteIfBound(Error);
		return;
	}

	OnSuccess.ExecuteIfBound(DialogueResponse);
}

void UZLAIServiceSubsystem::CompleteDecisionRequest(
	const FString& ExpectedRequestId,
	const FString& ExpectedNpcId,
	const int64 ExpectedStateVersion,
	const double SentAtSeconds,
	const int32 TtlMs,
	const bool bTransportSucceeded,
	const bool bTimedOut,
	const int32 HttpStatusCode,
	const FString& ResponseBody,
	FZLDecisionSuccessDelegate OnSuccess,
	FZLDecisionFailureDelegate OnFailure)
{
	check(IsInGameThread());
	if (!bTransportSucceeded)
	{
		FZLServiceError Error;
		Error.RequestId = ExpectedRequestId;
		Error.Category = bTimedOut ? EZLServiceErrorCategory::Timeout : EZLServiceErrorCategory::Network;
		Error.Code = bTimedOut ? TEXT("timeout") : TEXT("network_error");
		Error.Message = bTimedOut ? TEXT("Decision request timed out") : TEXT("Decision request did not complete");
		LogDecisionFailure(Error);
		OnFailure.ExecuteIfBound(Error);
		return;
	}
	if (!EHttpResponseCodes::IsOk(HttpStatusCode))
	{
		FZLServiceError Error;
		Error.Category = EZLServiceErrorCategory::Http;
		if (!ZLAIServiceProtocol::TryParseServiceError(ResponseBody, Error))
		{
			Error.Code = TEXT("http_error");
			Error.Message = TEXT("Service returned a non-success status");
		}
		Error.RequestId = ExpectedRequestId;
		Error.HttpStatusCode = HttpStatusCode;
		LogDecisionFailure(Error);
		OnFailure.ExecuteIfBound(Error);
		return;
	}
	if ((FPlatformTime::Seconds() - SentAtSeconds) * 1000.0 > static_cast<double>(TtlMs))
	{
		FZLServiceError Error;
		Error.RequestId = ExpectedRequestId;
		Error.Category = EZLServiceErrorCategory::Stale;
		Error.Code = TEXT("stale_response");
		Error.Message = TEXT("Decision response exceeded its local TTL");
		Error.HttpStatusCode = HttpStatusCode;
		LogDecisionFailure(Error);
		OnFailure.ExecuteIfBound(Error);
		return;
	}

	FZLDecisionResponse DecisionResponse;
	if (!ZLAIServiceProtocol::TryParseDecisionResponse(ResponseBody, DecisionResponse)
		|| DecisionResponse.RequestId != ExpectedRequestId
		|| DecisionResponse.NpcId != ExpectedNpcId
		|| DecisionResponse.StateVersion != ExpectedStateVersion)
	{
		FZLServiceError Error;
		Error.RequestId = ExpectedRequestId;
		Error.Category = EZLServiceErrorCategory::Parse;
		Error.Code = TEXT("parse_error");
		Error.Message = TEXT("Service returned an invalid Decision response");
		Error.HttpStatusCode = HttpStatusCode;
		LogDecisionFailure(Error);
		OnFailure.ExecuteIfBound(Error);
		return;
	}
	OnSuccess.ExecuteIfBound(DecisionResponse);
}
