import sys
import types
from pathlib import Path
import importlib.machinery


def test_wmbus_sensor_alias():
    components_path = Path(__file__).resolve().parents[2] / "components"

    esphome_pkg = types.ModuleType("esphome")
    esphome_pkg.__path__ = []
    esphome_pkg.__spec__ = importlib.machinery.ModuleSpec("esphome", loader=None, is_package=True)
    esphome_pkg.__spec__.submodule_search_locations = []

    components_pkg = types.ModuleType("esphome.components")
    components_pkg.__path__ = [str(components_path)]
    components_pkg.__spec__ = importlib.machinery.ModuleSpec(
        "esphome.components", loader=None, is_package=True
    )
    components_pkg.__spec__.submodule_search_locations = [str(components_path)]

    wmbus_meter_pkg = types.ModuleType(
        "esphome.components.wmbus.wmbus_meter"
    )
    wmbus_meter_pkg.__path__ = [
        str(components_path / "wmbus" / "wmbus_meter")
    ]
    wmbus_meter_pkg.__spec__ = importlib.machinery.ModuleSpec(
        "esphome.components.wmbus.wmbus_meter", loader=None, is_package=True
    )
    wmbus_meter_pkg.__spec__.submodule_search_locations = [
        str(components_path / "wmbus" / "wmbus_meter")
    ]

    meter_sensor_module = types.ModuleType(
        "esphome.components.wmbus.wmbus_meter.sensor"
    )

    class DummySensor:
        pass

    meter_sensor_module.Sensor = DummySensor

    wmbus_radio_pkg = types.ModuleType(
        "esphome.components.wmbus.wmbus_radio"
    )

    esphome_cv_pkg = types.ModuleType("esphome.config_validation")
    esphome_cv_pkg.Schema = lambda *args, **kwargs: None

    sys.modules.update(
        {
            "esphome": esphome_pkg,
            "esphome.components": components_pkg,
            "esphome.config_validation": esphome_cv_pkg,
            "esphome.components.wmbus.wmbus_radio": wmbus_radio_pkg,
            "esphome.components.wmbus.wmbus_meter": wmbus_meter_pkg,
            "esphome.components.wmbus.wmbus_meter.sensor": meter_sensor_module,
        }
    )

    from esphome.components.wmbus.sensor import Sensor
    from esphome.components.wmbus.wmbus_meter.sensor import (
        Sensor as MeterSensor,
    )

    assert Sensor is MeterSensor
    assert sys.modules["esphome.components.wmbus_meter"] is wmbus_meter_pkg
