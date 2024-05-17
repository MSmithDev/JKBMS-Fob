#ifndef WIDGETS_HPP
#define WIDGETS_HPP

#include "LovyanGFX.hpp"
#include "helpers/utils.h"
#include "helpers/jkbms.h"

class UIWidgets {
public:
    static void navBar(LGFX_Sprite canvas, bool UpKey, bool SelectKey, bool DownKey);
    static void fobBattery(LGFX_Sprite canvas, int x, int y, int w, int h, int percentage);
    static void statusBar(LGFX_Sprite canvas, GlobalState *globalState, JKBMSData *jkbmsData);
    static void searchingIcon(LGFX_Sprite canvas, int x, int y, int radius, int thickness);

    static void bmsGauge(LGFX_Sprite canvas, int x, int y, int rad, int thickness, float min, float max, float value, char* unit, unsigned int color, int precision);
    static void TemperatureBox(LGFX_Sprite canvas, int x, int y, int w, int h, float mosfet, float t1, float t2, unsigned int color);

    static void bmsBattery(LGFX_Sprite canvas, int x, int y, int w, int h, JKBMSData *jkData);

    static void arrowLabel(LGFX_Sprite canvas, int x, int y, bool up, bool down, int textSize, char* label);

    static void menuBoxRssi(LGFX_Sprite canvas, int x, int y, int w, int h, int selected, int numDevices, BLEScan scans[]);
};  



#endif // WIDGETS_HPP
