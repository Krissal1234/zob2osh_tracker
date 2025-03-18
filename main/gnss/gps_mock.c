#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// Mock GPS structure
typedef struct {
    double latitude;
    double longitude;
    double altitude;
    double speed;
    double course;
    char time[7];  // HHMMSS format for time
} MockGPS;

int random_range(int min, int max) {
    return (rand() % (max - min + 1)) + min;
}

void generate_time(char* time_buffer) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    snprintf(time_buffer, 7, "%02d%02d%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
}

// Function to generate GGA sentence (Global Positioning System Fix Data)
void generate_gga_sentence(MockGPS* gps, char* sentence) {
    snprintf(sentence, 100,
             "$GPGGA,%s,%.4f,N,%.4f,W,%d,08,0.9,%.1f,M,,M,,",
             gps->time, gps->latitude, gps->longitude,
             random_range(1, 2), gps->altitude);
}

// Function to generate RMC sentence (Recommended Minimum Navigation Information)
void generate_rmc_sentence(MockGPS* gps, char* sentence) {
    snprintf(sentence, 100,
             "$GPRMC,%s,A,%.4f,N,%.4f,W,%.1f,%.1f,%s,,A",
             gps->time, gps->latitude, gps->longitude,
             gps->speed, gps->course, gps->time);
}

void get_mock_gps_data(MockGPS* gps, char* sentence) {
    generate_time(gps->time);

    if (random_range(0, 1) == 0) {
        generate_gga_sentence(gps, sentence);
    } else {
        generate_rmc_sentence(gps, sentence);
    }
}

int main() {
    srand(time(NULL));  // Seed for random number generation

    MockGPS gps = {37.7749, -122.4194, 50.0, 5.0, 180.0};  // Default values (e.g., San Francisco)
    char nmea_sentence[100];

    // Simulate storing 5 NMEA sentences to the SD card
    for (int i = 0; i < 5; i++) {
        get_mock_gps_data(&gps, nmea_sentence);
        printf("Writing NMEA sentence: %s\n", nmea_sentence);
        write_to_sd_card(nmea_sentence);  // Store the sentence on the SD card
        sleep(1);  // Simulate a 1-second delay between sentences
    }

    return 0;
}
