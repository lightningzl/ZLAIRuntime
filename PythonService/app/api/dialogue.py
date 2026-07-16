"""Dialogue HTTP routes.

The protocol endpoint is implemented as part of M1-02. Keeping the router here
establishes the route layer without pulling protocol behavior into the skeleton.
"""

from fastapi import APIRouter


router = APIRouter(prefix="/v1")
