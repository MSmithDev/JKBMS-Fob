#ifndef CONTROL_SCREEN_HPP
#define CONTROL_SCREEN_HPP

#include "LovyanGFX.hpp"
#include "helpers/utils.hpp"
#include "esp_log.h"


struct ControlOption
{
    std::string name;
    bool highlighted;
};




void control_screen(LGFX_Sprite canvas, GlobalState *globalState);




#endif // CONTROL_SCREEN_HPP