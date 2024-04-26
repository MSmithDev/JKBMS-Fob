#include "gui/gui_task.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/queue.h"

#include <iostream>
#include <random>


#include "helpers/jkbms.h"
#include "helpers/utils.hpp"

extern "C" {
    #include "ble/ble_task.h"
}

static const char *TAG = "Main";


extern "C" void app_main()
{
    // Enable Power to LCD
    gpio_set_direction(GPIO_NUM_7, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_7, 1);

    // Create a task to run the GUI
    xTaskCreatePinnedToCore(gui_task, "gui_task", 4096, NULL, 1, NULL, 1);
    
    //BLE Task
    xTaskCreatePinnedToCore(ble_task, "ble_task", 4096, NULL, 1, NULL, 0);
}
