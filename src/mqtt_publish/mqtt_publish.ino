#include <WiFi.h>
#include <PubSubClient.h>

// Replace the next variables with your SSID/Password combination
const char* ssid = "3807444";
const char* password = "berlin2022";

// MQTT Broker IP address
const char* mqtt_server = "io.adafruit.com";
const int mqttPort = 1883;
#define IO_USERNAME  "jg223fp"
#define IO_KEY       "aio_pHcJ80PKSkjPHgfLR486iaNrdY6m"
String clientId = "movement_sensor_box";
const char* mqttTopic = "jg223fp/feeds/Dayli_flow";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;


float temperature = 0;

void setup() {
  Serial.begin(115200);
  connectWifi();
}

void connectWifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void connectMqtt() {
  client.setServer(mqtt_server, mqttPort);
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientId.c_str(), IO_USERNAME, IO_KEY)) {
            Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    connectWifi();
  }
  if (!client.connected()) {
    connectMqtt();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    
    // Temperature in Celsius
    temperature = 12;   

    // Convert the value to a char array
    char tempString[8];
    dtostrf(temperature, 1, 2, tempString);
    Serial.print("Temperature: ");
    Serial.println(tempString);
    client.publish(mqttTopic, tempString);
  }
}