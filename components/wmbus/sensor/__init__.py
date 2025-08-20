"""WMBus sensor platform alias.

This module re-exports the existing wmbus_meter sensor implementation so
users can configure sensors with ``platform: wmbus``.
"""

DEPENDENCIES = ["wmbus_meter"]

try:  # pragma: no cover - import side effects
    from ...wmbus_meter.sensor import *  # noqa: F401,F403
except ModuleNotFoundError as err:  # pragma: no cover - defensive
    if not err.name.endswith("wmbus_meter"):
        raise
    raise ModuleNotFoundError(
        "The wmbus_meter component is required for sensor.wmbus"
    ) from err
