#include "gui_task.hpp"
#include "LovyanGFX.hpp"
#include <string>
#include "LGFX_Config.hpp"
#include "freertos/queue.h"
#include "esp_log.h"

#include <iostream>
#include <iomanip>
#include <sstream>

// Images
#include "images/splash.h"
#include "images/mainui.h"
#include "images/NavBar.h"

// Screens
#include "screens/main_screen/main_screen.hpp"
#include "screens/settings_screen/settings_screen.hpp"
#include "screens/control_screen/control_screen.hpp"
#include "screens/info_screen/info_screen.hpp"

// JKBMS Helper
#include "helpers/jkbms.h"
#include "helpers/page_states.hpp"

LGFX display; // Assuming display is defined elsewhere

static const char *TAG = "GUI_Task";

extern QueueHandle_t gui_data_queue;

bool inSettings = false;
int currentScreen = 0;
// int settingsPageState = 0;

// Screen state
mainScreenState mainSS;
// Page States
settingsPageState settingsPS;
controlPageState controlPS;
infoPageState infoPS;

const char *floatToString(float value)
{
    static std::string str; // Made static to prolong its lifetime beyond the function scope
    std::ostringstream out;
    out << std::fixed << std::setprecision(2) << value;
    str = out.str();
    return str.c_str();
}

// func to convert int to string
const char *intToString(int value)
{
    static std::string str; // Made static to prolong its lifetime beyond the function scope
    std::ostringstream out;
    out << value;
    str = out.str();
    return str.c_str();
}

