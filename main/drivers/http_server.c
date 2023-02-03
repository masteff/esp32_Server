#include "esp_http_server.h"
#include "esp_log.h"
#include <string.h>

#include "http_server.h"
#include "tasks_common.h"
#include "wifi_driver.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))         //Macro to get the minimum from a and b

static const char *TAG = "HTTP_SERVER";

bool updating; 
char update[512]; 

char operatorBuff[40] = "\0";

// HTTP server task handle
static httpd_handle_t http_server_handle = NULL;

// UART to Server Communication task handle
static TaskHandle_t uart_server_task_handle = NULL;

// Queue Handles for Communication between server and UART
QueueHandle_t server_to_uart_queue_handle;
extern QueueHandle_t uart_to_server_queue_handle;               //defined in uart_driver.c


// Embedded files / need to be registered main/CMakeLists.txt
extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");
extern const uint8_t styles_css_start[] asm("_binary_styles_css_start");
extern const uint8_t styles_css_end[] asm("_binary_styles_css_end");
extern const uint8_t app_js_start[] asm("_binary_app_js_start");
extern const uint8_t app_js_end[] asm("_binary_app_js_end");
extern const uint8_t favicon_ico_start[] asm("_binary_favicon_ico_start");
extern const uint8_t favicon_ico_end[] asm("_binary_favicon_ico_end");

// function to send a message from server to uart task
static BaseType_t send_message_to_uart(server_uart_message_e msgID, char *message)
{
    server_uart_message_t msg;
    msg.msgID = msgID;
    msg.message = message;
    return xQueueSend(server_to_uart_queue_handle, &msg, portMAX_DELAY);
}

// task for handling messages from uart
static void uart_server_task(void *pvParamters)
{
    uart_server_message_t msg;

    for (;;)
    {
        if (uart_to_server_queue_handle)
        {
            if (xQueueReceive(uart_to_server_queue_handle, &msg, 50 / portTICK_RATE_MS))
            {
                switch (msg.msgID)
                {
                case UART_SERVER_START_UPDATING:
                    updating = true;
                    break;
                case UART_SERVER_STOP_UPDATING:
                    updating = false;
                    break;
                case UART_SERVER_GOT_UPDATE:
                    gotUpdate = true;
                    strcpy(update, msg.update);
                    break;
                default:
                    break;
                }
            }
        }
        else
        {
            ESP_LOGI(TAG, "queue not created");
        }
    }
}

/********************************************************************************
 * @brief 
 * all the handlers for different client requests
 * for file requests the embedded files need to be send back in the response
 * for custom routes the response is json formatted and can easily be extended
 *******************************************************************************/

static esp_err_t http_server_index_html_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "%s requested", req->uri);
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, (const char *)index_html_start, index_html_end - index_html_start);
    return ESP_OK;
}
static esp_err_t http_server_app_css_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "app.css requested");
    httpd_resp_set_type(req, "text/css");
    httpd_resp_send(req, (const char *)styles_css_start, styles_css_end - styles_css_start);
    return ESP_OK;
}
static esp_err_t http_server_app_js_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "app.js requested");
    httpd_resp_set_type(req, "application/javascript");
    httpd_resp_send(req, (const char *)app_js_start, app_js_end - app_js_start);
    return ESP_OK;
}
static esp_err_t http_server_favicon_ico_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "favicon.ico requested");
    httpd_resp_set_type(req, "image/x-icon");
    httpd_resp_send(req, (const char *)favicon_ico_start, favicon_ico_end - favicon_ico_start);
    return ESP_OK;
}
static esp_err_t btnStart_handler(httpd_req_t *req)
{
    send_message_to_uart(SERVER_UART_STATION_START, NULL);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, BTN_STATION_START_RES, strlen(BTN_STATION_START_RES));
    return ESP_OK;
}
static esp_err_t btnStop_handler(httpd_req_t *req)
{
    send_message_to_uart(SERVER_UART_STATION_STOP, NULL);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, BTN_STATION_STOP_RES, strlen(BTN_STATION_STOP_RES));
    return ESP_OK;
}
static esp_err_t btnStartUpdates_handler(httpd_req_t *req)
{
    send_message_to_uart(SERVER_UART_START_UPDATES, NULL);  
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, BTN_UPDATE_START_RES, strlen(BTN_UPDATE_START_RES));
    return ESP_OK;
}
static esp_err_t getUpdate_handler(httpd_req_t *req)
{
    // send_message_to_uart(SERVER_UART_STOP_UPDATES, NULL);
    if (updating)
    {
        if (gotUpdate)
        {
            char messageOK[512];
            char *end = "\"}";
            strcpy(messageOK, "{\"update\": \"");
            strcat(messageOK, update);
            strcat(messageOK, end);
            httpd_resp_send(req, messageOK, strlen(messageOK));
            memset(update, '\0', strlen(update));
            memset(messageOK, '\0', strlen(messageOK));
            gotUpdate = false;
        }
        else
        {
            char *message = "{\"update\": \"No update\"}";
            httpd_resp_send(req, message, strlen(message));
        }
    }
    return ESP_OK;
}
static esp_err_t btnStopUpdates_handler(httpd_req_t *req)
{
    send_message_to_uart(SERVER_UART_STOP_UPDATES, NULL);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, BTN_UPDATE_STOP_RES, strlen(BTN_UPDATE_STOP_RES));
    return ESP_OK;
}
static esp_err_t btnSetOperator_handler(httpd_req_t *req)
{
    int recv_len; 
    if ((recv_len = httpd_req_recv(req, operatorBuff, MIN(req->content_len, sizeof(operatorBuff)))) < 0)
    {
        // Check if timeout occurred
        if (recv_len == HTTPD_SOCK_ERR_TIMEOUT)
        {
            ESP_LOGI(TAG, "HTTPD_SOCK_ERR_TIMEOUT");
        }
        ESP_LOGI(TAG, "Error %d", recv_len);
        return ESP_FAIL;
    }

    send_message_to_uart(SERVER_UART_SET_OPERATOR, operatorBuff); 
    ESP_LOGI(TAG, "content: %s with length: %d", operatorBuff, recv_len);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, BTN_SET_OPERATOR_RES, strlen(BTN_SET_OPERATOR_RES));


    return ESP_OK;
}

