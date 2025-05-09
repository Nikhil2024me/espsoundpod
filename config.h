// ESP32 Soundpod - Configuration File
// Modified from original Arduino Soundpod project

#ifndef CONFIG_H
#define CONFIG_H

// Device Information
#define DEVICE_NAME "ESP32 Soundpod"
#define FIRMWARE_VERSION "1.0.0"

// Display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1       // Reset pin (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // I2C address for most SSD1306 displays

// MP3 Player settings
#define MAX_VOLUME 30
#define DEFAULT_VOLUME 15
#define VOLUME_STEP 2

// Battery settings for ESP32 ADC
#define BATTERY_MIN_VOLTAGE 3.2 // Minimum battery voltage
#define BATTERY_MAX_VOLTAGE 4.2 // Full battery voltage
#define BATTERY_READ_INTERVAL 60000 // Read battery every 60 seconds

// Power management
#define SLEEP_TIMEOUT 300000 // Sleep after 5 minutes of inactivity
#define DEEP_SLEEP_TIMEOUT 1800000 // Deep sleep after 30 minutes

// ESP32 specific power settings
#define CPU_FREQ_MHZ_ACTIVE 240  // Full speed when active
#define CPU_FREQ_MHZ_IDLE 80     // Lower speed when idle

// Storage settings
#define MAX_FILENAME_LENGTH 64
#define MAX_TRACKS 100

// ESP32 SPIFFS settings
#define CONFIG_FILE "/config.txt"
#define PLAYLIST_FILE "/playlist.txt"
#define LAST_STATE_FILE "/state.txt"

// Debug settings
#define DEBUG true // Set to false to disable Serial debugging

#endif // CONFIG_H