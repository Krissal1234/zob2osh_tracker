#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

config_t app_config;

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
        else if (strcmp(key, "SEARCH_FOR_FIX_TIMEOUT_MS") == 0) app_config.search_for_fix_timeout_ms = atoi(val);
        else if (strcmp(key, "UPLOAD_RETRY") == 0) app_config.upload_retry = atoi(val);
        else if (strcmp(key, "SERVER_USERNAME") == 0) strncpy(app_config.server_username, val, sizeof(app_config.server_username));
        else if (strcmp(key, "SERVER_PASSWORD") == 0) strncpy(app_config.server_password, val, sizeof(app_config.server_password));
        else if (strcmp(key, "ESP_SLEEP_TIME_MS") == 0) app_config.esp_sleep_time_ms = atoi(val);
    }

    fclose(f);
    return true;
}
