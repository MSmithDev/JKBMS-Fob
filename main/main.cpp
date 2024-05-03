#include "gui/gui_task.hpp"
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
#include "helpers/utils.hpp"

extern "C"
{
#include "ble/ble_task.h"
#include "tasks/fob_battery_monitor.h"
}

static const char *TAG = "Main";

JKBMSCommands jkbmsCommands = {
    // Enable Charge
    {0xAA, 0x55, 0x90, 0xEB, 0x1D, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // Disable Charge
    {0xAA, 0x55, 0x90, 0xEB, 0x1D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},

    // Enable Discharge
    {0xAA, 0x55, 0x90, 0xEB, 0x1E, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // Disable Discharge
    {0xAA, 0x55, 0x90, 0xEB, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},

    // Enable Balance
    {0xAA, 0x55, 0x90, 0xEB, 0x1F, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // Disable Balance
    {0xAA, 0x55, 0x90, 0xEB, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

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
