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
from app.providers.base import DialogueProvider
from app.providers.factory import create_dialogue_provider
from app.schemas.dialogue import ErrorDetail, ErrorResponse
from app.services.dialogue_service import DialogueService, InvalidDialogueRequest


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


async def _handle_internal_error(
    request: Request,
    _exception: Exception,
) -> JSONResponse:
    request_id = getattr(request.state, "request_id", "")
    LOGGER.error("Unhandled service error request_id=%s", request_id)
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
) -> FastAPI:
    """Create an application whose Provider is resolved only during startup."""

    @asynccontextmanager
    async def lifespan(application: FastAPI) -> AsyncIterator[None]:
        selected_provider = provider
        if selected_provider is None:
            selected_settings = settings or Settings.from_env()
            selected_provider = create_dialogue_provider(selected_settings)
        application.state.dialogue_service = DialogueService(selected_provider)
        yield

    application = FastAPI(title="ZL AI Service", version="0.2.0", lifespan=lifespan)
    application.add_exception_handler(InvalidDialogueRequest, _handle_invalid_request)
    application.add_exception_handler(RequestValidationError, _handle_validation_error)
    application.add_exception_handler(Exception, _handle_internal_error)
    application.include_router(dialogue_router)
    return application


app = create_app()
