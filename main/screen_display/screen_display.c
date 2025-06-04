#include "ssd1306_driver/ssd1306.h"
#include "screen_display.h"
#include <string.h>
#include <stdio.h>

#define SDA_PIN 21
#define SCL_PIN 22
#define RESET_PIN -1 // Set to -1 if not used

static SSD1306_t oled_dev;
static char battery_line[11] = {0};


void screen_display_init(void) {
    oled_dev._flip = false;
    i2c_master_init(&oled_dev, SDA_PIN, SCL_PIN, RESET_PIN);
    ssd1306_init(&oled_dev, 64, 48);

    // Initialize display
    ssd1306_clear_screen(&oled_dev, false);
    snprintf(battery_line, sizeof(battery_line), "Batt: --%%");
    ssd1306_display_text(&oled_dev, 0, battery_line, strlen(battery_line), false);
    ssd1306_show_buffer(&oled_dev);
}

void screen_display_battery_status(int battery_percent) {
    snprintf(battery_line, sizeof(battery_line), "Batt:%3d%%", battery_percent);
    ssd1306_display_text(&oled_dev, 0, battery_line, strlen(battery_line), false);
    ssd1306_show_buffer(&oled_dev);
}

void screen_display_log(const char *text) {
    ssd1306_display_text(&oled_dev, 1, "        ", 8, false);  // Clear previous log line
    ssd1306_display_text(&oled_dev, 1, text, strlen(text), false);
    ssd1306_show_buffer(&oled_dev);
}
