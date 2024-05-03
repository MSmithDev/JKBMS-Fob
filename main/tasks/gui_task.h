#ifndef GUI_TASK_H
#define GUI_TASK_H

#ifdef __cplusplus
extern "C" {
#endif
    
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"



 void gui_task(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif // GUI_TASK_H
