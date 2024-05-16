#include "main_screen.hpp"
#include "gui/widgets/widgets.hpp"



void main_screen(LGFX_Sprite canvas, GlobalState *globalState, JKBMSData *jkData){

                bool bleConnected = globalState->bleConnected;

                //If BLE is not connected, display a message and searching indicator
                if(!bleConnected) {
                    canvas.fillSprite(0x434343u); //Grey
                    canvas.setTextColor(TFT_BLACK);
                    canvas.setTextSize(2.75);
                    canvas.drawString("Searching", 32, 32);
                    UIWidgets::searchingIcon(canvas, 120, 80, 15, 5);
                } 
                
                //If BLE is connected, display BMS data
                else {
                    //canvas.fillSprite(0x434343u); //Grey
                    canvas.fillSprite(TFT_BLACK); //Black
                    canvas.setTextColor(TFT_BLACK);
                    unsigned int color;
                    
                    // Voltage gauge
                    color = Utils::getColorRedGreen(jkData->packVoltage, 70.0, 84.0);
                    UIWidgets::bmsGauge(canvas, 50, 40, 15, 8, 70.0, 84.0, jkData->packVoltage,"V", color, 1);
                    
                    // Power gauge
                    color = Utils::getColorGreenRed(jkData->packPower, 0.0, 3000.0);
                    UIWidgets::bmsGauge(canvas, 50, 75, 15, 8, 0.0, 3000.0, jkData->packPower,"W", color, 0);
                
                    //Ah remaining gauge
                    color = Utils::getColorRedGreen(jkData->capacityRemaining, 0.0, 34.0);
                    UIWidgets::bmsGauge(canvas, 50, 110, 15, 8, 0.0, 34.0, jkData->capacityRemaining,"Ah", color, 1);
                    
                    //Temperatures
                    UIWidgets::TemperatureBox(canvas, 190, 30, 50, 105, jkData->mosfetTemp, jkData->probe1Temp, jkData->probe2Temp, TFT_WHITE);

                    //Battery            (Canvas, x, y, w, h, jkData)
                    UIWidgets::bmsBattery(canvas, 130, 30, 40, 100, jkData);
                }

                
}