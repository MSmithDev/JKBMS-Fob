#include "control_screen.hpp"
#include "gui/images/SettingsIcon.h"
#include "gui/images/CommandsIcon.h"

extern QueueHandle_t ble_sender_queue;

#define TAG "control_screen"

int numOptions = 4;
ControlOption ctrlOption[4] = {
    {"Toggle Charge", false},
    {"Toggle Discharge", false},
    {"Toggle Balance", false},
    {"Back", false}};

void send_ble_cmd(uint16_t characteristic, uint8_t data[20])
{
    BLECmd cmd;
    cmd.characteristic = characteristic;
    memcpy(cmd.data, data, 20);
    xQueueSend(ble_sender_queue, &cmd, portMAX_DELAY);
}

void control_screen(LGFX_Sprite canvas, GlobalState *globalState, JKBMSData *jkbmsData)
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

        canvas.fillSprite(0x000000u);
        canvas.setTextColor(0xFF9900u);
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
                canvas.setTextColor(0xFF9900u, 0x292929u);
            }
            else
            {
                canvas.setTextColor(TFT_WHITE);
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

                ESP_LOGI(TAG, "Toggle charge command triggered");

                // Toggle charge enable/disable based on current state
                if (jkbmsData->canCharge)
                {
                    send_ble_cmd(0xFFE1, jkbmsCommands.disableCharge);
                }
                else
                {
                    send_ble_cmd(0xFFE1, jkbmsCommands.enableCharge);
                }

                break;

            case 1:

                ESP_LOGI(TAG, "Toggle discharge command triggered");

                // Toggle discharge enable/disable based on current state
                if (jkbmsData->canDischarge)
                {
                    send_ble_cmd(0xFFE1, jkbmsCommands.disableDischarge);
                }
                else
                {
                    send_ble_cmd(0xFFE1, jkbmsCommands.enableDischarge);
                }

                break;

            case 2:

                ESP_LOGI(TAG, "Toggle balance command triggered");

                // Toggle balance enable/disable based on current state
                if (jkbmsData->canBalance)
                {
                    send_ble_cmd(0xFFE1, jkbmsCommands.disableBalance);
                }
                else
                {
                    send_ble_cmd(0xFFE1, jkbmsCommands.enableBalance);
                }

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
        canvas.fillSprite(0x000000u);
        canvas.setTextColor(0xFF9900u);
        canvas.setTextSize(2.75);
        canvas.setTextDatum(TC_DATUM);
        canvas.drawString("Commands", 135, 100);
        canvas.setTextDatum(TL_DATUM);
        canvas.pushImage(135-32, 30, 64, 64, image_data_CommandsIcon, (uint16_t)0x07E0);
    }
    else if (!globalState->bleConnected)
    {

        canvas.fillSprite(0x000000u);
        canvas.setTextColor(0xFF9900u);
        canvas.setTextSize(2.75);
        canvas.setTextDatum(TC_DATUM);
        canvas.drawString("CMD NO BLE", 135, 100);
        canvas.setTextDatum(TL_DATUM);
        canvas.pushImage(135-32, 30, 64, 64, image_data_SettingsIcon, (uint16_t)0x07E0);
    }
}