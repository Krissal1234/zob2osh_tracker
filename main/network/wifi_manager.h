#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_err.h"
#include "../tracker/gps_tracker.h"
#include "config.h"

#define WIFI_SUCCESS (1 << 0)
#define WIFI_FAILURE (1 << 1)
#define TCP_SUCCESS (1 << 0)
#define TCP_FAILURE (1 << 1)
#define MAX_FAILURES 10

static const char *WIFI_SSID = app_config.wifi_ssid;
static const char *WIFI_PASSWORD = app_config.wifi_password;
static const char *TCP_SERVER_IP = app_config.server_url;
static const char *SERVER_USERNAME = app_config.server_username;
static const char *SERVER_PASSWORD = app_config.server_password;

esp_err_t connect_wifi(void);
esp_err_t connect_tcp_server(void);

bool upload_gnss_batch(const gnss_record_t *records, size_t count);
bool upload_gnss_record(const gnss_record_t *record);


#endif
