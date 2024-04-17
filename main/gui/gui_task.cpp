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
#include "images/bleStatus.h"
#include "images/bleStatusBlue.h"
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
extern QueueHandle_t bleConnection;

bool inSettings = false;
int currentScreen = 0;
// int settingsPageState = 0;

GlobalState globalState;

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

/**
 * Function to get the battery color based on voltage.
 * @param voltage The current voltage.
 * @param minVoltage The minimum voltage threshold.
 * @param maxVoltage The maximum voltage threshold.
 * @return The color as an unsigned integer in 0xRRGGBB format.
 */
unsigned int getBatteryColorNew(float voltage, float minVoltage, float maxVoltage)
{
    // Clamp the voltage to the range [minVoltage, maxVoltage]
    voltage = std::max(minVoltage, std::min(maxVoltage, voltage));

    // Normalize the voltage to a [0, 1] range
    float range = maxVoltage - minVoltage;
    float normalized = (voltage - minVoltage) / range;

    // Calculate the red and green components based on the voltage
    // Red decreases as the voltage increases, green increases as the voltage increases
    uint8_t red = static_cast<uint8_t>((1 - normalized) * 255);
    uint8_t green = static_cast<uint8_t>(normalized * 255);

    // Combine the components into a single unsigned integer
    // Format: 0xRRGGBB
    return (red << 16) | (green << 8);
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

//Fob battery widget
void fobBatteryWidget(LGFX_Sprite canvas,int x, int y, int w, int h, int percentage)
{
    int batteryRadius = 5;
    //draw battery outline
    canvas.drawRoundRect(x, y, w, h, batteryRadius, TFT_WHITE);
    //draw positive terminal
    canvas.fillRoundRect(x+w-1, y+(h/2)-4, 4, 8, 3,TFT_WHITE);

    //draw battery level
    canvas.fillRoundRect(x+1, y+1, (w-2)*(percentage/100.0), h-2, 5, getBatteryColorNew(percentage, 0, 100));
    


}


//Status Bar
void statusBar(LGFX_Sprite canvas, GlobalState &globalState, bool isConnected)
{
    int bleStatusX = 35;
    int bleStatusY = 0;

    // Black Bar
    canvas.fillRect(30, 0, 210, 20, TFT_BLACK);

    // BLE Status
    canvas.fillRect(bleStatusX+5,bleStatusY+2,10,16, isConnected ? TFT_GREEN : TFT_RED);
    canvas.pushImage(bleStatusX,bleStatusY,20,20, image_data_bleStatusBlue, (uint16_t)0x07E0); // Add BLE Status with transparent color

    //placeholder for icons
    canvas.setTextColor(TFT_WHITE);
    canvas.setTextSize(2);
    canvas.drawString("[] [] []", 60, 3);

    // Fob Battery
    fobBatteryWidget(canvas, 190, 0, 40, 20, 10);

    //Fob battery
    //TODO: Get battery level from I2C and display icon based on battery level
    
}

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

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    // Create a sprite for the background
    LGFX_Sprite bgSprite(&display);
    bgSprite.createSprite(240, 135);

    // Screens

    // Variable to remember the last state of the button
    bool lasUPKeyState = true;
    bool lasDownKeyState = true;
    bool lasSelectKeyState = true;

    globalState.CurrentPage = 0; // Main Page

    // BLE Scan Data
    BLEScan bleScan[20];
    bool bleConnectionVal = false;
    // Main GUI Loop
    while (1)
    {

         if (xQueueReceive(bleConnection, &bleConnectionVal, (TickType_t)5))
        {
             ESP_LOGI(TAG, "Got BLE Connection Data %i", bleConnectionVal);
         }

        

#include <stdbool.h>

        // Page States
        // Settings State

        bool curUPKeyState = gpio_get_level(GPIO_NUM_0) == 0;
        bool curDownKeyState = gpio_get_level(GPIO_NUM_2) == 1;
        bool curSelectKeyState = gpio_get_level(GPIO_NUM_1) == 0;

        // logic for UP key
        if (!lasUPKeyState && curUPKeyState)
        {
            globalState.upKey = true;
        }
        else
        {
            globalState.upKey = false;
        }

        // logic for Down key
        if (!lasDownKeyState && curDownKeyState)
        {
            globalState.downKey = true;
        }
        else
        {
            globalState.downKey = false;
        }

        // logic for Select key (inverted?)
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

            main_screen(bgSprite, &testRecv);
            navBar(bgSprite, curUPKeyState, curSelectKeyState, curDownKeyState);
            statusBar(bgSprite, globalState, bleConnectionVal);

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

            info_screen(bgSprite);
            navBar(bgSprite, curUPKeyState, curSelectKeyState, curDownKeyState);
            bgSprite.pushSprite(0, 0);
            break;

        case 2: // Control Page

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

            control_screen(bgSprite);
            navBar(bgSprite, curUPKeyState, curSelectKeyState, curDownKeyState);
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


            settings_screen(bgSprite, globalState);
            navBar(bgSprite, curUPKeyState, curSelectKeyState, curDownKeyState);
            bgSprite.pushSprite(0, 0);
            break;
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
