#include "widgets.hpp"

// Images
#include "gui/images/NavBar.h"
#include "gui/images/bleStatusBlue.h"
#include <map>

// temp logging
#include "esp_log.h"

// Navbar
void UIWidgets::navBar(LGFX_Sprite canvas, bool UpKey, bool SelectKey, bool DownKey)
{

    uint16_t ActiveColor = 0xFFFF;   // Green
    uint16_t InactiveColor = 0x0000; // Black

    // three squares for the buttons
    canvas.fillRect(1, 14, 27, 27, UpKey ? ActiveColor : InactiveColor);
    canvas.fillRect(1, 54, 27, 27, !SelectKey ? ActiveColor : InactiveColor); // Inverted
    canvas.fillRect(1, 95, 27, 27, DownKey ? ActiveColor : InactiveColor);

    canvas.pushImage(0, 0, 30, 135, image_data_NavBar, (uint16_t)0x07E0); // Add NavBar with transparent color
}

// Fob Battery Indicator
void UIWidgets::fobBattery(LGFX_Sprite canvas, int x, int y, int w, int h, int percentage)
{

    int batteryRadius = 5;
    // draw battery outline
    canvas.drawRoundRect(x, y, w, h, batteryRadius, TFT_WHITE);

    // draw positive terminal
    canvas.fillRoundRect(x + w - 1, y + (h / 2) - 4, 4, 8, 3, TFT_WHITE);

    // draw battery level
    canvas.fillRoundRect(x + 1, y + 1, (w - 2) * (percentage / 100.0), h - 2, 5, getBatteryColor(percentage, 0, 100));
}

void UIWidgets::statusBar(LGFX_Sprite canvas, GlobalState *globalState, JKBMSData *jkbmsData)
{

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

    // Testing
    jkbmsData->canCharge ? canvas.setTextColor(TFT_GREEN) : canvas.setTextColor(TFT_RED);
    canvas.drawString("[ ]", 60, 3);

    jkbmsData->canDischarge ? canvas.setTextColor(TFT_GREEN) : canvas.setTextColor(TFT_RED);
    canvas.drawString("[ ]", 100, 3);

    jkbmsData->canBalance ? canvas.setTextColor(TFT_GREEN) : canvas.setTextColor(TFT_RED);
    canvas.drawString("[ ]", 140, 3);

    // labels
    canvas.setTextColor(TFT_WHITE);
    canvas.setTextSize(1.5);
    canvas.drawString("C", 75, 5);
    canvas.drawString("D", 115, 5);
    canvas.drawString("B", 155, 5);

    // Fob Battery
    fobBattery(canvas, 190, 0, 40, 20, globalState->batteryPercentage);
}

// Searching Icon Animation
int searchingAngle = 0;
void UIWidgets::searchingIcon(LGFX_Sprite canvas, int x, int y, int r, int t)
{
    searchingAngle += 10;
    if (searchingAngle >= 360)
    {
        searchingAngle = 0;
    }

    canvas.fillArc(x, y, r - t, r, 0, 0 + searchingAngle, TFT_WHITE);
}

// BMS Gauge
void UIWidgets::bmsGauge(LGFX_Sprite canvas, int x, int y, int rad, int thickness, float min, float max, float value, char *unit, unsigned int color, int precision)
{

    // mabye store current font and restore it after drawing?
    // canvas.getFont();

    unsigned int ThemeColor = 0xbabcbbu;

    int mappedValue = Utils::mapFloatToInt(value, min, max, 90, 360); // Map the value to the gauge

    // Draw the gauge
    canvas.fillArc(x, y, rad - 1, (rad + 1) - thickness, 90, mappedValue, color);

    // Draw outline pass 1-2
    canvas.drawArc(x, y, rad, rad - thickness, 90, 360, ThemeColor);
    // canvas.drawArc(x, y, rad-1, (rad+1)-thickness, 90, 360, ThemeColor);

    // set label datum
    canvas.setTextDatum(TL_DATUM);
    canvas.setTextSize(0.75);
    canvas.setTextColor(ThemeColor);
    canvas.setFont(&fonts::Font4);
    std::string valueStr = floatToString(value, precision) + " " + unit;
    canvas.drawString(valueStr.c_str(), x + 5, y + 2);
}

// Temperature readouts

