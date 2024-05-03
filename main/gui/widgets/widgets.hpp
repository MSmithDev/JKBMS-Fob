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
};



#endif // WIDGETS_HPP
