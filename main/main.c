#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#include "drivers/wifi_driver.h"
#include "drivers/uart_driver.h"

#define LOGGING_OFF

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // since the monitor uart port is also used for communication with the station
    // esp logging should be turned off to avoid interference
    #ifdef LOGGING_OFF
        esp_log_level_set("*", ESP_LOG_NONE);
    #endif

    start_wifi();
    start_uart();
}
