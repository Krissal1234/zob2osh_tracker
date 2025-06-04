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
#include "config.h"
#include "screen_display/screen_display.h"
#define UPLOAD_PIN 33
#define UPLOAD_BUTTON 12
#define UPLOAD_BUTTON_DURATION_US (5 * 1000000)


static const char *TAG = "BUTTON";

// volatile bool upload_enabled = false;
RTC_DATA_ATTR bool upload_enabled = false;

volatile bool button_pressed = false;
volatile int64_t press_start_time = 0;
uint16_t interrupt_count = 0;

static void IRAM_ATTR upload_coordinates_handler(void *arg){

   upload_enabled = true;

}
// void test_offset_write(void) {
//   FILE *f = fopen("/sdcard/test_offset.bin", "w");
//   if (!f) {
//       perror("test_offset fopen");
//       ESP_LOGE(TAG, "Test offset file failed to open");
//       return;
//   }
//   fprintf(f, "1234");
//   fclose(f);
//   ESP_LOGI(TAG, "Test offset file written!");
// }

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

   screen_display_init();
   screen_display_battery_status(24);
   screen_display_log("ZOBBY");
   // Initialize OLED

 // 1. Initialize SD card first
 if (!sd_logger_init()) {
    ESP_LOGE(TAG, "SD card init failed, aborting");
    screen_display_log("SDINIT:!");
  }

  // 2. Load config from SD card
  if (!load_config_from_file("/sdcard/config.txt")) {
    ESP_LOGE(TAG, "Failed to load config file");
    screen_display_log("CONFIG:!");
  }

  ESP_LOGI(TAG, "Config loaded:");
  ESP_LOGI(TAG, "SSID: %s", app_config.wifi_ssid);
  ESP_LOGI(TAG, "Server: %s", app_config.server_url);
  ESP_LOGI(TAG, "Pass: %s", app_config.wifi_password);
  ESP_LOGI(TAG, "GPS Timeout: %d ms", app_config.search_for_fix_timeout_ms);

  // screen_display_clear_loop();
   esp_sleep_enable_ext0_wakeup(UPLOAD_PIN, 0);

   esp_sleep_wakeup_cause_t wake_cause = esp_sleep_get_wakeup_cause();

   if (wake_cause == ESP_SLEEP_WAKEUP_TIMER) {
       gps_tracker_run();
       if (upload_enabled) {
          screen_display_log("WIFI:...");
        if (connect_wifi() == WIFI_SUCCESS) {
            gps_tracker_upload();  // Reads and uploads GNSS records
        } else {
            screen_display_log("CONN:!!!");

            ESP_LOGE("MAIN", "Wi-Fi connection failed, cannot upload");
        }
         //enter logic to upload coordinates to server

           upload_enabled = false;
       }
   } else if (wake_cause == ESP_SLEEP_WAKEUP_EXT0) {
       upload_enabled = true;  // Set the flag if woken by button
      //  printf("Will upload coordinates in the next cycle");
   }

    int sleep_time_ms = app_config.esp_sleep_time_ms;
    uint64_t sleep_time_us = (uint64_t)sleep_time_ms * 1000;

   esp_sleep_enable_timer_wakeup((uint64_t)sleep_time_us);
   esp_deep_sleep_start();
  }


