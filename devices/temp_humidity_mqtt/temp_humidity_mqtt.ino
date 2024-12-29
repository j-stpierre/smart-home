#include <DHT.h>
#include <WiFi.h>
#include <esp_http_server.h>
#include <sensor_secrets.h>
#include <esp_log.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>

// Wi-Fi credentials
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// MQTT Broker
const char *mqtt_broker = "192.168.2.68";
const char *topic = "device/telemetry";
const char *mqtt_username = MQTT_USERNAME;
const char *mqtt_password = MQTT_PASSWORD;
const int mqtt_port = 30918;
String clientId;

//Logging
static const char *TAG = "TEMP-HUM_DEVICE";

//Sensor
#define DHTPIN 2       // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22  // DHT 22
DHT dht(DHTPIN, DHTTYPE);

//Endoint/MQTT
WiFiClient espClient;
PubSubClient client(espClient);

const int measureInterval = 30;
time_t lastCheck;

// NTP server
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0; // UTC offset in seconds
const int daylightOffset_sec = 3600; // Daylight saving time offset

void setup() {
  Serial.begin(115200);
  esp_log_level_set("*", ESP_LOG_INFO);

  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Initialize and sync time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  lastCheck = time(nullptr);

  start_webserver();

  //connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
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
  // client.subscribe(topic);

  // Initialize device.
  dht.begin();
  Serial.print("Sensor Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}

void loop() {
  client.loop();
  time_t time = ::time(nullptr);

  if ((time - lastCheck) > measureInterval) {
    sendTelemetry();
    lastCheck = time;
  }
}

// Endpoint 
esp_err_t get_handler(httpd_req_t *req) {

  const char *resp = buildPayload(); 
  ESP_LOGI(TAG, "Response sent: %s", resp);
  httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

httpd_uri_t uri_get = {
  .uri = "/reading",
  .method = HTTP_GET,
  .handler = get_handler,
  .user_ctx = NULL
};

httpd_handle_t start_webserver(void) {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  httpd_handle_t server = NULL;

  /* Start the httpd server */
  if (httpd_start(&server, &config) == ESP_OK) {
    /* Register URI handlers */
    httpd_register_uri_handler(server, &uri_get);
  }
  return server;
}

void stop_webserver(httpd_handle_t server) {
  if (server) {
    httpd_stop(server);
  }
}

// MQTT
// Add when need to send message to device
void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
      Serial.print((char) payload[i]);
  }
  Serial.println();
  Serial.println("-----------------------");
}

void sendTelemetry() {
  const char *payload = buildPayload();  

  //TODO handle error in reading

  client.publish(topic, payload);
  ESP_LOGI(TAG, "Published: %s", payload);
}

const char* buildPayload() {


  // static char resp[256];
  String deviceId = "F09E9E3B540C";
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  time_t epochTime = time(nullptr);
  struct tm *currentTime = localtime(&epochTime);

  // Format the time as a human-readable string
  char timeString[30];
  strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", currentTime);

  StaticJsonDocument<200> doc;
  doc["deviceId"] = "F09E9E3B540C";
  doc["timestamp-device"] = timeString;

  if (isnan(temperature) || isnan(humidity)) {
    doc["temperature"] = "error";
    doc["humidity"] = "error";
  } else {
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
  }
  static char resp[256];
  serializeJson(doc, resp);
  return resp;
}
