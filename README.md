# ZOBBY TRACKER

<div align="center">
  <p><b>Live Tracking the Journey from Malta to Kazakhstan</b></p>
  <p>
    Website: <a href="https://zob2osh.live">https://zob2osh.live</a> — <i>No longer active</i><br>
    Instagram: <a href="https://instagram.com/zob_to_osh">@zob_to_osh</a>
  </p>
  <img src="https://github.com/user-attachments/assets/902cf211-ed3a-4b00-ae93-d0f9a3fd30e4" width="300" alt="zob_to_osh" />
</div>

---

## Update: The Road Trip was a Success!

<div align="center">
  <img src="https://github.com/user-attachments/assets/1a63518b-9b90-4e53-80a6-2f3348fe9a28" width="550" alt="Full Route Map" />
  <p><i>The map above displays the raw GPX data captured directly by the Zobby Tracker.</i></p>
</div>

The map above displays the raw GPX data captured directly by the Zobby Tracker during our journey from Malta to Kazakhstan. Spanning thousands of kilometres of unpredictable terrain, the tracker went through extreme vibrations and harsh conditions.

Despite the physical beatings it took, the system **survived the entire trip with zero software issues**, consistently logging data across every border.

---

## Project Description

This project powers the live GPS tracking for the **ZOB to OSH** road trip — an overland journey from **Malta** to **Kazakhstan**, traversing Europe, Turkey, and Central Asia. 

A custom **ESP32-based GNSS tracker** records location data and uploads it to a secure server. The public was able to follow the trip in real-time through a web-based map.

---

## Hardware Components

| Component | Description |
| :--- | :--- |
| **ESP32 Dev Board** | Microcontroller with Wi-Fi and deep sleep support |
| **u-blox NEO-M8N** | High-accuracy GNSS module (GPS/GLONASS/Galileo) |
| **GPS Antenna** | Active external antenna for better satellite lock |
| **SD Card Module** | Stores GNSS records in binary format |
| **18650 Li-ion Battery** | Long-lasting power source |
| **TP4056 Charger** | Battery charger with protection and USB input |
| **Push Button** | Triggers upload manually or wakes device |

---
## Technical Deep Dive

### 1. Robust Persistence: Tracking with Offset Management
A critical feature of the firmware is its handling of Cold Storage on the SD card, ensuring zero data loss during extended periods without Wi-FI in the more remote countries where we had no internet access.


* **Persistent Cursors**: The tracker maintains an `UPLOAD_OFFSET_PATH` file that stores exactly where the last successful upload stopped.
* **Resume Capability**: On boot or wake, the system calls `read_upload_offset()`. If a batch upload fails due to poor connectivity, the cursor is not updated, ensuring data is never skipped and is instead retried during the next connection window.
* **Offset Alignment**: The parser performs safety checks to ensure the offset is always aligned to the size of a `gnss_record_t`, preventing file corruption or partial record reads.

### 2. Data Serialisation 
Instead of using bulky CSV or JSON files, the firmware writes raw C structs directly to the SD card using `fwrite`. This reduces the storage footprint, minimises CPU overhead, and extends SD card lifespan through reduced write amplification.

```cpp
/**
 * Optimized 21-byte Binary Record
 * No padding, packed for storage efficiency
 */
typedef struct __attribute__((__packed__)) {
    uint32_t timestamp;     // HHMMSS
    uint32_t date;          // DDMMYY or 0 if not available
    float latitude;
    float longitude;
    float altitude;
    uint8_t sentence_type;  // 1 = GGA, 2 = RMC
} gnss_record_t;
```

---

## Software Overview

### ESP32 Firmware
* Written in **C** using **FreeRTOS**.
* Parses NMEA sentences (`$GNRMC`, `$GPGGA`).
* Stores data locally and uploads via Wi-Fi when a connection is available.
* Implements deep sleep cycles between location fixes to maximize battery life.

### Python HTTPS Server
* Authenticated POST endpoint to receive data from the ESP32.
* Stores telemetry in a **SQLite** database.
* Provides a REST API for the frontend to fetch coordinates.

### Web Frontend
* Built with **Angular**.
* Displays the live "Last Seen" location and the historical path on an interactive map.

---

## Website Gallery

While the website is no longer running due to financial reasons, here are snapshots of the live interface that users interacted with during the adventure.

<div align="center">
  <table border="0">
    <tr>
    <!--  <td align="center">
        <img src="https://github.com/user-attachments/assets/57f5c531-9f93-4a18-9759-4672e12e7534" width="400" />
        <br /><i>Desktop Tracking Interface</i>
      </td> -->
      <td align="center">
        <img src="https://github.com/user-attachments/assets/bc539662-246b-4393-abbc-0d424a11c5ff" width="400" />
        <br /><i>Mobile View</i>
      </td>
    </tr>
  </table>
</div>


---
# RoadTrip Gallery

What a journey this road trip was... so many friends made along the way, funny interactions, and a few upset stomachs. Here are some highlights from the road.

<div align="center">
  <table border="0">
    <tr>
      <td align="center" valign="bottom">
        <img src="https://github.com/user-attachments/assets/cf705271-dd9a-47ac-90c6-a38d0a4ddd17" width="450" />
        <br /><i>The Gate of Hell, Turkmenistan</i>
      </td>
      <td align="center" valign="bottom">
        <img src="https://github.com/user-attachments/assets/06578bd8-ccca-4bd3-b11f-2803e6f7abff" width="340" />
        <br /><i>Zobby during development</i>
      </td>
    </tr>
  </table>

  <br />

  <table border="0">
    <tr>
      <td align="center" valign="top">
        <img src="https://github.com/user-attachments/assets/44fe37b6-e81c-4791-9d31-91a9d15040af" width="260" />
        <br /><i>Azerbaijan Mud Volcanoes</i>
      </td>
      <td align="center" valign="top">
        <img src="https://github.com/user-attachments/assets/3004ccb8-16d8-44b4-8b5f-399dc29f799a" width="260" />
        <br /><i>Zobby getting comfortable</i>
      </td>
      <td align="center" valign="top">
        <img src="https://github.com/user-attachments/assets/00581c1e-d286-4697-9cdc-fa769858710d" width="260" />
        <br /><i>Minor fix on the road</i>
      </td>
    </tr>
  </table>
</div>
