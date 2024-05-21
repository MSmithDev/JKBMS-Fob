
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "nvs_flash.h"
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

#define WAKE_UP_PIN GPIO_NUM_1 // Fob Select button

bool autoDeepSleep = false; // disabled for testing

// Global States
GlobalState globalState;
JKBMSData jkbmsData;

extern "C" void app_main()
{

    /*  -----------------------------
        Pin configuration
        -----------------------------  */
    
    // Deinitialize the wake-up button
    rtc_gpio_deinit(WAKE_UP_PIN);

    // Enable Power to LCD
    gpio_set_direction(GPIO_NUM_7, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_7, 1);


    /*  -----------------------------
        Initialize JKBMS State
        -----------------------------  */
    jkbmsData.canBalance = false;
    jkbmsData.canCharge = false;
    jkbmsData.canDischarge = false;



    /*  -----------------------------
        Initialize Global State
        -----------------------------  */
    globalState.bleConnected = false;
    globalState.inControl = false;
    globalState.inSettings = false;
    globalState.screenBrightness = 127;


    // Initialize NVS.
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    //read settings from NVS
    nvs_handle_t my_handle;
                esp_err_t err;
                err = nvs_open("storage", NVS_READWRITE, &my_handle);
                if (err != ESP_OK)
                {
                    ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
                }
                else
                {
                    //err = nvs_set_u8(my_handle, "screenBright", globalState.screenBrightness);
                    err = nvs_get_i16(my_handle, "screenBright", &globalState.screenBrightness);
                    if (err == ESP_OK)
                    {
                        ESP_LOGI(TAG, "NVS Read Brightness: %d", globalState.screenBrightness);
                    }
                    else
                    {
                        ESP_LOGI(TAG, "Error (%s) reading!", esp_err_to_name(err));
                    }
                    nvs_close(my_handle);
                }

    /*  -----------------------------
        Start Tasks
        -----------------------------  */

    // GUI
    xTaskCreatePinnedToCore(gui_task, "gui_task", 4096, NULL, 1, NULL, 1); // Priority 1, Core 1

    // Fob Battery Monitor
    xTaskCreatePinnedToCore(fobBatteryMonitor, "fob_battery_monitor_task", 4096, NULL, 1, NULL, 1); // Priority 1, Core 1

    // BLE
    xTaskCreatePinnedToCore(ble_task, "ble_task", 4096, NULL, 1, NULL, 0); // Priority 1, Core 0

    /*  -----------------------------
        Deep Sleep
        -----------------------------  */

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
