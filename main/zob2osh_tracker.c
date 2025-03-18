#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sd_card/sd_logger.h"
#include "network/wifi_manager.h"
#include "esp_log.h"
#include "gnss/gps_uart.h"
void app_main() {

  init_uart();
  wait_for_gps_fix();
  char nmea_sentence[128];




    while (1) {
        if (read_nmea_sentence(nmea_sentence, sizeof(nmea_sentence))) {
            printf("NMEA Sentence: %s", nmea_sentence);
        }



        vTaskDelay(10 / portTICK_PERIOD_MS); // Small delay to avoid CPU overload
    }

}
  void test_network(void){

    esp_err_t status = connect_wifi();
    if(status == WIFI_SUCCESS){
      ESP_LOGI("Main", "ESP connected successfully");
    }
    else{
      ESP_LOGI("Main","ESP connection failed");
    }


    esp_err_t stat = connect_tcp_server();
  }
  void test_sd_card(void){

  ESP_LOGI("MAIN", "Starting SD Logger Test...");

  // Initialize SD Card
  if (!sd_logger_init()) {
      ESP_LOGE("MAIN", "SD card initialization failed!");
      return;
  }

  test_write_file();
  test_read_file();
  }