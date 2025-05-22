#include "gps_uart.h"
#include <stdio.h>
#include <string.h>
#include "gps_uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"


#define MAX_WAIT_TIME 1000  // Max wait time (seconds) for a GPS fix

void init_uart() {
    uart_config_t uart_config = {
        .baud_rate = 9600,  // Set baud rate to match GPS (typically 9600 or 115200)
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
}

int wait_for_fix_and_get_nmea(char *output_sentence, size_t max_len, uint32_t timeout_ms) {
    int64_t start_time = esp_timer_get_time(); // microseconds
    char nmea_buffer[BUF_SIZE];

    while ((esp_timer_get_time() - start_time) / 1000 < timeout_ms) { //keep checking until timeout
        if (read_nmea_sentence(nmea_buffer, sizeof(nmea_buffer))) {
            printf("%s/n",nmea_buffer);
            if (sentence_has_fix(nmea_buffer)) {
                strncpy(output_sentence, nmea_buffer, max_len);

                return 1; // Got a fix and returned the sentence
            }
        }
    }

    return 0; // Timeout or no valid fix
}
bool sentence_has_fix(const char *sentence) {
    // Handle any GGA-type sentence
    if (strstr(sentence, "GGA")) {
        // Check fix quality field (7th)
        int comma_count = 0;
        const char *p = sentence;
        while (*p && comma_count < 6) {
            if (*p == ',') comma_count++;
            p++;
        }
        return *p != '0' && *p != ','; // Non-zero, non-empty = fix
    }

    // Handle any RMC-type sentence
    if (strstr(sentence, "RMC")) {
        // Check status field (3rd field = 2 commas in)
        int comma_count = 0;
        const char *p = sentence;
        while (*p && comma_count < 2) {
            if (*p == ',') comma_count++;
            p++;
        }
        return *p == 'A'; // A = fix, V = void
    }

    return false; // Not a fix sentence
}



int read_nmea_sentence(char *nmea_sentence, size_t max_length) {
    static char buffer[BUF_SIZE];
    static int buffer_pos = 0;
    char temp_char;

    while (uart_read_bytes(UART_NUM, (uint8_t *)&temp_char, 1, 20 / portTICK_PERIOD_MS) > 0) {
        if (temp_char == '$') {
            buffer_pos = 0; // Start of a new NMEA sentence
        }

        if (buffer_pos < BUF_SIZE - 1) {
            buffer[buffer_pos++] = temp_char;
        }

        // End of NMEA sentence detected (newline)
        if (temp_char == '\n') {
            buffer[buffer_pos] = '\0';  // Null-terminate string
            strncpy(nmea_sentence, buffer, max_length);
            buffer_pos = 0;  // Reset for next sentence
            return 1; // Successfully read a sentence
        }
    }

    return 0; // No valid NMEA sentence received
}

