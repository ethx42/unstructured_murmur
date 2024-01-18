#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <NewPing.h>

// Network credentials
const char* ssid1 = "bitches_brew";
const char* password1 = "Somebody is watching you, there're strangers 333";

const char* ssid2 = "MURMUR";
const char* password2 = "";

// OSC server details
const IPAddress outIp(192, 168, 0, 200);  // IP of the OSC server (e.g., TouchDesigner)
const unsigned int outPort = 9000;        // Port for sending OSC messages
const unsigned int localPort = 8888;      // Local port to listen for OSC messages

// IP and port of the third ESP32 that controls the smoke machine
const IPAddress smokeMachineIp(192, 168, 0, 122);
const unsigned int smokeMachinePort = 1234;

unsigned long lastTimeDistanceBelow77 = 0;
const unsigned long debounceDuration = 3000;

// Pin definitions
// Touch Pins: D4, D15, D13, D12, D14, D27
const int touchPins[] = {T0, T3, T4, T5, T6, T7};
const char* pinNames[] = {"/hex/1", "/hex/2", "/hex/3", "/hex/4", "/hex/5", "/hex/6"};
const int numberOfPins = sizeof(touchPins) / sizeof(touchPins[0]);

// OSC UDP instance
WiFiUDP Udp;

const int ledPinR = 18; // Red channel
const int ledPinG = 19; // Green channel 
const int ledPinB = 21; // Blue channel


// Ultrasonic Sensor
#define TRIGGER_PIN  32  // Trigger pin of the ultrasonic sensor
#define ECHO_PIN     35  // Echo pin of the ultrasonic sensor
#define MAX_DISTANCE 400 // Max distance to measure (in cm)
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

// Global variables to hold the timing information
unsigned long previousMillis = 0;        // Stores last time the OSC messages were sent
const long interval = 70;                // Interval at which to send OSC messages (milliseconds)

void setup() {
  Serial.begin(115200);
  Serial.println("Setting up ESP32/hex");

  // Try to connect to the first WiFi network
  if (!connectToWiFi(ssid1, password1)) {
    Serial.println("Trying the second WiFi network...");
    // If the first attempt fails, try the second network
    connectToWiFi(ssid2, password2);
  }

  if (WiFi.status() == WL_CONNECTED) {
    // WiFi is connected, proceed with the rest of the setup
    Serial.println("WiFi connected.");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    String mac = WiFi.macAddress();
    Serial.println("MAC Address: " + mac);
  } else {
    // Neither of the WiFi networks worked
    Serial.println("Could not connect to any WiFi network.");
  }

  // Begin listening for OSC messages
  Udp.begin(localPort);

  // Initialize the RGB LED pins as output
  pinMode(ledPinR, OUTPUT);
  pinMode(ledPinG, OUTPUT);
  pinMode(ledPinB, OUTPUT);

  // Set initial color to off (assuming common anode RGB LED)
  analogWrite(ledPinR, 0);
  analogWrite(ledPinG, 0);
  analogWrite(ledPinB, 0);
}

void loop() {
  unsigned long currentMillis = millis();

  // Send and receive touch sensor values via OSC at intervals
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;  // Save the last time you sent the OSC messages

    for (int i = 0; i < numberOfPins; i++) {
      int touchValue = touchRead(touchPins[i]);
      sendOSCMessage(pinNames[i], touchValue);
    }

    // Measure disntance with the ultrasonic sensor
    unsigned int uS = sonar.ping_median(5); // Do 5 pings, discard outliers and return median in microseconds
    int distance = uS / US_ROUNDTRIP_CM; // Convert ping time to distance and print result (0 = outside set distance range)
    // Serial.println(distance);

    // Send distance value via OSC
    sendOSCMessage("/hex/distance", distance);

    // Check if distance is below threshold and update the timer
    if (distance <= 77) {
      if (lastTimeDistanceBelow77 == 0) {
        // Start timing
        lastTimeDistanceBelow77 = currentMillis;
      }
    } else {
      // Reset timing if distance goes above threshold
      lastTimeDistanceBelow77 = 0;
    }

    // Call startSmokeOSCMessage if the condition has been true for more than 3 seconds
    if (lastTimeDistanceBelow77 > 0 && (currentMillis - lastTimeDistanceBelow77 >= debounceDuration)) {
      startSmokeOSCMessage(true);
    } else {
      startSmokeOSCMessage(false);
    }

    receiveOSCMessage();
  }
}

bool connectToWiFi(const char* ssid, const char* password) {
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  // Wait for a maximum time for the connection
  for (int i = 0; i < 20; i++) {  // Try for 10 seconds
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Connected to " + WiFi.SSID() + "!");
      return true;
    }
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connection failed.");
  return false;
}

void sendOSCMessage(const char* address, int touchValue) {
  OSCMessage msg(address);
  msg.add((float)touchValue);

  // Send to the original port (9000) TOUCH DESIGNER
  Udp.beginPacket(outIp, outPort);
  msg.send(Udp);
  Udp.endPacket();

  // Send to the new port (3333) PROCESSING
  Udp.beginPacket(outIp, 3333);
  msg.send(Udp);
  Udp.endPacket();

  msg.empty();  // Clear the message for reuse
}

void startSmokeOSCMessage(bool value) {
  OSCMessage msg("/hex/startSmokeMachine");
  msg.add(value);

  Udp.beginPacket(smokeMachineIp, smokeMachinePort);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty(); // Clear the message for reuse
}

void receiveOSCMessage() {
  int size = Udp.parsePacket();
  if (size > 0) {
    uint8_t *buffer = new uint8_t[size];
    Udp.read(buffer, size);

    OSCBundle bundle;
    bundle.fill(buffer, size); // Fill the bundle with the complete buffer
    
    delete[] buffer; // Free buffer's memory after usage

    processBundle(bundle);
  } else {
    // Serial.println("No packet received.");
  }
}

void processBundle(OSCBundle &bundle) {
  for (int i = 0; i < bundle.size(); i++) {
    OSCMessage *bundledMsg = bundle.getOSCMessage(i);
    if (bundledMsg) { // nullptr check is implicit
      oscDispatch(*bundledMsg);
    }
  }
}

// Handle the OSC messages based on their address
void oscDispatch(OSCMessage &msg) {
  char address[32];
  msg.getAddress(address, 0, 31); // Get OSC message address
  // Serial.print("Address: "); Serial.println(address);

  if (strcmp(address, "/hex/r") == 0) {
    oscRCallBack(msg);
  } else if (strcmp(address, "/hex/g") == 0) {
    oscGCallBack(msg);
  } else if (strcmp(address, "/hex/b") == 0) {
    oscBCallBack(msg);
  } else {
    // Serial.print("Unexpected address: "); Serial.println(address);
  }
}

// Callback functions for the OSC messages
void oscRCallBack(OSCMessage &msg) {
  analogWrite(ledPinR, (int)(msg.getFloat(0) * 255));
}

void oscGCallBack(OSCMessage &msg) {
  analogWrite(ledPinG, (int)(msg.getFloat(0) * 255));
}

void oscBCallBack(OSCMessage &msg) {
  analogWrite(ledPinB, (int)(msg.getFloat(0) * 255));
}