unsigned int getBatteryColor(float voltage)
{
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

int map_float_to_int(float input)
{
    if (input < 68.0 || input > 84.0)
    {
        // printf("Input out of range. Please enter a value between 68 and 84.\n");
        return 0; // Return an error code if input is out of range
    }

    // Linear mapping from range 68 to 84 to range 0 to 67
    return (int)((input - 68.0) * (67.0 / (84.0 - 68.0)));
}

// Widgets
// NavBar
void navBar(LGFX_Sprite bgSprite, bool UpKey, bool SelectKey, bool DownKey)
{

    uint16_t ActiveColor = 0xFFFF;   // Green
    uint16_t InactiveColor = 0x0000; // Black

    // three squares for the buttons
    bgSprite.fillRect(1, 14, 27, 27, UpKey ? ActiveColor : InactiveColor);
    bgSprite.fillRect(1, 54, 27, 27, !SelectKey ? ActiveColor : InactiveColor); // Inverted
    bgSprite.fillRect(1, 95, 27, 27, DownKey ? ActiveColor : InactiveColor);

    bgSprite.pushImage(0, 0, 30, 135, image_data_NavBar, (uint16_t)0x07E0); // Add NavBar with transparent color
}

// void settingsScreenold(LGFX_Sprite bgSprite,bool &SelectKey ,int pageState) {

//     if(currentScreen == 3 && SelectKey && settingsPageState == 0) {
//                     settingsPageState = 1; //Bluetooth
//                     inSettings = true;
//                     SelectKey = false;  //Reset select key
//                     ESP_LOGI(TAG, "Shoud only run once");
//                 }
//     if(settingsPageState == 4 && SelectKey) {
//                     inSettings = false;
//                     SelectKey = false;  //Reset select key
//                     settingsPageState = 0;
//                     ESP_LOGI(TAG, "Shoud only run once");
//                 }

//                 switch (settingsPageState)
//                 {
//                 case 0: //Settings Text

//                 bgSprite.setTextColor(TFT_BLACK);
//                 bgSprite.setTextSize(3);
//                 bgSprite.drawString("Settings", 50, 40);
//                     break;

//                 case 1: //Bluetooth
//                 bgSprite.setTextColor(TFT_BLACK);
//                 bgSprite.setTextSize(3);
//                 bgSprite.drawString("BlE Setup", 50, 40);
//                     break;

//                 case 2: //Sleep Modes
//                 bgSprite.setTextColor(TFT_BLACK);
//                 bgSprite.setTextSize(3);
//                 bgSprite.drawString("Sleep Modes", 50, 40);
//                     break;

//                 case 3: //Brightness
//                 bgSprite.setTextColor(TFT_BLACK);
//                 bgSprite.setTextSize(3);
//                 bgSprite.drawString("Brightness", 50, 40);
//                     break;

//                 case 4: //Back
//                 bgSprite.setTextColor(TFT_BLACK);
//                 bgSprite.setTextSize(3);
//                 bgSprite.drawString("Back", 50, 40);
//                     break;
//                 }
// }
// Queue for sending data to the GUI

extern QueueHandle_t ble_data_queue;
extern QueueHandle_t bleScan_data_queue;

void gui_task(void *pvParameters)
{
    ble_data_queue = xQueueCreate(5, sizeof(struct BLEControl));

    JKBMSData testRecv;
    mainSS.selectedOption = 0;
    mainSS.scrollable = true;
    mainSS.currentScreen = 0;

    // Initialize 3 buttons
    // Button UP
    gpio_pad_select_gpio(GPIO_NUM_0);
    gpio_set_direction(GPIO_NUM_0, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_NUM_0, GPIO_PULLUP_ONLY);
    // Button SELECT
    gpio_pad_select_gpio(GPIO_NUM_1);
    gpio_set_direction(GPIO_NUM_0, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_NUM_0, GPIO_PULLUP_ONLY);
    // Button DOWN
    gpio_pad_select_gpio(GPIO_NUM_2);
    gpio_set_direction(GPIO_NUM_0, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_NUM_0, GPIO_PULLUP_ONLY);

    display.init();
    display.setSwapBytes(true);
    display.setColorDepth(16);

    // Splash Screen
    display.pushImage(0, 0, 240, 135, image_data_splash);

    vTaskDelay(2000 / portTICK_PERIOD_MS);

    // Create a sprite for the background
    LGFX_Sprite bgSprite(&display);
    bgSprite.createSprite(240, 135);

    // Screens

    // Variable to remember the last state of the button
    bool lasUPKeyState = true;
    bool lasDownKeyState = true;
    bool lasSelectKeyState = true;
    bool SelectKey = false;
    
    // BLE Scan Data
    BLEScan bleScan[20];

    // Main GUI Loop
    while (1)
    {

        if (xQueueReceive(gui_data_queue, &(testRecv), (TickType_t)5))
        {
            ESP_LOGI(TAG, "Got data from queue! Pack (V): %f Pack (W): %f, Cell 0 (V): %f", testRecv.packVoltage, testRecv.packPower, testRecv.cellVoltages[0]);
        }

        //handle ble queue
        if(xQueueReceive(bleScan_data_queue, &(bleScan), (TickType_t)5))
        {
            for(int i = 0; i < 20; i++)
            {
                
                ESP_LOGI(TAG, "Got data from BLE queue! Device Name: %s, RSSI: %d", bleScan[i].deviceName, bleScan[i].rssi);
            
            }
            
        }

        // print mainSS state
        if (mainSS.selectedOption == 0)
        {

        }
            //ESP_LOGI(TAG, "Main Screen State: Selected Option: %d, Scrollable: %d, Current Screen: %d", mainSS.selectedOption, mainSS.scrollable, mainSS.currentScreen);

        if (mainSS.selectedOption == 3)
        {

        }
            //ESP_LOGI(TAG, "Settings Screen State: Current Selected: %d, Selected Setting : %d, isActive: %i, ShowOptions %i", settingsPS.currentSelection, settingsPS.selectedSetting, settingsPS.isActivePage, settingsPS.showOptions);

#include <stdbool.h>

        // Page States
        // Settings State

        bool curUPKeyState = gpio_get_level(GPIO_NUM_0) == 0;
        bool curDownKeyState = gpio_get_level(GPIO_NUM_2) == 1;
        bool curSelectKeyState = gpio_get_level(GPIO_NUM_1) == 0;

        switch (mainSS.currentScreen)
        {

        case 0: // Main Screen
            if (!lasSelectKeyState && curSelectKeyState)
            {
                SelectKey = false; // Reset select key
                ESP_LOGI(TAG, "Select on main does nothing");
                BLEControl test;
                test.connect=true;
                test.disconnect=false;
                test.startScan=true;
                test.stopScan=false;
                if (xQueueSend(ble_data_queue, &(test), portMAX_DELAY) != pdPASS)
                {
                    ESP_LOGI(TAG, "Failed to send array to queue");
                }
            }
            break;

        case 1: // Info Screen
            if (!lasSelectKeyState && curSelectKeyState)
            {
                SelectKey = false; // Reset select key
                ESP_LOGI(TAG, "Select on info does nothing");
                BLEControl test;
                test.connect=true;
                test.disconnect=false;
                test.startScan=false;
                test.stopScan=true;
                if (xQueueSend(ble_data_queue, &(test), portMAX_DELAY) != pdPASS)
                {
                    ESP_LOGI(TAG, "Failed to send array to queue");
                }
            }
            break;

        case 2: // Control Screen
            if (!lasSelectKeyState && curSelectKeyState)
            {
                SelectKey = false; // Reset select key
                ESP_LOGI(TAG, "Select on CTRL does nothing");
            }
            break;

        case 3: // Settings Screen
            if (!lasSelectKeyState && curSelectKeyState)
            {
                SelectKey = false; // Reset select key

                // If settings is selected, go to settings screen
                ESP_LOGI(TAG, "Select triggered settings screen");
                mainSS.selectedOption = 3;
                mainSS.scrollable = false;
                if (!settingsPS.isActivePage)
                    settingsPS.currentSelection = 1;
                settingsPS.isActivePage = true;
                settingsPS.showOptions = true;

                // If back is selected go back to main screen
                if (settingsPS.currentSelection == 4)
                {
                    ESP_LOGI(TAG, "Select triggered back from settings");
                    settingsPS.isActivePage = false;
                    settingsPS.showOptions = false;
                    settingsPS.selectedSetting = 0;
                    mainSS.selectedOption = 0;
                    mainSS.scrollable = true;
                }
            }

            // logic for settings screen options
            if ((!lasUPKeyState && curUPKeyState) && (settingsPS.isActivePage && settingsPS.showOptions))
            {
                settingsPS.currentSelection--;
                if (settingsPS.currentSelection < 1)
                {
                    settingsPS.currentSelection = 1;
                }
            }
            if ((!lasDownKeyState && curDownKeyState) && (settingsPS.isActivePage && settingsPS.showOptions))
            {
                settingsPS.currentSelection++;
                if (settingsPS.currentSelection > 4)
                {
                    settingsPS.currentSelection = 4;
                }
            }
            break;
        }

        // if on main scr
        if (mainSS.selectedOption == 0 && mainSS.scrollable)
        {
            if (!lasUPKeyState && curUPKeyState)
            {
                mainSS.currentScreen--;
                if (mainSS.currentScreen < 0)
                {
                    mainSS.currentScreen = 3;
                }
            }
            if (!lasDownKeyState && curDownKeyState)
            {
                mainSS.currentScreen++;
                if (mainSS.currentScreen > 3)
                {
                    mainSS.currentScreen = 0;
                }
            }
        }

        // Update last state to current state
        lasUPKeyState = curUPKeyState;
        lasDownKeyState = curDownKeyState;
        lasSelectKeyState = curSelectKeyState;

        bgSprite.setSwapBytes(true);
        bgSprite.setColorDepth(16);

        // switch case for different screens
        switch (mainSS.currentScreen)
        {

        case 0: // Main Screen
            if (SelectKey && !inSettings)
            {
                SelectKey = false; // Reset select key
                ESP_LOGI(TAG, "Select on main does nothing");
            }

            main_screen(bgSprite, &testRecv);
            navBar(bgSprite, curUPKeyState, curSelectKeyState, curDownKeyState);

            bgSprite.pushSprite(0, 0);
            break;

        case 1: // Info Screen 1
            if (SelectKey && !inSettings)
            {
                SelectKey = false; // Reset select key
                ESP_LOGI(TAG, "Select on info does nothing");
            }
            bgSprite.fillSprite(0x434343u); // Grey
            info_screen(bgSprite);
            navBar(bgSprite, curUPKeyState, curSelectKeyState, curDownKeyState);
            bgSprite.pushSprite(0, 0);

            break;
        case 2: // Control Screen
            if (SelectKey && !inSettings)
            {
                SelectKey = false; // Reset select key
                ESP_LOGI(TAG, "Select on CTRL does nothing");
            }
            bgSprite.fillSprite(0x434343u); // Grey
            control_screen(bgSprite);
            navBar(bgSprite, curUPKeyState, curSelectKeyState, curDownKeyState);
            bgSprite.pushSprite(0, 0);
            break;

        case 3: // settings screen

            bgSprite.fillSprite(0x434343u); // Grey
            settings_screen(bgSprite, settingsPS);
            navBar(bgSprite, curUPKeyState, curSelectKeyState, curDownKeyState);
            bgSprite.pushSprite(0, 0);
            break;

            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }
}