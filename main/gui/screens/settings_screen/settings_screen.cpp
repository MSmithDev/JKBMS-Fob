#include "settings_screen.hpp"
#include "freertos/queue.h"
#include "esp_log.h"
#include "nvs_flash.h"

#define TAG "settings_screen"

extern QueueHandle_t bleScan_data_queue;

int bleScanDevices = 0;
BLEScan bleScan[20];

int selectedDevice = -1;

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

    ESP_LOGI(TAG, "Devices found %d, selected Device %d", bleScanDevices, selectedDevice);
    // Pages
    //  1 - BLE Setup
    //  2 - Brightness
    //  3 - Sleep Modes
    //  4 - Back

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

                //check how many devices are found
                for (int i = 0; i < 20; i++)
                {
                    if (strlen(bleScan[i].deviceName) > 0)
                    {
                        bleScanDevices++;
                    }
                }
            }

            // Show scan results
            canvas.fillSprite(0x000000u); // Grey
            canvas.setTextColor(TFT_WHITE);
            canvas.setTextSize(2);
            canvas.drawString("Scanning", 35, 25);

            if (selectedDevice == -1)
            {
                canvas.setTextColor(TFT_BLACK, TFT_ORANGE);
                canvas.drawString("...", 50, 30 + (1 * 20));
                canvas.setTextColor(TFT_WHITE);
                canvas.drawString(bleScan[selectedDevice + 1].deviceName, 50, 30 + (2 * 20));
                canvas.drawString(bleScan[selectedDevice + 2].deviceName, 50, 30 + (3 * 20));

                if (globalState->selectKey)
                {
                    globalState->settingsPage = 1;
                    globalState->selectKey = false;
                }
            } else if (selectedDevice == 0)
            {
                canvas.setTextColor(TFT_WHITE);
                canvas.drawString("...", 50, 30 + (1 * 20));
                canvas.setTextColor(TFT_BLACK, TFT_ORANGE);
                canvas.drawString(bleScan[selectedDevice].deviceName, 50, 30 + (2 * 20));
                canvas.setTextColor(TFT_WHITE);
                canvas.drawString(bleScan[selectedDevice + 1].deviceName, 50, 30 + (3 * 20));
            } else if (selectedDevice == 1)
            {
                canvas.setTextColor(TFT_WHITE);
                canvas.drawString("...", 50, 30 + (1 * 20));
                canvas.drawString(bleScan[selectedDevice-1].deviceName, 50, 30 + (2 * 20));
                canvas.setTextColor(TFT_BLACK, TFT_ORANGE);
                canvas.drawString(bleScan[selectedDevice].deviceName, 50, 30 + (3 * 20));
                canvas.setTextColor(TFT_WHITE);
            }
        

            //handle up and down keys, if more than bleScanDevices, set to bleScanDevices, if less than 0, set to -1
            if (globalState->downKey)
            {
                selectedDevice++;
                if (selectedDevice > bleScanDevices)
                {
                    selectedDevice = bleScanDevices;
                }
            }
            if (globalState->upKey)
            {
                selectedDevice--;
                if (selectedDevice < -1)
                {
                    selectedDevice = -1;
                }
            }
            

            break;
        }
    }

    // OLD BELOW
    /*
    if (1 == 2)
    {
        switch (globalState->settingsPage)
        {
        case 0:
            if (globalState->selectKey)
            {
                globalState->inSettings = true;
                globalState->CurrentScreen = 3;
                globalState->settingsPage = 1;
                globalState->bleSetup = 0;
            }
            canvas.fillSprite(0x434343u); // Grey
            canvas.setTextColor(TFT_BLACK);
            canvas.setTextSize(2.75);
            canvas.drawString("Settings", 40, 40);
            break;

        case 1: // BLE Setup

            if (globalState->downKey && globalState->bleSetup == 0)
            {
                globalState->settingsPage = 2;
            }

            if (globalState->selectKey && globalState->bleSetup == 0)
            {
                globalState->inSettings = true;
                globalState->CurrentScreen = 3;
                globalState->settingsPage = 1;
                globalState->bleSetup = 1;
                globalState->selectKey = false;
            }

            if (globalState->bleSetup == 1)
            {

                if (xQueueReceive(bleScan_data_queue, &(bleScan), (TickType_t)5))
                {
                    ESP_LOGI(TAG, "Got Scan Data");
                }

                // Show scan results
                canvas.fillSprite(0x434343u); // Grey
                canvas.setTextColor(TFT_BLACK);
                canvas.setTextSize(2.5);
                canvas.drawString("Scanning:", 35, 0);
                canvas.setTextSize(2);

                if (globalState->bleSelectedDevice == -1)
                {
                    canvas.setTextColor(TFT_BLACK, TFT_ORANGE);
                    canvas.drawString("...", 50, 10 + (1 * 20));
                    canvas.setTextColor(TFT_BLACK);
                    canvas.drawString(bleScan[globalState->bleSelectedDevice + 1].deviceName, 50, 10 + (2 * 20));
                    canvas.drawString(bleScan[globalState->bleSelectedDevice + 2].deviceName, 50, 10 + (3 * 20));
                }

                if (globalState->bleSelectedDevice >= 0)
                {
                    if (globalState->bleSelectedDevice == 0)
                    {
                        canvas.drawString("...", 50, 10 + (1 * 20));
                        canvas.setTextColor(TFT_BLACK, TFT_ORANGE);
                        canvas.drawString(bleScan[globalState->bleSelectedDevice].deviceName, 50, 10 + (2 * 20));
                        canvas.setTextColor(TFT_BLACK);
                        canvas.drawString(bleScan[globalState->bleSelectedDevice + 1].deviceName, 50, 10 + (3 * 20));
                        canvas.drawString(bleScan[globalState->bleSelectedDevice + 2].deviceName, 50, 10 + (4 * 20));
                    }
                    else
                    {
                        canvas.drawString("...", 50, 10 + (1 * 20));
                        canvas.drawString(bleScan[globalState->bleSelectedDevice - 1].deviceName, 50, 10 + (2 * 20));
                        canvas.setTextColor(TFT_BLACK, TFT_ORANGE);
                        canvas.drawString(bleScan[globalState->bleSelectedDevice].deviceName, 50, 10 + (3 * 20));
                        canvas.setTextColor(TFT_BLACK);
                        canvas.drawString(bleScan[globalState->bleSelectedDevice + 1].deviceName, 50, 10 + (4 * 20));
                    }

                    // Save selected device to NVS
                    if (globalState->selectKey && globalState->bleSetup == 1)
                    {
                        nvs_handle_t my_handle;
                        esp_err_t err;
                        err = nvs_open("storage", NVS_READWRITE, &my_handle);
                        if (err != ESP_OK)
                        {
                            ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
                        }
                        else
                        {
                            err = nvs_set_str(my_handle, "bleDeviceName", bleScan[globalState->bleSelectedDevice].deviceName);
                            if (err != ESP_OK)
                            {
                                ESP_LOGI(TAG, "Error (%s) writing!", esp_err_to_name(err));
                            }
                            nvs_close(my_handle);
                        }
                        ESP_LOGI(TAG, "Selected Device: %s Saved!", bleScan[globalState->bleSelectedDevice].deviceName);
                        // restarting
                        esp_restart();
                    }
                }
                // if -1 is selected, go back to previous page
                if (globalState->selectKey && globalState->bleSelectedDevice == -1)
                {
                    ESP_LOGI(TAG, "... Back to previous page");
                    globalState->bleSetup = 0;
                    globalState->bleSelectedDevice = 0;
                }

                if (globalState->downKey)
                {
                    globalState->bleSelectedDevice++;
                    if (globalState->bleSelectedDevice > 20)
                    {
                        globalState->bleSelectedDevice = 20;
                    }
                }
                if (globalState->upKey)
                {
                    globalState->bleSelectedDevice--;
                    if (globalState->bleSelectedDevice < -1)
                    {
                        globalState->bleSelectedDevice = -1;
                    }
                }
            }
            else
            {
                canvas.fillSprite(0x434343u); // Grey
                canvas.setTextColor(TFT_BLACK);
                canvas.setTextSize(2.75);
                canvas.drawString("Ble Setup", 40, 40);
            }
            break;

        case 2: // Brightness

            if (globalState->downKey)
            {
                globalState->settingsPage = 3;
            }
            if (globalState->upKey)
            {
                globalState->settingsPage = 1;
            }

            canvas.fillSprite(0x434343u); // Grey
            canvas.setTextColor(TFT_BLACK);
            canvas.setTextSize(2.75);
            canvas.drawString("Brightness", 40, 40);
            break;

        case 3: // Sleep Modes

            if (globalState->downKey)
            {
                globalState->settingsPage = 4;
            }
            if (globalState->upKey)
            {
                globalState->settingsPage = 2;
            }

            canvas.fillSprite(0x434343u); // Grey
            canvas.setTextColor(TFT_BLACK);
            canvas.setTextSize(2.75);
            canvas.drawString("Sleep Modes", 40, 40);
            break;

        case 4: // Back

            if (globalState->downKey)
            {
                globalState->settingsPage = 4;
            }
            if (globalState->upKey)
            {
                globalState->settingsPage = 3;
            }

            // if selected, go back to main screen
            if (globalState->selectKey)
            {
                globalState->inSettings = false;
                globalState->CurrentScreen = 3;
                globalState->settingsPage = 0;
            }

            canvas.fillSprite(0x434343u); // Grey
            canvas.setTextColor(TFT_BLACK);
            canvas.setTextSize(2.75);
            canvas.drawString("Back", 40, 40);
        }

    }
    */
}