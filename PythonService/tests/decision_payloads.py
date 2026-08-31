def valid_decision_payload() -> dict:
    return {
        "request_id": "request-1",
        "npc_id": "npc_guard",
        "state_version": 12,
        "ttl_ms": 30_000,
        "trigger": {
            "event_id": "speech-42",
            "kind": "speech",
            "source_id": "player",
            "target_id": "npc_guard",
            "channels": ["auditory", "direct"],
            "content": "Please keep your distance.",
            "summary": "The player spoke to the guard.",
            "occurred_at_ms": 1_725_100_800_000,
        },
        "context": {
            "npc": {
                "display_name": "Guard",
                "role": "gate guard",
                "personality": ["cautious"],
                "speaking_style": "brief",
                "goals": ["keep order"],
            },
            "relationship": {
                "trust": -0.1,
                "affinity": 0.0,
                "fear": 0.2,
                "familiarity": 0.4,
            },
            "instant_state": {
                "fear": 0.2,
                "anger": 0.3,
                "curiosity": 0.1,
                "alert": 0.8,
            },
            "recent_history": [],
        },
        "allowed_tools": [
            {"name": "move_away", "target_ids": ["player"]},
            {"name": "stop", "target_ids": []},
        ],
    }
