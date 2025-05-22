# INTRODUCING ZOBBY TRACKER

**Live Tracking the Journey from Malta to Kazakhstan**  
Website: [https://zob2osh.live](https://zob2osh.live)  
Instagram: [@zob_to_osh](https://instagram.com/zob_to_osh)

![ZOB2OSH Logo](https://zob2osh.live/logo.png) <!-- Replace with the actual logo URL -->

---

## 🚗 Project Description

This project powers the live GPS tracking for the **ZOB to OSH** road trip — an overland journey from **Malta** to **Kazakhstan**, traversing Europe, Turkey, and Central Asia. 

A custom **ESP32-based GNSS tracker** records location data and uploads it to a secure server. The public can follow the trip in real-time through a web-based map.

---

## Features

- ✅ GNSS data logging: timestamp, latitude, longitude, altitude, satellite type
- 💾 Local data storage on SD card (binary format)
- 🔒 Secure Wi-Fi upload via HTTPS
- 🔘 Manual upload trigger via button
- 💤 Ultra low power usage with deep sleep mode
- 🗺️ Web frontend to visualise live location

---

## 🧰 Hardware Components

| Component              | Description                                           |
|-----------------------|-------------------------------------------------------|
| **ESP32 Dev Board**    | Microcontroller with Wi-Fi and deep sleep support     |
| **u-blox NEO-M8N**     | High-accuracy GNSS module (GPS/GLONASS/Galileo)       |
| **GPS Antenna**        | Active external antenna for better satellite lock     |
| **SD Card Module**     | Stores GNSS records in binary format                  |
| **18650 Li-ion Battery** | Long-lasting power source                           |
| **TP4056 Charger**     | Battery charger with protection and USB input         |
| **Push Button**        | Triggers upload manually or wakes device              |

---

## 🛠️ Software Overview

### ESP32 Firmware
- Written in C using FreeRTOS
- Parses NMEA sentences (`$GNRMC`, `$GPGGA`)
- Structures records as `gnss_record_t`
- Uploads via Wi-Fi if available
- Sleeps between fixes to conserve power

### Python HTTPS Server
- Authenticated POST endpoint for GNSS data from ESP32
- Stores data in SQLite database
- API serves latest coordinates to frontend

### Web Frontend
- Built with Angular
- Fetches latest GNSS record from the server
- Displays current location on an interactive map
