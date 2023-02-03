#ifndef MAIN_DRIVERS_UART_DRIVER_H
#define MAIN_DRIVERS_UART_DRIVER_H

#include "driver/uart.h"
#include "driver/gpio.h"


// UART configuration
#define TXD_PIN                 UART_PIN_NO_CHANGE
#define RXD_PIN                 UART_PIN_NO_CHANGE
#define RTS_PIN                 UART_PIN_NO_CHANGE
#define CTS_PIN                 UART_PIN_NO_CHANGE
#define UART_PORT_NUM           UART_NUM_0
#define UART_BAUD_RATE          115200
#define BUF_SIZE                256

// UART to station messages
#define UART_STATION_START      "Station:Start:::"
#define UART_STATION_STOP       "Station:Stop:::"
#define UART_UPDATES_START      "Updates:Start:::"
#define UART_UPDATES_STOP       "Updates:Stop:::"

void start_uart(void);

#endif