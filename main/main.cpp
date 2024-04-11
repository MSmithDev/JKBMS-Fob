#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "LovyanGFX.hpp"
#include "LGFX_Config.hpp"
#include <iostream>
#include <string>

LGFX display;

// Function that will be executed as a task
void gui_task(void *pvParameters)
{

    display.init();
    display.fillScreen(TFT_BLACK);
    uint32_t count = ~0;

    float angle = 0;

    static LGFX_Sprite arcSprite(&display);
    static LGFX_Sprite bgSprite(&display);

    arcSprite.createSprite(60, 60);
    bgSprite.createSprite(240, 135);
    display.setSwapBytes(true);

    //Splash Screen
    bgSprite.fillSprite(TFT_BLACK);
    bgSprite.setTextSize(2);
    bgSprite.drawString("Splash Screen", 0, 0, 2);
    bgSprite.pushSprite(0, 0);
    // A delay to show the splash screen
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    
    //Main GUI Loop
    while (1)
    {

        std::string s_angle = std::to_string(static_cast<int>(angle));

        bgSprite.fillSprite(TFT_BLACK);
        arcSprite.fillSprite(TFT_BLACK);
        arcSprite.setColor(TFT_ORANGE);
        arcSprite.fillArc(30, 30, 20, 29, 0, angle);
        arcSprite.drawString(s_angle.c_str(), 20, 20, 2);
        arcSprite.pushSprite(&bgSprite, 0, 0);
        bgSprite.pushSprite(0, 0);

        
        angle=angle+2;
        if (angle > 360)
        {
            angle = 0;
        }

        // A delay to prevent the task from consuming all CPU time
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

extern "C" void app_main()
{
    //Enable Power to LCD
    gpio_set_direction(GPIO_NUM_7, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_7, 1);

    //Create a task to run the GUI
    //xTaskCreate(gui_task, "gui_task", 4096, NULL, 1, NULL);
    xTaskCreatePinnedToCore(gui_task, "gui_task", 4096, NULL, 1, NULL, 1);
}
