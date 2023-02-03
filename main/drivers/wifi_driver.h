#ifndef MAIN_DRIVERS_WIFI_DRIVER_H_
#define MAIN_DRIVERS_WIFI_DRIVER_H_

#include <string.h>
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

#include "lwip/netdb.h"
#include "lwip/err.h"
#include "lwip/sys.h"

// wifi AP configuration
#define WIFI_SSID               "Test Station 1"
#define WIFI_PASSWORD           "Password"
#define WIFI_CHANNEL            1
#define WIFI_SSID_HIDDEN        0
#define WIFI_MAX_CONN           2
#define WIFI_BEACON_INTERVAL    100
#define WIFI_IP                 "192.168.0.1"
#define WIFI_GATEWAY            "192.168.0.1"
#define WIFI_NETMASK            "255.255.255.0"

extern esp_netif_t* esp_netif_ap;

//Message type for Queues
typedef enum wifi_message
{
    WIFI_MSG_WIFI_STARTED = 0,
    WIFI_MSG_START_HTTP_SERVER,
    WIFI_MSG_STATION_JOINED,
    WIFI_MSG_STATION_LEFT,
} wifi_message_e;

typedef struct wifi_queue_message
{
    wifi_message_e msgID;
} wifi_queue_message_t;

void start_wifi(void);

#endif