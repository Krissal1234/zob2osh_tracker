#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sd_logger.h"
#include "esp_log.h"

void app_main() {
    ESP_LOGI("MAIN", "Starting SD Logger Test...");

    // Initialize SD Card
    if (!sd_logger_init()) {
        ESP_LOGE("MAIN", "SD card initialization failed!");
        return;
    }

    test_write_file();
    test_read_file();
    while(1){
      vTaskDelay(1000);
    }
}