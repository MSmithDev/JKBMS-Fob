
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/queue.h"
#include "esp_sleep.h"
#include "driver/rtc_io.h"
#include <iostream>
#include <random>
#include "helpers/jkbms.h"
#include "helpers/utils.h"

#include "tasks/ble_task.h"
#include "tasks/gui_task.h"
#include "tasks/fob_battery_monitor.h"


static const char *TAG = "Main";



#define WAKE_UP_PIN GPIO_NUM_1 // Select button

bool autoDeepSleep = false;

extern "C" void app_main()
{
    // revert wake up pin to normal use
    rtc_gpio_deinit(WAKE_UP_PIN);

    // Enable Power to LCD
    gpio_set_direction(GPIO_NUM_7, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_7, 1);

    // GUI Task
    xTaskCreatePinnedToCore(gui_task, "gui_task", 4096, NULL, 1, NULL, 1);
    
    // Fob Battery Monitor Task
    xTaskCreatePinnedToCore(fobBatteryMonitor, "fob_battery_monitor_task", 4096, NULL, 1, NULL, 1);

    // BLE Task
    xTaskCreatePinnedToCore(ble_task, "ble_task", 4096, NULL, 1, NULL, 0);

    //If Deep Sleep is enabled, enter deep sleep after 30 seconds
    if (autoDeepSleep)
    {
        // Deep Sleep timer TODO: Make extendable by button presses
        ESP_LOGI(TAG, "Entering deep sleep in 30 seconds...");
        vTaskDelay(pdMS_TO_TICKS(30000));

        // Configure the wake-up button
        rtc_gpio_pulldown_en(WAKE_UP_PIN);
        esp_sleep_enable_ext0_wakeup(WAKE_UP_PIN, 1);
        ESP_LOGI(TAG, "Entering deep sleep now...");

        // Enter deep sleep
        esp_deep_sleep_start();
    }
}
