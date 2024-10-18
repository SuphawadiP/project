/*
  Optical Heart Rate Detection (PBA Algorithm) using the MAX30105 Breakout
  By: Nathan Seidle @ SparkFun Electronics
  Date: October 2nd, 2016
  https://github.com/sparkfun/MAX30105_Breakout

  This is a demo to show the reading of heart rate or beats per minute (BPM) using
  a Peripheral Beat Amplitude (PBA) algorithm.

  It is best to attach the sensor to your finger using a rubber band or other tightening
  device. Humans are generally bad at applying constant pressure to a thing. When you
  press your finger against the sensor, it varies enough to cause the blood in your
  finger to flow differently, which causes the sensor readings to go wonky.

  Hardware Connections (Breakout board to Arduino):
  - 5V = 5V (3.3V is allowed)
  - GND = GND
  - SDA = A4 (or SDA)
  - SCL = A5 (or SCL)
  - INT = Not connected

  The MAX30105 Breakout can handle 5V or 3.3V I2C logic. We recommend powering the board with 5V
  but it will also run at 3.3V.
*/

#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

#define DHT_PIN 19      // Pin where DHT is connected
#define SWITCH_PIN 17   // Pin where the switch is connected
#define WIFI_SSID "ngai"
#define WIFI_PASS "123456789"
#define API_KEY "7fRt29bQiQ1RADp3w7ScHtRJ8Tdrp9fQLE0IByFtXuZMtCARpUaPudVwZxn2z1RCf4ufxpRIpkl13tapFYM0afYrg5zzDWeQFGYpYid31RSWM4hA9K83HsmRmj8fdbJM"

// Initialize DHT sensor
DHT dht(DHT_PIN, DHT11);

MAX30105 particleSensor; // Create an instance of the MAX30105 sensor

const byte RATE_SIZE = 4; // Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; // Array of heart rates
byte rateSpot = 0; // Current position in the rates array
long lastBeat = 0; // Time at which the last beat occurred

float beatsPerMinute; // Variable to store calculated BPM
int beatAvg; // Variable to store average BPM

void send_sensor_data(long irValue, float bpm, float avgBpm) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;  // Create an HTTPClient instance
        String url = "http://192.168.137.151:3300/sensor-data";  // Ensure this IP is correct

        // Prepare the JSON payload
        String post_data = "{ \"irValue\":" + String(irValue) + 
                           ", \"bpm\":" + String(bpm) + 
                           ", \"avgBpm\":" + String(avgBpm) + 
                           ", \"apiKey\":\"" + API_KEY + "\"}";

        // Configure the HTTP request
        http.begin(url);  // Specify the URL
        http.addHeader("Content-Type", "application/json");  // Specify content-type header

        // Send the request
        int httpResponseCode = http.POST(post_data);  // Send the HTTP POST request

        // Check for the response
        if (httpResponseCode > 0) {
            Serial.printf("HTTP Response code: %d\n", httpResponseCode);
            if (httpResponseCode == 200) {
                Serial.println("Data sent successfully");
            }
        } else {
            Serial.printf("Error sending data: %s\n", http.errorToString(httpResponseCode).c_str());
        }
        http.end();  // Close connection
    } else {
        Serial.println("WiFi not connected");
    }
}

void setup() {
    Serial.begin(115200); // Initialize serial communication
    Serial.println("Initializing...");

    // Initialize DHT sensor
    dht.begin();

    // Initialize WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.println("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected to WiFi!");

    // Initialize sensor
    if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) { // Use default I2C port, 400kHz speed
        Serial.println("MAX30105 was not found. Please check wiring/power.");
        while (1); // Stop the program if sensor is not found
    }

    Serial.println("Place your index finger on the sensor with steady pressure.");
    particleSensor.setup(); // Configure sensor with default settings
    particleSensor.setPulseAmplitudeRed(0x0A); // Turn Red LED to low to indicate sensor is running
    particleSensor.setPulseAmplitudeGreen(0); // Turn off Green LED

    // Set up switch pin mode
    pinMode(SWITCH_PIN, INPUT_PULLUP); // Assuming the switch is active low
}

int l = 0;
    // float humidity;
    // float temperature ;
void loop() {
    // Read temperature and humidity

    
    // if(l%500==0)
    // {
    //     humidity = dht.readHumidity();
    //     temperature = dht.readTemperature();

    // // Check if any reads failed
    // if (isnan(humidity) || isnan(temperature)) {
    //     Serial.println("Failed to read from DHT sensor!");
    //     delay(2000);  // Wait a few seconds before retrying
    //     return;
    // }
    // }
    // // Read the switch status (LOW means pressed)
    // bool switchStatus = digitalRead(SWITCH_PIN) == LOW;

    // // Debug output for DHT readings
    // if(l%50 == 0)
    // Serial.printf("Temperature: %.2f°C, Humidity: %.2f%%, Switch: %s\n", 
    //               temperature, humidity, switchStatus ? "ON" : "OFF");

    // Heart rate detection logic
    long irValue = particleSensor.getIR(); // Get the infrared value from the sensor

    if (checkForBeat(irValue) == true) {
        // We sensed a beat!
        long delta = millis() - lastBeat; // Calculate the time since the last beat
        lastBeat = millis(); // Update lastBeat to current time
        
        beatsPerMinute = 60 / (delta / 1000.0); // Calculate BPM

        // Validate BPM value
        if (beatsPerMinute < 255 && beatsPerMinute > 20) {
            rates[rateSpot++] = (byte)beatsPerMinute; // Store this reading in the array
            rateSpot %= RATE_SIZE; // Wrap variable

            // Take average of readings
            beatAvg = 0;
            for (byte x = 0; x < RATE_SIZE; x++) {
                beatAvg += rates[x];
            }
            beatAvg /= RATE_SIZE; // Calculate average BPM
        }
    }

if(l%50 == 0)
{
    // Output heart rate results to serial monitor
    Serial.print("IR=");
    Serial.print(irValue);
    Serial.print(", BPM=");
    Serial.print(beatsPerMinute);
    Serial.print(", Avg BPM=");
    Serial.print(beatAvg);

    // Check if finger is placed on the sensor
    if (irValue < 50000) {
        Serial.print(" No finger?");
    }

    Serial.println(); // New line for next output
}
    // Send sensor data to the server
    
  if(l%250 == 0)
    send_sensor_data(irValue, beatsPerMinute, beatAvg);

    // Delay before the next reading
    //delay(1);  // Wait 1 second
    l++;
}