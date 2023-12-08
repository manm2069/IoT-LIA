//libraries to integrate functionality
#include <WiFi.h>          //wifi connection
#include <PubSubClient.h>  //MQTT messaging
#include <Arduino.h>       //Input and output

//#include <Adafruit_Sensor.h>  // Initialize the Adafruit unified sensor library
#include <DHT.h>  // Initialize the DHT sensor library
//#include <DHT_U.h>            // Initialize the DHT sensor library
#define DHTPIN 16      // Defines pin 7 as the Arduino pin connected to the DHT sensor
#define DHTTYPE DHT11  // Defines DHT11 as the DHT sensor used in the circuit
// Initializes Arduino pin 7 as the pin connected to the DHT11 sensor
//DHT_Unified dht(DHTPIN, DHTTYPE);
DHT dht(DHTPIN, DHTTYPE);
// Creates an unisgned 32 bit integer variable called "delayMS"
int temperatureTrueValue = 0;
int humidityTrueValue = 0;

// Potentionmeter
const int moisturePin = 34;
int moistureValue = 0;
int moistureTrueValue = 0;

const int lightPin = 35;
int lightValue = 0;
int lightTrueValue = 0;

// Wi-Fi credentials: replace with those of your network
const char* ssid = "alivio";            // The name of the WiFi network
const char* password = "alivioalivio";  // The WiFi network passkey

// MQTT broker details: replace with your own
const char* mqtt_server = "192.168.205.18";    // The MQTT broker's hostname or IP address
const int mqtt_port = 1883;                    // MQTT broker port (1883 is default)
const char* moisture_topic = "soil/moisture";  // MQTT topic to publish messages
const char* temperature_topic = "tank/temperature";
const char* humidity_topic = "tank/humidity";
const char* light_topic = "tank/light";
// MQTT client name prefix (will add MAC address)
String name = "ESP32Client_";

// Create an instance of the WiFiClient class
WiFiClient espClient;
// Create an instance of the PubSubClient class
PubSubClient client(espClient);

// Timer for publishing every 5 seconds
unsigned long previousMillis = 0;
const long interval = 500;

void setup() {
  // Start Serial communication
  Serial.begin(115200);

  // Read the MAC address
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  // Convert MAC address to a string
  char macStr[18];  // MAC address is 12 characters long without separators, plus null terminator
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  // Concatenate the name prefix with the MAC address
  name = name + macStr;

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Set MQTT server and port
  client.setServer(mqtt_server, mqtt_port);

  dht.begin();  // Begins the DHT11 sensor
}

void loop() {
  // Connect to MQTT if necessary
  if (!client.connected()) {
    connect();
  }

  // Get the current time
  unsigned long currentMillis = millis();

  // Publish a message every 5 seconds
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Read potentiometer value
    moistureValue = analogRead(moisturePin);
    moistureTrueValue = map(moistureValue, 700, 2800, 100, 0);
    String moistMessage = String(moistureTrueValue);
    client.publish(moisture_topic, moistMessage.c_str());

    lightValue = analogRead(lightPin);
    lightTrueValue = map(lightValue, 0, 4100, 0, 100);
    String lightMessage = String(lightTrueValue);
    client.publish(light_topic, lightMessage.c_str());

    float temperatureValue = dht.readTemperature();
    temperatureTrueValue = temperatureValue * 100;
    float humidityValue = dht.readHumidity();
    humidityTrueValue = humidityValue * 100;

    String temperatureMessage = String(temperatureTrueValue);
    client.publish(temperature_topic, temperatureMessage.c_str());
    String humidityMessage = String(humidityTrueValue);
    client.publish(humidity_topic, humidityMessage.c_str());
  }

  // Allow the PubSubClient to process incoming messages
  client.loop();
}

void connect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");

    // Attempt to connect
    if (client.connect(name.c_str())) {
      Serial.println("Connected to MQTT broker");
    } else {
      Serial.print("Failed to connect to MQTT broker, rc=");
      Serial.print(client.state());
      Serial.println("Try again in 5 seconds");
      delay(5000);
    }
  }
}
