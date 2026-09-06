#include "wireless.h"
#include <esp_log.h>

int sock;
struct sockaddr_in addr_drone = {};
struct sockaddr_in addr_station = {};

static const char* TAG = "wireless";

///////////////////////////////////////////////////////////////////////
/// @brief WiFi event handler function. 
/// @details This function is called when a WiFi event occurs. 
///////////////////////////////////////////////////////////////////////
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data){}

///////////////////////////////////////////////////////////////////////
/// @brief Initializes the wireless module. 
///////////////////////////////////////////////////////////////////////
void wireless_init()
{
    nvs_flash_init();

    esp_netif_init();
    esp_event_loop_create_default();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_register(WIFI_EVENT,
                                        ESP_EVENT_ANY_ID,
                                        &wifi_event_handler,
                                        NULL,
                                        NULL);

    esp_netif_create_default_wifi_ap();

    wifi_config_t wifi_config = 
    {
        .ap = {
            .ssid = "ESP32",
            .password = "12345678",
            .ssid_len = 0,
            .channel = 1,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .ssid_hidden = 0,
            .max_connection = 4,                     /**< Max number of stations allowed to connect in. Please note that soft-AP and ESP-NOW share the same encryption hardware keys, so the max_connection parameter will be affected by CONFIG_ESP_WIFI_ESPNOW_MAX_ENCRYPT_NUM. */
            .beacon_interval = 100,                  /**< Beacon interval which should be multiples of 100. Unit: TU(time unit, 1 TU = 1024 us). Range: 100 ~ 60000. Default value: 100 */
            .csa_count = 3,                          /**< Channel Switch Announcement Count. Notify the station that the channel will switch after the csa_count beacon intervals. Default value: 3 */
            .dtim_period = 1,                        /**< Dtim period of soft-AP. Range: 1 ~ 10. Default value: 1 */
            .pairwise_cipher = WIFI_CIPHER_TYPE_CCMP, /**< Pairwise cipher of SoftAP, group cipher will be derived using this. Cipher values are valid starting from WIFI_CIPHER_TYPE_TKIP, enum values before that will be considered as invalid and default cipher suites(TKIP+CCMP) will be used. Valid cipher suites in softAP mode are WIFI_CIPHER_TYPE_TKIP, WIFI_CIPHER_TYPE_CCMP, WIFI_CIPHER_TYPE_TKIP_CCMP, WIFI_CIPHER_TYPE_GCMP and WIFI_CIPHER_TYPE_GCMP256. */
            .ftm_responder = false,                  /**< Enable FTM Responder mode */
            .pmf_cfg = {                             /**< Configuration for Protected Management Frame */
                .capable = true,
                .required = false
            },
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,        /**< Configuration for SAE PWE derivation method. Default value :2 (WPA3_SAE_PWE_BOTH) */
            .transition_disable = false,             /**< Whether to enable transition disable feature */
            .sae_ext = false,                        /**< Enable SAE EXT feature. SOC_GCMP_SUPPORT is required for this feature. */
            .wpa3_compatible_mode = false,           /**< Enable WPA3 compatible authmode feature. Note: Enabling this will override the AP configuration's authmode and pairwise_cipher. The AP will operate as a WPA2 access point for all stations except for those that support WPA3 compatible mode. Only WPA3 compatibility mode stations will be able to use WPA3-SAE */
            .reserved = 0,
            .bss_max_idle_cfg = {
                .period = WIFI_AP_DEFAULT_MAX_IDLE_PERIOD,
                .protected_keep_alive = 1,
            },
            .gtk_rekey_interval = 3600,
        }
    };
    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    esp_wifi_start();
    
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    addr_station.sin_family = AF_INET;
    addr_station.sin_port = htons(6000);
    addr_station.sin_addr.s_addr = inet_addr("192.168.4.2");

    addr_drone.sin_family = AF_INET;
    addr_drone.sin_port = htons(6000);
    addr_drone.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(sock, (struct sockaddr*)&addr_drone, sizeof(addr_drone));

    ESP_LOGI(TAG, "Done setting up WIFI");
}

input_commands_t com = {};

///////////////////////////////////////////////////////////////////////
/// @brief Reads and processes packet recevied from station.
///////////////////////////////////////////////////////////////////////
void wireless_read()
{
    drone_packet_t packet;
    recvfrom(sock, &packet, sizeof(drone_packet_t), 0, NULL, NULL);
    ESP_LOGI(TAG, "Packet header: %u, Payload %u %u %u %u", packet.header, packet.payload[0], packet.payload[1], packet.payload[2], packet.payload[3]);

    switch(packet.header)
    {
        case DRONE_HEADER_REMOTE_COMMANDS:
            onboard_led_on();
            com = *(input_commands_t*)packet.payload;
            Feedback::set_remote_commands(com);
            break;
        case DRONE_HEADER_HOVER:
            onboard_led_off();
            break;
        case DRONE_HEADER_LAND:
            onboard_led_off();
            break;
        case DRONE_HEADER_EMERGENCY_STOP:
            onboard_led_off();
            break;
        case DRONE_HEADER_WATCH_DOG:
            break;
        default:
            // Handle unknown command
            break;
    }
}


void wireless_transmit()
{
    Feedback::get_active_remote_commands(&com);
    sendto(sock, &com, sizeof(input_commands_t), 0, (struct sockaddr*)&addr_station, sizeof(addr_station));  
}