#include "wifi_manager.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "esp_http_client.h"
#include <inttypes.h>


/** GLOBAL VARIABLES **/
static EventGroupHandle_t wifi_event_group;
static int s_retry_num = 0;
static const char *TAG = "WIFI_UPLOAD";

#define UPLOAD_SERVER_URL "http://192.168.0.101:8080"

bool upload_gnss_record(const gnss_record_t *record) {
    if (!record) {
        ESP_LOGE(TAG, "Null GNSS record passed to upload");
        return false;
    }

    esp_http_client_config_t config = {
        .url = UPLOAD_SERVER_URL,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 5000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_set_header(client, "Content-Type", "application/octet-stream");
    esp_http_client_set_post_field(client, (const char *)record, GNSS_RECORD_SIZE);

    esp_err_t err = esp_http_client_perform(client);
    esp_http_client_cleanup(client);

    if (err == ESP_OK) {
        int status = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "POST success: HTTP %d", status);
        return status == 200;
    } else {
        ESP_LOGE(TAG, "POST failed: %s", esp_err_to_name(err));
        return false;
    }
}
bool upload_gnss_batch(const gnss_record_t *records, size_t count) {
    if (!records || count == 0) {
        ESP_LOGE(TAG, "Invalid batch upload parameters");
        return false;
    }

    connect_wifi();  // Ensure we're connected

    esp_http_client_config_t config = {
        .url = UPLOAD_SERVER_URL,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 10000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/octet-stream");

    size_t total_size = count * GNSS_RECORD_SIZE;
    esp_http_client_set_post_field(client, (const char *)records, total_size);

    esp_err_t err = esp_http_client_perform(client);
    esp_http_client_cleanup(client);

    if (err == ESP_OK) {
        int status = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "Batch POST success: HTTP %d", status);
        return status == 200;
    } else {
        ESP_LOGE(TAG, "Batch POST failed: %s", esp_err_to_name(err));
        return false;
    }
}

// Configurable network parameters

/** FUNCTION DEFINITIONS **/
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        ESP_LOGI(TAG, "Connecting to AP...");
        vTaskDelay(pdMS_TO_TICKS(3000));
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < MAX_FAILURES)
        {
            ESP_LOGI(TAG, "Reconnecting to AP...");
            esp_wifi_connect();
            s_retry_num++;
        }
        else
        {
            xEventGroupSetBits(wifi_event_group, WIFI_FAILURE);
        }
    }
}

static void ip_event_handler(void* arg, esp_event_base_t event_base,
                             int32_t event_id, void* event_data)
{
    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "STA IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_SUCCESS);
    }
}

static bool wifi_initialized = false;

esp_err_t connect_wifi()
{
    esp_err_t ret;

    if (!wifi_initialized) {
        // Init NVS only once
        ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
        }
        ESP_ERROR_CHECK(ret);

        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        esp_netif_create_default_wifi_sta();

        wifi_initialized = true;  // mark initialized
    }

    // Delay to allow proper stack readiness after deep sleep
    vTaskDelay(pdMS_TO_TICKS(300));

    wifi_event_group = xEventGroupCreate();
    esp_event_handler_instance_t wifi_handler_event_instance;
    esp_event_handler_instance_t got_ip_event_instance;

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &wifi_handler_event_instance));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &ip_event_handler,
                                                        NULL,
                                                        &got_ip_event_instance));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = { 0 };  // Zero-init

    strncpy((char *)wifi_config.sta.ssid, WIFI_SSID, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, WIFI_PASSWORD, sizeof(wifi_config.sta.password));

    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "STA initialization complete");

    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
                                           WIFI_SUCCESS | WIFI_FAILURE,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    esp_err_t status = WIFI_FAILURE;

    if (bits & WIFI_SUCCESS) {
        ESP_LOGI(TAG, "Connected to AP");
        status = WIFI_SUCCESS;
    } else {
        ESP_LOGE(TAG, "Failed to connect to AP");
    }

    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, got_ip_event_instance));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_handler_event_instance));
    vEventGroupDelete(wifi_event_group);

    return status;
}


esp_err_t connect_tcp_server(void)
{
    struct sockaddr_in serverInfo = {0};
    char readBuffer[1024] = {0};

    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = inet_addr(TCP_SERVER_IP);
    serverInfo.sin_port = htons(TCP_SERVER_PORT);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        ESP_LOGE(TAG, "Failed to create a socket");
        return TCP_FAILURE;
    }

    if (connect(sock, (struct sockaddr *)&serverInfo, sizeof(serverInfo)) != 0)
    {
        ESP_LOGE(TAG, "Failed to connect to server");
        close(sock);
        return TCP_FAILURE;
    }

    ESP_LOGI(TAG, "Connected to TCP server");
    bzero(readBuffer, sizeof(readBuffer));
    int r = read(sock, readBuffer, sizeof(readBuffer) - 1);
    for (int i = 0; i < r; i++) {
        putchar(readBuffer[i]);
    }

    if (strcmp(readBuffer, "HELLO") == 0)
    {
        ESP_LOGI(TAG, "Received HELLO message");
    }

    return TCP_SUCCESS;
}
