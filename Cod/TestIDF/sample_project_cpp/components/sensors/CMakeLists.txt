idf_component_register(
    SRCS "sensors.cpp"
    INCLUDE_DIRS "include"
    REQUIRES esp_driver_i2c led pins
)