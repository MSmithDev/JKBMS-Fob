#include "gui_task.hpp"
#include "LovyanGFX.hpp"
#include <string>
#include "LGFX_Config.hpp"
#include "freertos/queue.h"
#include "esp_log.h"

#include <iostream>
#include <iomanip>
#include <sstream>

//Images
#include "images/splash.h"
#include "images/mainui.h"
#include "images/NavBar.h"

LGFX display; // Assuming display is defined elsewhere

static const char* TAG = "GUI_Task";

extern QueueHandle_t gui_data_queue;

bool inSettings = false;
int currentScreen = 0;
int settingsPageState = 0;

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

int map_float_to_int(float input) {
    if (input < 68.0 || input > 84.0) {
        //printf("Input out of range. Please enter a value between 68 and 84.\n");
        return 0;  // Return an error code if input is out of range
    }

    // Linear mapping from range 68 to 84 to range 0 to 67
    return (int)((input - 68.0) * (67.0 / (84.0 - 68.0)));
}

//Main Screen
    void mainScreen(LGFX_Sprite bgSprite, JKBMSData testRecv) {
         bgSprite.fillSprite(TFT_WHITE);
        //draw battery voltage bar
        //int barvalue = map_float_to_int(testRecv.packVoltage);
        bgSprite.fillRect(163, 8, map_float_to_int(testRecv.packVoltage), 20, getBatteryColor(testRecv.packVoltage)); //Test blue
        //draw voltage text
        bgSprite.setTextColor(TFT_BLACK);
        bgSprite.setTextSize(1.5);
        bgSprite.drawString(floatToString(testRecv.packVoltage), 175, 13);

        //Push overlay UI
        bgSprite.pushImage(0, 0, 240, 135, image_data_mainui, (uint16_t)0x07E0);
        bgSprite.pushSprite(0, 0);
    }

//Widgets
//NavBar
void navBar(LGFX_Sprite bgSprite,bool UpKey, bool SelectKey, bool DownKey) {
    
    uint16_t ActiveColor = 0xFFFF; //Green
    uint16_t InactiveColor = 0x0000; //Black


    //three squares for the buttons
    bgSprite.fillRect(1, 14, 27, 27, UpKey ? ActiveColor : InactiveColor);
    bgSprite.fillRect(1, 54, 27, 27, !SelectKey ? ActiveColor : InactiveColor); //Inverted
    bgSprite.fillRect(1, 95, 27, 27, DownKey ? ActiveColor : InactiveColor);


    bgSprite.pushImage(0, 0, 30, 135, image_data_NavBar, (uint16_t)0x07E0); //Add NavBar with transparent color
}

void settingsScreen(LGFX_Sprite bgSprite,bool &SelectKey ,int pageState) {
   
    if(currentScreen == 3 && SelectKey && settingsPageState == 0) {
                    settingsPageState = 1; //Bluetooth
                    inSettings = true;
                    SelectKey = false;  //Reset select key
                    ESP_LOGI(TAG, "Shoud only run once");
                }
    if(settingsPageState == 4 && SelectKey) {
                    inSettings = false;
                    SelectKey = false;  //Reset select key
                    settingsPageState = 0;
                    ESP_LOGI(TAG, "Shoud only run once");
                }
                
                switch (settingsPageState)
                {
                case 0: //Settings Text

                bgSprite.setTextColor(TFT_BLACK);
                bgSprite.setTextSize(3);
                bgSprite.drawString("Settings", 50, 40);
                    break;
                
                case 1: //Bluetooth
                bgSprite.setTextColor(TFT_BLACK);
                bgSprite.setTextSize(3);
                bgSprite.drawString("BlE Setup", 50, 40);
                    break;
                
                case 2: //Sleep Modes
                bgSprite.setTextColor(TFT_BLACK);
                bgSprite.setTextSize(3);
                bgSprite.drawString("Sleep Modes", 50, 40);
                    break;
                
                case 3: //Brightness
                bgSprite.setTextColor(TFT_BLACK);
                bgSprite.setTextSize(3);
                bgSprite.drawString("Brightness", 50, 40);
                    break;
                
                case 4: //Back
                bgSprite.setTextColor(TFT_BLACK);
                bgSprite.setTextSize(3);
                bgSprite.drawString("Back", 50, 40);
                    break;
                }
}

