import esphome.config_validation as cv
import sys

CODEOWNERS = ["@SzczepanLeon", "@kubasaw"]

# Ensure radio and meter components are available when referencing this
# package from ESPHome's external_components section. The shared
# `wmbus_common` module now lives inside this package, so it is included
# automatically and does not need to be declared as an external
# dependency.
DEPENDENCIES = ["wmbus_radio", "wmbus_meter"]

# Re-export modules so users can access them via the wmbus namespace.
try:
    from . import wmbus_radio as radio  # noqa: F401
except ImportError:  # pragma: no cover - runtime optional dependency
    radio = None

try:
    from . import wmbus_meter as meter  # noqa: F401
except ImportError:  # pragma: no cover - runtime optional dependency
    meter = None

try:
    from . import wmbus_common  # noqa: F401
except ImportError:  # pragma: no cover - runtime optional dependency
    wmbus_common = None

if radio is not None:
    sys.modules.setdefault("esphome.components.wmbus_radio", radio)

if meter is not None:
    sys.modules.setdefault("esphome.components.wmbus_meter", meter)

if wmbus_common is not None:
    sys.modules.setdefault("esphome.components.wmbus_common", wmbus_common)

try:
    from . import sensor  # noqa: F401
except ImportError:  # pragma: no cover - runtime optional dependency
    sensor = None

__all__ = ["radio", "meter", "sensor", "wmbus_common"]

CONFIG_SCHEMA = cv.Schema({})

async def to_code(config):
    pass
