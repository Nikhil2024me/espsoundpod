// ESP32 Soundpod - Database Handler
// Modified from original Arduino Soundpod project

#ifndef DBHANDLER_H
#define DBHANDLER_H

#include <Arduino.h>
#include "SPIFFS.h"
#include "config.h"

// Track information structure
struct TrackInfo {
  String filename;
  String title;
  String artist;
  String album;
  int trackNumber;
};

// Last playback state structure
struct PlaybackState {
  int lastTrack;
  int lastVolume;
  bool wasPlaying;
};

// Global variables
TrackInfo trackList[MAX_TRACKS];
int tracksLoaded = 0;
PlaybackState lastState;

// Initialize database
void initDatabase() {
  // Initialize SPIFFS if not already done
  if (!SPIFFS.begin(true)) {
    Serial.println("Failed to mount SPIFFS");
    return;
  }
  
  // Check if config file exists
  if (!SPIFFS.exists(CONFIG_FILE)) {
    Serial.println("Config file not found, creating default");
    createDefaultConfig();
  }
  
  // Load track information
  loadTrackInfo();
  
  Serial.println("Database initialized");
}

// Create default configuration file
void createDefaultConfig() {
  File configFile = SPIFFS.open(CONFIG_FILE, "w");
  if (!configFile) {
    Serial.println("Failed to create config file");
    return;
  }
  
  // Write default settings
  configFile.println("volume=" + String(DEFAULT_VOLUME));
  configFile.println("lastTrack=1");
  configFile.println("wasPlaying=false");
  
  configFile.close();
  Serial.println("Default config created");
}

// Load track information from SD card
void loadTrackInfo() {
  // In a real implementation, this would scan the SD card
  // and extract ID3 tags from MP3 files
  
  // For this example, we'll create some dummy data
  // In a real implementation, you would parse the SD card
  
  // Reset track counter
  tracksLoaded = 0;
  
  Serial.println("Loading track information from SD card...");
  
  // Create some placeholder track data
  // In a real implementation, this would come from scanning the SD card
  for (int i = 0; i < 10 && i < MAX_TRACKS; i++) {
    trackList[i].filename = "/music/track" + String(i+1) + ".mp3";
    trackList[i].title = "Track " + String(i+1);
    trackList[i].artist = "Demo Artist";
    trackList[i].album = "Demo Album";
    trackList[i].trackNumber = i+1;
    tracksLoaded++;
  }
  
  Serial.print("Loaded ");
  Serial.print(tracksLoaded);
  Serial.println(" tracks");
}

// Get track information by index
TrackInfo getTrackInfo(int index) {
  if (index >= 0 && index < tracksLoaded) {
    return trackList[index];
  } else {
    // Return empty track info if index is invalid
    TrackInfo emptyTrack;
    emptyTrack.filename = "";
    emptyTrack.title = "Invalid Track";
    emptyTrack.artist = "";
    emptyTrack.album = "";
    emptyTrack.trackNumber = 0;
    return emptyTrack;
  }
}

// Save last playback state
void savePlaybackState(int track, int volume, bool playing) {
  File stateFile = SPIFFS.open(LAST_STATE_FILE, "w");
  if (!stateFile) {
    Serial.println("Failed to open state file for writing");
    return;
  }
  
  // Write state
  stateFile.println("lastTrack=" + String(track));
  stateFile.println("lastVolume=" + String(volume));
  stateFile.println("wasPlaying=" + String(playing ? "true" : "false"));
  
  stateFile.close();
  Serial.println("Playback state saved");
}

// Load last playback state
PlaybackState loadPlaybackState() {
  PlaybackState state;
  state.lastTrack = 1;
  state.lastVolume = DEFAULT_VOLUME;
  state.wasPlaying = false;
  
  // Check if state file exists
  if (!SPIFFS.exists(LAST_STATE_FILE)) {
    Serial.println("State file not found, using defaults");
    return state;
  }
  
  File stateFile = SPIFFS.open(LAST_STATE_FILE, "r");
  if (!stateFile) {
    Serial.println("Failed to open state file for reading");
    return state;
  }
  
  // Read each line and parse key-value pairs
  while (stateFile.available()) {
    String line = stateFile.readStringUntil('\n');
    int separatorPos = line.indexOf('=');
    
    if (separatorPos > 0) {
      String key = line.substring(0, separatorPos);
      String value = line.substring(separatorPos + 1);
      
      // Remove any trailing whitespace
      value.trim();
      
      if (key == "lastTrack") {
        state.lastTrack = value.toInt();
      } else if (key == "lastVolume") {
        state.lastVolume = value.toInt();
      } else if (key == "wasPlaying") {
        state.wasPlaying = (value == "true");
      }
    }
  }
  
  stateFile.close();
  Serial.println("Playback state loaded");
  return state;
}

// Create a playlist
bool createPlaylist(String name, int trackCount, int* trackIndices) {
  String filename = "/" + name + ".playlist";
  
  File playlistFile = SPIFFS.open(filename, "w");
  if (!playlistFile) {
    Serial.println("Failed to create playlist file");
    return false;
  }
  
  // Write playlist header
  playlistFile.println("name=" + name);
  playlistFile.println("count=" + String(trackCount));
  
  // Write track indices
  for (int i = 0; i < trackCount; i++) {
    playlistFile.println(String(trackIndices[i]));
  }
  
  playlistFile.close();
  Serial.println("Playlist created: " + name);
  return true;
}

// Load a playlist
int* loadPlaylist(String name, int* trackCount) {
  String filename = "/" + name + ".playlist";
  
  // Default values
  static int tracks[MAX_TRACKS];
  *trackCount = 0;
  
  // Check if playlist file exists
  if (!SPIFFS.exists(filename)) {
    Serial.println("Playlist file not found: " + name);
    return tracks;
  }
  
  File playlistFile = SPIFFS.open(filename, "r");
  if (!playlistFile) {
    Serial.println("Failed to open playlist file");
    return tracks;
  }
  
  // Read playlist header
  String line = playlistFile.readStringUntil('\n');
  // Skip name line
  line = playlistFile.readStringUntil('\n');
  
  if (line.startsWith("count=")) {
    *trackCount = line.substring(6).toInt();
  }
  
  // Read track indices
  for (int i = 0; i < *trackCount && i < MAX_TRACKS; i++) {
    if (playlistFile.available()) {
      line = playlistFile.readStringUntil('\n');
      tracks[i] = line.toInt();
    } else {
      break;
    }
  }
  
  playlistFile.close();
  Serial.println("Playlist loaded: " + name + " with " + String(*trackCount) + " tracks");
  return tracks;
}

// List all available playlists
String* listPlaylists(int* count) {
  static String playlists[20]; // Maximum 20 playlists
  *count = 0;
  
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  
  while (file && *count < 20) {
    String filename = file.name();
    
    // Check if file is a playlist
    if (filename.endsWith(".playlist")) {
      // Remove extension and path
      int lastSlash = filename.lastIndexOf('/');
      int lastDot = filename.lastIndexOf('.');
      
      if (lastSlash < 0) lastSlash = -1;
      if (lastDot > 0) {
        playlists[*count] = filename.substring(lastSlash + 1, lastDot);
        (*count)++;
      }
    }
    
    file = root.openNextFile();
  }
  
  Serial.print("Found ");
  Serial.print(*count);
  Serial.println(" playlists");
  
  return playlists;
}
#endif 