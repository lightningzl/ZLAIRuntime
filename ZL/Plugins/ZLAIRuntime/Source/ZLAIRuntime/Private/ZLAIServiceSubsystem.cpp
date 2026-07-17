#include "ZLAIServiceSubsystem.h"

#include "Async/Async.h"
#include "HttpModule.h"
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

	const TCHAR* ErrorCategoryToString(const EZLServiceErrorCategory Category)
	{
		switch (Category)
		{
		case EZLServiceErrorCategory::Client: return TEXT("client");
		case EZLServiceErrorCategory::Network: return TEXT("network");
		case EZLServiceErrorCategory::Timeout: return TEXT("timeout");
		case EZLServiceErrorCategory::Http: return TEXT("http");
		case EZLServiceErrorCategory::Parse: return TEXT("parse");
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
}

FString UZLAIServiceSubsystem::SendDialogueRequest(
	const FString& NpcId,
	const FString& PlayerInput,
	FZLDialogueSuccessDelegate OnSuccess,
	FZLDialogueFailureDelegate OnFailure)
{
	const UZLAIServiceSettings* Settings = GetDefault<UZLAIServiceSettings>();
	const FString ServiceBaseUrl = Settings->ServiceBaseUrl;
	const float RequestTimeoutSeconds = FMath::Max(Settings->RequestTimeoutSeconds, 0.1f);

	FZLDialogueRequest DialogueRequest;
	DialogueRequest.RequestId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);
	DialogueRequest.NpcId = NpcId;
	DialogueRequest.PlayerInput = PlayerInput;

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
