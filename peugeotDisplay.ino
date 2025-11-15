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

bool inited = false;

struct Data {
  float voltage;
  float boost;
  float coolantTemp;
};
volatile Data latest;
volatile Data current;
volatile bool dataReady = false;


void handle_boost(){ 
  lv_chart_set_next_value(ui_Chart1, series, (int)(latest.boost * 10));

  // needle
  if (current.boost != latest.boost) {
    lv_img_set_angle(ui_Image2, (int)(latest.boost * 120 - 600));
  }

} 

void handle_voltage(){ 
  if (current.voltage != latest.voltage) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%.1fV", latest.voltage);
    lv_label_set_text(ui_Label1, buf);
  }
} 

void handle_coolant(){ 
  if (current.coolantTemp != latest.coolantTemp) {
    char buf1[16];
    snprintf(buf1, sizeof(buf1), "%.1f C", latest.coolantTemp);
    lv_label_set_text(ui_Label2, buf1);
  }
} 

void onReceive(const esp_now_recv_info *info, const uint8_t *data, int len) {
  if (sscanf((const char*)data, "%f-%f-%f", &latest.voltage, &latest.coolantTemp, &latest.boost) == 3)
    dataReady = true;
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
    inited = true;
}

void loop() {
  lvgl_port_lock(-1);
  if (dataReady) {
    dataReady = false;

    handle_boost();
    handle_voltage();
    handle_coolant();

    memcpy((void*)&current, (const void*)&latest, sizeof(Data));
  }
  lvgl_port_unlock();

  delay(5);
}
