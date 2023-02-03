#ifndef MAIN_DRIVERS_HTTP_SERVER_H
#define MAIN_DRIVERS_HTTP_SERVER_H

// server json responses
#define BTN_STATION_START_RES       "{\"reply\": \"Starting Test\"}"
#define BTN_STATION_STOP_RES        "{\"reply\": \"Stopping Test\"}"
#define BTN_UPDATE_START_RES        "{\"update\": \"Starting Updates\"}"
#define BTN_UPDATE_STOP_RES         "{\"update\": \"Stopping Updates\"}"
#define BTN_SET_OPERATOR_RES        "{\"reply\": \"Operator set\"}"

typedef enum http_server_message
{
    HTTP_MSG_WIFI_CONNECT_INIT = 0,
    HTTP_MSG_WIFI_CONNECT_SUCCESS,
    HTTP_MSG_WIFI_CONNECT_FAIL,
} http_server_message_e;

typedef struct http_server_queue_message
{
    http_server_message_e msgID;
} http_server_queue_message;


BaseType_t http_server_monitor_send_message(http_server_message_e msgID);

void http_server_start(void);

void http_server_stop(void);

#endif