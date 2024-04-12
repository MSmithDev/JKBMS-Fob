#include "gui_task.hpp"
#include "LovyanGFX.hpp"
#include <string>
#include "LGFX_Config.hpp"
#include "freertos/queue.h"
#include "esp_log.h"

#include <iostream>
#include <iomanip>
#include <sstream>

LGFX display; // Assuming display is defined elsewhere

static const char* TAG = "GUI_Task";

extern QueueHandle_t gui_data_queue;

const char* floatToString(float value) {
    static std::string str;  // Made static to prolong its lifetime beyond the function scope
    std::ostringstream out;
    out << std::fixed << std::setprecision(2) << value;
    str = out.str();
    return str.c_str();
}

//func to convert int to string
const char* intToString(int value) {
    static std::string str;  // Made static to prolong its lifetime beyond the function scope
    std::ostringstream out;
    out << value;
    str = out.str();
    return str.c_str();
}

unsigned int getBatteryColor(float voltage) {
    // Clamp the voltage to the range [68.00, 84.00]
    voltage = std::max(68.00f, std::min(84.00f, voltage));

    // Normalize the voltage to a [0, 1] range
    float normalized = (voltage - 68.00f) / (16.00f); // 84.00 - 68.00 = 16.00

    // Calculate the red and green components based on the voltage
    // Red decreases as the voltage increases, green increases as the voltage increases
    uint8_t red = static_cast<uint8_t>((1 - normalized) * 255);
    uint8_t green = static_cast<uint8_t>(normalized * 255);

    // Combine the components into a single unsigned integer
    // Format: 0xRRGGBB
    return (red << 16) | (green << 8);
}


void gui_task(void *pvParameters) {

    JKBMSData testRecv;


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

        if(xQueueReceive(gui_data_queue, &(testRecv), (TickType_t)5)){
            ESP_LOGI(TAG, "Got data from queue! Pack (V): %f Pack (W): %f, Cell 0 (V): %f", testRecv.packVoltage, testRecv.packPower, testRecv.cellVoltages[0]);

        }



        //set bg black
        bgSprite.fillSprite(TFT_BLACK);

        //draw pack voltage
        bgSprite.setTextColor(TFT_WHITE);
        bgSprite.setTextSize(1);
        bgSprite.drawString("Pack Voltage: ", 0, 0, 1);
        bgSprite.setTextColor(getBatteryColor(testRecv.packVoltage));
        bgSprite.drawString(floatToString(testRecv.packVoltage), 0, 20, 1);

        //draw pack power
        bgSprite.setTextColor(TFT_WHITE);
        bgSprite.setTextSize(1);
        bgSprite.drawString("Pack Power: ", 0, 40, 1);
        bgSprite.drawString(floatToString(testRecv.packPower), 0, 60, 1);

        //draw cell voltages
        bgSprite.setTextColor(TFT_WHITE);
        bgSprite.setTextSize(1);
        bgSprite.drawString("Cells: ", 120, 0, 1);
        bgSprite.setTextSize(1.1);
        
        //Cells 0-7
        for(int i = 0; i < 8; i++){
            bgSprite.drawString(floatToString(testRecv.cellVoltages[i]), 100, 20 + (i * 14), 1);
        }

        //Cells 8-15
        for(int i = 8; i < 16; i++){
            bgSprite.drawString(floatToString(testRecv.cellVoltages[i]), 140, 20 + ((i - 8) * 14), 1);
        }

        //Cells 16-23
        for(int i = 16; i < 24; i++){
            bgSprite.drawString(floatToString(testRecv.cellVoltages[i]), 180, 20 + ((i - 16) * 14), 1);
        }

        bgSprite.pushSprite(0, 0);



        //std::string s_angle = std::to_string(static_cast<int>(angle));
        // bgSprite.fillSprite(TFT_BLACK);
        // arcSprite.fillSprite(TFT_BLACK);
        // arcSprite.setColor(TFT_ORANGE);
        // arcSprite.fillArc(30, 30, 20, 29, 0, angle);
        // arcSprite.drawString(s_angle.c_str(), 20, 20, 2);
        // arcSprite.pushSprite(&bgSprite, 0, 0);
        // bgSprite.pushSprite(0, 0);
        
        // angle += 2;
        // if (angle > 360) {
        //     angle = 0;
        // }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
