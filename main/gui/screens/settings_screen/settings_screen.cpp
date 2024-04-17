#include "settings_screen.hpp"
#include "helpers/page_states.hpp"
#include "freertos/queue.h"
#include "esp_log.h"
#include "nvs_flash.h"

#define TAG "settings_screen"

extern QueueHandle_t bleScan_data_queue;
BLEScan bleScan[20];

void settings_screen(LGFX_Sprite canvas, GlobalState &globalState)
{

    switch (globalState.settingsPage)
    {
    case 0:
        if(globalState.selectKey)
        {
            globalState.inSettings = true;
            globalState.CurrentScreen = 3;
            globalState.settingsPage = 1;
            globalState.bleSetup = 0;
        }
        canvas.fillSprite(0x434343u); // Grey
        canvas.setTextColor(TFT_BLACK);
        canvas.setTextSize(2.75);
        canvas.drawString("Settings", 40, 40);
        break;

    case 1: // BLE Setup

        if(globalState.downKey && globalState.bleSetup == 0)
        {
            globalState.settingsPage = 2;
        }

        if(globalState.selectKey && globalState.bleSetup == 0)
        {
            globalState.inSettings = true;
            globalState.CurrentScreen = 3;
            globalState.settingsPage = 1;
            globalState.bleSetup = 1;
            globalState.selectKey = false;
        }
        
        if(globalState.bleSetup == 1) {

            if (xQueueReceive(bleScan_data_queue, &(bleScan), (TickType_t)5))
        {
            ESP_LOGI(TAG, "Got Scan Data");
        }

        //Show scan results
        canvas.fillSprite(0x434343u); // Grey
        canvas.setTextColor(TFT_BLACK);
        canvas.setTextSize(2.5);
        canvas.drawString("Scanning:", 35, 0);
        canvas.setTextSize(2);
        
        if(globalState.bleSelectedDevice == -1)
        {
            canvas.setTextColor(TFT_BLACK, TFT_ORANGE);
            canvas.drawString("...", 50, 10 + (1 * 20));
            canvas.setTextColor(TFT_BLACK);
            canvas.drawString(bleScan[globalState.bleSelectedDevice+1].deviceName, 50, 10 + (2 * 20));
            canvas.drawString(bleScan[globalState.bleSelectedDevice+2].deviceName, 50, 10 + (3 * 20));
        }
        if(globalState.bleSelectedDevice >= 0)
        {
            if(globalState.bleSelectedDevice == 0)
            {
                canvas.drawString("...", 50, 10 + (1 * 20));
                canvas.setTextColor(TFT_BLACK, TFT_ORANGE);
                canvas.drawString(bleScan[globalState.bleSelectedDevice].deviceName, 50, 10 + (2 * 20));
                canvas.setTextColor(TFT_BLACK);
                canvas.drawString(bleScan[globalState.bleSelectedDevice+1].deviceName, 50, 10 + (3 * 20));
                canvas.drawString(bleScan[globalState.bleSelectedDevice+2].deviceName, 50, 10 + (4 * 20));

            }
            else {
                canvas.drawString("...", 50, 10 + (1 * 20));
                canvas.drawString(bleScan[globalState.bleSelectedDevice-1].deviceName, 50, 10 + (2 * 20));
                canvas.setTextColor(TFT_BLACK, TFT_ORANGE);
                canvas.drawString(bleScan[globalState.bleSelectedDevice].deviceName, 50, 10 + (3 * 20));
                canvas.setTextColor(TFT_BLACK);
                canvas.drawString(bleScan[globalState.bleSelectedDevice+1].deviceName, 50, 10 + (4 * 20));
            }

            if(globalState.selectKey) {
                nvs_handle_t my_handle;
                esp_err_t err;
                err = nvs_open("storage", NVS_READWRITE, &my_handle);
                if (err != ESP_OK) {
                    ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
                } else {
                    err = nvs_set_str(my_handle, "bleDeviceName", bleScan[globalState.bleSelectedDevice].deviceName);
                    if (err != ESP_OK) {
                        ESP_LOGI(TAG, "Error (%s) writing!", esp_err_to_name(err));
                    }
                    nvs_close(my_handle);
                
                }
                ESP_LOGI(TAG, "Selected Device: %s Saved!", bleScan[globalState.bleSelectedDevice].deviceName);
                //restarting
                esp_restart();
                
            }
        }
        

        if(globalState.downKey)
        {
            globalState.bleSelectedDevice++;
            if(globalState.bleSelectedDevice > 20)
            {
                globalState.bleSelectedDevice = 20;
            }
        }
        if(globalState.upKey)
        {
            globalState.bleSelectedDevice--;
            if(globalState.bleSelectedDevice < -1)
            {
                globalState.bleSelectedDevice = -1;
            }
        }

    


        }
        else{
        canvas.fillSprite(0x434343u); // Grey
        canvas.setTextColor(TFT_BLACK);
        canvas.setTextSize(2.75);
        canvas.drawString("Ble Setup", 40, 40);
        }
        break;

        case 2: //Brightness

        if(globalState.downKey)
        {
            globalState.settingsPage = 3;
        }
        if(globalState.upKey)
        {
            globalState.settingsPage = 1;
        }
        

        canvas.fillSprite(0x434343u); // Grey
        canvas.setTextColor(TFT_BLACK);
        canvas.setTextSize(2.75);
        canvas.drawString("Brightness", 40, 40);
        break;

        case 3: //Sleep Modes

        if(globalState.downKey)
        {
            globalState.settingsPage = 4;
        }
        if(globalState.upKey)
        {
            globalState.settingsPage = 2;
        }

        canvas.fillSprite(0x434343u); // Grey
        canvas.setTextColor(TFT_BLACK);
        canvas.setTextSize(2.75);
        canvas.drawString("Sleep Modes", 40, 40);
        break;

        case 4: //Back

        if(globalState.downKey)
        {
            globalState.settingsPage = 4;
        }
        if(globalState.upKey)
        {
            globalState.settingsPage = 3;
        }

        //if selected, go back to main screen
        if(globalState.selectKey)
        {
            globalState.inSettings = false;
            globalState.CurrentScreen = 3;
            globalState.settingsPage = 0;
        }

        canvas.fillSprite(0x434343u); // Grey
        canvas.setTextColor(TFT_BLACK);
        canvas.setTextSize(2.75);
        canvas.drawString("Back", 40, 40);

    }


    // if show options is true, display the options
    // if(pageState.showOptions) {

    // switch (pageState.currentSelection)
    // {
    // case 1:
    //     canvas.fillSprite(0x434343u); // Grey
    //     canvas.setTextColor(TFT_BLACK);
    //     canvas.setTextSize(2.75);
    //     canvas.drawString("BLE Setup", 40, 40);
    //     break;
    
    // case 2:
    //     canvas.fillSprite(0x434343u); // Grey
    //     canvas.setTextColor(TFT_BLACK);
    //     canvas.setTextSize(2.75);
    //     canvas.drawString("Brightness", 40, 40);
    //     break;
    
    // case 3:
    //     canvas.fillSprite(0x434343u); // Grey
    //     canvas.setTextColor(TFT_BLACK);
    //     canvas.setTextSize(2.75);
    //     canvas.drawString("Sleep Modes", 40, 40);
    //     break;

    // case 4:
    //     canvas.fillSprite(0x434343u); // Grey
    //     canvas.setTextColor(TFT_BLACK);
    //     canvas.setTextSize(2.75);
    //     canvas.drawString("Back", 40, 40);
    //     break;
    
    // };
    // }
    // else {
    //     canvas.fillSprite(0x434343u); // Grey
    //     canvas.setTextColor(TFT_BLACK);
    //     canvas.setTextSize(2.75);
    //     canvas.drawString("Settings", 40, 40);
    // }

    //if (pageState.currentSelection == 1) do ble setup

    // if(!pageState.showOptions && pageState.currentSelection == 11){

        // if (xQueueReceive(bleScan_data_queue, &(bleScan), (TickType_t)5))
        // {
        //     ESP_LOGI(TAG, "Got Scan Data");
        // }

        // //Show scan results
        // canvas.fillSprite(0x434343u); // Grey
        // canvas.setTextColor(TFT_BLACK);
        // canvas.setTextSize(2.5);
        // canvas.drawString("Scan", 0, 40);
        // canvas.setTextSize(2);
        // for (int i = 0; i < 20; i++)
        // {
        //     canvas.drawString(bleScan[i].deviceName, 0, 60 + (i * 20));
        // }
        

    // }

}