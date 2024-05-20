#include "fob_battery_monitor.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include <max1704x.h>
#include "helpers/utils.h"

extern struct GlobalState globalState;
// Logging tag
static const char *TAG = "FobBatteryMonitor";

// i2c pins
#define I2C_MASTER_SDA GPIO_NUM_3
#define I2C_MASTER_SCL GPIO_NUM_4

void fobBatteryMonitor(void *pvParameters)
{
    ESP_ERROR_CHECK(i2cdev_init());
    esp_err_t r;
    max1704x_t dev = {0};
    max1704x_config_t config = {0};
    max1704x_status_t status = {0};
    uint16_t version = 0;
    float voltage = 0;
    float soc_percent = 0;
    float rate_change = 0;

    /**
     * Set up I2C bus to communicate with MAX1704X
     */

    dev.model = MAX17048_9;

    ESP_ERROR_CHECK(max1704x_init_desc(&dev, 0, I2C_MASTER_SDA, I2C_MASTER_SCL));
    ESP_ERROR_CHECK(max1704x_quickstart(&dev));
    ESP_ERROR_CHECK(max1704x_get_version(&dev, &version));
    ESP_LOGI(TAG, "Version: %d\n", version);
    /**
     * Get current MAX1704X voltage, SOC, and rate of change every 5 seconds
     */

    while (1)
    {
        r = max1704x_get_voltage(&dev, &voltage);

        if (r == ESP_OK)
        {
            ESP_LOGI(TAG, "Voltage: %.2fV , Percent: %f", voltage, soc_percent);
        }
        else
        {
            ESP_LOGI(TAG, "Error %d: %s", r, esp_err_to_name(r));
        }

        r = max1704x_get_soc(&dev, &soc_percent);
        if (r == ESP_OK)
        {
            // ESP_LOGI(TAG, "SOC: %.2f%%", soc_percent);
            globalState.batteryPercentage = soc_percent;
        }
        else
        {
            ESP_LOGI(TAG, "Error %d: %s", r, esp_err_to_name(r));
        }

        r = max1704x_get_crate(&dev, &rate_change);
        if (r == ESP_OK)
        {
            // ESP_LOGI(TAG, "SOC rate of change: %.2f%%", rate_change);
        }
        else
        {
            ESP_LOGI(TAG, "Error %d: %s", r, esp_err_to_name(r));
        }
        
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}