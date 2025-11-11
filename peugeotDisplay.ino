#define CONFIG_LVGL_PORT_ROTATION_DEGREE 0
#include <Arduino.h>
#include <esp_display_panel.hpp>
#include <lvgl.h>
#include "lv_conf.h"
#include "lvgl_v8_port.h"
#include "ui.h"

#include <esp_now.h>
#include <WiFi.h>


using namespace esp_panel::drivers;
using namespace esp_panel::board;


Board *board = nullptr;
static lv_chart_series_t *series;


void onReceive(const esp_now_recv_info *info, const uint8_t *data, int len) {
  char msg[len + 1];
  memcpy(msg, data, len);
  msg[len] = 0;
  Serial.printf("Received from %02X:%02X:%02X:%02X:%02X:%02X => %s\n",
                info->src_addr[0], info->src_addr[1], info->src_addr[2],
                info->src_addr[3], info->src_addr[4], info->src_addr[5], msg);
}

void setup()
{
    Serial.begin(115200);

    Serial.println("Initializing board");

    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) {
      Serial.println("ESP-NOW init failed");
      return;
    }
    esp_now_register_recv_cb(onReceive);
    
    board = new Board();
    board->init();
    
    #if LVGL_PORT_AVOID_TEARING_MODE
      auto lcd = board->getLCD();
      lcd->configFrameBufferNumber(LVGL_PORT_DISP_BUFFER_NUM);
      #if ESP_PANEL_DRIVERS_BUS_ENABLE_RGB && CONFIG_IDF_TARGET_ESP32S3
          auto lcd_bus = lcd->getBus();
          if (lcd_bus->getBasicAttributes().type == ESP_PANEL_BUS_TYPE_RGB) {
              static_cast<BusRGB *>(lcd_bus)->configRGB_BounceBufferSize(lcd->getFrameWidth() * 10);
          }
      #endif
    #endif
    
    assert(board->begin());
    
    Serial.println("Initializing LVGL");
    lvgl_port_init(board->getLCD(), board->getTouch());
    
    lvgl_port_lock(-1);

    ui_init();
    series = lv_chart_get_series_next(ui_Chart1, NULL);
    
    lvgl_port_unlock();
    
    Serial.println("Setup complete");
}

void loop() {
    static int angle = -60;
    static unsigned long pauseStartTime = 0;
    static bool isPaused = false;
    
    if (angle <= 30) {
        isPaused = false;  // Reset pause state when animating
        
        if (ui_Image2 != NULL) {
            lvgl_port_lock(-1);
            lv_img_set_angle(ui_Image2, angle*10); 
            lv_chart_set_next_value(ui_Chart1, series, (int)(0.5*angle+60));
            lvgl_port_unlock();
            angle += 1;
            
            if (angle > 30) {
                // Start the 10 second pause
                isPaused = true;
                pauseStartTime = millis();
            }
        } else {
            Serial.println("ui_Image2 is NULL!");
        }
    } else {
        // Non-blocking 10 second wait
        if (!isPaused) {
            // Just started pausing
            isPaused = true;
            pauseStartTime = millis();
        }
        
        if (millis() - pauseStartTime >= 10000) {
            // 10 seconds have passed, reset angle
            angle = -60;
            isPaused = false;
        }
    }
}
