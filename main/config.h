#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>  // <-- Add this

typedef struct {
    char wifi_ssid[64];
    char wifi_password[64];
    char server_url[128];
    int gps_timeout;
    int upload_retry;
    char server_username[64];
    char server_password[64];
} config_t;

extern config_t app_config;

bool load_config_from_file(const char *path);

#endif
