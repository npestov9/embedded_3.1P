
#include <HCSR04.h>
#include <WiFiNINA.h>

//please enter your sensitive data in the Secret tab
char ssid[] = "";
char pass[] = "";

WiFiClient client;

char   HOST_NAME[] = "maker.ifttt.com";
String PATH_NAME = "/trigger/sketch_received/with/key/-";
String queryString = "?value1=57&value2=25";

// HC-SR04 Sensor pins
UltraSonicDistanceSensor distanceSensor(3, 2);  

int status = WL_IDLE_STATUS;

// Threshold settings
const float DISTANCE_THRESHOLD = 100.0; // 100 cm = 1 meter
bool objectDetected = false; // To prevent multiple triggers


void setup() {
  Serial.begin(9600);
  
  // Connect to WiFi - using your working approach
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
    
    // Wait 10 seconds for connection
    delay(10000);
  }
  
  Serial.println("Connected to WiFi");
  printWifiStatus();
}

void loop() {
  // Measure distance
  float distance = distanceSensor.measureDistanceCm();
  
  // Check if reading is valid
  if (distance > 0) {
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
    
    // Check if distance is less than 1 meter (100 cm)
    if (distance < DISTANCE_THRESHOLD) {
      // Check if this is a new detection
      if (!objectDetected) {
        Serial.println("Object detected within 1 meter!");
        sendToIFTTT(distance);
        objectDetected = true; // Set flag to prevent multiple triggers
      }
    } else {
      // Reset the flag when object moves away
      if (objectDetected) {
        Serial.println("Object moved away from 1 meter range");
        objectDetected = false;
      }
    }
  } else {
    Serial.println("Error reading sensor data");
  }
  
  // Wait before next reading
  delay(1000); // Shortened delay for more responsive detection
}

void sendToIFTTT(float distance) {
  if (client.connect(HOST_NAME, 80)) {
    Serial.println("Connected to IFTTT");
    
    // Create a simpler query string (problems often happen with special characters)
    String path = "/trigger/sketch_received/with/key/cpTwdgRCiVhRofR-SLhCGO";
    String jsonData = "{\"value1\":\"" + String(distance) + "\",\"value2\":\"Proximity Alert\",\"value3\":\"Object detected\"}";
    
    // Print the request details for debugging
    Serial.print("Sending request to path: ");
    Serial.println(path);
    
    // Send HTTP POST request instead of GET
    client.println("POST " + path + " HTTP/1.1");
    client.println("Host: maker.ifttt.com");
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(jsonData.length());
    client.println("Connection: close");
    client.println();
    client.println(jsonData);
    
    Serial.println("Waiting for response...");
    
    // Wait for server response
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }
    
    // Read and display the response
    Serial.println("Response from IFTTT:");
    while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.println(line);
    }
  } else {
    Serial.println("Connection to IFTTT failed");
  }
  client.stop();
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}