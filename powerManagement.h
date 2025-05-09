// ESP32 Soundpod - Power Management
// Modified from original Arduino Soundpod project, optimized for ESP32

#ifndef POWERMANAGEMENT_H
#define POWERMANAGEMENT_H

#include <Arduino.h>
#include <esp_sleep.h>
#include <esp_pm.h>
#include "config.h"

// External references
extern void setBatteryPercentage(int percentage);
extern void stopPlayback();
extern void savePlaybackState(int track, int volume, bool playing);
extern int currentTrack;
extern int currentVolume;
extern bool isPlaying;

// Power management variables
unsigned long lastActivityTime = 0;
unsigned long lastBatteryCheckTime = 0;
int batteryPercentage = 100;
bool lowPowerMode = false;
bool batteryLow = false;

// ESP32 specific power management
esp_pm_config_esp32_t pm_config;

// Initialize power management
void initPowerManagement() {
  // Configure ADC for battery monitoring
  pinMode(BATTERY_LEVEL_PIN, INPUT);
  analogReadResolution(12); // ESP32 has 12-bit ADC
  
  // Set up ESP32 power management if dynamic frequency scaling is desired
  pm_config.max_freq_mhz = CPU_FREQ_MHZ_ACTIVE;
  pm_config.min_freq_mhz = CPU_FREQ_MHZ_IDLE;
  pm_config.light_sleep_enable = true;
  
  esp_pm_configure(&pm_config);
  
  Serial.println("Power management initialized");
  
  // Initial battery check
  checkBatteryLevel();
}

// Record activity to prevent sleep
void recordActivity() {
  lastActivityTime = millis();
  
  // If in low power mode, exit it
  if (lowPowerMode) {
    lowPowerMode = false;
    // Set CPU to full speed
    pm_config.max_freq_mhz = CPU_FREQ_MHZ_ACTIVE;
    esp_pm_configure(&pm_config);
    Serial.println("Exiting low power mode");
  }
}

// Check battery level
void checkBatteryLevel() {
  // Only check battery periodically to save power
  if (millis() - lastBatteryCheckTime < BATTERY_READ_INTERVAL) {
    return;
  }
  
  lastBatteryCheckTime = millis();
  
  // Read battery voltage
  int rawValue = analogRead(BATTERY_LEVEL_PIN);
  float voltage = rawValue * (3.3 / 4095.0) * 2; // Assuming voltage divider
  
  // Convert to percentage
  float percentage = (voltage - BATTERY_MIN_VOLTAGE) / 
                    (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE) * 100.0;
  
  // Constrain to valid range
  percentage = constrain(percentage, 0, 100);
  
  // Update battery percentage
  batteryPercentage = (int)percentage;
  setBatteryPercentage(batteryPercentage);
  
  Serial.print("Battery: ");
  Serial.print(batteryPercentage);
  Serial.print("% (");
  Serial.print(voltage);
  Serial.println("V)");
  
  // Check for low battery
  if (batteryPercentage <= 10 && !batteryLow) {
    batteryLow = true;
    handleLowBattery();
  } else if (batteryPercentage > 15) {
    batteryLow = false;
  }
}

// Handle low battery condition
void handleLowBattery() {
  Serial.println("WARNING: Low battery!");
  
  // Save state before potential shutdown
  savePlaybackState(currentTrack, currentVolume, isPlaying);
  
  // If battery is critically low, enter deep sleep
  if (batteryPercentage <= 5) {
    Serial.println("CRITICAL: Battery critically low, entering deep sleep");
    
    // Stop playback to reduce power consumption
    stopPlayback();
    
    // Wait for serial output to complete
    delay(500);
    
    // Enter deep sleep
    esp_deep_sleep_start();
  }
}

// Check if device should enter sleep mode
void checkPowerStatus() {
  // Check battery level
  checkBatteryLevel();
  
  // Check for inactivity
  unsigned long inactiveTime = millis() - lastActivityTime;
  
  // If inactive for too long, enter low power mode
  if (!lowPowerMode && inactiveTime > SLEEP_TIMEOUT) {
    enterLowPowerMode();
  }
  
  // If inactive for extended period, enter deep sleep
  if (inactiveTime > DEEP_SLEEP_TIMEOUT) {
    enterDeepSleep();
  }
}

// Enter low power mode
void enterLowPowerMode() {
  Serial.println("Entering low power mode");
  lowPowerMode = true;
  
  // Reduce CPU frequency
  pm_config.max_freq_mhz = CPU_FREQ_MHZ_IDLE;
  esp_pm_configure(&pm_config);
  
  // Dim display or other power-saving measures could be added here
}

// Enter deep sleep mode
void enterDeepSleep() {
  Serial.println("Entering deep sleep mode");
  
  // Save current state
  savePlaybackState(currentTrack, currentVolume, isPlaying);
  
  // Stop playback
  stopPlayback();
  
  // Configure wake-up sources for ESP32
  // Configure button GPIOs as wake sources
  esp_sleep_enable_ext0_wakeup(BUTTON_PLAY_PIN, LOW); // Any button press can wake
  
  // Additional wake sources could be added here
  
  // Wait for serial output to complete
  delay(500);
  
  // Enter deep sleep
  esp_deep_sleep_start();
  
  // Code after this will not run until device wakes from sleep
}

// Wake from sleep
void handleWakeUp() {
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  
  Serial.print("Wakeup caused by: ");
  switch(wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:
      Serial.println("Button press");
      break;
    case ESP_SLEEP_WAKEUP_TIMER:
      Serial.println("Timer");
      break;
    default:
      Serial.println("Other reason");
      break;
  }
  
  // Record activity to prevent immediate sleep
  recordActivity();
}

#endif // POWERMANAGEMENT_H