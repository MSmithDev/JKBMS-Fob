#ifndef GUI_TASK_HPP
#define GUI_TASK_HPP

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#include "helpers/jkbms.hpp"

extern "C" {

 void gui_task(void *pvParameters);

}

#endif // GUI_TASK_HPP
