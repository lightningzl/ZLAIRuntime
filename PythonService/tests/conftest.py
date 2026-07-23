"""Suite-wide safety guards for deterministic offline tests."""

import socket
from collections.abc import Iterator
from typing import Any

import pytest


_LOOPBACK_HOSTS = {"127.0.0.1", "::1", "localhost"}


@pytest.fixture(autouse=True)
def isolate_tests_from_credentials_and_external_network(
    monkeypatch: pytest.MonkeyPatch,
) -> Iterator[None]:
    """Remove the real API key and reject non-loopback socket connections."""
    monkeypatch.delenv("OPENAI_API_KEY", raising=False)

    original_create_connection = socket.create_connection
    original_connect = socket.socket.connect

    def guarded_create_connection(
        address: tuple[str, int],
        *args: Any,
        **kwargs: Any,
    ) -> socket.socket:
        if address[0].lower() not in _LOOPBACK_HOSTS:
            raise AssertionError(
                f"external network access is not allowed during tests: {address[0]}"
            )
        return original_create_connection(address, *args, **kwargs)

    def guarded_connect(
        network_socket: socket.socket,
        address: tuple[str, int],
    ) -> None:
        if address[0].lower() not in _LOOPBACK_HOSTS:
            raise AssertionError(
                f"external network access is not allowed during tests: {address[0]}"
            )
        original_connect(network_socket, address)

    monkeypatch.setattr(socket, "create_connection", guarded_create_connection)
    monkeypatch.setattr(socket.socket, "connect", guarded_connect)
    yield
