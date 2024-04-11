#include "gui_task.hpp"
#include "LovyanGFX.hpp"
#include <string>
#include "LGFX_Config.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

LGFX display; // Assuming display is defined elsewhere

void gui_task(void *pvParameters) {
    display.init();
    display.fillScreen(TFT_BLACK);
    float angle = 0;

    LGFX_Sprite arcSprite(&display);
    LGFX_Sprite bgSprite(&display);

    arcSprite.createSprite(60, 60);
    bgSprite.createSprite(240, 135);
    display.setSwapBytes(true);

    //Splash Screen
    bgSprite.fillSprite(TFT_BLACK);
    bgSprite.setTextSize(2);
    bgSprite.drawString("Splash Screen", 0, 0, 2);
    bgSprite.pushSprite(0, 0);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    
    //Main GUI Loop
    while (1) {
        std::string s_angle = std::to_string(static_cast<int>(angle));

        bgSprite.fillSprite(TFT_BLACK);
        arcSprite.fillSprite(TFT_BLACK);
        arcSprite.setColor(TFT_ORANGE);
        arcSprite.fillArc(30, 30, 20, 29, 0, angle);
        arcSprite.drawString(s_angle.c_str(), 20, 20, 2);
        arcSprite.pushSprite(&bgSprite, 0, 0);
        bgSprite.pushSprite(0, 0);
        
        angle += 2;
        if (angle > 360) {
            angle = 0;
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
