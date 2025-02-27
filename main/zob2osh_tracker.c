#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sd_card/sd_logger.h"
#include "network/wifi_manager.h"
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

    esp_err_t status = connect_wifi();
    if(status == WIFI_SUCCESS){
      ESP_LOGI("Main", "ESP connected successfully");
    }
    else{
      ESP_LOGI("Main","ESP connection failed");
    }

    while(1){
      vTaskDelay(1000);
    }
}