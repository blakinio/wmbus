import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.components import spi
from esphome import pins
from esphome.const import (
    CONF_CS_PIN,
    CONF_RESET_PIN,
    CONF_FREQUENCY,
    CONF_ID,
)

DEPENDENCIES = ["spi"]
MULTI_CONF = True

wmbus_radio_ns = cg.esphome_ns.namespace("wmbus_radio")
WMBusRadio = wmbus_radio_ns.class_("WMBusRadio", cg.Component, spi.SPIDevice)

# Configuration keys
CONF_RADIO_TYPE = "radio_type"
CONF_GDO0_PIN = "gdo0_pin"
CONF_GDO2_PIN = "gdo2_pin"
CONF_IRQ_PIN = "irq_pin"

# Supported radio types
RADIO_TYPES = ["SX1276", "CC1101"]

def validate_radio_config(config):
    """Validate radio-specific configuration"""
    radio_type = config[CONF_RADIO_TYPE]
    
    if radio_type == "CC1101":
        # CC1101 requires GDO pins
        if CONF_GDO0_PIN not in config:
            raise cv.Invalid("gdo0_pin is required for CC1101")
        if CONF_GDO2_PIN not in config:
            raise cv.Invalid("gdo2_pin is required for CC1101")
        # IRQ pin not used for CC1101
        if CONF_IRQ_PIN in config:
            raise cv.Invalid("irq_pin is not supported for CC1101, use gdo0_pin and gdo2_pin instead")
            
    elif radio_type == "SX1276":
        # SX1276 requires IRQ pin
        if CONF_IRQ_PIN not in config:
            raise cv.Invalid("irq_pin is required for SX1276")
        # GDO pins not used for SX1276
        if CONF_GDO0_PIN in config or CONF_GDO2_PIN in config:
            raise cv.Invalid("gdo0_pin and gdo2_pin are not supported for SX1276, use irq_pin instead")
    
    return config

CONFIG_SCHEMA = cv.All(
    cv.Schema({
        cv.GenerateID(): cv.declare_id(WMBusRadio),
        cv.Required(CONF_RADIO_TYPE): cv.one_of(*RADIO_TYPES, upper=True),
        cv.Required(CONF_CS_PIN): pins.gpio_output_pin_schema,
        cv.Optional(CONF_RESET_PIN): pins.gpio_output_pin_schema,
        cv.Optional(CONF_FREQUENCY, default=868.95): cv.float_range(min=300.0, max=928.0),
        
        # CC1101 specific pins
        cv.Optional(CONF_GDO0_PIN): pins.gpio_input_pin_schema,
        cv.Optional(CONF_GDO2_PIN): pins.gpio_input_pin_schema,
        
        # SX1276 specific pins
        cv.Optional(CONF_IRQ_PIN): pins.gpio_input_pin_schema,
        
    }).extend(cv.COMPONENT_SCHEMA).extend(spi.spi_device_schema()),
    validate_radio_config
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await spi.register_spi_device(var, config)
    
    # Set radio type
    cg.add(var.set_radio_type(config[CONF_RADIO_TYPE]))
    
    # Set CS pin
    cs_pin = await cg.gpio_pin_expression(config[CONF_CS_PIN])
    cg.add(var.set_cs_pin(cs_pin))
    
    # Set reset pin if provided
    if CONF_RESET_PIN in config:
        reset_pin = await cg.gpio_pin_expression(config[CONF_RESET_PIN])
        cg.add(var.set_reset_pin(reset_pin))
    
    # Set frequency
    cg.add(var.set_frequency(config[CONF_FREQUENCY]))
    
    # Configure radio-specific pins
    if config[CONF_RADIO_TYPE] == "CC1101":
        if CONF_GDO0_PIN in config:
            gdo0_pin = await cg.gpio_pin_expression(config[CONF_GDO0_PIN])
            cg.add(var.set_gdo0_pin(gdo0_pin))
        if CONF_GDO2_PIN in config:
            gdo2_pin = await cg.gpio_pin_expression(config[CONF_GDO2_PIN])
            cg.add(var.set_gdo2_pin(gdo2_pin))
            
    elif config[CONF_RADIO_TYPE] == "SX1276":
        if CONF_IRQ_PIN in config:
            irq_pin = await cg.gpio_pin_expression(config[CONF_IRQ_PIN])
            cg.add(var.set_irq_pin(irq_pin))