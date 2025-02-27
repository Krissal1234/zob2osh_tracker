#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_err.h"

/** DEFINES **/
#define WIFI_SUCCESS (1 << 0)
#define WIFI_FAILURE (1 << 1)
#define TCP_SUCCESS (1 << 0)
#define TCP_FAILURE (1 << 1)
#define MAX_FAILURES 10

static const char *WIFI_SSID = "kris-hotspot";
static const char *WIFI_PASSWORD = "kris1234";
static const char *TCP_SERVER_IP = "192.168.0.101";
static const uint16_t TCP_SERVER_PORT = 12345;
/** FUNCTION DECLARATIONS **/
esp_err_t connect_wifi(void);
esp_err_t connect_tcp_server(void);

#endif // WIFI_MANAGER_H
