// ESP32 Soundpod - MP3 Handler
// Modified from original Arduino Soundpod project

#ifndef MP3HANDLER_H
#define MP3HANDLER_H

#include <DFRobotDFPlayerMini.h>
#include "config.h"

// External references
extern HardwareSerial playerSerial;
extern void setTrackInfo(String trackName, String artistName, int trackNum, int total);
extern void setPlayingStatus(bool playing);
extern void setVolume(int volume);

// Create MP3 player instance
DFRobotDFPlayerMini mp3Player;

// MP3 player status variables
int currentVolume = DEFAULT_VOLUME;
int currentTrack = 1;
int totalTracks = 0;
bool isPlaying = false;
unsigned long lastTrackCheckTime = 0;
unsigned long trackCheckInterval = 1000; // Check if track finished every second

// Initialize MP3 player
void initMP3Player() {
  Serial.println("Initializing DFPlayer Mini...");
  
  if (!mp3Player.begin(playerSerial)) {
    Serial.println("Unable to begin DFPlayer Mini");
    Serial.println("1.Please recheck the connection!");
    Serial.println("2.Please insert the SD card!");
    while(true) {
      delay(0); // Code to halt
    }
  }
  
  Serial.println("DFPlayer Mini online.");
  
  // Set volume
  mp3Player.volume(currentVolume);
  
  // Get total number of tracks on SD card
  delay(100); // Small delay before command
  totalTracks = mp3Player.readFileCounts();
  delay(100);
  
  if (totalTracks <= 0) {
    totalTracks = 0;
    Serial.println("No files found on SD card");
  } else {
    Serial.print("Total tracks: ");
    Serial.println(totalTracks);
  }
  
  // Set EQ
  mp3Player.EQ(DFPLAYER_EQ_NORMAL);
  
  // Set device
  mp3Player.outputDevice(DFPLAYER_DEVICE_SD);
}

// Start playing current track
void startPlayback() {
  if (totalTracks > 0) {
    mp3Player.play(currentTrack);
    isPlaying = true;
    setPlayingStatus(true);
    
    // Get track info from database and update display
    String trackName = "Track " + String(currentTrack); // Placeholder
    String artistName = "Unknown Artist"; // Placeholder
    setTrackInfo(trackName, artistName, currentTrack, totalTracks);
    
    Serial.print("Playing track: ");
    Serial.println(currentTrack);
  } else {
    Serial.println("No tracks available to play");
  }
}

// Pause playback
void pausePlayback() {
  mp3Player.pause();
  isPlaying = false;
  setPlayingStatus(false);
  Serial.println("Playback paused");
}

// Resume playback
void resumePlayback() {
  mp3Player.start();
  isPlaying = true;
  setPlayingStatus(true);
  Serial.println("Playback resumed");
}

// Toggle between play and pause
void togglePlayPause() {
  if (isPlaying) {
    pausePlayback();
  } else {
    resumePlayback();
  }
}

// Play next track
void playNextTrack() {
  if (currentTrack < totalTracks) {
    currentTrack++;
  } else {
    currentTrack = 1; // Loop back to first track
  }
  
  startPlayback();
  Serial.print("Next track: ");
  Serial.println(currentTrack);
}

// Play previous track
void playPreviousTrack() {
  if (currentTrack > 1) {
    currentTrack--;
  } else {
    currentTrack = totalTracks; // Loop to last track
  }
  
  startPlayback();
  Serial.print("Previous track: ");
  Serial.println(currentTrack);
}

// Increase volume
void increaseVolume() {
  if (currentVolume < MAX_VOLUME) {
    currentVolume += VOLUME_STEP;
    if (currentVolume > MAX_VOLUME) {
      currentVolume = MAX_VOLUME;
    }
    mp3Player.volume(currentVolume);
    setVolume(currentVolume);
    Serial.print("Volume up: ");
    Serial.println(currentVolume);
  }
}

// Decrease volume
void decreaseVolume() {
  if (currentVolume > 0) {
    currentVolume -= VOLUME_STEP;
    if (currentVolume < 0) {
      currentVolume = 0;
    }
    mp3Player.volume(currentVolume);
    setVolume(currentVolume);
    Serial.print("Volume down: ");
    Serial.println(currentVolume);
  }
}

// Set specific track by number
void playTrackByNumber(int trackNumber) {
  if (trackNumber > 0 && trackNumber <= totalTracks) {
    currentTrack = trackNumber;
    startPlayback();
  } else {
    Serial.print("Invalid track number: ");
    Serial.println(trackNumber);
  }
}

// Handle audio playback in main loop
void handleAudioPlayback() {
  // Check if current track has finished playing
  if (isPlaying && (millis() - lastTrackCheckTime > trackCheckInterval)) {
    lastTrackCheckTime = millis();
    
    // On ESP32, we can use the DFPlayer's available() method more reliably
    if (mp3Player.available()) {
      uint8_t type = mp3Player.readType();
      int value = mp3Player.read();
      
      // Check if track finished
      if (type == DFPlayerPlayFinished) {
        Serial.print("Track finished: ");
        Serial.println(value);
        playNextTrack(); // Auto-play next track
      }
    }
  }
}

// Stop playback
void stopPlayback() {
  mp3Player.stop();
  isPlaying = false;
  setPlayingStatus(false);
  Serial.println("Playback stopped");
}

// Set equalizer mode
void setEQ(uint8_t eq) {
  mp3Player.EQ(eq);
}

// Get current status information
String getPlayerStatus() {
  String status = "Track: " + String(currentTrack) + "/" + String(totalTracks);
  status += ", Volume: " + String(currentVolume);
  status += ", Status: " + String(isPlaying ? "Playing" : "Paused");
  return status;
}

#endif // MP3HANDLER_H