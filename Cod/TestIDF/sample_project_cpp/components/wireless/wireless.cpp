#include "wireless.h"
#include <esp_log.h>

static int sock;
static struct sockaddr_in addr_drone = {};
static struct sockaddr_in addr_station = {};

SemaphoreHandle_t xMutexWireless;

static const char* TAG = "wireless";

///////////////////////////////////////////////////////////////////////
/// @brief Transmit mode options. 
/// @details This enum lists all the transmit modes implemented
///////////////////////////////////////////////////////////////////////
typedef enum 
{
    TRANSMIT_MODE_IDLE,
    TRANSMIT_MODE_DUTY,
    TRANSMIT_MODE_TELEMETRY,
    TRANSMIT_MODE_VOLTAGE,

} transmit_mode_t;

static transmit_mode_t transmit_mode_state = TRANSMIT_MODE_IDLE; 

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
    xMutexWireless = xSemaphoreCreateMutex();

    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_register(
        WIFI_EVENT,
        ESP_EVENT_ANY_ID,
        &wifi_event_handler,
        NULL,
        NULL
    );
    esp_netif_create_default_wifi_ap();

    wifi_config_t wifi_config = 
    {
        .ap = {
            .ssid =                 "Drona_ESP32",
            .password =             "12345678",
            .ssid_len =             0,
            .channel =              1,
            .authmode =             WIFI_AUTH_WPA2_PSK,
            .ssid_hidden =          0,
            .max_connection =       4,                      /**< Max number of stations allowed to connect in. Please note that soft-AP and ESP-NOW share the same encryption hardware keys, so the max_connection parameter will be affected by CONFIG_ESP_WIFI_ESPNOW_MAX_ENCRYPT_NUM. */
            .beacon_interval =      100,                    /**< Beacon interval which should be multiples of 100. Unit: TU(time unit, 1 TU = 1024 us). Range: 100 ~ 60000. Default value: 100 */
            .csa_count =            3,                      /**< Channel Switch Announcement Count. Notify the station that the channel will switch after the csa_count beacon intervals. Default value: 3 */
            .dtim_period =          1,                      /**< Dtim period of soft-AP. Range: 1 ~ 10. Default value: 1 */
            .pairwise_cipher =      WIFI_CIPHER_TYPE_CCMP,  /**< Pairwise cipher of SoftAP, group cipher will be derived using this. Cipher values are valid starting from WIFI_CIPHER_TYPE_TKIP, enum values before that will be considered as invalid and default cipher suites(TKIP+CCMP) will be used. Valid cipher suites in softAP mode are WIFI_CIPHER_TYPE_TKIP, WIFI_CIPHER_TYPE_CCMP, WIFI_CIPHER_TYPE_TKIP_CCMP, WIFI_CIPHER_TYPE_GCMP and WIFI_CIPHER_TYPE_GCMP256. */
            .ftm_responder =        false,                  /**< Enable FTM Responder mode */
            .pmf_cfg = {                                    /**< Configuration for Protected Management Frame */
                .capable = true,
                .required = false
            },
            .sae_pwe_h2e =          WPA3_SAE_PWE_BOTH,      /**< Configuration for SAE PWE derivation method. Default value :2 (WPA3_SAE_PWE_BOTH) */
            .transition_disable =   false,                  /**< Whether to enable transition disable feature */
            .sae_ext =              false,                  /**< Enable SAE EXT feature. SOC_GCMP_SUPPORT is required for this feature. */
            .wpa3_compatible_mode = false,                  /**< Enable WPA3 compatible authmode feature. Note: Enabling this will override the AP configuration's authmode and pairwise_cipher. The AP will operate as a WPA2 access point for all stations except for those that support WPA3 compatible mode. Only WPA3 compatibility mode stations will be able to use WPA3-SAE */
            .reserved =             0,
            .bss_max_idle_cfg = {
                .period = WIFI_AP_DEFAULT_MAX_IDLE_PERIOD,
                .protected_keep_alive = 1,
            },
            .gtk_rekey_interval =   3600,
        }
    };
    esp_wifi_set_mode(WIFI_MODE_AP);

    // Set STA interface protocol to 802.11bgn
    uint8_t protocol = WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N;
    esp_wifi_set_protocol(WIFI_IF_STA, protocol);

    // Set STA interface bandwidth to 40 MHz
    esp_wifi_set_bandwidth(WIFI_IF_STA, WIFI_BW40);

    // Set the config struct 
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config);

    // Starts WIFI Task
    esp_wifi_start();  

    // Starts lwIP and UDP Task                                 
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);    

    // Set station connection data for socket
    addr_station.sin_family = AF_INET;
    addr_station.sin_port = htons(6000);
    addr_station.sin_addr.s_addr = inet_addr("192.168.4.2");

    // Set drone connection data for socket
    addr_drone.sin_family = AF_INET;
    addr_drone.sin_port = htons(6000);
    addr_drone.sin_addr.s_addr = htonl(INADDR_ANY);

    // Start socket
    bind(sock, (struct sockaddr*)&addr_drone, sizeof(addr_drone));

    ESP_LOGI(TAG, "Done setting up WIFI");
}


