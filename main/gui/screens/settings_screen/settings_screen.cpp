#include "settings_screen.hpp"
#include "freertos/queue.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "gui/widgets/widgets.hpp"

#define TAG "settings_screen"

extern QueueHandle_t bleScan_data_queue;

int bleScanDevices = 0;
BLEScan bleScan[20];

int selectedDevice = 0;

void handleNav(GlobalState *globalState, int *page, int prev, int next)
{
    if (globalState->downKey)
    {
        *page = next;
    }
    if (globalState->upKey)
    {
        *page = prev;
    }
}

void settings_screen(LGFX_Sprite canvas, GlobalState *globalState)
{

    // ESP_LOGI(TAG, "Devices found %d, selected Device %d", bleScanDevices, selectedDevice);
    //  Pages
    //   1 - BLE Setup
    //   2 - Brightness
    //   3 - Sleep Modes
    //   4 - Back

    // Listen for select key if not in settings
    if (!globalState->inSettings && globalState->selectKey)
    {
        globalState->inSettings = true;
        globalState->settingsPage = 1;

        globalState->selectKey = false; // stop multiple presses
    }

    if (!globalState->inSettings)
    {
        canvas.fillSprite(0x434343u); // Grey
        canvas.setTextColor(TFT_BLACK);
        canvas.setTextSize(2.75);
        canvas.drawString("Settings", 40, 40);
    }
    else
    {

        switch (globalState->settingsPage)
        {
        case 1:                           // Ble Setup
            canvas.fillSprite(0x434343u); // Grey
            canvas.setTextColor(TFT_BLACK);
            canvas.setTextSize(2.75);
            canvas.drawString("BLE Set", 40, 40);

            handleNav(globalState, &globalState->settingsPage, 4, 2);
            if (globalState->selectKey)
            {
                globalState->settingsPage = 11;
                globalState->selectKey = false;
            }
            break;

        case 2:                           // Brightness
            canvas.fillSprite(0x434343u); // Grey
            canvas.setTextColor(TFT_BLACK);
            canvas.setTextSize(2.75);
            canvas.drawString("Brightness", 40, 40);

            handleNav(globalState, &globalState->settingsPage, 1, 3);
            break;

        case 3:                           // Sleep Modes
            canvas.fillSprite(0x434343u); // Grey
            canvas.setTextColor(TFT_BLACK);
            canvas.setTextSize(2.75);
            canvas.drawString("Sleep Modes", 40, 40);

            handleNav(globalState, &globalState->settingsPage, 2, 4);
            break;

        case 4:                           // Back
            canvas.fillSprite(0x434343u); // Grey
            canvas.setTextColor(TFT_BLACK);
            canvas.setTextSize(2.75);
            canvas.drawString("Back", 40, 40);

            handleNav(globalState, &globalState->settingsPage, 3, 1);

            if (globalState->selectKey)
            {
                globalState->inSettings = false;
                globalState->settingsPage = 0;
            }
            break;

        case 11: // Ble Setup
            // recv ble scan data
            if (xQueueReceive(bleScan_data_queue, &(bleScan), (TickType_t)5))
            {
                ESP_LOGI(TAG, "Got Scan Data");
                bleScanDevices = 0;
                // check how many devices are found
                for (int i = 0; i < 20; i++)
                {
                    if (strlen(bleScan[i].deviceName) > 0)
                    {
                        bleScanDevices++;
                    }
                }
            }
            canvas.setTextSize(1.75);
            canvas.setTextDatum(TC_DATUM);
            canvas.drawString("Select BMS Device", 135, 25);
            canvas.setTextDatum(TL_DATUM);
            UIWidgets::menuBoxRssi(canvas, 50, 40, 180, 90, selectedDevice, bleScanDevices, bleScan);

            if (globalState->selectKey)
            {
                if (selectedDevice == 0)
                {
                    globalState->settingsPage = 1;
                    globalState->selectKey = false;
                }
                else
                {
                    globalState->selectKey = false;

                    // save selected device to NVS
                    nvs_handle_t my_handle;
                    esp_err_t err;
                    err = nvs_open("storage", NVS_READWRITE, &my_handle);
                    if (err != ESP_OK)
                    {
                        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
                    }
                    else
                    {
                        err = nvs_set_str(my_handle, "bleDeviceName", bleScan[selectedDevice].deviceName);
                        if (err != ESP_OK)
                        {
                            ESP_LOGI(TAG, "Error (%s) writing!", esp_err_to_name(err));
                        }
                        nvs_close(my_handle);
                    }
                    ESP_LOGI(TAG, "Selected Device: %s Saved!", bleScan[selectedDevice].deviceName);

                    // restarting
                    esp_restart();
                }
            }

            if (globalState->downKey)
            {
                selectedDevice++;
                if (selectedDevice >= bleScanDevices)
                {
                    selectedDevice = bleScanDevices - 1;
                }
            }
            if (globalState->upKey)
            {
                selectedDevice--;
                if (selectedDevice < 0)
                {
                    selectedDevice = 0;
                }
            }

            break;
        }
    }
}