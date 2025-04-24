#ifndef SD_LOGGER_H
#define SD_LOGGER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "driver/sdspi_host.h"
#include "driver/sdmmc_host.h"
#include "sdmmc_cmd.h"
#include "driver/gpio.h"

// Define SD card SPI pins based on your mapping
#define SD_MISO  19  // (D0)
#define SD_MOSI  23  // (CMD)
#define SD_SCK   18  // (CLK)
#define SD_CS    5   // (Chip Select)
#define EXAMPLE_MAX_CHAR_SIZE    64
#define MOUNT_POINT "/sdcard"
// #define NAME_OF_FILE "geolocation.txt"
#define GNSS_LOG_PATH MOUNT_POINT "/gnss_log.bin"
#define UPLOAD_OFFSET_PATH MOUNT_POINT "/upload_offset.bin"

// Function Prototypes
void test_write_file(void);
void test_read_file(void);
void log_gnss_data(const char *gnss_string, size_t string_length);
bool sd_logger_init(void);

#endif // SD_LOGGER_H
