import esphome.config_validation as cv

CODEOWNERS = ["@SzczepanLeon", "@kubasaw"]

# Ensure radio and meter components are available when referencing
# this package from ESPHome's external_components section.
DEPENDENCIES = ["wmbus_radio", "wmbus_meter"]

# Re-export modules so users can access them via the wmbus namespace.
try:
    from .. import wmbus_radio as radio  # noqa: F401
except ImportError:  # pragma: no cover - runtime optional dependency
    radio = None

try:
    from .. import wmbus_meter as meter  # noqa: F401
except ImportError:  # pragma: no cover - runtime optional dependency
    meter = None

try:
    from . import sensor  # noqa: F401
except ImportError:  # pragma: no cover - runtime optional dependency
    sensor = None

__all__ = ["radio", "meter", "sensor"]

CONFIG_SCHEMA = cv.Schema({})

async def to_code(config):
    pass