void UIWidgets::TemperatureBox(LGFX_Sprite canvas, int x, int y, int w, int h, float mosfet, float t1, float t2, unsigned int color)
{

    unsigned int ThemeColor = 0xbabcbbu;

    // Container
    // canvas.drawRect(x, y, w, h, TFT_ORANGE);
    canvas.setTextDatum(TC_DATUM);
    canvas.setFont(&fonts::Font2);
    canvas.setTextColor(ThemeColor);

    // Mosfet
    canvas.setTextColor(ThemeColor);
    canvas.setTextSize(1);
    canvas.drawString("Mosfet", x + (w / 2), y);
    // canvas.drawFastHLine(x, y+16, w, TFT_ORANGE);
    std::string valueStr = floatToString(mosfet, 1) + "C";
    canvas.setTextSize(1.1);
    canvas.setTextColor(Utils::getColorGreenRed(mosfet, 0, 80));
    canvas.drawString(valueStr.c_str(), x + (w / 2), y + 18);

    // Temp1
    canvas.setTextColor(ThemeColor);
    canvas.setTextSize(1);
    // canvas.drawFastHLine(x, y+35, w, TFT_ORANGE);
    canvas.drawString("Temp 1", x + (w / 2), y + 35);
    // canvas.drawFastHLine(x, y+51, w, TFT_ORANGE);
    valueStr = floatToString(t1, 1) + "C";
    canvas.setTextSize(1.1);
    canvas.setTextColor(Utils::getColorGreenRed(t1, 0, 80));
    canvas.drawString(valueStr.c_str(), x + (w / 2), y + 53);

    // Temp2
    canvas.setTextColor(ThemeColor);
    canvas.setTextSize(1);
    // canvas.drawFastHLine(x, y+70, w, TFT_ORANGE);
    canvas.drawString("Temp 2", x + (w / 2), y + 70);
    // canvas.drawFastHLine(x, y+86, w, TFT_ORANGE);
    valueStr = floatToString(t2, 1) + "C";
    canvas.setTextSize(1.1);
    canvas.setTextColor(Utils::getColorGreenRed(t2, 0, 80));
    canvas.drawString(valueStr.c_str(), x + (w / 2), y + 88);
}

void UIWidgets::bmsBattery(LGFX_Sprite canvas, int x, int y, int w, int h, JKBMSData *jkData)
{
    unsigned int ThemeColor = 0xbabcbbu;
    unsigned int CellLineColor = 0x000000;

    int NumberCells = 10;
    int batteryRadius = 5;

    // Draw battery level
    int fillHeight = (h - 2) * (jkData->packPercentage / 100.0); // Adjust the height to fit within the outline
    canvas.fillRoundRect(x + 1, y + h - 1 - fillHeight, w - 2, fillHeight, batteryRadius, getBatteryColor(jkData->packPercentage, 0, 100));

    // Draw battery outline
    canvas.drawRoundRect(x, y, w, h, batteryRadius - 1, ThemeColor);

    // Draw positive terminal on top center
    canvas.fillRoundRect(x + (w / 2) - 4, y - 2, 8, 4, 3, ThemeColor);

    // Draw lines for each cell
    for (int i = 1; i < NumberCells; i++)
    {

        // canvas.drawFastHLine(x+1, y-1 + (i * (h / NumberCells)), w-2, CellLineColor);
        canvas.drawFastHLine(x + 1, y + (i * (h / NumberCells)), w - 2, CellLineColor);
        // canvas.drawFastHLine(x+1, y+1 + (i * (h / NumberCells)), w-2, CellLineColor);
    }

    // Draw Empty/Full Labels
    canvas.setTextColor(TFT_RED);
    canvas.setTextSize(1);
    canvas.drawString("E", x + w + 5, y + h - 10);

    canvas.setTextColor(TFT_GREEN);
    canvas.drawString("F", x + w + 5, y + 5);
}

void UIWidgets::menuBoxRssi(LGFX_Sprite canvas, int x, int y, int w, int h, int selected, int numDevices, BLEScan scans[]) {
    // Define colors
    unsigned int textColor = 0xFFFFFF;
    unsigned int selectedColor = 0xFF9900;
    unsigned int selectedBackgroundColor = 0x292929;
    unsigned int backColor = 0xFF0000;

    // Menu item configuration
    int itemHeight = 20;
    int startX = x + 10;
    int startY = y + 10;

    // Calculate the range of items to display
    int startIndex = (selected > 1) ? selected - 1 : 0;
    int endIndex = (numDevices > startIndex + 2) ? startIndex + 2 : numDevices - 1;

    // Set text size
    canvas.setTextSize(1.5);

    // Draw the menu items
    for (int i = startIndex; i <= endIndex; ++i) {
        // Determine text color and background color
        if (i == 0 && selected != 0) {
            // Special case for "Back" item when not selected
            canvas.setTextColor(backColor);
        } else if (i == selected) {
            // Selected item
            if (i == 0) {
                // Selected "Back" item
                canvas.setTextColor(backColor, selectedBackgroundColor);
            } else {
                // Regular selected item
                canvas.setTextColor(selectedColor, selectedBackgroundColor);
            }
        } else {
            // Regular unselected item
            canvas.setTextColor(textColor);
        }

        // Draw the device name
        int drawX = (i == 0) ? startX - 20 : startX;
        canvas.drawString(scans[i].deviceName, drawX, startY + (i - startIndex) * itemHeight);

        // Draw RSSI label or value
        canvas.setTextDatum(TR_DATUM);
        if (i == 0) {
            // RSSI label for "Back" item
            canvas.setTextColor(TFT_RED);
            canvas.drawString("RSSI", x + w - 10, startY + (i - startIndex) * itemHeight);
        } else {
            // RSSI value for regular items
            canvas.drawString(intToString(scans[i].rssi), x + w - 10, startY + (i - startIndex) * itemHeight);
        }
        canvas.setTextDatum(TL_DATUM);
    }
}



