#ifndef GPS_UART_H
#define GPS_UART_H

#include <stdio.h>
#include <string.h>
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define UART_NUM        UART_NUM_2
#define TX_PIN          17
#define RX_PIN          16
#define BUF_SIZE        1024

void init_uart();
int read_nmea_sentence(char *nmea_sentence, size_t max_length);
void wait_for_gps_fix();
int wait_for_fix_and_get_nmea(char *output_sentence, size_t max_len, uint32_t timeout_ms);
bool sentence_has_fix(const char *sentence);
void gps_uart_loop();

#endif
