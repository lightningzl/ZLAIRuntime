#include "ZLAIServiceSubsystem.h"

#include "Async/Async.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "ZLAIServiceProtocol.h"

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

	void DispatchFailure(FZLServiceError Error, FZLDialogueFailureDelegate OnFailure)
	{
		AsyncTask(ENamedThreads::GameThread, [Error = MoveTemp(Error), OnFailure = MoveTemp(OnFailure)]() mutable
		{
			OnFailure.ExecuteIfBound(Error);
		});
	}
}

FString UZLAIServiceSubsystem::SendDialogueRequest(
	const FString& ServiceBaseUrl,
	const FString& NpcId,
	const FString& PlayerInput,
	FZLDialogueSuccessDelegate OnSuccess,
	FZLDialogueFailureDelegate OnFailure)
{
	FZLDialogueRequest DialogueRequest;
	DialogueRequest.RequestId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);
	DialogueRequest.NpcId = NpcId;
	DialogueRequest.PlayerInput = PlayerInput;

	const FString Endpoint = BuildDialogueEndpoint(ServiceBaseUrl);
	if (ServiceBaseUrl.TrimStartAndEnd().IsEmpty())
	{
		FZLServiceError Error;
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

	const TWeakObjectPtr<UZLAIServiceSubsystem> WeakThis(this);
	HttpRequest->OnProcessRequestComplete().BindLambda(
		[WeakThis,
			ExpectedRequestId = DialogueRequest.RequestId,
			ExpectedNpcId = DialogueRequest.NpcId,
			OnSuccess,
			OnFailure](
			FHttpRequestPtr,
			FHttpResponsePtr Response,
			bool bSucceeded) mutable
		{
			const int32 StatusCode = Response.IsValid() ? Response->GetResponseCode() : 0;
			const FString ResponseBody = Response.IsValid() ? Response->GetContentAsString() : FString();

			AsyncTask(ENamedThreads::GameThread,
				[WeakThis,
					ExpectedRequestId = MoveTemp(ExpectedRequestId),
					ExpectedNpcId = MoveTemp(ExpectedNpcId),
					bSucceeded,
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
		Error.Code = TEXT("network_error");
		Error.Message = TEXT("Dialogue request did not complete");
		OnFailure.ExecuteIfBound(Error);
		return;
	}

	if (!EHttpResponseCodes::IsOk(HttpStatusCode))
	{
		FZLServiceError Error;
		if (!ZLAIServiceProtocol::TryParseServiceError(ResponseBody, Error))
		{
			Error.RequestId = ExpectedRequestId;
			Error.Code = TEXT("http_error");
			Error.Message = TEXT("Service returned a non-success status");
		}

		Error.HttpStatusCode = HttpStatusCode;
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
		Error.Code = TEXT("parse_error");
		Error.Message = TEXT("Service returned an invalid dialogue response");
		Error.HttpStatusCode = HttpStatusCode;
		OnFailure.ExecuteIfBound(Error);
		return;
	}

	OnSuccess.ExecuteIfBound(DialogueResponse);
}
