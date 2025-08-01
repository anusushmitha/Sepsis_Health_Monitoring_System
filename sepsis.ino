#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30100_PulseOximeter.h"
#include <ESP8266WiFi.h>  // For ESP8266 Wi-Fi connection
#include <PubSubClient.h>  // For MQTT communication

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

// Wi-Fi credentials
const char* ssid = "Redmi";
const char* password = "12345678";

// MQTT Broker settings
const char* mqtt_server = "broker.emqx.io";  // Example public broker
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

// Create objects for the MLX90614 sensor and OLED display
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Create object for the MAX30100 sensor
PulseOximeter pox;

// Variables for heart rate and SpO2
float heartRate = 89;
float spo2 = 82;
unsigned long lastReportTime = 0;

// Callback function for MAX30100 to update heart rate and SpO2
void onBeatDetected() {
  Serial.println("Beat detected!");
}

// Function to connect to Wi-Fi
void setup_wifi() {
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

// Function to reconnect to MQTT broker if connection is lost
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client")) {  // No username and password needed
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  setup_wifi();
  
  // Set MQTT server details
  client.setServer(mqtt_server, mqtt_port);

  // Initialize the MLX90614 sensor
  if (!mlx.begin()) {
    Serial.println("Error connecting to MLX90614 sensor!");
    while (1);
  }

  // Initialize the MAX30100 sensor
  if (!pox.begin()) {
    Serial.println("Error connecting to MAX30100 sensor!");
    while (1);
  }
  pox.setOnBeatDetectedCallback(onBeatDetected);

  // Initialize the OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (1);
  }

  // Set text properties
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  Serial.println("Sensors, Wi-Fi, and OLED initialized.");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Read temperature from MLX90614
  float ambientTemp = mlx.readAmbientTempC();
  float objectTemp = mlx.readObjectTempC();

  // Update the MAX30100 sensor readings
  pox.update();
  heartRate = pox.getHeartRate();
  spo2 = pox.getSpO2();

  // Publish sensor data to MQTT broker
  char tempPayload[50], hrPayload[50], spo2Payload[50];
  
  snprintf(tempPayload, sizeof(tempPayload), "Ambient Temp: %.2f C, Object Temp: %.2f C", ambientTemp, objectTemp);
  snprintf(hrPayload, sizeof(hrPayload), "Heart Rate: %.2f BPM", heartRate);
  snprintf(spo2Payload, sizeof(spo2Payload), "SpO2: %.2f %%", spo2);

  client.publish("sensor/temperature", tempPayload);
  client.publish("sensor/heartrate", hrPayload);
  client.publish("sensor/spo2", spo2Payload);

  // Display the data on OLED
  display.clearDisplay();

  // Display ambient and object temperatures
  display.setCursor(0, 0);
  display.print("Ambient Temp: ");
  display.print(ambientTemp);
  display.println(" C");

  display.setCursor(0, 10);
  display.print("Object Temp: ");
  display.print(objectTemp);
  display.println(" C");

  // Display heart rate and SpO2
  display.setCursor(0, 30);
  display.print("Heart Rate: ");
  display.print(heartRate);
  display.println(" BPM");

  display.setCursor(0, 40);
  display.print("SpO2: ");
  display.print(spo2);
  display.println(" %");

  // Show the buffer content on the OLED
  display.display();

  // Print values to the Serial Monitor as well
  Serial.print("Ambient Temp: ");
  Serial.print(ambientTemp);
  Serial.println(" C");

  Serial.print("Object Temp: ");
  Serial.print(objectTemp);
  Serial.println(" C");

  Serial.print("Heart Rate: ");
  Serial.print(heartRate);
  Serial.println(" BPM");

  Serial.print("SpO2: ");
  Serial.print(spo2);
  Serial.println(" %");

  delay(1000);  // Delay before updating the readings
}
