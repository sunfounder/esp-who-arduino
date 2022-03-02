#include "who_human_face_detection.hpp"

#include "esp_camera.h"

#include "esp-dl/include/image/dl_image.hpp"
#include "esp-dl/include/model_zoo/human_face_detect_msr01.hpp"
#include "esp-dl/include/model_zoo/human_face_detect_mnp01.hpp"

#include "who_ai_utils.hpp"

#define TWO_STAGE_ON 1

#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_ARDUHAL_ESP_LOG)
#include "esp32-hal-log.h"
#define TAG ""
#else
#include "esp_log.h"
static const char *TAG = "human_face_detection";
#endif


static QueueHandle_t xQueueFrameI = NULL;
static QueueHandle_t xQueueEvent = NULL;
static QueueHandle_t xQueueFrameO = NULL;
static QueueHandle_t xQueueResult = NULL;

static bool gEvent = true;
static bool gReturnFB = true;


static void task_process_handler(void *arg)
{
    camera_fb_t *frame = NULL;
    uint8_t result[14];
    HumanFaceDetectMSR01 detector(0.3F, 0.3F, 10, 0.3F);
#if TWO_STAGE_ON
    HumanFaceDetectMNP01 detector2(0.4F, 0.3F, 10);
#endif

    while (true)
    {
        if (gEvent)
        {
            if (xQueueReceive(xQueueFrameI, &frame, portMAX_DELAY))
            {
                ESP_LOGI(TAG, "detecting...");
#if TWO_STAGE_ON
                std::list<dl::detect::result_t> &detect_candidates = detector.infer((uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3});
                std::list<dl::detect::result_t> &detect_results = detector2.infer((uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3}, detect_candidates);
#else
                std::list<dl::detect::result_t> &detect_results = detector.infer((uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3});
#endif

                if (detect_results.size() > 0) {
                    ESP_LOGI(TAG, "detect_results.size() = %d", (int)detect_results.size());
                    draw_detection_result((uint16_t *)frame->buf, frame->height, frame->width, detect_results);
                    get_detection_result(detect_results, result);
                    print_detection_result(detect_results);
                    if (xQueueResult) {
                        xQueueSend(xQueueResult, &result, portMAX_DELAY);
                    }
                }
            }

            if (xQueueFrameO) {
                xQueueSend(xQueueFrameO, &frame, portMAX_DELAY);
            } else if (gReturnFB) {
                esp_camera_fb_return(frame);
            } else {
                free(frame);
            }
        }
    }
}

static void task_event_handler(void *arg)
{
    while (true)
    {
        xQueueReceive(xQueueEvent, &(gEvent), portMAX_DELAY);
    }
}

void register_human_face_detection(const QueueHandle_t frame_i,
                                   const QueueHandle_t event,
                                   const QueueHandle_t result,
                                   const QueueHandle_t frame_o,
                                   const bool camera_fb_return)
{
    xQueueFrameI = frame_i;
    xQueueFrameO = frame_o;
    xQueueEvent = event;
    xQueueResult = result;
    gReturnFB = camera_fb_return;

    xTaskCreatePinnedToCore(task_process_handler, TAG, 4 * 1024, NULL, 5, NULL, 0);
    if (xQueueEvent)
        xTaskCreatePinnedToCore(task_event_handler, TAG, 4 * 1024, NULL, 5, NULL, 1);
    ESP_LOGI(TAG, "register_human_face_detection");
}
