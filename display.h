// ESP32 Soundpod - Display Handler
// Modified from original Arduino Soundpod project

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

// Create the OLED display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Current display state
enum DisplayState {
  DISPLAY_WELCOME,
  DISPLAY_NOW_PLAYING,
  DISPLAY_MENU,
  DISPLAY_VOLUME,
  DISPLAY_BATTERY_LOW
};

DisplayState currentDisplayState = DISPLAY_WELCOME;
unsigned long lastDisplayUpdate = 0;
unsigned long displayTimeout = 3000; // Time to show temporary screens (like volume)

// Track information
String currentTrackName = "";
String currentArtistName = "";
int currentTrackNumber = 0;
int totalTracks = 0;
int currentVolume = DEFAULT_VOLUME;
int batteryPercentage = 100;
bool isPlaying = false;

// Add these function declarations after the variable declarations 
// but before any function definitions in display.h

// Function declarations
void displayWelcomeScreen();
void displayNowPlaying();
void displayMenu();
void displayVolume();
void displayBatteryLow();
void updateDisplay();
void setTrackInfo(String trackName, String artistName, int trackNum, int total);
void setPlayingStatus(bool playing);
void setVolume(int volume);
void setBatteryPercentage(int percentage);
void showMenu();
void showNowPlaying();

// Initialize the display
void initDisplay() {
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  
  // Clear the buffer
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.cp437(true); // Use full 256 char 'Code Page 437' font
  
  if (DEBUG) {
    Serial.println("Display initialized");
  }
}

// Show welcome screen
void displayWelcomeScreen() {
  display.clearDisplay();
  
  // Display logo/welcome text
  display.setTextSize(2);
  display.setCursor(10, 10);
  display.println(F("ESP32"));
  display.setCursor(10, 30);
  display.println(F("Soundpod"));
  
  // Display version
  display.setTextSize(1);
  display.setCursor(30, 50);
  display.print(F("v"));
  display.println(FIRMWARE_VERSION);
  
  display.display();
  currentDisplayState = DISPLAY_WELCOME;
}

// Update the display based on current state
void updateDisplay() {
  // Check if we need to transition from temporary displays
  if ((currentDisplayState == DISPLAY_VOLUME || currentDisplayState == DISPLAY_BATTERY_LOW) && 
      (millis() - lastDisplayUpdate > displayTimeout)) {
    currentDisplayState = DISPLAY_NOW_PLAYING;
  }
  
  // Update display based on state
  switch (currentDisplayState) {
    case DISPLAY_WELCOME:
      // Already handled in displayWelcomeScreen()
      break;
      
    case DISPLAY_NOW_PLAYING:
      displayNowPlaying();
      break;
      
    case DISPLAY_MENU:
      displayMenu();
      break;
      
    case DISPLAY_VOLUME:
      displayVolume();
      break;
      
    case DISPLAY_BATTERY_LOW:
      displayBatteryLow();
      break;
  }
}

// Display now playing screen
void displayNowPlaying() {
  display.clearDisplay();
  
  // Top status bar
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("Track: "));
  display.print(currentTrackNumber);
  display.print(F("/"));
  display.print(totalTracks);
  
  // Battery indicator on the right
  display.setCursor(98, 0);
  display.print(F("Bat:"));
  display.print(batteryPercentage);
  display.print(F("%"));
  
  // Track info - truncate if too long
  display.setCursor(0, 16);
  display.setTextSize(1);
  String trackDisplay = currentTrackName;
  if (trackDisplay.length() > 21) {
    trackDisplay = trackDisplay.substring(0, 18) + "...";
  }
  display.println(trackDisplay);
  
  // Artist info
  display.setCursor(0, 26);
  String artistDisplay = currentArtistName;
  if (artistDisplay.length() > 21) {
    artistDisplay = artistDisplay.substring(0, 18) + "...";
  }
  display.println(artistDisplay);
  
  // Play/pause status
  display.setCursor(0, 40);
  display.setTextSize(2);
  if (isPlaying) {
    display.println(F("Playing"));
  } else {
    display.println(F("Paused"));
  }
  
  // Volume indicator at bottom
  display.setCursor(0, 56);
  display.setTextSize(1);
  display.print(F("Vol: "));
  
  // Draw simple volume bar
  int barWidth = map(currentVolume, 0, MAX_VOLUME, 0, 70);
  display.drawRect(30, 56, 70, 8, SSD1306_WHITE);
  display.fillRect(30, 56, barWidth, 8, SSD1306_WHITE);

  display.display();
}

// Display menu screen
void displayMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(25, 0);
  display.println(F("MENU OPTIONS"));
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  
  // Menu options
  display.setCursor(5, 15);
  display.println(F("1. Browse All Tracks"));
  display.setCursor(5, 25);
  display.println(F("2. Playlists"));
  display.setCursor(5, 35);
  display.println(F("3. Settings"));
  display.setCursor(5, 45);
  display.println(F("4. About"));
  
  display.setCursor(0, 15);
  display.print(F(">"));  // Cursor indicator
  
  display.display();
}

// Display volume change screen
void displayVolume() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(30, 10);
  display.println(F("VOLUME"));
  
  // Display numeric volume
  display.setTextSize(2);
  display.setCursor(48, 25);
  display.print(currentVolume);
  
  // Draw volume bar
  display.drawRect(14, 48, 100, 10, SSD1306_WHITE);
  int barWidth = map(currentVolume, 0, MAX_VOLUME, 0, 100);
  display.fillRect(14, 48, barWidth, 10, SSD1306_WHITE);
  
  display.display();
  lastDisplayUpdate = millis();
}

// Display low battery warning
void displayBatteryLow() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(5, 10);
  display.println(F("BATTERY"));
  display.setCursor(15, 30);
  display.println(F("LOW!"));
  
  // Draw battery icon
  display.drawRect(32, 50, 64, 14, SSD1306_WHITE);
  display.drawRect(96, 53, 6, 8, SSD1306_WHITE);
  
  // Fill battery based on percentage
  int fillWidth = map(batteryPercentage, 0, 100, 0, 64);
  display.fillRect(32, 50, fillWidth, 14, SSD1306_WHITE);
  
  display.display();
  lastDisplayUpdate = millis();
}

// Set current track info
void setTrackInfo(String trackName, String artistName, int trackNum, int total) {
  currentTrackName = trackName;
  currentArtistName = artistName;
  currentTrackNumber = trackNum;
  totalTracks = total;
  
  // Update display next time updateDisplay is called
  currentDisplayState = DISPLAY_NOW_PLAYING;
}

// Set playing status
void setPlayingStatus(bool playing) {
  isPlaying = playing;
  // Update display next time updateDisplay is called
}

// Set volume and show volume screen
void setVolume(int volume) {
  currentVolume = volume;
  currentDisplayState = DISPLAY_VOLUME;
  lastDisplayUpdate = millis();
}

// Set battery percentage and show warning if low
void setBatteryPercentage(int percentage) {
  batteryPercentage = percentage;
  
  // Show warning if battery is low
  if (percentage <= 15) {
    currentDisplayState = DISPLAY_BATTERY_LOW;
    lastDisplayUpdate = millis();
  }
}

// Show menu screen
void showMenu() {
  currentDisplayState = DISPLAY_MENU;
}

// Return to now playing screen
void showNowPlaying() {
  currentDisplayState = DISPLAY_NOW_PLAYING;
}

#endif // DISPLAY_H