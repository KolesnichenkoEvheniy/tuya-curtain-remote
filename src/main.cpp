#include <Arduino.h>
#include <WiFi.h>
#include <Bounce2.h>
#include "TuyaAPI.h"
#include "config.h"
#include <time.h>
#include <sys/time.h>

// Pin definitions
const int potentiometerPin = 34; // Potentiometer pin
const int buttonPin = 35;        // Button pin

// Objects for Tuya API and button debouncer
TuyaAPI tuya(TUYA_CLIENT_ID, TUYA_SECRET, TUYA_DEVICE_ID, TUYA_API_URL);
Bounce buttonDebouncer;

// Fixed percentage for testing
const int TEST_PERCENTAGE = 90; // Set blinds to 50% open
const unsigned long TEST_INTERVAL = 30000; // Test every 30 seconds

void setup() {
    Serial.begin(115200);

    // Connect to Wi-Fi using credentials from config.h
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    configTime(0, 0, "pool.ntp.org", "time.nist.gov");

    Serial.println("Waiting for NTP time sync: ");
    time_t now = time(nullptr);
    while (now < 8 * 3600 * 2) {
        delay(500);
        Serial.print(".");
        now = time(nullptr);
    }
    Serial.println("");
    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);
    Serial.print("Current time: ");
    Serial.println(asctime(&timeinfo));

    // Set up button and debounce handler
    pinMode(buttonPin, INPUT_PULLUP); // Button is connected with pull-up to VCC
    buttonDebouncer.attach(buttonPin);
    buttonDebouncer.interval(10); // Set debounce interval (10 ms)
}

void loop() {
    static unsigned long lastTestTime = 0;
    unsigned long currentTime = millis() - TEST_INTERVAL; // Subtract TEST_INTERVAL to run the test immediately

    // Run the test every TEST_INTERVAL milliseconds
    if (currentTime - lastTestTime >= TEST_INTERVAL) {
        lastTestTime = currentTime;

        Serial.print("Setting blinds to ");
        Serial.print(TEST_PERCENTAGE);
        Serial.println("%");

        // Send command to control blinds via Tuya API
        if (tuya.setBlindsPosition(TEST_PERCENTAGE)) {
            Serial.println("Successfully set blinds percentage");
        } else {
            Serial.println("Failed to set blinds percentage");
        }
    }
}
