#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sd_card/sd_logger.h"
#include "network/wifi_manager.h"
#include "tracker/gps_tracker.h"
#include "esp_log.h"
#include "gnss/gps_uart.h"

#define upload_pin 21

#define UPLOAD_BUTTON 12
#define UPLOAD_BUTTON_DURATION_US (5 * 1000000)


static const char *TAG = "BUTTON";

volatile bool upload_enabled = false;
volatile bool button_pressed = false;
volatile int64_t press_start_time = 0;

void app_main() {


  init_uart();
  // wait_for_gps_fix();

  char nmea_sentence[128];
  bool upload_requested = false;
  // if (wait_for_fix_and_get_nmea(nmea_sentence, sizeof(nmea_sentence), 30000)) {
  //     printf("Fix acquired! Sentence: %s\n", nmea_sentence);
  // } else {
  //     printf("Timed out waiting for GPS fix.\n");
  // }
  while(1){

  gps_tracker_run(); // Tries to obtain a fix for 60 seconds, if successful logs to the sd card

  //Add logic to detect interrupt and turn on a flag

   if (upload_requested) {
        upload_requested = false;
        // upload_data_to_cloud();    // do the heavy lifting here
    }
  // vTaskDelay(pdMS_TO_TICKS(100));

  // sd_logger_init();
  // test_write_file();
//   vTaskDelay(1000 / portTICK_PERIOD_MS); // Small delay to avoid CPU overload
  }

}

void IRAM_ATTR gpio_isr_handler(void *arg) {



}


// //   void test_network(void){
// //     esp_err_t status = connect_wifi();
// //     if(status == WIFI_SUCCESS){
// //       ESP_LOGI("Main", "ESP connected successfully");
// //     }
// //     else{
// //       ESP_LOGI("Main","ESP connection failed");
// //     }


// //     esp_err_t stat = connect_tcp_server();
// //   }
// //   void test_sd_card(void){

// //   ESP_LOGI("MAIN", "Starting SD Logger Test...");

// //   // Initialize SD Card
// //   if (!sd_logger_init()) {
// //       ESP_LOGE("MAIN", "SD card initialization failed!");
// //       return;
// //   }

// //   test_write_file();
// //   test_read_file();
//   }