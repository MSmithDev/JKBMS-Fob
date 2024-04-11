#include "gui/gui_task.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"



extern "C" void app_main() {
    // Enable Power to LCD
    gpio_set_direction(GPIO_NUM_7, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_7, 1);

    // Create a task to run the GUI
    xTaskCreatePinnedToCore(gui_task, "gui_task", 4096, NULL, 1, NULL, 1);
}
