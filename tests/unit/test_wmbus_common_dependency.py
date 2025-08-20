import sys
import types
import importlib.machinery
import importlib.util
from pathlib import Path

import pytest


class _DummyCVSchema:
    def extend(self, *_args, **_kwargs):
        return self

def _import_meter(with_common: bool) -> None:
    original_modules = sys.modules.copy()
    try:
        for key in list(sys.modules):
            if key.startswith("components") or key.startswith("esphome"):
                del sys.modules[key]

        esphome_pkg = types.ModuleType("esphome")
        esphome_pkg.__path__ = []
        sys.modules["esphome"] = esphome_pkg

        esphome_components_pkg = types.ModuleType("esphome.components")
        esphome_components_pkg.__path__ = []
        sys.modules["esphome.components"] = esphome_components_pkg

        cv = types.ModuleType("esphome.config_validation")
        cv.Schema = lambda *a, **k: _DummyCVSchema()
        cv.GenerateID = lambda *a, **k: (lambda *_a2, **_k2: None)
        cv.declare_id = lambda *a, **k: None
        cv.Optional = lambda *a, **k: (lambda x: x)
        cv.Required = lambda *a, **k: (lambda x: x)
        cv.hex_int = 0
        cv.Any = lambda *a, **k: None
        cv.All = lambda *a, **k: None
        cv.string_strict = str
        cv.ensure_list = lambda x: x
        cv.enum = lambda *a, **k: None
        cv.invalid = lambda *a, **k: None
        cv.use_id = lambda *a, **k: None
        cv.Lambda = lambda *a, **k: None
        cv.one_of = lambda *a, **k: None
        cv.boolean = lambda *a, **k: None
        cv.COMPONENT_SCHEMA = _DummyCVSchema()
        sys.modules["esphome.config_validation"] = cv

        cg = types.ModuleType("esphome.codegen")
        class _DummyClass:
            def __init__(self, *_a, **_k):
                pass

            def operator(self, *_a, **_k):
                return self

        class _DummyNamespace:
            def class_(self, *_a, **_k):
                return _DummyClass()

        class _DummyGlobalNS:
            def enum(self, _name, is_class=False):
                return types.SimpleNamespace(Any=1, C1=2, T1=3)

        class _DummyESPNamespace:
            def namespace(self, _name):
                return _DummyNamespace()

        cg.esphome_ns = _DummyESPNamespace()
        cg.global_ns = _DummyGlobalNS()
        cg.Component = type("Component", (), {})
        cg.std_string = type("std_string", (), {})
        cg.std_vector = types.SimpleNamespace(template=lambda *a, **k: object())
        cg.uint8 = int
        cg.new_Pvariable = lambda *a, **k: object()
        cg.add = lambda *a, **k: None
        cg.get_variable = lambda *a, **k: object()
        cg.register_component = lambda *a, **k: None
        sys.modules["esphome.codegen"] = cg

        const = types.ModuleType("esphome.const")
        for name in [
            "CONF_ID",
            "CONF_TYPE",
            "CONF_KEY",
            "CONF_PAYLOAD",
            "CONF_TRIGGER_ID",
            "CONF_MODE",
        ]:
            setattr(const, name, name)
        sys.modules["esphome.const"] = const

        auto = types.ModuleType("esphome.automation")
        class _Trigger:
            @classmethod
            def template(cls, *_a, **_k):
                return cls

        def _register_action(*_a, **_k):
            def decorator(fn):
                return fn
            return decorator

        auto.Trigger = _Trigger
        auto.register_action = _register_action
        auto.validate_automation = lambda *_a, **_k: None
        auto.build_automation = lambda *_a, **_k: None
        sys.modules["esphome.automation"] = auto

        mqtt = types.ModuleType("esphome.components.mqtt")
        class _MQTTSchema:
            def extend(self, *_a, **_k):
                return self
        mqtt.MQTT_PUBLISH_ACTION_SCHEMA = _MQTTSchema()
        class _MQTTPublishAction:
            pass
        mqtt.MQTTPublishAction = _MQTTPublishAction
        mqtt.mqtt_publish_action_to_code = lambda *_a, **_k: None
        sys.modules["esphome.components.mqtt"] = mqtt

        components_pkg = types.ModuleType("components")
        components_pkg.__path__ = []
        sys.modules["components"] = components_pkg

        wmbus_pkg = types.ModuleType("components.wmbus")
        wmbus_pkg.__path__ = []
        sys.modules["components.wmbus"] = wmbus_pkg

        wmbus_radio = types.ModuleType("components.wmbus.wmbus_radio")
        class _RadioComponent:
            pass
        wmbus_radio.RadioComponent = _RadioComponent
        sys.modules["components.wmbus.wmbus_radio"] = wmbus_radio

        if with_common:
            wmbus_common = types.ModuleType("components.wmbus_common")
            wmbus_common.validate_driver = lambda *_a, **_k: None
            sys.modules["components.wmbus_common"] = wmbus_common
            sys.modules["esphome.components.wmbus_common"] = wmbus_common

        meter_path = (
            Path(__file__).resolve().parents[2]
            / "components"
            / "wmbus"
            / "wmbus_meter"
            / "__init__.py"
        )
        loader = importlib.machinery.SourceFileLoader(
            "components.wmbus.wmbus_meter", str(meter_path)
        )
        spec = importlib.util.spec_from_loader(loader.name, loader)
        module = importlib.util.module_from_spec(spec)
        sys.modules[loader.name] = module
        loader.exec_module(module)
    finally:
        sys.modules.clear()
        sys.modules.update(original_modules)


def test_wmbus_meter_imports_without_wmbus_common():
    _import_meter(True)
    _import_meter(False)
