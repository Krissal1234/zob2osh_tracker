#include "gps_tracker.h"
#include "../gnss/gps_uart.h"
#include "../sd_card/sd_logger.h"
#include <stdio.h>
#include <string.h>


void gps_tracker_run() {
    char fix_sentence[128];

    printf(" Waiting for GPS fix...\n");

    if (!wait_for_fix_and_get_nmea(fix_sentence, sizeof(fix_sentence), GPS_TIMEOUT_MS)) {
        printf("GPS fix not acquired within timeout.\n");
        return;
    }else{
        printf("GPS fix acquired: %s\n", fix_sentence);
        // log_gnss_data(fix_sentence, strlen(fix_sentence));
    }
}
