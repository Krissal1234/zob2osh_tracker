#ifndef GPS_UART_H
#define GPS_UART_H

#include <stdio.h>
#include <string.h>
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// UART Configuration
#define UART_NUM        UART_NUM_2   // Using UART2 for GPS
#define TX_PIN          17           // TX Pin (not used, GPS transmits)
#define RX_PIN          16           // RX Pin (connect to GPS TX)
#define BUF_SIZE        1024         // Buffer size for incoming data

// Function prototypes
void init_uart();
int read_nmea_sentence(char *nmea_sentence, size_t max_length);
void wait_for_gps_fix();
#endif // GPS_UART_H
