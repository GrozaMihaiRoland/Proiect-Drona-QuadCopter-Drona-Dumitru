idf_component_register(
    SRCS "main.cpp"
    INCLUDE_DIRS "."
    REQUIRES feedback motors wireless battery led sensors esp_driver_ledc 
)