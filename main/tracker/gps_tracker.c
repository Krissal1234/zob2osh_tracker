#include "gps_tracker.h"
#include "../gnss/gps_uart.h"
#include "../sd_card/sd_logger.h"
#include <stdio.h>
#include "esp_log.h"
#include <string.h>
#include "../network/wifi_manager.h"  // Adjust path if needed
#include "../tracker/gps_tracker.h"      // Struct + GNSS_RECORD_SIZE
#include <inttypes.h>  // Needed for PRIu32
static const char *TAG = "GPS_TRACker";
#define BATCH_SIZE 10

void gps_tracker_upload() {
    FILE *f = fopen(GNSS_LOG_PATH, "rb");
    if (!f) {
        ESP_LOGE(TAG, "Failed to open GNSS log file");
        return;
    }
    size_t offset = read_upload_offset();

    if (fseek(f, offset, SEEK_SET) != 0) {
        ESP_LOGE(TAG, "Failed to seek in file");
        fclose(f);
        return;
    }
    ESP_LOGI(TAG, "Reading GNSS log from offset: %zu (record size: %d)", offset, sizeof(gnss_record_t));

    if (offset % sizeof(gnss_record_t) != 0) {
        ESP_LOGE(TAG, "Invalid offset: %zu (not aligned to record size)", offset);
        return;

    }

    gnss_record_t records[BATCH_SIZE];
    size_t records_read;
    size_t current_offset = offset;
    bool uploaded_any = false;


    while ((records_read = fread(records, sizeof(gnss_record_t), BATCH_SIZE, f)) > 0) {
        for (size_t i = 0; i < records_read; ++i) {
            gnss_record_t *r = &records[i];
            ESP_LOGI(TAG, "Read Record %zu: ts=%" PRIu32 ", date=%" PRIu32 ", lat=%f, lon=%f, alt=%f, type=%d",
                     i, r->timestamp, r->date, r->latitude, r->longitude, r->altitude, r->sentence_type);
        }

        if (upload_gnss_batch(records, records_read)) {
            current_offset += records_read * sizeof(gnss_record_t);
            uploaded_any = true;
            ESP_LOGI(TAG, "Uploaded %zu records. New offset: %zu", records_read, current_offset);
        } else {
            ESP_LOGW(TAG, "Batch upload failed. Stopping at offset: %zu", current_offset);
            break;
        }
    }

    fclose(f);

    if (uploaded_any && current_offset != offset) {
        save_upload_offset(current_offset);
        ESP_LOGI(TAG, "Saved new upload offset: %zu", current_offset);
    } else {
        ESP_LOGI(TAG, "No new data uploaded. Offset unchanged.");
    }
}

void gps_tracker_run() {
    char fix_sentence[128];

    printf(" Waiting for GPS fix...\n");

    //testing purposes
    // char fix_sentence[128] = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
    // printf("GPS fix acquired (TEST): %s\n", fix_sentence);

    // log_gnss_data_struct(fix_sentence);

    if (!wait_for_fix_and_get_nmea(fix_sentence, sizeof(fix_sentence), GPS_TIMEOUT_MS)) {
        printf("GPS fix not acquired within timeout.\n");
        return;
    }else{
        printf("GPS fix acquired: %s\n", fix_sentence);
        log_gnss_data_struct(fix_sentence);
    }
}
