idf_component_register(
    SRCS
        "main.c"
        "config.c"
        "sd_card/sd_logger.c"
        "network/wifi_manager.c"
        "gnss/gps_uart.c"
        "tracker/gps_tracker.c"
        "tls_cert/root_cert.c"
        "screen_display/screen_display.c"
        "ssd1306_driver/ssd1306.c"
        "ssd1306_driver/ssd_1306_i2c_new.c"  # <-- Add this
        INCLUDE_DIRS "." "screen_display"

)
