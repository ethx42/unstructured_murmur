#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h> 

// Network credentials
const char* ssid = "bitches_brew";
const char* password = "Somebody is watching you, there're strangers 333";

// OSC server details
const IPAddress outIp(192, 168, 0, 103); // IP of the OSC server (e.g., TouchDesigner)
const unsigned int outPort = 9000;       // Port for sending OSC messages
const unsigned int localPort = 8888;     // Local port to listen for OSC messages

// Pin definitions
const int touchPins[] = {T0, T2, T3, T4, T5, T6};
const char* pinNames[] = {"/hex/1", "/hex/2", "/hex/3", "/hex/4", "/hex/5", "/hex/6"};
const int numberOfPins = sizeof(touchPins) / sizeof(touchPins[0]);

// OSC UDP instance
WiFiUDP Udp;

const int ledPinB = 25; // Blue channel
const int ledPinG = 26; // Green channel
const int ledPinR = 27; // Red channel

// Global variables to hold the timing information
unsigned long previousMillis = 0;        // Stores last time the OSC messages were sent
const long interval = 100;               // Interval at which to send OSC messages (milliseconds)

void setup() {
  Serial.begin(115200);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

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
  
  // Send touch sensor values via OSC at intervals
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;  // Save the last time you sent the OSC messages
    
    for (int i = 0; i < numberOfPins; i++) {
      int touchValue = touchRead(touchPins[i]);
      sendOSCMessage(pinNames[i], touchValue);
    }
  }

  // Continuously check for incoming OSC messages without delay
  receiveOSCMessage();
}

void sendOSCMessage(const char* address, int touchValue) {
  OSCMessage msg(address);
  msg.add((float)touchValue); // OSC typically uses float values
  Udp.beginPacket(outIp, outPort);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty(); // Clear the message for reuse
}

void receiveOSCMessage() {
  int size = Udp.parsePacket();
  if (size > 0) {
    // Debug: Print the size of the incoming packet
    Serial.print("Received packet size: ");
    Serial.println(size);

    uint8_t *buffer = new uint8_t[size];
    Udp.read(buffer, size);

    // Debug: Print the raw buffer contents
    Serial.print("Received packet contents: ");
    for(int i = 0; i < size; i++) {
      Serial.print(buffer[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    OSCBundle bundle;
    for (int i = 0; i < size; i++) {
      bundle.fill(buffer[i]);
    }
    delete[] buffer;

    // Debug: Print the number of messages in the bundle
    Serial.print("Number of messages in the bundle: ");
    Serial.println(bundle.size());

    for (int i = 0; i < bundle.size(); i++) {
      OSCMessage *bundledMsg = bundle.getOSCMessage(i);
      if (bundledMsg != nullptr) {
        char address[32];
        bundledMsg->getAddress(address, 0, 32);
        Serial.print("OSC Address: ");
        Serial.println(address);

        if (strcmp(address, "/hex/r") == 0) {
          oscRCallBack(*bundledMsg);
        } else if (strcmp(address, "/hex/g") == 0) {
          oscGCallBack(*bundledMsg);
        } else if (strcmp(address, "/hex/b") == 0) {
          oscBCallBack(*bundledMsg);
        } else {
          // Debug: Print out if an unexpected address is received
          Serial.print("Unexpected OSC address: ");
          Serial.println(address);
        }
      } else {
        // Debug: Print out if a message was null
        Serial.println("Received a null OSC message in the bundle.");
      }
    }
  } else {
    // Debug: Print out if no packet was received
    Serial.println("No packet received.");
  }
}


// Callbacks separados para cada dirección
void oscRCallBack(OSCMessage &msg) {
  handleColorMessage(msg, "Received OSC message for R channel");
  float value = msg.getFloat(0);
   Serial.print("R channel value: ");
  Serial.println(value);
  analogWrite(ledPinR, (int)(value * 255));
}

void oscGCallBack(OSCMessage &msg) {
  handleColorMessage(msg, "Received OSC message for G channel");
  float value = msg.getFloat(0);
  Serial.print("G channel value: ");
  Serial.println(value);
  analogWrite(ledPinG, (int)(value * 255));
}

void oscBCallBack(OSCMessage &msg) {
  handleColorMessage(msg, "Received OSC message for B channel");
  float value = msg.getFloat(0);
  Serial.print("B channel value: ");
  Serial.println(value);
  analogWrite(ledPinB, (int)(value * 255));
}

// Función de manejo genérica para imprimir y controlar el LED
void handleColorMessage(OSCMessage &msg, const char* debugMessage) {
  float value = msg.getFloat(0);
  Serial.println(debugMessage);
  Serial.print(", value: ");
  Serial.println(value);
}