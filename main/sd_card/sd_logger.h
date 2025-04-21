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
#define NAME_OF_FILE "geolocation.txt"
// GPS Data Structure (12 bytes per entry)
// typedef struct {
//     uint32_t timestamp; // 4 bytes
//     float longitude;  // 4 bytes
//     float latitude;   // 4 bytes
// } GPSLogEntry;

// Function Prototypes
void test_write_file(void);
void test_read_file(void);
void log_gnss_data(const char *gnss_string, size_t string_length);
bool sd_logger_init(void);
// bool sd_log_data(const GPSLogEntry *entry);
// uint32_t sd_get_last_byte_offset(void);
// void sd_set_last_byte_offset(uint32_t offset);

#endif // SD_LOGGER_H
