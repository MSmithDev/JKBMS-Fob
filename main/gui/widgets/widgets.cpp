#include "widgets.hpp"

//Images
#include "gui/images/NavBar.h"
#include "gui/images/bleStatusBlue.h"

//Navbar
void UIWidgets::navBar(LGFX_Sprite canvas, bool UpKey, bool SelectKey, bool DownKey) {
    
    uint16_t ActiveColor = 0xFFFF;   // Green
    uint16_t InactiveColor = 0x0000; // Black

    // three squares for the buttons
    canvas.fillRect(1, 14, 27, 27, UpKey ? ActiveColor : InactiveColor);
    canvas.fillRect(1, 54, 27, 27, !SelectKey ? ActiveColor : InactiveColor); // Inverted
    canvas.fillRect(1, 95, 27, 27, DownKey ? ActiveColor : InactiveColor);

    canvas.pushImage(0, 0, 30, 135, image_data_NavBar, (uint16_t)0x07E0); // Add NavBar with transparent color

}

//Fob Battery Indicator
void UIWidgets::fobBattery(LGFX_Sprite canvas, int x, int y, int w, int h, int percentage) {
    
    int batteryRadius = 5;
    // draw battery outline
    canvas.drawRoundRect(x, y, w, h, batteryRadius, TFT_WHITE);

    // draw positive terminal
    canvas.fillRoundRect(x + w - 1, y + (h / 2) - 4, 4, 8, 3, TFT_WHITE);

    // draw battery level
    canvas.fillRoundRect(x + 1, y + 1, (w - 2) * (percentage / 100.0), h - 2, 5, getBatteryColor(percentage, 0, 100));

}

void UIWidgets::statusBar(LGFX_Sprite canvas, GlobalState *globalState, JKBMSData *jkbmsData) {
    
    int bleStatusX = 35;
    int bleStatusY = 0;
    bool isConnected = globalState->bleConnected;

    // Black Bar
    canvas.fillRect(30, 0, 210, 20, TFT_BLACK);

    // BLE Status
    canvas.fillRect(bleStatusX + 5, bleStatusY + 2, 10, 16, isConnected ? TFT_GREEN : TFT_RED);
    canvas.pushImage(bleStatusX, bleStatusY, 20, 20, image_data_bleStatusBlue, (uint16_t)0x07E0); // Add BLE Status with transparent color

    // placeholder for icons
    canvas.setTextColor(TFT_WHITE);
    canvas.setTextSize(2);

    //Testing
    jkbmsData->canCharge ? canvas.setTextColor(TFT_GREEN) : canvas.setTextColor(TFT_RED);
    canvas.drawString("[ ]", 60, 3);

    jkbmsData->canDischarge ? canvas.setTextColor(TFT_GREEN) : canvas.setTextColor(TFT_RED);
    canvas.drawString("[ ]", 100, 3);

    jkbmsData->canBalance ? canvas.setTextColor(TFT_GREEN) : canvas.setTextColor(TFT_RED);
    canvas.drawString("[ ]", 140, 3);

    //labels
    canvas.setTextColor(TFT_WHITE);
    canvas.setTextSize(1.5);
    canvas.drawString("C", 75, 5);
    canvas.drawString("D", 115, 5);
    canvas.drawString("B", 155, 5);

    // Fob Battery
    fobBattery(canvas, 190, 0, 40, 20, globalState->batteryPercentage);
    

}