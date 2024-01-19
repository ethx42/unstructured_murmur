#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>

const char* ssid1 = "bitches_brew";
const char* password1 = "Somebody is watching you, there're strangers 333";
const char* ssid2 = "MURMUR";
const char* password2 = "";
const unsigned int localPort = 1234;
const int relayPin = 18;
WiFiUDP Udp;

unsigned long relayActivatedTime = 0;
const unsigned long relayOnDuration = 12000; // 12 seconds
bool relayActivated = false;

void setup() {
  Serial.begin(115200);
  Serial.println("Setting up ESP32/SMOKE MACHINE");
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);
  if (!connectToWiFi(ssid1, password1)) {
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

  Udp.begin(localPort);
}

void loop() {
  receiveOSCMessage();
  checkRelayStatus();
}

void checkRelayStatus() {
  unsigned long currentTime = millis();
  if (relayActivated && currentTime - relayActivatedTime >= relayOnDuration) {
    digitalWrite(relayPin, HIGH); // Turn off the relay
    relayActivated = false;
    Serial.println("Relay turned OFF.");
  }
}

bool connectToWiFi(const char* ssid, const char* password) {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  for (int i = 0; i < 20; i++) {
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

void receiveOSCMessage() {
  int size = Udp.parsePacket();
  if (size > 0) {
    uint8_t *buffer = new uint8_t[size];
    Udp.read(buffer, size);
    OSCBundle bundle;
    bundle.fill(buffer, size);
    delete[] buffer;
    processBundle(bundle);
  }
}

void processBundle(OSCBundle &bundle) {
  for (int i = 0; i < bundle.size(); i++) {
    OSCMessage *bundledMsg = bundle.getOSCMessage(i);
    if (bundledMsg) {
      processMessage(*bundledMsg);
    }
  }
}

void processMessage(OSCMessage &msg) {
  char address[32];
  msg.getAddress(address, 0, 31);
  bool value = msg.getBoolean(0);
  if (value) {
    Serial.print("Received OSC message at address: ");
    Serial.println(address);
    Serial.println("Value: true");
    if ((strcmp(address, "/startSmokeMachine") == 0) && !relayActivated) {
      digitalWrite(relayPin, LOW); // Turn on the relay
      relayActivated = true;
      relayActivatedTime = millis();
      Serial.println("Relay turned ON.");
    }
  }
}
