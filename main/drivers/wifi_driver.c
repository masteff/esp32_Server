#include "wifi_driver.h"
#include "tasks_common.h"
#include "http_server.h"


static const char *TAG = "WIFI_DRIVER";

static QueueHandle_t wifi_driver_queue_handle;

// wifi object to configure
esp_netif_t* esp_netif_ap  = NULL;


// function for sending internal messages
BaseType_t wifi_send_message(wifi_message_e msgID)
{
    wifi_queue_message_t msg;
    msg.msgID = msgID;
    return xQueueSend(wifi_driver_queue_handle, &msg, portMAX_DELAY);
}

// logging wifi events
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)                      
{
    wifi_event_ap_staconnected_t* event_connected = NULL;
    wifi_event_ap_stadisconnected_t* event_disconnedted = NULL;
    switch(event_id)
    {
        case WIFI_EVENT_AP_START:
            ESP_LOGI(TAG, "WIFI_EVENT_AP_START");
            break;

        case WIFI_EVENT_AP_STOP:
            ESP_LOGI(TAG, "WIFI_EVENT_AP_STOP");
            break;

        case WIFI_EVENT_AP_STACONNECTED:
            event_connected = (wifi_event_ap_staconnected_t*) event_data;
            ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",MAC2STR(event_connected->mac), event_connected->aid); 
            break;

        case WIFI_EVENT_AP_STADISCONNECTED:
            event_disconnedted = (wifi_event_ap_stadisconnected_t*) event_data;
            ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",MAC2STR(event_disconnedted->mac), event_disconnedted->aid);
            break;
        
        default:
            break;
    }
}

// general wifi task for initialisation and handling message queue
static void wifi_task(void *pvParameters)
{
    wifi_queue_message_t msg;

    // Init wifi event handler
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler, NULL, NULL));

    // Init the TCP stack
    ESP_ERROR_CHECK(esp_netif_init());

	// Default WiFi config - operations must be in this order!
	wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	esp_netif_ap = esp_netif_create_default_wifi_ap();

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_SSID,
            .ssid_len = strlen(WIFI_SSID),
            .channel = WIFI_CHANNEL,
            .password = WIFI_PASSWORD,
            .max_connection = WIFI_MAX_CONN,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .ssid_hidden = WIFI_SSID_HIDDEN
        },
    };

	// Configure DHCP for the AP
	esp_netif_ip_info_t ap_ip_info;
	memset(&ap_ip_info, 0x00, sizeof(ap_ip_info));

    // Config DHCP server IP
	esp_netif_dhcps_stop(esp_netif_ap);					                        ///> must call this first
	inet_pton(AF_INET, WIFI_IP, &ap_ip_info.ip);		                        ///> Assign access point's static IP, GW, and netmask
	inet_pton(AF_INET, WIFI_GATEWAY, &ap_ip_info.gw);
	inet_pton(AF_INET, WIFI_NETMASK, &ap_ip_info.netmask);
	ESP_ERROR_CHECK(esp_netif_set_ip_info(esp_netif_ap, &ap_ip_info));			///> Statically configure the network interface
	ESP_ERROR_CHECK(esp_netif_dhcps_start(esp_netif_ap));						///> Start the AP DHCP server (for connecting stations e.g. your mobile device)

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));


    ESP_ERROR_CHECK(esp_wifi_start());
    wifi_send_message(WIFI_MSG_WIFI_STARTED);
    wifi_send_message(WIFI_MSG_START_HTTP_SERVER);

    // handling messages, e.g. starting the server
    for(;;)
    {
        if(xQueueReceive(wifi_driver_queue_handle, &msg, portMAX_DELAY))
        {
            switch(msg.msgID)
            {
                case WIFI_MSG_WIFI_STARTED:
                    ESP_LOGI(TAG, "start_wifi finished with SSID:%s password:%s channel:%d",
                             WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);
                    break;

                case WIFI_MSG_START_HTTP_SERVER:
                    ESP_LOGI(TAG, "Sarting HTTP server...");
                    http_server_start();
                    break;

                case WIFI_MSG_STATION_JOINED:
                    break;

                case WIFI_MSG_STATION_LEFT:
                    break;

                default:
                    break;
            }
        }
    }
}


// function to be called from main to start wifi
void start_wifi(void)
{
    ESP_LOGI(TAG, "Starting Wifi...");

    // Disable default wifi logging
    esp_log_level_set("wifi", ESP_LOG_NONE);

    // Create message queue
    wifi_driver_queue_handle = xQueueCreate(3, sizeof(wifi_queue_message_t));

    // Start wifi task
    xTaskCreatePinnedToCore(wifi_task, "wifi_task", WIFI_TASK_STACK_SIZE, NULL, WIFI_TASK_PRIORITY, NULL, WIFI_TASK_CORE_ID);

}