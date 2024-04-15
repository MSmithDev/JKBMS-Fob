#include "settings_screen.hpp"
#include "helpers/page_states.hpp"

void settings_screen(LGFX_Sprite canvas, settingsPageState pageState)
{
    // if show options is true, display the options
    if(pageState.showOptions) {

    switch (pageState.currentSelection)
    {
    case 1:
        canvas.fillSprite(0x434343u); // Grey
        canvas.setTextColor(TFT_BLACK);
        canvas.setTextSize(2.75);
        canvas.drawString("BLE Setup", 40, 40);
        break;
    
    case 2:
        canvas.fillSprite(0x434343u); // Grey
        canvas.setTextColor(TFT_BLACK);
        canvas.setTextSize(2.75);
        canvas.drawString("Brightness", 40, 40);
        break;
    
    case 3:
        canvas.fillSprite(0x434343u); // Grey
        canvas.setTextColor(TFT_BLACK);
        canvas.setTextSize(2.75);
        canvas.drawString("Sleep Modes", 40, 40);
        break;

    case 4:
        canvas.fillSprite(0x434343u); // Grey
        canvas.setTextColor(TFT_BLACK);
        canvas.setTextSize(2.75);
        canvas.drawString("Back", 40, 40);
        break;
    
    };
    }
    else {
        canvas.fillSprite(0x434343u); // Grey
        canvas.setTextColor(TFT_BLACK);
        canvas.setTextSize(2.75);
        canvas.drawString("Settings", 40, 40);
    }

}