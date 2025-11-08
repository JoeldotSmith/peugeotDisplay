#include <Arduino.h>
#include <esp_display_panel.hpp>
#include <lvgl.h>
#include "lv_conf.h"
#include "lvgl_v8_port.h"
#include "ui.h"


using namespace esp_panel::drivers;
using namespace esp_panel::board;


Board *board = nullptr;
static lv_chart_series_t *series;

void setup()
{
    Serial.begin(115200);
    Serial.println("Initializing board");
    
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

    if (ui_Image2 != NULL) {
        lv_img_set_angle(ui_Image2, angle*10); 

        angle += 1;
        if (angle >= 60) angle = -60;
    } else {
        Serial.println("ui_Image1 is NULL!");
    }
    
    delay(100);
}
