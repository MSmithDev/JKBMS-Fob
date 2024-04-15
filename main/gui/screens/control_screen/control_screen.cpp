#include "control_screen.hpp"

void control_screen(LGFX_Sprite canvas){

                //mainScreen(bgSprite, testRecv);
                canvas.fillSprite(0x434343u); //Grey
                //add nav bar
                //navBar(canvas, currentStateButton0, currentStateSelectKey, currentStateButton2);
                canvas.setTextColor(TFT_BLACK);
                canvas.setTextSize(2.75);
                canvas.drawString("CNTRL screen", 40, 40);
                
}