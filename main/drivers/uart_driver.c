#include "uart_driver.h"
#include "tasks_common.h"
#include "esp_log.h"
#include <string.h>

const char *TAG = "uart_driver";

// Queue Handles for Communication between server and UART
QueueHandle_t uart_to_server_queue_handle;
extern QueueHandle_t server_to_uart_queue_handle;

// function to send a message from uart to server task
BaseType_t send_message_to_server(uart_server_message_e msgID, char *update)
{
    uart_server_message_t msg;
    msg.msgID = msgID;
    msg.update = update;
    return xQueueSend(uart_to_server_queue_handle, &msg, portMAX_DELAY);
}

static void server_uart_task(void *pvParamters) 
{
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    int intr_alloc_flags = 0;

    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, TXD_PIN, RXD_PIN, RTS_PIN, CTS_PIN));

    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *)malloc(BUF_SIZE);
    // memset(data, '\0', BUF_SIZE);

    // char *start_str = "Station:Start\r\n";
    // char *stop_str = "Station:Stop\r\n";
    // char *start_update_str = "Updates:Start\r\n";
    // char *stop_update_str = "Updates:Stop\r\n";
    char *operator_start_str = "Operator:";
    char *operator_end_str = COMMAND_TERMINATION;
    int length = 0;
    
    for(;;)
    {
        // Read data from the UART
        ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_PORT_NUM, (size_t*)&length));

        if (length > 0 )
        {
            memset(data, '\0', BUF_SIZE);
            length = uart_read_bytes(UART_PORT_NUM, data, length, 100); 

            uart_flush(UART_PORT_NUM);

            if (strstr((const char *) data, COMMAND_TERMINATION))
            {
                if (strstr((const char *) data, "UpdateStart"))
                    send_message_to_server(UART_SERVER_START_UPDATING, NULL);
                else if (strstr((const char *) data, "UpdateStop"))
                    send_message_to_server(UART_SERVER_STOP_UPDATING, NULL);
                else
                {
                    data[strlen((const char *) data) - 3 ] = '\0';
                    send_message_to_server(UART_SERVER_GOT_UPDATE, (const char *) data);
                }

            }

        }

        server_uart_message_t msg;

        if (server_to_uart_queue_handle)
        {
            if (xQueueReceive(server_to_uart_queue_handle, &msg, 50 / portTICK_RATE_MS))
            {
                switch (msg.msgID)
                {
                case SERVER_UART_STATION_START:
                    uart_write_bytes(UART_PORT_NUM, (const char *) UART_STATION_START, strlen(UART_STATION_START));
                    break;
                case SERVER_UART_STATION_STOP:
                    uart_write_bytes(UART_PORT_NUM, (const char *) UART_STATION_STOP, strlen(UART_STATION_STOP));
                    break;
                case SERVER_UART_SET_OPERATOR:
                    strcat(setOperator, operator_start_str);
                    strcat(setOperator, msg.message);
                    strcat(setOperator, operator_end_str);
                    uart_write_bytes(UART_PORT_NUM, (const char *)setOperator, strlen(setOperator));
                    memset(&msg.message[0], '\0', strlen(msg.message));
                    ESP_LOGI(TAG, "%s", setOperator);
                    memset(&setOperator[0], '\0', strlen(setOperator));
                    break;
                case SERVER_UART_START_UPDATES:
                    uart_write_bytes(UART_PORT_NUM, (const char *) UART_UPDATES_START, strlen(UART_UPDATES_START));
                    break;
                case SERVER_UART_STOP_UPDATES:
                    uart_write_bytes(UART_PORT_NUM, (const char *) UART_UPDATES_STOP, strlen(UART_UPDATES_STOP));
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

// global function for starting UART
void start_uart(void)
{
    // message queue
    uart_to_server_queue_handle = xQueueCreate(3, sizeof(server_uart_message_t));
    // create uart communication task
    xTaskCreatePinnedToCore(server_uart_task, "server_uart_task", SERVER_UART_TASK_STACK_SIZE, NULL,
                            SERVER_UART_TASK_PRIORITY, NULL, SERVER_UART_TASK_CORE_ID);
}