/*********************************************************************************
 * @brief 
 * helper function for the http server
 * creates the message queue and communication task
 * configures and starts the server
 * registers the request handlers with the routes
 ********************************************************************************/

static httpd_handle_t http_server_configure(void)
{
    // message queue
    server_to_uart_queue_handle = xQueueCreate(3, sizeof(server_uart_message_t));
    // // create http server uart communication task
    xTaskCreatePinnedToCore(uart_server_task, "uart_server_task",
                            UART_SERVER_TASK_STACK_SIZE, NULL,
                            UART_SERVER_TASK_PRIORITY, &uart_server_task_handle,
                            UART_SERVER_TASK_CORE_ID);

    // server object with default congfig
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    // core the http server will run on
    config.core_id = HTTP_SERVER_TASK_CORE_ID;
    // httper server priority
    config.task_priority = HTTP_SERVER_TASK_PRIORITY;
    // bump the stack size
    config.stack_size = HTTP_SERVER_TASK_STACK_SIZE;
    // increase uri handlers
    config.max_uri_handlers = 20;
    // Increase the timeout limits
    config.recv_wait_timeout = 10;
    config.send_wait_timeout = 10;

    ESP_LOGI(TAG, "http_server_configure: starting server on port : %d with task priority %d", config.server_port, config.task_priority);

    // start the http server
    if (httpd_start(&http_server_handle, &config) == ESP_OK)
    {
        ESP_LOGI(TAG, "http_server_configure: registering uri handlers");

        // register index.html handler
        httpd_uri_t index_html = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = http_server_index_html_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(http_server_handle, &index_html);

        // register app.css handler
        httpd_uri_t app_css = {
            .uri = "/styles.css",
            .method = HTTP_GET,
            .handler = http_server_app_css_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(http_server_handle, &app_css);

        // register app.js handler
        httpd_uri_t app_js = {
            .uri = "/app.js",
            .method = HTTP_GET,
            .handler = http_server_app_js_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(http_server_handle, &app_js);

        // register favicon.ico handler
        httpd_uri_t favicon_ico = {
            .uri = "/favicon.ico",
            .method = HTTP_GET,
            .handler = http_server_favicon_ico_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(http_server_handle, &favicon_ico);

        // register setOperator Post handler
        httpd_uri_t btnSetOperator = {
            .uri = "/btnSetOperator",
            .method = HTTP_POST,
            .handler = btnSetOperator_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(http_server_handle, &btnSetOperator);

        // register btnStart handler
        httpd_uri_t btnStart = {
            .uri = "/btnStart",
            .method = HTTP_GET,
            .handler = btnStart_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(http_server_handle, &btnStart);

        // register btnStop handler
        httpd_uri_t btnStop = {
            .uri = "/btnStop",
            .method = HTTP_GET,
            .handler = btnStop_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(http_server_handle, &btnStop);

        // register btnStartUpdates handler
        httpd_uri_t btnStartUpdates = {
            .uri = "/btnStartUpdates",
            .method = HTTP_GET,
            .handler = btnStartUpdates_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(http_server_handle, &btnStartUpdates);

        // register getUpdate handler
        httpd_uri_t getUpdate = {
            .uri = "/getUpdate",
            .method = HTTP_GET,
            .handler = getUpdate_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(http_server_handle, &getUpdate);

        // register btnStopUpdates handler
        httpd_uri_t btnStopUpdates = {
            .uri = "/btnStopUpdates",
            .method = HTTP_GET,
            .handler = btnStopUpdates_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(http_server_handle, &btnStopUpdates);

    }

    return http_server_handle;
}

// global function for starting the server
void http_server_start(void)
{
    if (http_server_handle == NULL)
    {
        http_server_handle = http_server_configure();
    }
}

// global function for stopping the server
void http_server_stop(void)
{
    if (http_server_handle)
    {
        ESP_ERROR_CHECK(httpd_stop(http_server_handle));
        ESP_LOGI(TAG, "Stopping http server");
        http_server_handle = NULL;
    }

    if (uart_server_task_handle)
        uart_server_task_handle = NULL;
}