///////////////////////////////////////////////////////////////////////
/// @brief Reads and processes packet recevied from station.
///////////////////////////////////////////////////////////////////////
void wireless_read()
{
    drone_packet_t packet;
    recvfrom(sock, &packet, sizeof(drone_packet_t), 0, NULL, NULL);
    ESP_LOGI(TAG, "Packet header: %d, Payload %d %d %d %d", packet.header, packet.payload[0], packet.payload[1], packet.payload[2], packet.payload[3]);

    if(xSemaphoreTake(xMutexWireless, 0) == pdTRUE)
    {
        // Command parse code
        switch(packet.header)
        {
            // Connection integrity check
            case COMMAND_HEADER_WATCH_DOG:
                // NOT IMPLEMENTED
                break;

            // Turn ON / OFF Drone motors
            case COMMAND_HEADER_SET_ACTIVE:
                onboard_led_on();
                Feedback::set_state(DRONE_STATE_ACTIVE);
                break;
            case COMMAND_HEADER_SET_IDLE:
                onboard_led_off();
                Feedback::set_state(DRONE_STATE_IDLE);
                break;

            // Set drone parameters
            case COMMAND_HEADER_SET_DUTY:
                duty_commands_t duty;
                memcpy(&duty, packet.payload, sizeof(duty));
                Feedback::send_motor_commands(duty);        
                break;
            case COMMAND_HEADER_SET_COMMANDS:
                input_commands_t com;
                memcpy(&com, packet.payload, sizeof(com));
                Feedback::set_active_commands(com);    
                break;

            // Tells drone what commands to send to station
            case COMMAND_HEADER_TRANSMIT_MODE_IDLE:
                transmit_mode_state = TRANSMIT_MODE_IDLE;
                break;
            case COMMAND_HEADER_TRANSMIT_MODE_DUTY:
                transmit_mode_state = TRANSMIT_MODE_DUTY;
                break;
            case COMMAND_HEADER_TRANSMIT_MODE_TELEMETRY:
                transmit_mode_state = TRANSMIT_MODE_TELEMETRY;
                break;
            case COMMAND_HEADER_TRANSMIT_MODE_VOLTAGE:
                transmit_mode_state = TRANSMIT_MODE_VOLTAGE;
                break;                
            default:
                // Handle unknown command
                break;
        }

        xSemaphoreGive(xMutexWireless);
    }
}

///////////////////////////////////////////////////////////////////////
/// @brief Sends packet to station.
///////////////////////////////////////////////////////////////////////
void wireless_transmit()
{
    if(xSemaphoreTake(xMutexWireless, 0) == pdTRUE)
    {
        ESP_LOGI(TAG, "Entered trasnmit, tmode = %d", transmit_mode_state);
        switch(transmit_mode_state)
        {
            case TRANSMIT_MODE_IDLE:
                break;
            case TRANSMIT_MODE_DUTY:
                duty_commands_t duty;
                Feedback::get_active_duty(&duty);
                sendto(
                    sock, 
                    &duty, 
                    sizeof(duty_commands_t), 
                    0, 
                    (struct sockaddr*)&addr_station, 
                    sizeof(addr_station)
                );
                break;
            case TRANSMIT_MODE_TELEMETRY:
                input_commands_t com;
                Feedback::get_active_commands(&com);
                sendto(
                    sock, 
                    &com, 
                    sizeof(input_commands_t), 
                    0, 
                    (struct sockaddr*)&addr_station, 
                    sizeof(addr_station)
                );
                break;     
            case TRANSMIT_MODE_VOLTAGE:
                int voltage = battery_read();
                sendto(
                    sock, 
                    &voltage, 
                    sizeof(int), 
                    0, 
                    (struct sockaddr*)&addr_station, 
                    sizeof(addr_station)
                );
                break;    
        }

        xSemaphoreGive(xMutexWireless);
    }
}