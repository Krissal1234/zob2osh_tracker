#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>  // <-- Add this

config_t app_config;  // global instance

bool load_config_from_file(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return false;

    char line[128];
    while (fgets(line, sizeof(line), f)) {
        char *key = strtok(line, "=");
        char *val = strtok(NULL, "\n");
        if (!key || !val) continue;

        if (strcmp(key, "WIFI_SSID") == 0) strncpy(app_config.wifi_ssid, val, sizeof(app_config.wifi_ssid));
        else if (strcmp(key, "WIFI_PASSWORD") == 0) strncpy(app_config.wifi_password, val, sizeof(app_config.wifi_password));
        else if (strcmp(key, "SERVER_URL") == 0) strncpy(app_config.server_url, val, sizeof(app_config.server_url));
        else if (strcmp(key, "GPS_TIMEOUT") == 0) app_config.gps_timeout = atoi(val);
        else if (strcmp(key, "UPLOAD_RETRY") == 0) app_config.upload_retry = atoi(val);
    }

    fclose(f);
    return true;
}