void gui_task(void *pvParameters) {

    JKBMSData testRecv;
    

    //Initialize 3 buttons
    //Button UP
    gpio_pad_select_gpio(GPIO_NUM_0);
    gpio_set_direction(GPIO_NUM_0, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_NUM_0, GPIO_PULLUP_ONLY);
    //Button SELECT
    gpio_pad_select_gpio(GPIO_NUM_1);
    gpio_set_direction(GPIO_NUM_0, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_NUM_0, GPIO_PULLUP_ONLY);
    //Button DOWN
    gpio_pad_select_gpio(GPIO_NUM_2);
    gpio_set_direction(GPIO_NUM_0, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_NUM_0, GPIO_PULLUP_ONLY);




    display.init();
    display.setSwapBytes(true);
    display.setColorDepth(16);
    
    //Splash Screen    
    display.pushImage(0, 0, 240, 135, image_data_splash);

    vTaskDelay(2000 / portTICK_PERIOD_MS);
    
    //Create a sprite for the background
    LGFX_Sprite bgSprite(&display);
    bgSprite.createSprite(240, 135);


    //Screens

    
    // Variable to remember the last state of the button
bool lastStateButton0 = true;
bool lastStateButton2 = true;
bool lastStateSelectKey = true;
bool SelectKey = false;


    //Main GUI Loop
    while (1) {

        if(xQueueReceive(gui_data_queue, &(testRecv), (TickType_t)5)){
            ESP_LOGI(TAG, "Got data from queue! Pack (V): %f Pack (W): %f, Cell 0 (V): %f", testRecv.packVoltage, testRecv.packPower, testRecv.cellVoltages[0]);

        }
        //ESP_LOGI(TAG, "current screen: %d", currentScreen);
        //button states
        ESP_LOGI(TAG, "CurrentScreen %d, Button 0: %d, Button 1: %d, Button 2: %d SelectKey: %i, InSettings: %i, settingPageState %i", currentScreen,gpio_get_level(GPIO_NUM_0), gpio_get_level(GPIO_NUM_1), gpio_get_level(GPIO_NUM_2),SelectKey,inSettings,settingsPageState);

        #include <stdbool.h>
    
    //Page States
    //Settings State
    



    bool currentStateButton0 = gpio_get_level(GPIO_NUM_0) == 0;
    bool currentStateButton2 = gpio_get_level(GPIO_NUM_2) == 1;
    bool currentStateSelectKey = gpio_get_level(GPIO_NUM_1) == 0;

    // Check for a high-to-low transition on GPIO_NUM_0 (button pressed)
    if (!lastStateButton0 && currentStateButton0 && !inSettings) {
        currentScreen++;
        if (currentScreen > 3) {
            currentScreen = 0;
        }
    }
    //settings screens
    if (!lastStateButton0 && currentStateButton0 && inSettings) {
        settingsPageState++;
        if (settingsPageState > 4) {
            settingsPageState = 0;
        }
    }

    // Check for a low-to-high transition on GPIO_NUM_2 (button released)
    if (!lastStateButton2 && currentStateButton2 && !inSettings) {
        currentScreen--;
        if (currentScreen < 0) {
            currentScreen = 3;
        }
    }
    if (!lastStateButton2 && currentStateButton2 && inSettings) {
        settingsPageState--;
        if (settingsPageState < 1) {
            settingsPageState = 4;
        }
    }

    // Check for a high-to-low transition on GPIO_NUM_4 ("select" key pressed)
    if (lastStateSelectKey && !currentStateSelectKey) {
        // Perform the action associated with the "select" key
        // Example action: Toggle a boolean flag, trigger a function, etc.
        SelectKey = true;
    }

    // Update last state to current state
    lastStateButton0 = currentStateButton0;
    lastStateButton2 = currentStateButton2;
    lastStateSelectKey = currentStateSelectKey;




        bgSprite.setSwapBytes(true);
        bgSprite.setColorDepth(16);
        
        //switrch case for different screens
        //switrch case for different screens
        switch (currentScreen) {

            case 0: //Main Screen
                //Pass sprite and data to main screen function
                //mainScreen(bgSprite, testRecv);
                bgSprite.fillSprite(0x434343u); //Grey
                //add nav bar
                navBar(bgSprite, currentStateButton0, currentStateSelectKey, currentStateButton2);

                bgSprite.pushSprite(0, 0);

                break;


            case 1: //Cell Voltage Screen
                //Cell Voltage Screen
                bgSprite.fillSprite(0x434343u); //Grey
                //add nav bar
                navBar(bgSprite, currentStateButton0, currentStateSelectKey, currentStateButton2);

                bgSprite.setTextColor(TFT_BLACK);
                bgSprite.setTextSize(1.5);
                bgSprite.drawString("Cell Voltage Screen", 40, 10);
                bgSprite.drawString("Cell 0 Voltage: ", 40, 30);
                bgSprite.drawString(floatToString(testRecv.cellVoltages[0]), 10, 50);
                bgSprite.drawString("Cell 1 Voltage: ", 40, 70);
                bgSprite.drawString(floatToString(testRecv.cellVoltages[1]), 10, 90);
                bgSprite.drawString("Cell 2 Voltage: ", 40, 110);
                bgSprite.drawString(floatToString(testRecv.cellVoltages[2]), 10, 130);
                bgSprite.pushSprite(0, 0);
                break;
            case 2: //cell resistance screen
                bgSprite.fillSprite(0x434343u); //Grey
                navBar(bgSprite, currentStateButton0, currentStateSelectKey, currentStateButton2);

                bgSprite.setTextColor(TFT_BLACK);
                bgSprite.setTextSize(1.5);
                bgSprite.drawString("Cell Resistance Screen", 40, 10 );
                bgSprite.drawString("Cell 0 Resistance: ", 40, 30);
                bgSprite.drawString(floatToString(testRecv.cellResistances[0]), 40, 50);
                bgSprite.drawString("Cell 1 Resistance: ", 40, 70);
                bgSprite.drawString(floatToString(testRecv.cellResistances[1]), 40, 90);
                bgSprite.pushSprite(0, 0);
                break;
            
            case 3: //settings screen
                bgSprite.fillSprite(0x434343u); //Grey
                
                navBar(bgSprite, currentStateButton0, currentStateSelectKey, currentStateButton2);
                
                settingsScreen(bgSprite, SelectKey,inSettings);
                
                bgSprite.pushSprite(0, 0);


                
                break;
        
       

        


        vTaskDelay(10 / portTICK_PERIOD_MS);
     }
    }
}