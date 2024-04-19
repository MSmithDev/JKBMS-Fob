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


QueueHandle_t gui_data_queue;



void blesenderTask(void *parameter)
{


    JKBMSData test;

    gui_data_queue = xQueueCreate(5, sizeof(JKBMSData));
    while (1)
    {
        // Make dummy data
        test.avgCellVoltage = randFloat(3.0, 4.2);
        test.packVoltage = randFloat(68.0, 84.0);
        test.packPower = randFloat(0.0, 3000.0);
        test.deltaCellVoltage = randFloat(0.0, 0.1);

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
