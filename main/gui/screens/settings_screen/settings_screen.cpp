#include "settings_screen.hpp"
#include "helpers/page_states.hpp"
#include "freertos/queue.h"
#include "esp_log.h"

#define TAG "settings_screen"

extern QueueHandle_t bleScan_data_queue;
BLEScan bleScan[20];

void settings_screen(LGFX_Sprite canvas, settingsPageState pageState)
{
    // if show options is true, display the options
    if(pageState.showOptions) {

    switch (pageState.currentSelection)
    {
    case 1:
        canvas.fillSprite(0x434343u); // Grey
        canvas.setTextColor(TFT_BLACK);
        canvas.setTextSize(2.75);
        canvas.drawString("BLE Setup", 40, 40);
        break;
    
    case 2:
        canvas.fillSprite(0x434343u); // Grey
        canvas.setTextColor(TFT_BLACK);
        canvas.setTextSize(2.75);
        canvas.drawString("Brightness", 40, 40);
        break;
    
    case 3:
        canvas.fillSprite(0x434343u); // Grey
        canvas.setTextColor(TFT_BLACK);
        canvas.setTextSize(2.75);
        canvas.drawString("Sleep Modes", 40, 40);
        break;

    case 4:
        canvas.fillSprite(0x434343u); // Grey
        canvas.setTextColor(TFT_BLACK);
        canvas.setTextSize(2.75);
        canvas.drawString("Back", 40, 40);
        break;
    
    };
    }
    else {
        canvas.fillSprite(0x434343u); // Grey
        canvas.setTextColor(TFT_BLACK);
        canvas.setTextSize(2.75);
        canvas.drawString("Settings", 40, 40);
    }

    //if (pageState.currentSelection == 1) do ble setup

    if(!pageState.showOptions && pageState.currentSelection == 11){

        if (xQueueReceive(bleScan_data_queue, &(bleScan), (TickType_t)5))
        {
            ESP_LOGI(TAG, "Got Scan Data");
        }

        //Show scan results
        canvas.fillSprite(0x434343u); // Grey
        canvas.setTextColor(TFT_BLACK);
        canvas.setTextSize(2.5);
        canvas.drawString("Scan", 0, 40);
        canvas.setTextSize(2);
        for (int i = 0; i < 20; i++)
        {
            canvas.drawString(bleScan[i].deviceName, 0, 60 + (i * 20));
        }
        

    }

}