#ifndef MAIN_DRIVERS_TASKS_COMMON_H_
#define MAIN_DRIVERS_TASKS_COMMON_H_

// WIFI task
#define WIFI_TASK_STACK_SIZE                        4096
#define WIFI_TASK_PRIORITY                          5
#define WIFI_TASK_CORE_ID                           0

// HTTP server task
#define HTTP_SERVER_TASK_STACK_SIZE                 8192
#define HTTP_SERVER_TASK_PRIORITY                   4
#define HTTP_SERVER_TASK_CORE_ID                    0

// uart to server communication task
#define UART_SERVER_TASK_STACK_SIZE                 4096
#define UART_SERVER_TASK_PRIORITY                   3
#define UART_SERVER_TASK_CORE_ID                    0

// server to uart communication task
#define SERVER_UART_TASK_STACK_SIZE                 2048
#define SERVER_UART_TASK_PRIORITY                   2
#define SERVER_UART_TASK_CORE_ID                    0

#define COMMAND_TERMINATION                      ":::"

char setOperator[32];
bool gotUpdate;                                      // gobal variable to track updating status


typedef enum server_uart_messages
{
    SERVER_UART_INIT = 0,
    SERVER_UART_STATION_START,
    SERVER_UART_STATION_STOP,
    SERVER_UART_SET_OPERATOR,
    SERVER_UART_START_UPDATES,
    SERVER_UART_STOP_UPDATES
} server_uart_message_e;

typedef struct server_uart_message
{
    server_uart_message_e msgID;
    char *message;
} server_uart_message_t;

typedef enum uart_server_messages
{
    UART_SERVER_INIT = 0,
    UART_SERVER_START_UPDATING,
    UART_SERVER_STOP_UPDATING,
    UART_SERVER_GOT_UPDATE
} uart_server_message_e;

typedef struct uart_server_message
{
    uart_server_message_e msgID;
    char *update;
} uart_server_message_t;


#endif