#include <Wire.h>
#include <Adafruit_PM25AQI.h>
#include <Adafruit_SGP30.h>
#include <SensirionI2cSht4x.h>
#include <ArduinoJson.h>
#include <time.h>
#include <sensor_secrets.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <esp_log.h>


// Wi-Fi credentials
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// Wi-Fi Logging
const char* udpAddress = "192.168.2.10"; // Replace with your computer's IP
const int udpPort = 1234;
WiFiUDP udp;

// MQTT Broker
const char *mqtt_broker = "192.168.2.68";
const char *topic = "device/telemetry";
const char *mqtt_username = MQTT_USERNAME;
const char *mqtt_password = MQTT_PASSWORD;
const int mqtt_port = 30918;
String clientId;

//Logging
static const char *TAG = "AIR_DEVICE";

//Endoint/MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Create sensor objects
Adafruit_SGP30 sgp30;
// Adafruit_SHT4x sht4;
Adafruit_PM25AQI aqi = Adafruit_PM25AQI();

const float OFFSET = 4.0;

#ifdef NO_ERROR
#undef NO_ERROR
#endif
#define NO_ERROR 0
SensirionI2cSht4x sensor;
static char errorMessage[64];
static int16_t error;

const int measureInterval = 30;
time_t lastCheck;

// NTP server
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0; // UTC offset in seconds
const int daylightOffset_sec = 3600; // Daylight saving time offset

void setup() {
  Serial.begin(115200); // Start serial communication

  esp_log_level_set("*", ESP_LOG_INFO);

  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  sendUDPLog("WiFi connected");


  // Initialize and sync time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // while (!Serial) delay(10);
  Wire.begin();         // Initialize I²C bus

  sendUDPLog("Wire Begin");


  // Wait three seconds for sensor to boot up!
  delay(3000);

  lastCheck = time(nullptr);

  // Initialize SGP30 sensor
  if (!sgp30.begin()) {
    Serial.println("SGP30 not detected. Check wiring!");
    while (1); // Stop execution if sensor is not found
  }
  Serial.println("SGP30 initialized.");

  sensor.begin(Wire, SHT40_I2C_ADDR_44);

  sensor.softReset();
  delay(10);
  uint32_t serialNumber = 0;
  error = sensor.serialNumber(serialNumber);
  if (error != NO_ERROR) {
      Serial.print("Error trying to execute serialNumber(): ");
      errorToString(error, errorMessage, sizeof errorMessage);
      Serial.println(errorMessage);
      return;
  }
  Serial.print("serialNumber: ");
  Serial.print(serialNumber);
  Serial.println();
  Serial.println("SHT4 initialized.");

  // Initialize SGP30 air quality baseline
  if (!sgp30.IAQinit()) {
    Serial.println("Failed to initialize IAQ for SGP30.");
  }

  Serial1.begin(9600, SERIAL_8N1, 44,43);

  // There are 3 options for connectivity!
  //if (! aqi.begin_I2C()) {      // connect to the sensor over I2C
  if (! aqi.begin_UART(&Serial1)) { // connect to the sensor over hardware serial
  //if (! aqi.begin_UART(&pmSerial)) { // connect to the sensor over software serial 
    Serial.println("Could not find PM 2.5 sensor!");
    while (1) delay(10);
  }

  //connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setBufferSize(1024);
  while (!client.connected()) {
      clientId = "esp32-client-";
      clientId += String(WiFi.macAddress());
      Serial.printf("The client %s connects to the public MQTT broker\n", clientId.c_str());
      if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
          Serial.println("Public MQTT broker connected");
      } else {
          Serial.print("failed with state ");
          Serial.print(client.state());
          delay(2000);
      }
  }
  Serial.print("Sensor Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");

  sendUDPLog("Sensor Ready!");

}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  time_t time = ::time(nullptr);

  if ((time - lastCheck) > measureInterval) {
    sendTelemetry();
    lastCheck = time;
  }
}


void sendTelemetry() {
  const char *payload = buildPayload();  

  Serial.println(payload);

  if (client.publish(topic, payload)) {
    Serial.println("Message published successfully!");
    ESP_LOGI(TAG, "Published: %s", payload);
  } else {
    Serial.println("Failed to publish message. Check the connection or topic.");
  }
  
}

const char* buildPayload() {

  StaticJsonDocument<300> doc;
  
  time_t epochTime = time(nullptr);
  struct tm *currentTime = localtime(&epochTime);

  // Format the time as a human-readable string
  char timeString[30];
  strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", currentTime);

  doc["deviceId"] = String(WiFi.macAddress());
  doc["timestamp-device"] = timeString;

  float aTemperature = 0.0;
  float aHumidity = 0.0;
  delay(20);
  error = sensor.measureMediumPrecision(aTemperature, aHumidity);
  if (error != NO_ERROR) {
    Serial.print("Error trying to execute measureLowestPrecision(): ");
    errorToString(error, errorMessage, sizeof errorMessage);
    Serial.println(errorMessage);
    doc["temperature"] = NULL;
    doc["humidity"] = NULL;
  } else {
    doc["temperature"] = aTemperature - OFFSET;
    doc["humidity"] = aHumidity;
  }

  PM25_AQI_Data data;

  doc["pm10s"] = -1;
  doc["pm25s"] = -1;
  doc["pm100s"] = -1;
  doc["pm10e"] = -1;
  doc["pm25e"] = -1;
  doc["pm100e"] = -1;
  doc["particles03"] = -1;
  doc["particles05"] = -1;
  doc["particles10"] = -1;
  doc["particles25"] = -1;
  doc["particles50"] = -1;
  doc["particles100"] = -1;
  
  if (aqi.read(&data)) {
    Serial.println("AQI reading success");
    doc["pm10s"] = data.pm10_standard;
    doc["pm25s"] = data.pm25_standard;
    doc["pm100s"] = data.pm100_standard;
    doc["pm10e"] = data.pm10_env;
    doc["pm25e"] = data.pm25_env;
    doc["pm100e"] = data.pm100_env;
    doc["particles03"] = data.particles_03um;
    doc["particles05"] = data.particles_05um;
    doc["particles10"] = data.particles_10um;
    doc["particles25"] = data.particles_25um;
    doc["particles50"] = data.particles_25um;
    doc["particles100"] = data.particles_100um;
  }

  // Read SGP30 data
  doc["eCO2"] = -1; // ppm
  doc["TVOC"] = -1; // ppb

  if (sgp30.IAQmeasure()) {
    doc["eCO2"] = sgp30.eCO2;
    doc["TVOC"] = sgp30.TVOC;
  } 
  
  static char resp[400];
  serializeJson(doc, resp);
  return resp;
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    clientId = "esp32-client-";
    clientId += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public MQTT broker\n", clientId.c_str());
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
        Serial.println("Public MQTT broker connected");
    } else {
        Serial.print("failed with state ");
        Serial.print(client.state());
        delay(2000);
    }
  }
}

void sendUDPLog(String logMessage) {
  udp.beginPacket(udpAddress, udpPort);
  udp.write((uint8_t*)logMessage.c_str(), logMessage.length());
  udp.endPacket();
}


