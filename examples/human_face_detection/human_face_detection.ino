#include "who_camera.h"
#include <WiFi.h>
#include "camera_server.hpp"
#include "who_human_face_detection.hpp"

#define CAMERA_VERTICAL_FLIP 1
#define CAMERA_PIN_PWDN 26
#define CAMERA_PIN_RESET -1
#define CAMERA_PIN_XCLK 32
#define CAMERA_PIN_SIOD 13
#define CAMERA_PIN_SIOC 12
#define CAMERA_PIN_VSYNC 27
#define CAMERA_PIN_HREF 25
#define CAMERA_PIN_PCLK 19
#define CAMERA_PIN_Y2 5
#define CAMERA_PIN_Y3 14
#define CAMERA_PIN_Y4 4
#define CAMERA_PIN_Y5 15
#define CAMERA_PIN_Y6 18
#define CAMERA_PIN_Y7 23
#define CAMERA_PIN_Y8 36
#define CAMERA_PIN_Y9 39

static QueueHandle_t xQueueAIFrame = NULL;
static QueueHandle_t xQueueHttpFrame = NULL;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  WiFi.begin("MakerStarsHall", "sunfounder");

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
    
  register_camera(
      PIXFORMAT_JPEG, FRAMESIZE_QVGA, 1, xQueueAIFrame, CAMERA_VERTICAL_FLIP,
      CAMERA_PIN_Y2, CAMERA_PIN_Y3, CAMERA_PIN_Y4, CAMERA_PIN_Y5, CAMERA_PIN_Y6,
      CAMERA_PIN_Y7, CAMERA_PIN_Y8, CAMERA_PIN_Y9, CAMERA_PIN_XCLK,
      CAMERA_PIN_PCLK, CAMERA_PIN_VSYNC, CAMERA_PIN_HREF, CAMERA_PIN_SIOD,
      CAMERA_PIN_SIOC, CAMERA_PIN_PWDN, CAMERA_PIN_RESET);
  register_human_face_detection(xQueueAIFrame, NULL, NULL, xQueueHttpFrame);
  register_httpd(xQueueHttpFrame, NULL, true);
}

void loop() {
}
