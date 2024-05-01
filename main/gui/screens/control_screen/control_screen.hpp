#ifndef CONTROL_SCREEN_HPP
#define CONTROL_SCREEN_HPP

#include "LovyanGFX.hpp"
#include "helpers/utils.hpp"
#include "helpers/jkbms.h"
#include "esp_log.h"
#include "freertos/queue.h"


struct ControlOption
{
    std::string name;
    bool highlighted;
};




void control_screen(LGFX_Sprite canvas, GlobalState *globalState, JKBMSData *jkbmsData);




#endif // CONTROL_SCREEN_HPP