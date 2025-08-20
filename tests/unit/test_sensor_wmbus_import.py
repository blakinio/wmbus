import pytest


def test_import_sensor_wmbus():
    module = pytest.importorskip("components.wmbus.sensor")
    assert module.DEPENDENCIES == ["wmbus_meter"]
