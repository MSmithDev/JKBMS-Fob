#include "info_screen.hpp"
#include "gui/images/CommandsIcon.h"

void info_screen(LGFX_Sprite canvas, GlobalState *globalState, JKBMSData *jkData)
{

    if (globalState->bleConnected)
    {
        canvas.fillSprite(0x000000u);
        canvas.setTextColor(0xFF9900u);
        canvas.setTextSize(2);
        canvas.drawString("Info page", 32, 22);

    }
    else
    {
        canvas.fillSprite(0x000000u);
        canvas.setTextColor(0xFF9900u);
        canvas.setTextSize(2.75);
        canvas.setTextDatum(TC_DATUM);
        canvas.drawString("Info No Ble", 135, 100);
        canvas.setTextDatum(TL_DATUM);
        canvas.pushImage(135-32, 30, 64, 64, image_data_CommandsIcon, (uint16_t)0x07E0);
    }
}