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
    static uint32_t chart_last_update = 0;
    static int angle = -60;

    if (millis() - chart_last_update >= 100) {
        if (ui_Image2 != NULL) {
            // Check if the image is visible
            bool visible = !lv_obj_has_flag(ui_Image1, LV_OBJ_FLAG_HIDDEN);
            lv_coord_t x = lv_obj_get_x(ui_Image1);
            lv_coord_t y = lv_obj_get_y(ui_Image1);

            Serial.printf("angle: %d, visible: %d, pos: (%d, %d)\n", angle, visible, x, y);
            lv_img_set_angle(ui_Image2, angle*10); 

        } else {
            Serial.println("ui_Image1 is NULL!");
        }
        angle += 1;
        if (angle >= 60) angle = -60;

        chart_last_update = millis();
    }

    
    delay(5);
}
