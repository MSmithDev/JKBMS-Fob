#ifndef GUI_TASK_HPP
#define GUI_TASK_HPP

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

struct JKBMSData{
    float packVoltage;
    float packPower;
    float avgCellVoltage;
    float deltaCellVoltage;
    float cellVoltages[24];
    float cellResistances[24];
};

extern "C" {

 void gui_task(void *pvParameters);
}

#endif // GUI_TASK_HPP
