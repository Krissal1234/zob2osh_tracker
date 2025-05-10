#include "sd_logger.h"
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "driver/spi_common.h"
#include "driver/sdspi_host.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include <errno.h>   // <-- at the top
#include "../tracker/gps_tracker.h"  // Adjust the path based on your structure

static const char *TAG = "SD_LOGGER";

sdmmc_card_t *card;

static float parse_nmea_latlon(const char *field, const char *direction) {
    float value = atof(field);
    int degrees = (int)(value / 100);
    float minutes = value - (degrees * 100);
    float decimal = degrees + (minutes / 60.0f);
    if (direction[0] == 'S' || direction[0] == 'W') {
        decimal *= -1.0f;
    }
    return decimal;
}
size_t read_upload_offset() {
    FILE *f = fopen(UPLOAD_OFFSET_PATH, "rb");
    if (!f) {
        ESP_LOGW(TAG, "Offset file not found, starting from 0");
        return 0;
    }
    size_t offset = 0;
    fread(&offset, sizeof(offset), 1, f);
    fclose(f);
    return offset;
}
void save_upload_offset(size_t offset) {
    FILE *f = fopen(UPLOAD_OFFSET_PATH, "wb");
    if (!f) {
        ESP_LOGE(TAG, "Failed to save upload offset");
        return;
    }
    fwrite(&offset, sizeof(offset), 1, f);
    fclose(f);
}

void log_gnss_data_struct(const char *nmea_sentence) {
    ESP_LOGI(TAG, "Parsing and logging GNSS data: %s", nmea_sentence);

    char sentence_copy[128];
    strncpy(sentence_copy, nmea_sentence, sizeof(sentence_copy));
    sentence_copy[sizeof(sentence_copy) - 1] = '\0';

    char *fields[20] = {0};
    int i = 0;
    char *token = strtok(sentence_copy, ",");
    while (token && i < 20) {
        fields[i++] = token;
        token = strtok(NULL, ",");
    }

    if (i < 7 || (strncmp(fields[0], "$GPGGA", 6) != 0 && strncmp(fields[0], "$GNRMC", 6) != 0)) {
        ESP_LOGW(TAG, "Unsupported or incomplete GNSS sentence: %s", fields[0]);
        return;
    }

    uint32_t timestamp = 0;
    float latitude = 0.0f, longitude = 0.0f, altitude = 0.0f;
    uint8_t fix_quality = 0;

    if (strncmp(fields[0], "$GPGGA", 6) == 0) {
        // Parse GGA
        timestamp = (fields[1] && strlen(fields[1])) ? atoi(fields[1]) : 0;
        latitude = (fields[2] && fields[3]) ? parse_nmea_latlon(fields[2], fields[3]) : 0.0f;
        longitude = (fields[4] && fields[5]) ? parse_nmea_latlon(fields[4], fields[5]) : 0.0f;
        fix_quality = (fields[6] && strlen(fields[6])) ? atoi(fields[6]) : 0;
        altitude = (fields[9] && strlen(fields[9])) ? atof(fields[9]) : 0.0f;
    } else if (strncmp(fields[0], "$GNRMC", 6) == 0) {
        // Parse RMC
        timestamp = (fields[1] && strlen(fields[1])) ? atoi(fields[1]) : 0;
        // Field 2 is status: A=active, V=void
        if (fields[2] && fields[2][0] == 'A') {
            fix_quality = 1;
        } else {
            fix_quality = 0;
        }
        latitude = (fields[3] && fields[4]) ? parse_nmea_latlon(fields[3], fields[4]) : 0.0f;
        longitude = (fields[5] && fields[6]) ? parse_nmea_latlon(fields[5], fields[6]) : 0.0f;
        altitude = 0.0f; // RMC has no altitude
    }

    if (fix_quality == 0) {
        ESP_LOGW(TAG, "No GNSS fix, skipping record");
        return;
    }

    gnss_record_t record = {
        .timestamp = timestamp,
        .latitude = latitude,
        .longitude = longitude,
        .altitude = altitude,
        .fix_quality = fix_quality
    };

    FILE *f = fopen(GNSS_LOG_PATH, "ab");
    if (!f) {
        ESP_LOGE(TAG, "Failed to open GNSS log file");
        return;
    }

    size_t written = fwrite(&record, sizeof(record), 1, f);
    fsync(fileno(f));
    fclose(f);

    if (written != 1) {
        ESP_LOGE(TAG, "Failed to write GNSS record to file");
    } else {
        ESP_LOGI(TAG, "Logged GNSS record (lat=%.6f, lon=%.6f, alt=%.1f)", latitude, longitude, altitude);
    }
}



void log_gnss_data(const char *gnss_string, size_t string_length) {
    ESP_LOGI(TAG, "Logging GNSS data...");

    // save_upload_offset(1034);

    FILE *f = fopen(GNSS_LOG_PATH, "a");
    if (!f) {
        ESP_LOGE(TAG, "Failed to open GNSS log file");
        return;
    }

    size_t start_offset = ftell(f);  // Capture position before writing

    size_t written = fwrite(gnss_string, 1, string_length, f);
    fsync(fileno(f));
    fclose(f);


    if (written != string_length) {
        ESP_LOGE(TAG, "Write error: only %zu of %zu bytes written", written, string_length);
        return;
    }

    ESP_LOGI(TAG, "GNSS data logged (%zu bytes)", written);
    // vTaskDelay(pdMS_TO_TICKS(100));
    // Save initial upload offset if needed
    save_upload_offset(start_offset);
}





bool sd_logger_init(void){
    gpio_reset_pin(SD_MOSI);
    gpio_reset_pin(SD_MISO);
    gpio_reset_pin(SD_SCK);
    gpio_reset_pin(SD_CS);
    gpio_set_pull_mode(SD_MISO, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(SD_MOSI, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(SD_SCK, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(SD_CS, GPIO_PULLUP_ONLY);

  ESP_LOGI(TAG,"Initialising SD Card...");

  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
    .format_if_mount_failed = false,
    .max_files=6,
    .allocation_unit_size = 16 * 1024
  };
  sdmmc_host_t host = SDSPI_HOST_DEFAULT();
  host.max_freq_khz = 20000;

  spi_bus_config_t bus_cfg = {
      .mosi_io_num = SD_MOSI,
      .miso_io_num = SD_MISO,
      .sclk_io_num = SD_SCK,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz = 4096
  };

  // Initialize SPI Bus
  esp_err_t ret = spi_bus_initialize(SPI2_HOST, &bus_cfg, 1);
  if (ret != ESP_OK) {
      ESP_LOGE(TAG, "SPI bus init failed: %s", esp_err_to_name(ret));
      return false;
  }

  sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
  slot_config.gpio_cs = SD_CS;
  slot_config.host_id = SPI2_HOST;
  vTaskDelay(pdMS_TO_TICKS(100));

  // Mount SD Card
  ret = esp_vfs_fat_sdspi_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &card);
  if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to mount SD card: %s", esp_err_to_name(ret));
      return false;
  }
  ESP_LOGI(TAG, "SD card mounted successfully");
  sdmmc_card_print_info(stdout,card);

  return true;
}


