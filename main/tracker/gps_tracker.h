#ifndef GPS_TRACKER_H
#define GPS_TRACKER_H

#define GPS_TIMEOUT_MS 60000 // gps timeout, tries to find a fix within a minute

void gps_tracker_run();  // Starts fix + tracking + SD logging

#endif
