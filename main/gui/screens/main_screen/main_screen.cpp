#include "main_screen.hpp"
#include "gui/widgets/widgets.hpp"
#include "gui/LGFX_Config.hpp"

#include "gui/images/SearchIcon.h"
#include "gui/images/SearchIconAnim.h"

bool firstRun = true;
float searchAngle = 0.0;
extern LGFX display;

//define sprite
 LGFX_Sprite searchAnimationSprite;
 
void setupAnim() {
    searchAnimationSprite.setColorDepth(16);
    searchAnimationSprite.setSwapBytes(true);
    searchAnimationSprite.createSprite(64, 64);
    searchAnimationSprite.setPivot(32, 32);
    searchAnimationSprite.pushImage(0, 0, 64, 64, image_data_SearchIconAnim);
}

void main_screen(LGFX_Sprite canvas, GlobalState *globalState, JKBMSData *jkData)
{
    if(firstRun) {
        setupAnim();
        firstRun = false;
    }

    bool bleConnected = globalState->bleConnected;

    // If BLE is not connected, display a message and searching indicator
    if (!bleConnected)
    {
        if(globalState->hasSavedDevice)
        {
        
        canvas.fillSprite(0x000000u);
        canvas.setTextColor(0xFF9900u);
        canvas.setTextSize(2.75);
        canvas.setTextDatum(TC_DATUM);
        canvas.drawString("Searching", 135, 100);
        canvas.setTextDatum(TL_DATUM);
        canvas.pushImage(135 - 32, 30, 64, 64, image_data_SearchIcon, (uint16_t)0x07E0);

        // reset search angle >= 360
        if(searchAngle >= 360.0) searchAngle = 0.0;

        // Push rotated indicator
        searchAnimationSprite.pushRotateZoom(&canvas, 135, 62, searchAngle, 1.0, 1.0, (uint16_t)0x07E0);

        // increment search angle
        searchAngle += 10.0;

        }
        else
        {
            canvas.fillSprite(0x000000u);
            canvas.setTextColor(0xFF9900u);
            canvas.setTextSize(2.75);
            canvas.setTextDatum(TC_DATUM);
            canvas.drawString("No Saved", 135, 30);
            canvas.drawString("Device", 135, 60);
            canvas.setTextSize(2.5);
            canvas.setTextColor(0xFF9900u, 0x292929u);
            canvas.drawString(">Setup Now<", 135, 90);
            canvas.setTextDatum(TL_DATUM);

            if(globalState->selectKey)
            {
                globalState->CurrentPage = 3;
                globalState->settingsPage = 11;
                globalState->inSettings = true;

                globalState->selectKey = false;
            }
        }

    }

    // If BLE is connected, display BMS data
    else
    {
        // canvas.fillSprite(0x434343u); //Grey
        canvas.fillSprite(TFT_BLACK); // Black
        canvas.setTextColor(TFT_BLACK);
        unsigned int color;

        // Voltage gauge
        color = Utils::getColorRedGreen(jkData->packVoltage, 70.0, 84.0);
        UIWidgets::bmsGauge(canvas, 50, 40, 15, 8, 70.0, 84.0, jkData->packVoltage, "V", color, 1);

        // Power gauge
        color = Utils::getColorGreenRed(jkData->packPower, 0.0, 3000.0);
        UIWidgets::bmsGauge(canvas, 50, 75, 15, 8, 0.0, 3000.0, jkData->packPower, "W", color, 0);

        // Ah remaining gauge
        color = Utils::getColorRedGreen(jkData->capacityRemaining, 0.0, 34.0);
        UIWidgets::bmsGauge(canvas, 50, 110, 15, 8, 0.0, 34.0, jkData->capacityRemaining, "Ah", color, 1);

        // Temperatures
        UIWidgets::TemperatureBox(canvas, 190, 30, 50, 105, jkData->mosfetTemp, jkData->probe1Temp, jkData->probe2Temp, TFT_WHITE);

        // Battery            (Canvas, x, y, w, h, jkData)
        UIWidgets::bmsBattery(canvas, 130, 30, 40, 100, jkData);
    }
}