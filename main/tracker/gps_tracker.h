#ifndef GNSS_RECORD_H
#define GNSS_RECORD_H

#include <stdint.h>
#include "config.h"
#define GPS_TIMEOUT_MS app_config.gps_timeout

typedef struct __attribute__((__packed__)) {
    uint32_t timestamp;
    float latitude;
    float longitude;
    float altitude;
    uint8_t fix_quality;
} gnss_record_t;

#define GNSS_RECORD_SIZE sizeof(gnss_record_t)
void gps_tracker_run();
void gps_tracker_upload();
#endif // GNSS_RECORD_H
