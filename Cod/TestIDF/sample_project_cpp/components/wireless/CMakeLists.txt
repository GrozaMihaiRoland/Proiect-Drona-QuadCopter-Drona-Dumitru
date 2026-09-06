idf_component_register(
    SRCS "wireless.cpp"
    INCLUDE_DIRS "include"
    REQUIRES led feedback esp_wifi esp_event esp_netif nvs_flash 
)