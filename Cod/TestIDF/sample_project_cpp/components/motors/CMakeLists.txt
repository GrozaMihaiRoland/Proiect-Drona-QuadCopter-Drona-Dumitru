idf_component_register(
    SRCS "motors.cpp"
    INCLUDE_DIRS "include"
    REQUIRES esp_driver_i2c esp_driver_ledc led pins
)