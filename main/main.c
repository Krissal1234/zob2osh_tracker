#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sd_card/sd_logger.h"
#include "network/wifi_manager.h"
#include "tracker/gps_tracker.h"
#include "esp_log.h"
#include "gnss/gps_uart.h"
#include "esp_sleep.h"
#include "tracker/gps_tracker.h"
#include "config.h"
#include "driver/adc.h"
#include "screen_display/screen_display.h"
#define UPLOAD_PIN 4
#define UPLOAD_BUTTON 4
#define UPLOAD_BUTTON_DURATION_US (5 * 1000000)


static const char *TAG = "BUTTON";

RTC_DATA_ATTR bool upload_enabled = false;

volatile bool button_pressed = false;
volatile int64_t press_start_time = 0;
uint16_t interrupt_count = 0;

#define BATTERY_ADC_CHANNEL ADC1_CHANNEL_5
#define ADC_MAX_READING     4095.0
#define REF_VOLTAGE         3.3
#define BATT_FULL_VOLTAGE   3.28            // Estimated voltage when battery is 100%
#define BATT_EMPTY_VOLTAGE  2.7             // Estimated voltage when battery is 0% - from custom battery used in project

void init_battery_adc() {
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(BATTERY_ADC_CHANNEL, ADC_ATTEN_DB_11);
}

float read_battery_voltage() {
    int raw = adc1_get_raw(BATTERY_ADC_CHANNEL);
    float voltage = (raw / ADC_MAX_READING) * REF_VOLTAGE;
    return voltage;
}

int battery_percent(float analog_voltage) {
  const float v_table[] = {
      3.231, 3.154, 3.076, 2.999, 2.922, 2.845,
      2.768, 2.691, 2.614, 2.537, 2.460, 2.307
  };
  const int p_table[] = {
      100,  90,  80,  70,  60,  50,
       40,  30,  20,  10,   5,   0
  };
  const int len = sizeof(v_table) / sizeof(v_table[0]);

  if (analog_voltage >= v_table[0]) return 100;
  if (analog_voltage <= v_table[len - 1]) return 0;

  for (int i = 0; i < len - 1; i++) {
      if (analog_voltage <= v_table[i] && analog_voltage > v_table[i + 1]) {
          float v1 = v_table[i];
          float v2 = v_table[i + 1];
          int p1 = p_table[i];
          int p2 = p_table[i + 1];
          // Linear interpolation
          return p1 + (int)((analog_voltage - v1) * (p2 - p1) / (v2 - v1));
      }
  }

  return 0;
}



static void IRAM_ATTR upload_coordinates_handler(void *arg){

   upload_enabled = true;

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
   screen_display_init();
   init_battery_adc();
   float batt_voltage = read_battery_voltage();
   int batt_percent = battery_percent(batt_voltage);

   ESP_LOGI(TAG, "Battery voltage: %.2f V", batt_voltage);
   ESP_LOGI(TAG, "Battery percent: %d%%", batt_percent);
   screen_display_battery_status(batt_percent);

   screen_display_log("ZOBBY");

 // Initialising SD card
 if (!sd_logger_init()) {
    ESP_LOGE(TAG, "SD card init failed, aborting");
    screen_display_log("SDINIT:!");
  }

  // Loading config from SD card
  if (!load_config_from_file("/sdcard/config.txt")) {
    ESP_LOGE(TAG, "Failed to load config file");
    screen_display_log("CONFIG:!");
  }

  ESP_LOGI(TAG, "Config loaded:");
  ESP_LOGI(TAG, "SSID: %s", app_config.wifi_ssid);
  ESP_LOGI(TAG, "Server: %s", app_config.server_url);
  ESP_LOGI(TAG, "Pass: %s", app_config.wifi_password);
  ESP_LOGI(TAG, "GPS Timeout: %d ms", app_config.search_for_fix_timeout_ms);

   esp_sleep_enable_ext0_wakeup(UPLOAD_PIN, 0);

   esp_sleep_wakeup_cause_t wake_cause = esp_sleep_get_wakeup_cause();

   if (wake_cause == ESP_SLEEP_WAKEUP_TIMER) {
       gps_tracker_run();
       if (upload_enabled) {
          screen_display_log("WIFI:...");
        if (connect_wifi() == WIFI_SUCCESS) {
            gps_tracker_upload();
        } else {
            screen_display_log("CONN:!!!");

            ESP_LOGE("MAIN", "Wi-Fi connection failed, cannot upload");
        }

           upload_enabled = false;
       }
   } else if (wake_cause == ESP_SLEEP_WAKEUP_EXT0) {
       upload_enabled = true;  //upload button pressed
       printf("Will upload coordinates in the next cycle");
   }

    int sleep_time_ms = app_config.esp_sleep_time_ms;
    uint64_t sleep_time_us = (uint64_t)sleep_time_ms * 1000;

   esp_sleep_enable_timer_wakeup((uint64_t)sleep_time_us);
   esp_deep_sleep_start();
  }


