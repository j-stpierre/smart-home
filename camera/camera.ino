#include "esp_camera.h"
#include <WiFi.h>
#include "camera_secrets.h"
#include <WebServer.h>

// Wi-Fi credentials
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

const String validToken = BEARER_TOKEN;
#define LED_PIN 4

// Create a web server on port 80
WebServer server(80);

void configureCameraSettings()
{
  sensor_t * s = esp_camera_sensor_get(); //see certs.h for more info
  s->set_brightness(s, 0);     // -2 to 2 **************************
  s->set_contrast(s, 0);       // -2 to 2
  s->set_saturation(s, 1);     // -2 to 2
  s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
  s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
  s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
  s->set_wb_mode(s, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
  s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
  s->set_aec2(s, 0);           // 0 = disable , 1 = enable
  s->set_ae_level(s, 0);       // -2 to 2
  s->set_aec_value(s, 300);    // 0 to 1200
  s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
  s->set_agc_gain(s, 30);       // 0 to 30
  s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
  s->set_bpc(s, 0);            // 0 = disable , 1 = enable
  s->set_wpc(s, 1);            // 0 = disable , 1 = enable
  s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
  s->set_lenc(s, 1);           // 0 = disable , 1 = enable
  s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
  s->set_vflip(s, 0);          // 0 = disable , 1 = enable
  s->set_dcw(s, 1);            // 0 = disable , 1 = enable
  s->set_colorbar(s, 0);       // 0 = disable , 1 = enable
}

// Camera configuration
void setupCamera() {

  pinMode(LED_PIN, OUTPUT);
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;
  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;
  config.pin_sccb_sda = 26;
  config.pin_sccb_scl = 27;
  config.pin_pwdn = 32;
  config.pin_reset = -1;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  config.frame_size = FRAMESIZE_QVGA; 
  config.jpeg_quality = 20;  // Higher number means lower quality
  config.fb_count = 2;
  config.grab_mode = CAMERA_GRAB_LATEST;

  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
  } else {
    configureCameraSettings();
  }
  return;
}

// Handle image capture when an HTTP request is made
void handle_capture() {
  if (!isBearerTokenValid()){
    server.send(401, "text/plain", "Unauthorized");
    return;
  }
  // digitalWrite(LED_PIN, HIGH);
  // delay(100);

  // Capture a frame
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    server.send(500, "text/plain", "Camera capture failed");
    return;
  }

  // Send HTTP response with captured image
  server.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0");
  server.sendHeader("Content-Type", "image/jpeg");
  server.send_P(200, "image/jpeg", (const char*)fb->buf, fb->len);
  
  // Release the frame buffer
  esp_camera_fb_return(fb);
  // digitalWrite(LED_PIN, LOW);
}

// Function to check for the Bearer token in the Authorization header
bool isBearerTokenValid() {
  if (server.hasHeader("Authorization")) {
    String authHeader = server.header("Authorization");

    // Check if the header starts with "Bearer " (case-sensitive)
    if (authHeader.startsWith("Bearer ")) {
      String token = authHeader.substring(7);  // Extract the token part
      return (token == validToken);            // Compare with the valid token
    }
  }
  return false;
}

void setup() {

  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  // Initialize the camera on request
  setupCamera();

  // Define route for capturing images
  server.on("/capture", HTTP_GET, handle_capture);

  // Start the server
  server.begin();
  Serial.println("Server started");
}

void loop() {
  // Handle incoming HTTP requests
  server.handleClient();
}
