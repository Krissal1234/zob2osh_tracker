#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sd_card/sd_logger.h"
#include "network/wifi_manager.h"
#include "tracker/gps_tracker.h"
#include "esp_log.h"
#include "gnss/gps_uart.h"
#include "esp_sleep.h"
#include "tracker/gps_tracker.h"  // Adjust the path as needed based on your folder layout

#define UPLOAD_PIN 33
#define UPLOAD_BUTTON 12
#define UPLOAD_BUTTON_DURATION_US (5 * 1000000)


static const char *TAG = "BUTTON";

// volatile bool upload_enabled = false;
RTC_DATA_ATTR bool upload_enabled = false;

volatile bool button_pressed = false;
volatile int64_t press_start_time = 0;
uint16_t interrupt_count = 0;
uint64_t sleepTimeSeconds = 20;

static void IRAM_ATTR upload_coordinates_handler(void *arg){

   upload_enabled = true;

}
void test_offset_write(void) {
  FILE *f = fopen("/sdcard/test_offset.bin", "w");
  if (!f) {
      perror("test_offset fopen");
      ESP_LOGE(TAG, "Test offset file failed to open");
      return;
  }
  fprintf(f, "1234");
  fclose(f);
  ESP_LOGI(TAG, "Test offset file written!");
}

void app_main() {
   //interrup pin setup for GPIO 21 - UPLOAD PIN
   gpio_reset_pin(UPLOAD_PIN);
   gpio_set_direction(UPLOAD_PIN,GPIO_MODE_INPUT);
   gpio_set_pull_mode(UPLOAD_PIN, GPIO_PULLUP_ONLY);
   gpio_set_intr_type(UPLOAD_PIN, GPIO_INTR_POSEDGE);
   gpio_install_isr_service(0);
   gpio_isr_handler_add(UPLOAD_PIN, upload_coordinates_handler, NULL);
   gpio_intr_enable(UPLOAD_PIN);

   //uart init before polling gps data
   init_uart();

   //sd card init
   if(!sd_logger_init()){
   printf("SD CARD init failed");
   }
  //  test_offset_write();

   esp_sleep_enable_ext0_wakeup(UPLOAD_PIN, 0);

   esp_sleep_wakeup_cause_t wake_cause = esp_sleep_get_wakeup_cause();

   if (wake_cause == ESP_SLEEP_WAKEUP_TIMER) {
       gps_tracker_run();
       if (upload_enabled) {
        if (connect_wifi() == WIFI_SUCCESS) {
            gps_tracker_upload();  // Reads and uploads GNSS records
        } else {
            ESP_LOGE("MAIN", "Wi-Fi connection failed, cannot upload");
        }
         //enter logic to upload coordinates to server

           upload_enabled = false;
       }
   } else if (wake_cause == ESP_SLEEP_WAKEUP_EXT0) {
       upload_enabled = true;  // Set the flag if woken by button
      //  printf("Will upload coordinates in the next cycle");
   }

   esp_sleep_enable_timer_wakeup((uint64_t)sleepTimeSeconds * 1000000);
   esp_deep_sleep_start();
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