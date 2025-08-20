import esphome.config_validation as cv

CODEOWNERS = ["@SzczepanLeon", "@kubasaw"]

# Ensure radio and meter components are available when referencing
# this package from ESPHome's external_components section.
DEPENDENCIES = ["wmbus_radio", "wmbus_meter"]

# Re-export modules so users can access them via the wmbus namespace.
from .. import wmbus_radio as radio  # noqa: F401
from .. import wmbus_meter as meter  # noqa: F401

__all__ = ["radio", "meter"]

CONFIG_SCHEMA = cv.Schema({})

async def to_code(config):
    pass
