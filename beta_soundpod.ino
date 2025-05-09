#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <DFRobotDFPlayerMini.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "SPIFFS.h"

// Include our custom header files
#include "config.h"
#include "display.h"
#include "mp3Handler.h"
#include "dbHandler.h"
#include "powerManagement.h"

// Define ESP32 specific pins
// These are placeholder values - adjust according to your specific ESP32 wiring
#define DFPLAYER_RX_PIN 16  // Connect to TX on DFPlayer
#define DFPLAYER_TX_PIN 17  // Connect to RX on DFPlayer
#define OLED_SDA_PIN 21     // Default ESP32 SDA
#define OLED_SCL_PIN 22     // Default ESP32 SCL
#define BUTTON_PREV_PIN 25
#define BUTTON_PLAY_PIN 26
#define BUTTON_NEXT_PIN 27
#define BUTTON_VOL_UP_PIN 32
#define BUTTON_VOL_DOWN_PIN 33
#define BATTERY_LEVEL_PIN 34  // ADC pin for battery monitoring

// Create a software serial for DFPlayer communication
HardwareSerial playerSerial(1); // Use UART1 on ESP32

// Variables for button debouncing
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

// Setup function
void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println("ESP32 Soundpod Starting...");
  
  // Initialize MP3 player serial
  playerSerial.begin(9600, SERIAL_8N1, DFPLAYER_RX_PIN, DFPLAYER_TX_PIN);
  
  // Initialize SPIFFS for storage
  if(!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  
  // Use ESP32's built-in LED_BUILTIN if available
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Setup button pins as inputs with pull-up resistors
  pinMode(BUTTON_PREV_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PLAY_PIN, INPUT_PULLUP);
  pinMode(BUTTON_NEXT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_VOL_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_VOL_DOWN_PIN, INPUT_PULLUP);
  
  // Setup battery monitoring
  pinMode(BATTERY_LEVEL_PIN, INPUT);
  
  // Initialize I2C for OLED
  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  
  // Initialize modules
  initDisplay();      // Setup OLED display
  initMP3Player();    // Setup DFPlayer Mini
  initDatabase();     // Setup database/storage
  initPowerManagement(); // Setup power management
  
  // Display welcome message
  displayWelcomeScreen();
  delay(2000);
  
  // Load last played track information
  loadLastPlayState();
  
  Serial.println("ESP32 Soundpod Ready!");
}

// Main loop
void loop() {
  // Check power status
  checkPowerStatus();
  
  // Handle button input with debouncing
  handleButtons();
  
  // Update display with current status
  updateDisplay();
  
  // Play audio as needed
  handleAudioPlayback();
  
  // Allow ESP32 to handle background tasks
  yield();
  
  // Use ESP32's light sleep to save power during idle periods
  if (isIdle()) {
    Serial.println("Entering light sleep mode");
    esp_sleep_enable_timer_wakeup(5000000); // 5 seconds
    esp_light_sleep_start();
    Serial.println("Waking from light sleep");
  }
}

// Button handling function
void handleButtons() {
  // Check button states with debouncing
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // Previous button
    if (digitalRead(BUTTON_PREV_PIN) == LOW) {
      playPreviousTrack();
      lastDebounceTime = millis();
    }
    
    // Play/Pause button
    if (digitalRead(BUTTON_PLAY_PIN) == LOW) {
      togglePlayPause();
      lastDebounceTime = millis();
    }
    
    // Next button
    if (digitalRead(BUTTON_NEXT_PIN) == LOW) {
      playNextTrack();
      lastDebounceTime = millis();
    }
    
    // Volume up button
    if (digitalRead(BUTTON_VOL_UP_PIN) == LOW) {
      increaseVolume();
      lastDebounceTime = millis();
    }
    
    // Volume down button
    if (digitalRead(BUTTON_VOL_DOWN_PIN) == LOW) {
      decreaseVolume();
      lastDebounceTime = millis();
    }
  }
}

// Check if the device is in idle state (could be expanded based on original code)
bool isIdle() {
  // Placeholder function - implement based on your requirements
  // Return true if no activity for certain period
  return false; // For now, always return false to avoid sleep
}

// Load last playback state from storage
void loadLastPlayState() {
  // Implement loading last play state from SPIFFS
  Serial.println("Loading last play state");
  // Code to load from SPIFFS would go here
}

// Save current playback state to storage
void savePlayState() {
  // Implement saving current play state to SPIFFS
  Serial.println("Saving play state");
  // Code to save to SPIFFS would go here
}