"""FastAPI application entry point and protocol error mapping."""

from collections.abc import AsyncIterator
from contextlib import asynccontextmanager
import logging
from typing import Any, Mapping

from fastapi import FastAPI, Request
from fastapi.exceptions import RequestValidationError
from fastapi.responses import JSONResponse

from app.api.dialogue import router as dialogue_router
from app.core.settings import Settings
from app.memory.base import DialogueMemoryRepository, MemoryRepositoryError
from app.memory.sqlite_repository import SQLiteDialogueMemoryRepository
from app.providers.base import DialogueProvider
from app.providers.errors import (
    DialogueProviderError,
    ProviderAuthenticationError,
    ProviderRateLimitError,
    ProviderTimeoutError,
    ProviderUnavailableError,
)
from app.providers.factory import create_dialogue_provider
from app.schemas.dialogue import ErrorDetail, ErrorResponse
from app.services.dialogue_service import DialogueService, InvalidDialogueRequest
from app.services.memory_service import DialogueMemoryService, MemoryService


LOGGER = logging.getLogger(__name__)


def _request_id_from_body(body: Any) -> str:
    if not isinstance(body, Mapping):
        return ""

    request_id = body.get("request_id")
    return request_id if isinstance(request_id, str) else ""


def _error_response(
    *,
    status_code: int,
    request_id: str,
    code: str,
    message: str,
) -> JSONResponse:
    payload = ErrorResponse(
        request_id=request_id,
        error=ErrorDetail(code=code, message=message),
    )
    return JSONResponse(status_code=status_code, content=payload.model_dump(mode="json"))


async def _handle_invalid_request(
    _request: Request,
    exception: InvalidDialogueRequest,
) -> JSONResponse:
    return _error_response(
        status_code=400,
        request_id=exception.request_id,
        code="invalid_request",
        message=exception.message,
    )


async def _handle_validation_error(
    _request: Request,
    exception: RequestValidationError,
) -> JSONResponse:
    return _error_response(
        status_code=422,
        request_id=_request_id_from_body(exception.body),
        code="validation_error",
        message="request validation failed",
    )


def _provider_error_details(
    exception: DialogueProviderError,
) -> tuple[int, str, str]:
    if isinstance(exception, ProviderAuthenticationError):
        return 503, "provider_auth_error", "dialogue provider authentication failed"
    if isinstance(exception, ProviderRateLimitError):
        return 429, "provider_rate_limited", "dialogue provider rate limited the request"
    if isinstance(exception, ProviderTimeoutError):
        return 504, "provider_timeout", "dialogue provider timed out"
    if isinstance(exception, ProviderUnavailableError):
        return 503, "provider_unavailable", "dialogue provider is unavailable"
    return 502, "provider_error", "dialogue provider failed"


async def _handle_provider_error(
    request: Request,
    exception: DialogueProviderError,
) -> JSONResponse:
    request_id = getattr(request.state, "request_id", "")
    status_code, code, message = _provider_error_details(exception)
    LOGGER.warning(
        "Dialogue provider failed request_id=%s npc_id=%s provider=%s "
        "has_context=%s history_count=%d has_memory=%s "
        "category=%s http_status=%d",
        request_id,
        getattr(request.state, "npc_id", ""),
        exception.provider,
        getattr(request.state, "has_context", False),
        getattr(request.state, "history_count", 0),
        getattr(request.state, "has_memory", False),
        code,
        status_code,
    )
    return _error_response(
        status_code=status_code,
        request_id=request_id,
        code=code,
        message=message,
    )


async def _handle_memory_error(
    request: Request,
    _exception: MemoryRepositoryError,
) -> JSONResponse:
    request_id = getattr(request.state, "request_id", "")
    LOGGER.error(
        "Dialogue memory request failed request_id=%s npc_id=%s "
        "has_memory=%s category=memory_error",
        request_id,
        getattr(request.state, "npc_id", ""),
        getattr(request.state, "has_memory", False),
    )
    return _error_response(
        status_code=500,
        request_id=request_id,
        code="internal_error",
        message="internal server error",
    )


async def _handle_internal_error(
    request: Request,
    _exception: Exception,
) -> JSONResponse:
    request_id = getattr(request.state, "request_id", "")
    LOGGER.error(
        "Unhandled service error request_id=%s npc_id=%s has_memory=%s "
        "category=internal_error",
        request_id,
        getattr(request.state, "npc_id", ""),
        getattr(request.state, "has_memory", False),
    )
    return _error_response(
        status_code=500,
        request_id=request_id,
        code="internal_error",
        message="internal server error",
    )


def create_app(
    *,
    settings: Settings | None = None,
    provider: DialogueProvider | None = None,
    memory_service: MemoryService | None = None,
    memory_repository: DialogueMemoryRepository | None = None,
) -> FastAPI:
    """Create an application whose Provider is resolved only during startup."""
    if memory_service is not None and memory_repository is not None:
        raise ValueError("inject either memory_service or memory_repository, not both")

    @asynccontextmanager
    async def lifespan(application: FastAPI) -> AsyncIterator[None]:
        selected_provider = provider
        selected_settings = settings
        if selected_provider is None:
            selected_settings = selected_settings or Settings.from_env()
            selected_provider = create_dialogue_provider(selected_settings)

        selected_memory_service = memory_service
        active_repository: DialogueMemoryRepository | None = None
        should_build_memory = (
            selected_memory_service is None
            and (
                provider is None
                or selected_settings is not None
                or memory_repository is not None
            )
        )
        if should_build_memory:
            if selected_settings is None:
                selected_settings = Settings(dialogue_provider="stub")
            active_repository = memory_repository or SQLiteDialogueMemoryRepository(
                selected_settings.memory_database_path
            )
            active_repository.initialize()
            selected_memory_service = DialogueMemoryService(
                active_repository,
                max_turns=selected_settings.memory_max_turns,
            )

        application.state.dialogue_service = DialogueService(
            selected_provider,
            selected_memory_service,
        )
        try:
            yield
        finally:
            if active_repository is not None:
                active_repository.close()

    application = FastAPI(title="ZL AI Service", version="0.2.0", lifespan=lifespan)
    application.add_exception_handler(InvalidDialogueRequest, _handle_invalid_request)
    application.add_exception_handler(RequestValidationError, _handle_validation_error)
    application.add_exception_handler(DialogueProviderError, _handle_provider_error)
    application.add_exception_handler(MemoryRepositoryError, _handle_memory_error)
    application.add_exception_handler(Exception, _handle_internal_error)
    application.include_router(dialogue_router)
    return application


app = create_app()
