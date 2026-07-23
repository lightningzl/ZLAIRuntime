"""Protocol tests for Provider error mapping and sanitized logging."""

import logging

from fastapi.testclient import TestClient
import pytest

from app.main import create_app
from app.providers.base import DialogueProviderRequest, DialogueProviderResult
from app.providers.errors import (
    DialogueProviderError,
    ProviderAuthenticationError,
    ProviderInvalidResponseError,
    ProviderRateLimitError,
    ProviderTimeoutError,
    ProviderUnavailableError,
)


VALID_REQUEST = {
    "request_id": "req-provider-error",
    "npc_id": "npc-1",
    "player_input": "hello",
}


class FailingProvider:
    def __init__(self, error: DialogueProviderError) -> None:
        self._error = error
        self.call_count = 0

    def generate(
        self,
        _request: DialogueProviderRequest,
    ) -> DialogueProviderResult:
        self.call_count += 1
        raise self._error


@pytest.mark.parametrize(
    ("error", "status_code", "code", "message"),
    [
        (
            ProviderAuthenticationError("kimi", "sensitive auth detail"),
            503,
            "provider_auth_error",
            "dialogue provider authentication failed",
        ),
        (
            ProviderRateLimitError("kimi", "sensitive rate detail"),
            429,
            "provider_rate_limited",
            "dialogue provider rate limited the request",
        ),
        (
            ProviderTimeoutError("kimi", "sensitive timeout detail"),
            504,
            "provider_timeout",
            "dialogue provider timed out",
        ),
        (
            ProviderUnavailableError("kimi", "sensitive unavailable detail"),
            503,
            "provider_unavailable",
            "dialogue provider is unavailable",
        ),
        (
            ProviderInvalidResponseError("kimi", "sensitive response detail"),
            502,
            "provider_error",
            "dialogue provider failed",
        ),
        (
            DialogueProviderError("kimi", "sensitive generic detail"),
            502,
            "provider_error",
            "dialogue provider failed",
        ),
    ],
)
def test_provider_errors_map_to_sanitized_protocol_responses(
    error: DialogueProviderError,
    status_code: int,
    code: str,
    message: str,
    caplog: pytest.LogCaptureFixture,
) -> None:
    provider = FailingProvider(error)
    caplog.set_level(logging.WARNING, logger="app.main")

    with TestClient(
        create_app(provider=provider),
        raise_server_exceptions=False,
    ) as client:
        response = client.post("/v1/dialogue", json=VALID_REQUEST)

    assert response.status_code == status_code
    assert response.json() == {
        "request_id": VALID_REQUEST["request_id"],
        "error": {"code": code, "message": message},
    }
    assert provider.call_count == 1
    assert str(error) not in response.text
    assert str(error) not in caplog.text
    assert "request_id=req-provider-error" in caplog.text
    assert "provider=kimi" in caplog.text
    assert f"category={code}" in caplog.text
