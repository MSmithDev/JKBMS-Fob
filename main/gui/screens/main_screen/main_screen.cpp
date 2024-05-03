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
                    canvas.fillSprite(0x434343u); //Grey
                    canvas.setTextColor(TFT_BLACK);
                    canvas.setTextSize(2);
                    canvas.drawString("Raw Data", 32, 22);
                    
                    canvas.setTextSize(1.5);
                    //Extract Strings
                    std::string voltage = "Voltage: " + floatToString(jkData->packVoltage, 2);
                    std::string current = "Power: " + floatToString(jkData->packPower, 2);
                    std::string avgCellV = "Avg Cell V: " + floatToString(jkData->avgCellVoltage, 3);
                    std::string cycleAh = "Cycle Ah: " + floatToString(jkData->cycleAh, 2);
                    std::string cycles = "Cycles: " + std::to_string(jkData->cycleCount);
                    std::string percent = "Percent: " + std::to_string(jkData->packPercentage) + "%";

                    canvas.drawString(voltage.c_str(), 40, 40);
                    canvas.drawString(current.c_str(), 40, 55);
                    canvas.drawString(avgCellV.c_str(), 40, 70);
                    canvas.drawString(cycleAh.c_str(), 40, 85);
                    canvas.drawString(cycles.c_str(), 40, 100);
                    canvas.drawString(percent.c_str(), 40, 115);
                    
                
                }

                
}