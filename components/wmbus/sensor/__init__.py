"""WMBus sensor platform alias.

This module re-exports the existing wmbus_meter sensor implementation so
users can configure sensors with ``platform: wmbus``.
"""

from ...wmbus_meter.sensor import *  # noqa: F401,F403
