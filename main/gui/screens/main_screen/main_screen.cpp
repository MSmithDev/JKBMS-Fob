#include "main_screen.hpp"



void main_screen(LGFX_Sprite canvas, JKBMSData *jkData){

                canvas.fillSprite(0x434343u); //Grey
                canvas.setTextColor(TFT_BLACK);
                canvas.setTextSize(2.75);
                canvas.drawString("Main screen", 50, 40);
                
}