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

static const char *TAG = "SD_LOGGER";

sdmmc_card_t *card;

static esp_err_t s_write_file(const char *path, char *data){
    ESP_LOGI(TAG, "Opening file %s", path);
    FILE *f = fopen(path, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }
    fprintf(f, data);
    fclose(f);
    ESP_LOGI(TAG, "File written");

    return ESP_OK;
}

static esp_err_t s_read_file(const char *path)
{
    ESP_LOGI(TAG, "Reading file %s", path);
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }
    char line[EXAMPLE_MAX_CHAR_SIZE];
    fgets(line, sizeof(line), f);
    fclose(f);

    // strip newline
    char *pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(TAG, "Read from file: '%s'", line);

    return ESP_OK;
}
void test_write_file(void){
  const char *file_hello = MOUNT_POINT "/hello.txt";
  char data[EXAMPLE_MAX_CHAR_SIZE];

  snprintf(data, EXAMPLE_MAX_CHAR_SIZE, "%s %s!\n", "Hello", card->cid.name);

  esp_err_t ret = s_write_file(file_hello, data);  // Declare ret here
  if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to write file");
      return;
  }
}

void test_read_file(void){
  const char *file_foo = MOUNT_POINT "/hello.txt";

  esp_err_t ret = s_read_file(file_foo);  // Declare ret here
  if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to read file");
      return;
  }
}


bool sd_logger_init(void){
  ESP_LOGI(TAG,"Initialising SD Card...");

  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
    .format_if_mount_failed = false,
    .max_files=2,
    .allocation_unit_size = 16 * 1024
  };
  sdmmc_host_t host = SDSPI_HOST_DEFAULT();
  host.max_freq_khz = 5000;

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


