#include "gui/gui_task.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/queue.h"

#include <iostream>
#include <random>

#include "ble/ble_task.hpp"
#include "helpers/jkbms.hpp"

static const char *TAG = "Main";
















QueueHandle_t gui_data_queue;

// Function to generate a random float in the range [min_val, max_val]
float randFloat(float min_val, float max_val)
{
    // Initialize the random number generator only once
    static int seed_initialized = 0;
    if (!seed_initialized)
    {
        srand(time(NULL)); // Use current time as seed for random generator
        seed_initialized = 1;
    }

    // Generate a random float in the range [0, 1]
    float scale = rand() / (float)RAND_MAX;

    // Adjust the scale to the desired range and return
    return min_val + scale * (max_val - min_val);
}

void blesenderTask(void *parameter)
{
    int arrayToSend[10] = {84, 100, 0, 3, 4, 5, 6, 7, 8, 9};

    JKBMSData test;

    gui_data_queue = xQueueCreate(5, sizeof(JKBMSData));
    while (1)
    {
        // Make dummy data
        test.avgCellVoltage = 4.0;
        test.packVoltage = randFloat(68.0, 84.0);
        test.packPower = 0.0;
        test.deltaCellVoltage = 0.05;

        // cell voltages test data is only 20s
        for (int i = 0; i < 24; i++)
        {
            if (i < 20)
                test.cellVoltages[i] = 3.3;
            else
                test.cellVoltages[i] = 0;
        }

        // cell resistances
        for (int i = 0; i < 24; i++)
        {
            if (i < 20)
                test.cellResistances[i] = 1.23;
            else
                test.cellResistances[i] = 0;
        }

        if (xQueueSend(gui_data_queue, &(test), portMAX_DELAY) != pdPASS)
        {
            ESP_LOGI(TAG, "Failed to send array to queue");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

extern "C" void app_main()
{
    // Enable Power to LCD
    gpio_set_direction(GPIO_NUM_7, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_7, 1);

    // Create a task to run the GUI
    xTaskCreatePinnedToCore(gui_task, "gui_task", 4096, NULL, 1, NULL, 1);

    // Create test sender
    xTaskCreatePinnedToCore(blesenderTask, "bleSender_Task", 4096, NULL, 0, NULL, 0);

    //BLE Task
    xTaskCreatePinnedToCore(ble_task, "ble_task", 4096, NULL, 1, NULL, 0);
}
