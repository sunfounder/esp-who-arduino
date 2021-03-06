#include "who_camera.h"
#include <WiFi.h>
#include "camera_server.hpp"
#include "who_human_face_detection.hpp"

#define CAMERA_VERTICAL_FLIP 1

#define SSID "ESP32_CAM_TEST"
#define PASSWORD "12345678"

static QueueHandle_t xQueueAIFrame = NULL;
static QueueHandle_t xQueueHttpFrame = NULL;
static QueueHandle_t xQueueAIData = NULL;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");

  xQueueAIFrame = xQueueCreate(2, sizeof(camera_fb_t *));
  xQueueHttpFrame = xQueueCreate(2, sizeof(camera_fb_t *));
  xQueueAIData = xQueueCreate(2, sizeof(int) * 4);
    
  register_camera(
      PIXFORMAT_RGB565, FRAMESIZE_QVGA, 1, xQueueAIFrame, CAMERA_VERTICAL_FLIP,
      CAMERA_PIN_Y2, CAMERA_PIN_Y3, CAMERA_PIN_Y4, CAMERA_PIN_Y5, CAMERA_PIN_Y6,
      CAMERA_PIN_Y7, CAMERA_PIN_Y8, CAMERA_PIN_Y9, CAMERA_PIN_XCLK,
      CAMERA_PIN_PCLK, CAMERA_PIN_VSYNC, CAMERA_PIN_HREF, CAMERA_PIN_SIOD,
      CAMERA_PIN_SIOC, CAMERA_PIN_PWDN, CAMERA_PIN_RESET);
  register_human_face_detection(xQueueAIFrame, NULL, xQueueAIData, xQueueHttpFrame, true);
  register_httpd(xQueueHttpFrame, NULL, true);
}

void loop() {
  uint8_t data[4];
  if (xQueueReceive(xQueueAIData, &data, portMAX_DELAY)) {
    Serial.print("AI data: {");
    for (int i = 0; i < 4; i++) {
      Serial.print(data[i]);
      Serial.print(", ");
    }
    Serial.println("}");
  }
}
