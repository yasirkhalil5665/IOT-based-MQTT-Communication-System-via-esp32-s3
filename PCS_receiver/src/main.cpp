#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Wi-Fi credentials
const char* ssid = "Wifi Name";
const char* password = "Wifi Password";

// MQTT Broker settings
const char* mqtt_server = "Server Ip";
const int mqtt_port = 1883;
const char* mqtt_topic = "esp/data";

// OLED settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

WiFiClient espClient;
PubSubClient client(espClient);

void displayData(float temperature, float humidity) {
  display.clearDisplay();
  
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("-- ESP32 Receiver --");
  
  display.setTextSize(2);
  display.setCursor(0, 20);
  display.print("T: ");
  display.print(temperature, 1);
  display.println(" C");
  
  display.setCursor(0, 44);
  display.print("H: ");
  display.print(humidity, 1);
  display.println(" %");
  
  display.display();
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.println("========================================");
  Serial.print("Message received: ");
  Serial.println(message);
  Serial.println("========================================");

  // Parse temperature and humidity from JSON
  float temperature = 0;
  float humidity = 0;

  int tempIndex = message.indexOf("temperature\":") + 13;
  int tempEnd = message.indexOf(",", tempIndex);
  temperature = message.substring(tempIndex, tempEnd).toFloat();

  int humIndex = message.indexOf("humidity\":") + 10;
  int humEnd = message.indexOf("}", humIndex);
  humidity = message.substring(humIndex, humEnd).toFloat();

  // Display on OLED
  displayData(temperature, humidity);
}

void setup_wifi() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Connecting to WiFi...");
  display.display();

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("WiFi Connected!");
  display.println(WiFi.localIP().toString());
  display.display();
  delay(1000);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Connecting MQTT...");
    display.display();
    
    String clientId = "ESP32_Receiver_";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(mqtt_topic);
      Serial.println("Subscribed to topic: esp/data");
      Serial.println("Receiver ready - waiting for messages...");

      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("MQTT Connected!");
      display.println("Waiting for data...");
      display.display();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Receiver with OLED");

  // Initialize I2C with custom pins
  Wire.begin(8, 9);  // SDA=8, SCL=9

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed!");
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("ESP32 Receiver");
  display.display();
  delay(1000);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
