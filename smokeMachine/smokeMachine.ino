#include <WiFi.h>

const char* ssid1 = "bitches_brew";
const char* password1 = "Somebody is watching you, there're strangers 333";
const char* ssid2 = "MURMUR";
const char* password2 = "";
const int relayPin = 18;

unsigned long previousMillis = 0;
const unsigned long interval = 1200000;
const unsigned long relayOnDuration = 20000;
bool relayState = HIGH;

void setup() {
  Serial.begin(115200);
  Serial.println("Setting up ESP32/SMOKE MACHINE");
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, relayState);
  
  if (!connectToWiFi(ssid1, password1)) {
    connectToWiFi(ssid2, password2);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected.");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    String mac = WiFi.macAddress();
    Serial.println("MAC Address: " + mac);
  } else {
    Serial.println("Could not connect to any WiFi network.");
  }
}

void loop() {
  unsigned long currentMillis = millis();

  if (relayState == HIGH && currentMillis - previousMillis >= interval) {
    relayState = LOW;
    digitalWrite(relayPin, relayState);
    Serial.println("Relay turned ON.");
    previousMillis = currentMillis;
  }

  if (relayState == LOW && currentMillis - previousMillis >= relayOnDuration) {
    relayState = HIGH;
    digitalWrite(relayPin, relayState);
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
