#include "control_screen.hpp"



#define TAG "control_screen"

int numOptions = 4;
ControlOption ctrlOption[4] = {
    {"Test1", false},
    {"Test2", false},
    {"Test3", false},
    {"Back", false}};

void control_screen(LGFX_Sprite canvas, GlobalState *globalState)
{

    // Listen for select key if not in control
    if (!globalState->inControl && globalState->selectKey && globalState->bleConnected)
    {
        globalState->inControl = true;
        
        globalState->selectKey = false; // stop multiple presses
    }

    // BLE Connected and CTRL selected
    if (globalState->inControl && globalState->bleConnected)
    {

        if (globalState->downKey)
        {
            ESP_LOGI(TAG, "CTRL OPTION %d", globalState->controlOption);
            globalState->controlOption++;
            if (globalState->controlOption > numOptions - 1)
            {
                globalState->controlOption = 0;
            }
        }
        if (globalState->upKey)
        {
            ESP_LOGI(TAG, "CTRL OPTION %d", globalState->controlOption);
            globalState->controlOption--;
            if (globalState->controlOption < 0)
            {
                globalState->controlOption = numOptions - 1;
            }
        }

        canvas.fillSprite(0x434343u); // Grey
        canvas.setTextColor(TFT_BLACK);
        canvas.setTextSize(2);
        canvas.drawString("CTRL CMDS", 32, 22);

        // Draw options
        for (int i = 0; i < numOptions; i++)
        {
            if (i == globalState->controlOption)
            {
                ctrlOption[i].highlighted = true;
            }
            else
            {
                ctrlOption[i].highlighted = false;
            }

            // set text color based on highlighted state
            if (ctrlOption[i].highlighted)
            {
                canvas.setTextColor(TFT_BLACK, TFT_LIGHTGRAY);
            }
            else
            {
                canvas.setTextColor(TFT_BLACK);
            }

            canvas.setTextSize(1.5);
            canvas.drawString(ctrlOption[i].name.c_str(), 40, 40 + (i * 20));
        }

        // Listen for select key if back is selected
        if (globalState->selectKey)
        {
            switch (globalState->controlOption)
            {
            case 0: // Test1 command

                ESP_LOGI(TAG, "Test1 command triggered");
                break;

            case 1:

                ESP_LOGI(TAG, "Test2 command triggered");
                break;

            case 2:

                ESP_LOGI(TAG, "Test3 command triggered");
                break;

            case 3: // Back
                // leave control mode
                globalState->inControl = false;
                globalState->controlOption = 0;
                break;
            }
        }
    }
    else if (!globalState->inControl && globalState->bleConnected)
    {
        // Placeholder
        canvas.fillSprite(0x434343u); // Grey
        canvas.setTextColor(TFT_BLACK);
        canvas.setTextSize(2.75);
        canvas.drawString("CTRLs", 40, 40);
        canvas.drawString("Commands", 40, 80);
    }
    else if (!globalState->bleConnected)
    {

        // No BLE connection screen
        canvas.fillSprite(0x434343u); // Grey
        canvas.setTextColor(TFT_BLACK);
        canvas.setTextSize(2.75);
        canvas.drawString("CTRLs", 40, 40);
        canvas.drawString("no BLE", 40, 80);
    }
}