#ifndef BLE_TASK_HPP
#define BLE_TASK_HPP

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "helpers/jkbms.hpp"

extern "C" {

 void ble_task(void *pvParameters);

}

#endif // BLE_TASK_HPP
