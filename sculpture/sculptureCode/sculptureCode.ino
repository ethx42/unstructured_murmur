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
const int touchPins[] = {T0, T3, T4, T5, T6, T7};
const char* pinNames[] = {"/hex/1", "/hex/2", "/hex/3", "/hex/4", "/hex/5", "/hex/6"};
const int numberOfPins = sizeof(touchPins) / sizeof(touchPins[0]);

// OSC UDP instance
WiFiUDP Udp;

const int ledPinR = 18; // Red channel
const int ledPinG = 19; // Green channel
const int ledPinB = 21; // Blue channel

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
  delay(100);

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
    uint8_t *buffer = new uint8_t[size];
    Udp.read(buffer, size);

    OSCBundle bundle;
    bundle.fill(buffer, size); // Llenar el bundle con el buffer completo
    
    delete[] buffer; // Liberar la memoria del buffer después de usarlo

    processBundle(bundle);
  } else {
    Serial.println("No packet received."); // Solo un log para la ausencia de paquetes
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

// Esta función maneja los mensajes entrantes basados en su dirección OSC
void oscDispatch(OSCMessage &msg) {
  char address[32];
  msg.getAddress(address, 0, 31); // Obtiene la dirección del mensaje OSC
  Serial.print("Address: "); Serial.println(address); // Log esencial


  if (strcmp(address, "/hex/r") == 0) {
    oscRCallBack(msg);
  } else if (strcmp(address, "/hex/g") == 0) {
    oscGCallBack(msg);
  } else if (strcmp(address, "/hex/b") == 0) {
    oscBCallBack(msg);
  } else {
    Serial.print("Unexpected address: "); Serial.println(address); // Log para direcciones inesperadas
  }
}

// Las funciones callback ahora simplemente manejan el mensaje
void oscRCallBack(OSCMessage &msg) {
  analogWrite(ledPinR, (int)(msg.getFloat(0) * 255));
}

void oscGCallBack(OSCMessage &msg) {
  analogWrite(ledPinG, (int)(msg.getFloat(0) * 255));
}

void oscBCallBack(OSCMessage &msg) {
  analogWrite(ledPinB, (int)(msg.getFloat(0) * 255));
}