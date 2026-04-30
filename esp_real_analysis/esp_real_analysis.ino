#include "esp_camera.h"
#include "SD_MMC.h"
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Sap2021";
const char* password = "10108683";
const char* serverIP = "192.168.1.8";

String serverURL = "http://" + String(serverIP) + ":5000/upload";
String websiteURL = "http://" + String(serverIP) + "/greenhouse/";

// pic counter
int imageCount = 0;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(".");
  }

Serial.println("WiFi Connected");
  // starting cam
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
  config.jpeg_quality = 12;
  config.fb_count = 1;

  esp_camera_init(&config);

  // starting SD Card
  if (!SD_MMC.begin()) {
    Serial.println("SD Card Mount Failed");
    return;
  }

  Serial.println("SD Ready");
}

String arduinoData = "";

void loop() {
  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    if (msg.startsWith("SENSORS:")) {
      arduinoData = msg.substring(8);
      Serial.println("Got Sensors: " + arduinoData);
    }
  }

  // Capture Image
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  // 1. Upload to AI Server
  HTTPClient http;
  http.begin(serverURL);
  http.addHeader("Content-Type", "application/octet-stream");
  int httpResponseCode = http.POST(fb->buf, fb->len);
  
  if (httpResponseCode > 0) {
    String result = http.getString();
    Serial.println("AI Result: " + result);
    // Send health status to Arduino
    Serial.println("PLANT_HEALTH:" + result);
  } else {
    Serial.println("Error on AI upload: " + String(httpResponseCode));
  }
  http.end();

  // 2. Save to SD Card
  String path = "/image_" + String(imageCount++) + ".jpg";
  File file = SD_MMC.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
  } else {
    file.write(fb->buf, fb->len);
    file.close();
    Serial.println("Saved to SD: " + path);
  }
  esp_camera_fb_return(fb);

  // 3. Update Website Sensors
  if (arduinoData.length() > 0) {
    int firstComma = arduinoData.indexOf(',');
    int secondComma = arduinoData.indexOf(',', firstComma + 1);
    int thirdComma = arduinoData.indexOf(',', secondComma + 1);

    String temp = arduinoData.substring(0, firstComma);
    String hum = arduinoData.substring(firstComma + 1, secondComma);
    String soil = arduinoData.substring(secondComma + 1, thirdComma);
    String light = arduinoData.substring(thirdComma + 1);

    http.begin(websiteURL + "updateSensors.php");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String postData = "temperature=" + temp + "&humidity=" + hum + "&soil_moisture=" + soil + "&light=" + light;
    int webResponse = http.POST(postData);
    http.end();
  }

  // 4. Get Current Plant from Website
  http.begin(websiteURL + "getCurrentPlant.php");
  int getCode = http.GET();
  if (getCode > 0) {
    String payload = http.getString();
    // Assuming JSON like {"plant_name":"BASIL"}
    if (payload.indexOf("plant_name") != -1) {
       int start = payload.indexOf("plant_name\":\"") + 13;
       int end = payload.indexOf("\"", start);
       String plantName = payload.substring(start, end);
       Serial.println("SET_PLANT:" + plantName); // Send to Arduino
    }
  }
  http.end();

  delay(30000); // 30 second cycle
}