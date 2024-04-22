#include "info_screen.hpp"

void info_screen(LGFX_Sprite canvas, GlobalState *globalState, JKBMSData *jkData)
{

    if (globalState->bleConnected)
    {
        canvas.fillSprite(0x434343u); // Grey
        canvas.setTextColor(TFT_BLACK);
        canvas.setTextSize(2);
        canvas.drawString("Raw Data", 32, 22);

        //TODO: Change  how this works, they all won't fit on the screen
        // 4 columns
        for (int x = 1; x < 5; x++)
        {
            // 6 rows of data
            std::string cell1 = "C:" + floatToString(jkData->cellVoltages[x * 1], 2);
            std::string cell2 = "C:" + floatToString(jkData->cellVoltages[x * 2], 2);
            std::string cell3 = "C:" + floatToString(jkData->cellVoltages[x * 3], 2);
            std::string cell4 = "C:" + floatToString(jkData->cellVoltages[x * 4], 2);
            std::string cell5 = "C:" + floatToString(jkData->cellVoltages[x * 5], 2);
            std::string cell6 = "C:" + floatToString(jkData->cellVoltages[x * 6], 2);

            canvas.setTextSize(1.5);
            canvas.drawString(cell1.c_str(), -30+(60 * x), 40);
            canvas.drawString(cell2.c_str(), -30+(60 * x), 55);
            canvas.drawString(cell3.c_str(), -30+(60 * x), 70);
            canvas.drawString(cell4.c_str(), -30+(60 * x), 85);
            canvas.drawString(cell5.c_str(), -30+(60 * x), 100);
            canvas.drawString(cell6.c_str(), -30+(60 * x), 115);

        }
    }
    else
    {
        canvas.fillSprite(0x434343u); // Grey
        canvas.setTextColor(TFT_BLACK);
        canvas.setTextSize(2.75);
        canvas.drawString("Info screen", 40, 40);
        canvas.drawString("no BLE", 40, 80);
    }
}