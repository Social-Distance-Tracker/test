*****************************************************
* Decawave DWM1001 to MQTT via ESP32
*****************************************************

#include <WiFi.h>
#include <PubSubClient.h>
#include <HardwareSerial.h>

const char* ssid = "SSID";
const char* password = "pswd";

WiFiClient espClient;

const char* mqtt_server = "url.com/upload";

PubSubClient client(espClient);

const int bufferSize = 128;
char inputBuffer[bufferSize];
int bufferPointer = 0;
char inByte;


HardwareSerial SerialRTLS(2);
#define RXD2 16
#define TXD2 17

void setup_wifi() {

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);

  }

}

void setup_gateway() {
  char inByte;
  
  SerialRTLS.print("\r\r");
  delay(2000);
  if (SerialRTLS.available() >= 1) {    
    while (SerialRTLS.available()) {  
      inByte = char(SerialRTLS.read());
      Serial.print(inByte);
    }
  }
  Serial.println();
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Command arrived [");
  Serial.print(topic);
  Serial.print("] ");

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    SerialRTLS.write(payload[i]); 
  }

  Serial.println(); 
}

void reconnect() {

  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("connected");

      client.publish("rtls/ESP32Status", "Connected to MQTT Server.");

      client.subscribe("rtls/Command");
      client.publish("rtls/Info", "Send commands by topic rtls/Command.");
    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");

      delay(5000);
    }
  }
}

void setup() {

  SerialRTLS.begin(115200); 

  randomSeed(analogRead(0));
  
  setup_wifi();
  setup_gateway();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  if (SerialRTLS.available() >= 1) {

    while (SerialRTLS.available()) {
      inByte = char(SerialRTLS.read());
      if (inByte == '\n') {
        inputBuffer[bufferPointer++] = '\0';
        client.publish("rtls/StreamPos", inputBuffer);
        bufferPointer = 0;
      }
      else {

        if ( bufferPointer < bufferSize - 1 )  
          inputBuffer[bufferPointer++] = inByte;
      }
    }    
  }
}
