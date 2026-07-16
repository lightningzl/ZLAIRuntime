"""FastAPI application entry point."""

from fastapi import FastAPI

from app.api.dialogue import router as dialogue_router


def create_app() -> FastAPI:
    """Create the HTTP application without starting a server as an import side effect."""
    application = FastAPI(title="ZL AI Service", version="0.1.0")
    application.include_router(dialogue_router)
    return application


app = create_app()
