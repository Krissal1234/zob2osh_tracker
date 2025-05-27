#ifndef GNSS_RECORD_H
#define GNSS_RECORD_H

#include <stdint.h>
#include "config.h"
#define GPS_TIMEOUT_MS app_config.search_for_fix_timeout_ms

typedef struct __attribute__((__packed__)) {
    uint32_t timestamp;     // HHMMSS
    uint32_t date;          // DDMMYY or 0 if not available
    float latitude;
    float longitude;
    float altitude;
    uint8_t sentence_type;  // 1 = GGA, 2 = RMC
} gnss_record_t;



void gps_tracker_run();
void gps_tracker_upload();
#endif // GNSS_RECORD_H
