idf_component_register(
    SRCS "battery.cpp"
    INCLUDE_DIRS "include"
    REQUIRES esp_adc led pins
)