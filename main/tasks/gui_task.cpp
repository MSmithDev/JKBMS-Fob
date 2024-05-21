#include "gui_task.h"
#include "LovyanGFX.hpp"
#include "gui/LGFX_Config.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
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

// Helpers
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


void handle_page_navigation()
{
    if (globalState.upKey)
    {
        switch(globalState.CurrentPage)
        {
            case 0:
                // if BLE is connected, go to the next page otherwise go to the last page
                globalState.bleConnected ? globalState.CurrentPage-- : globalState.CurrentPage = 3;
                break;

            case 3:    
                globalState.bleConnected ? globalState.CurrentPage-- : globalState.CurrentPage = 0;
                break;
            
            default:
                globalState.CurrentPage--;
                break;
        }
        if (globalState.CurrentPage < 0) globalState.CurrentPage = 3;
        
    }
    if (globalState.downKey)
    {
        switch(globalState.CurrentPage)
        {
            case 0:
                globalState.bleConnected ? globalState.CurrentPage++ : globalState.CurrentPage = 3;
                break;

            case 3:    
                globalState.bleConnected ? globalState.CurrentPage++ : globalState.CurrentPage = 0;
                break;
            
            default:
                globalState.CurrentPage++;
                break;
        }
        if (globalState.CurrentPage > 3) globalState.CurrentPage = 0;
    }
}


void gui_task(void *pvParameters)
{
    ble_data_queue = xQueueCreate(5, sizeof(BLEControl));

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
    display.setBrightness(globalState.screenBrightness);
    display.setSwapBytes(true);
    display.setColorDepth(16);

    // Splash Screen
    display.pushImage(0, 0, 240, 135, image_data_splash);
    vTaskDelay(pdMS_TO_TICKS(1000));

    LGFX_Sprite canvas(&display);
    canvas.createSprite(240, 135);

    // Variable to remember the last state of the buttons
    bool lastUPKeyState = true;
    bool lastDownKeyState = true;
    bool lastSelectKeyState = true;

    globalState.CurrentPage = 0; // Main Page

    // Main GUI Loop
    while (1)
    {
        // Handle BLE connection and JKBMS data
        if (xQueueReceive(bleConnection, &globalState.bleConnected, (TickType_t)5))
        {
            ESP_LOGI(TAG, "Got BLE Connection Data %i", globalState.bleConnected);
        }

        if (xQueueReceive(jkbms_data_queue, &jkbmsData, (TickType_t)5))
        {
            // Handle JKBMS Data if needed
        }

        // Get the current state of the buttons
        bool curUPKeyState = gpio_get_level(GPIO_NUM_0) == 0;
        bool curDownKeyState = gpio_get_level(GPIO_NUM_2) == 1;
        bool curSelectKeyState = gpio_get_level(GPIO_NUM_1) == 0;

        globalState.upKey = !lastUPKeyState && curUPKeyState;
        globalState.downKey = !lastDownKeyState && curDownKeyState;
        globalState.selectKey = !lastSelectKeyState && curSelectKeyState;

        //if any key is pressed, reset the sleep timer
        if(globalState.upKey || globalState.downKey || globalState.selectKey)
        {
            globalState.resetSleepTimer = true;
        }

        // Update last state to current state
        lastUPKeyState = curUPKeyState;
        lastDownKeyState = curDownKeyState;
        lastSelectKeyState = curSelectKeyState;

        canvas.setSwapBytes(true);
        canvas.setColorDepth(16);

        // Handle the current page
        switch (globalState.CurrentPage)
        {
            case 0: // Main Page
                handle_page_navigation();
                main_screen(canvas, &globalState, &jkbmsData);
                break;
            case 1: // Info Page
                // TODO: Make select key toggle between info screens
                handle_page_navigation();
                info_screen(canvas, &globalState, &jkbmsData);
                break;
            case 2: // Control Page
                if (!globalState.inControl) handle_page_navigation();
                control_screen(canvas, &globalState, &jkbmsData);
                break;
            case 3: // Settings Page
                if (!globalState.inSettings) handle_page_navigation();
                settings_screen(canvas, &globalState, &display);
                
                break;
        }

        UIWidgets::navBar(canvas, curUPKeyState, curSelectKeyState, curDownKeyState);
        UIWidgets::statusBar(canvas, &globalState, &jkbmsData);
        canvas.pushSprite(0, 0);

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}


