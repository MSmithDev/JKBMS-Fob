#include "gui_task.h"
#include "LovyanGFX.hpp"
#include <string>
#include "gui/LGFX_Config.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include "driver/gpio.h"
// Widgets
#include "gui/widgets/widgets.hpp"

// Images
#include "gui/images/splash.h"
#include "gui/images/mainui.h"

// Screens
#include "gui/screens/main_screen/main_screen.hpp"
#include "gui/screens/settings_screen/settings_screen.hpp"
#include "gui/screens/control_screen/control_screen.hpp"
#include "gui/screens/info_screen/info_screen.hpp"

//helpers
#include "helpers/jkbms.h"
#include "helpers/utils.h"





static const char *TAG = "GUI_Task";

extern GlobalState globalState;
extern JKBMSData jkbmsData;

LGFX display;

extern QueueHandle_t jkbms_data_queue;
extern QueueHandle_t bleConnection;

// Queue for sending data to the GUI
extern QueueHandle_t ble_data_queue;
extern QueueHandle_t bleScan_data_queue;

void gui_task(void *pvParameters)
{

    

    ble_data_queue = xQueueCreate(5, sizeof(BLEControl));

    

    // Initialize 3 buttons
    // Button UP
    gpio_pad_select_gpio(GPIO_NUM_0);
    gpio_set_direction(GPIO_NUM_0, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_NUM_0, GPIO_PULLUP_ONLY);
    // Button SELECT
    gpio_pad_select_gpio(GPIO_NUM_1);
    gpio_set_direction(GPIO_NUM_1, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_NUM_1, GPIO_PULLUP_PULLDOWN);
    // Button DOWN
    gpio_pad_select_gpio(GPIO_NUM_2);
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_NUM_2, GPIO_PULLUP_PULLDOWN);

    display.init();
    display.setSwapBytes(true);
    display.setColorDepth(16);

    // Splash Screen
    display.pushImage(0, 0, 240, 135, image_data_splash);

    vTaskDelay(pdMS_TO_TICKS(1000));

    // Create a sprite for the background
    LGFX_Sprite bgSprite(&display);
    bgSprite.createSprite(240, 135);

    // Screens

    // Variable to remember the last state of the button
    bool lasUPKeyState = true;
    bool lasDownKeyState = true;
    bool lasSelectKeyState = true;

    globalState.CurrentPage = 0; // Main Page

    // Main GUI Loop
    while (1)
    {

        if (xQueueReceive(bleConnection, &globalState.bleConnected, (TickType_t)5))
        {
            ESP_LOGI(TAG, "Got BLE Connection Data %i", globalState.bleConnected);
        }

        if (xQueueReceive(jkbms_data_queue, &jkbmsData, (TickType_t)5))
        {
            // ESP_LOGI(TAG, "Got BLE Fake Data");
        }



        // Get the current state of the buttons
        bool curUPKeyState = gpio_get_level(GPIO_NUM_0) == 0;
        bool curDownKeyState = gpio_get_level(GPIO_NUM_2) == 1;
        bool curSelectKeyState = gpio_get_level(GPIO_NUM_1) == 0;

        if (!lasUPKeyState && curUPKeyState)
        {
            globalState.upKey = true;
        }
        else
        {
            globalState.upKey = false;
        }

        if (!lasDownKeyState && curDownKeyState)
        {
            globalState.downKey = true;
        }
        else
        {
            globalState.downKey = false;
        }

        if (!lasSelectKeyState && curSelectKeyState)
        {
            globalState.selectKey = true;
        }
        else
        {
            globalState.selectKey = false;
        }

        // Update last state to current state
        lasUPKeyState = curUPKeyState;
        lasDownKeyState = curDownKeyState;
        lasSelectKeyState = curSelectKeyState;

        bgSprite.setSwapBytes(true);
        bgSprite.setColorDepth(16);

        switch (globalState.CurrentPage)
        {

        case 0: // Main Page

            // Update the current page based on the button press
            if (globalState.upKey)
            {
                globalState.CurrentPage--;
                if (globalState.CurrentPage < 0)
                {
                    globalState.CurrentPage = 3;
                }
            }
            if (globalState.downKey)
            {
                globalState.CurrentPage++;
                if (globalState.CurrentPage > 3)
                {
                    globalState.CurrentPage = 0;
                }
            }

            main_screen(bgSprite, &globalState, &jkbmsData);
            UIWidgets::navBar(bgSprite, curUPKeyState, curSelectKeyState, curDownKeyState);
            UIWidgets::statusBar(bgSprite, &globalState, &jkbmsData);

            bgSprite.pushSprite(0, 0);
            break;

        case 1: // Info Page

            // Update the current page based on the button press
            if (globalState.upKey)
            {
                globalState.CurrentPage--;
                if (globalState.CurrentPage < 0)
                {
                    globalState.CurrentPage = 3;
                }
            }
            if (globalState.downKey)
            {
                globalState.CurrentPage++;
                if (globalState.CurrentPage > 3)
                {
                    globalState.CurrentPage = 0;
                }
            }

            info_screen(bgSprite, &globalState, &jkbmsData);
            UIWidgets::navBar(bgSprite, curUPKeyState, curSelectKeyState, curDownKeyState);
            UIWidgets::statusBar(bgSprite, &globalState, &jkbmsData);
            bgSprite.pushSprite(0, 0);
            break;

        case 2: // Control Page

            
            if (globalState.inControl == false)
            {
                // Update the current page based on the button press
                if (globalState.upKey)
                {
                    globalState.CurrentPage--;
                    if (globalState.CurrentPage < 0)
                    {
                        globalState.CurrentPage = 0;
                    }
                }
                if (globalState.downKey)
                {
                    globalState.CurrentPage++;
                    if (globalState.CurrentPage > 3)
                    {
                        globalState.CurrentPage = 0;
                    }
                }
            }
            

            control_screen(bgSprite, &globalState, &jkbmsData);
            UIWidgets::navBar(bgSprite, curUPKeyState, curSelectKeyState, curDownKeyState);
            UIWidgets::statusBar(bgSprite, &globalState, &jkbmsData);
            bgSprite.pushSprite(0, 0);
            break;

        case 3: // Settings Page

            // Update the current page based on the button press
            if (globalState.settingsPage == 0)
            {
                if (globalState.upKey)
                {
                    globalState.CurrentPage--;
                    if (globalState.CurrentPage < 0)
                    {
                        globalState.CurrentPage = 3;
                    }
                }
                if (globalState.downKey)
                {
                    globalState.CurrentPage++;
                    if (globalState.CurrentPage > 3)
                    {
                        globalState.CurrentPage = 0;
                    }
                }
            }

            settings_screen(bgSprite, &globalState);
            UIWidgets::navBar(bgSprite, curUPKeyState, curSelectKeyState, curDownKeyState);
            UIWidgets::statusBar(bgSprite, &globalState, &jkbmsData);

            bgSprite.pushSprite(0, 0);
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
