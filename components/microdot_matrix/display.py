import esphome.codegen as cg
import esphome.config_validation as cv

from esphome.components import display
from esphome.components import i2c
from esphome.const import CONF_ID, CONF_LAMBDA, CONF_PAGES


DEPENDENCIES = ["i2c"]

microdot_matrix = cg.esphome_ns.namespace("microdot_matrix")
MicrodotMatrix = microdot_matrix.class_("MicrodotMatrix", cg.PollingComponent, display.DisplayBuffer, i2c.I2CDevice)

CONFIG_SCHEMA = cv.All(
    display.FULL_DISPLAY_SCHEMA
    .extend(cv.polling_component_schema("1s"))
    .extend(
        {
            cv.GenerateID(): cv.declare_id(MicrodotMatrix),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(i2c.i2c_device_schema(0x61)),
    cv.has_at_most_one_key(CONF_PAGES, CONF_LAMBDA),
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await display.register_display(var, config)

    if CONF_LAMBDA in config:
        lambda_ = await cg.process_lambda(
            config[CONF_LAMBDA], [(display.DisplayBufferRef, "it")], return_type=cg.void
        )
        cg.add(var.set_writer(lambda_))
    await i2c.register_i2c_device(var, config)