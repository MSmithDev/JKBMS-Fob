#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "nvs.h"
#include "nvs_flash.h"

#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "ble_task.h"
#include "nvs_flash.h"

#include "helpers/jkbms.h"

//Log tag
#define TAG "BLE_Task"

// JK-BMS UUIDs
#define REMOTE_SERVICE_UUID 0xFFE0
#define REMOTE_NOTIFY_CHAR_UUID 0xFFE1

// BLE Profile
#define PROFILE_NUM 1
#define PROFILE_A_APP_ID 0
#define INVALID_HANDLE 0

//JK-BMS Data structs
struct BLEScan bleScan[20];
struct JKBMSData jkbmsData;

int packetChunk = 0; // 0 = rdy, 1-3 packets
int packetType = 0;  // 1 = info, 2 = cells

// Queue handles
QueueHandle_t bleScan_data_queue;
QueueHandle_t bleConnection;
QueueHandle_t jkbms_data_queue;

// Auto connect device name
char remote_device_name[32] = "JK-BD6A20S6P";

//State variables
static bool connect = false;
static bool deviceReady = false;
static bool get_server = false;
static esp_gattc_char_elem_t *char_elem_result = NULL;
static esp_gattc_descr_elem_t *descr_elem_result = NULL;

uint16_t gatc_if = 0xff;

/* Declare static functions */
static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
static void esp_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);
static void gattc_profile_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);

void decodeJKBMSData(uint8_t *data, uint16_t len, int *packetChunk, int *packetType)
{

    // Check if info packet in first 5 bytes
    uint8_t expected_infopacket[5] = {0x55, 0xAA, 0xEB, 0x90, 0x03};
    uint8_t expected_cellpacket[5] = {0x55, 0xAA, 0xEB, 0x90, 0x02};

    // Check if the data is a valid JKBMS packet
    if (len >= 20)
    {
        // Check what packet we are receiving
        if (memcmp(data, expected_infopacket, 5) == 0)
        {
            *packetType = 1;
            *packetChunk = 0;
        }
        else if (memcmp(data, expected_cellpacket, 5) == 0)
        {
            *packetType = 2;
            *packetChunk = 1;
        }
        else
        {
            if (*packetChunk > 0)
            {
                *packetType = 2;
            }
            else
            {
                *packetType = 0;
            }
        }
        // Handle each packet type
        switch (*packetType)
        {
        case 1: // Info packet

            break;

        case 2: // Cell packet
            //ESP_LOGI(TAG, "Got Cell packet! Packet Type: %i Packet Chunk: %i", *packetType, *packetChunk);

            switch (*packetChunk) // 1-3 packets
            {
            case 1:
                ESP_LOGI(TAG, "Processing packet 1 LEN[%i]", len);
                // esp_log_buffer_hex(TAG, data, len);

                // Packet 1 length should be 128
                if (len == 128)
                {

                    // Cell voltages [6:46]
                    for (int i = 0; i < 23; i++)
                    {
                        jkbmsData.cellVoltages[i] = (float)((data[7 + (i * 2)] << 8) | data[6 + (i * 2)]) / 1000.0;
                    }

                    // Cell Average Voltage [58-59]
                    jkbmsData.avgCellVoltage = (float)((data[59] << 8) | data[58]) / 1000.0;

                    // Get Cell Delta Voltage [60:61]
                    jkbmsData.deltaCellVoltage = (float)((data[61] << 8) | data[60]) / 1000.0;

                    // Get Cell Resistances [64:104]
                    for (int i = 0; i < 23; i++)
                    {
                        jkbmsData.cellResistances[i] = (float)((data[65 + (i * 2)] << 8) | data[64 + (i * 2)]) / 1000.0;
                    }

                    // Get Battery Voltage [118:121]
                    jkbmsData.packVoltage = (float)((data[121] << 24) | (data[120] << 16) | (data[119] << 8) | data[118]) / 1000.0;

                    // Get Battery Power (Watts) [122:125]
                    jkbmsData.packPower = (float)((data[125] << 24) | (data[124] << 16) | (data[123] << 8) | data[122]) / 1000.0;
                }

                // Next packet should be 2
                if (*packetChunk == 1)
                {
                    *packetChunk = 2;
                }
                break;

            case 2:
                ESP_LOGI(TAG, "Processing packet 2 LEN[%i]", len);
                // esp_log_buffer_hex(TAG, data, len);
                //  Next packet should be 3
                if (*packetChunk == 2)
                {
                    *packetChunk = 3;
                }
                break;

            case 3:
                ESP_LOGI(TAG, "Processing packet 3 LEN[%i]", len);
                // esp_log_buffer_hex(TAG, data, len);

                // Packet 3 length should be 128
                if (len == 128)
                {

                    // get cycle count r2[0:3]
                    jkbmsData.cycleCount = (int)((data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0]);

                    // Get cycle Ah [4:7]
                    jkbmsData.cycleAh = (float)((data[7] << 24) | (data[6] << 16) | (data[5] << 8) | data[4]) / 1000.0;

                    // Get Current Charge [39:40]
                    jkbmsData.currentCharge = (float)((data[40] << 8) | data[39]) / 1000.0;

                    // Get Current Discharge [41:42]
                    jkbmsData.currentDischarge = (float)((data[42] << 8) | data[41]) / 1000.0;
                }

                // Next packet should be 0
                if (*packetChunk == 3)
                {
                    *packetChunk = 0;
                }
                break;

            default:
                ESP_LOGI(TAG, "Got Chunk 0 in cell packet!");
                break;
            }

            break;

        default:
            break;
        }
    }
}

static esp_bt_uuid_t remote_filter_service_uuid = {
    .len = ESP_UUID_LEN_16,
    .uuid = {
        .uuid16 = REMOTE_SERVICE_UUID,
    },
};

static esp_bt_uuid_t remote_filter_char_uuid = {
    .len = ESP_UUID_LEN_16,
    .uuid = {
        .uuid16 = REMOTE_NOTIFY_CHAR_UUID,
    },
};

static esp_bt_uuid_t notify_descr_uuid = {
    .len = ESP_UUID_LEN_16,
    .uuid = {
        .uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG,
    },
};

static esp_ble_scan_params_t ble_scan_params = {
    .scan_type = BLE_SCAN_TYPE_ACTIVE,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
    .scan_interval = 0x50,
    .scan_window = 0x30,
    .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE};

struct gattc_profile_inst
{
    esp_gattc_cb_t gattc_cb;
    uint16_t gattc_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_start_handle;
    uint16_t service_end_handle;
    uint16_t char_handle;
    esp_bd_addr_t remote_bda;
};

/* One gatt-based profile one app_id and one gattc_if, this array will store the gattc_if returned by ESP_GATTS_REG_EVT */
static struct gattc_profile_inst gl_profile_tab[PROFILE_NUM] = {
    [PROFILE_A_APP_ID] = {
        .gattc_cb = gattc_profile_event_handler,
        .gattc_if = ESP_GATT_IF_NONE, /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
    },
};

static void gattc_profile_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    esp_ble_gattc_cb_param_t *p_data = (esp_ble_gattc_cb_param_t *)param;

    switch (event)
    {
    case ESP_GATTC_REG_EVT:
        ESP_LOGI(TAG, "REG_EVT");
        esp_err_t scan_ret = esp_ble_gap_set_scan_params(&ble_scan_params);
        if (scan_ret)
        {
            ESP_LOGE(TAG, "set scan params error, error code = %x", scan_ret);
        }
        break;
    case ESP_GATTC_CONNECT_EVT:
    {
        ESP_LOGI(TAG, "ESP_GATTC_CONNECT_EVT conn_id %d, if %d", p_data->connect.conn_id, gattc_if);
        gl_profile_tab[PROFILE_A_APP_ID].conn_id = p_data->connect.conn_id;
        memcpy(gl_profile_tab[PROFILE_A_APP_ID].remote_bda, p_data->connect.remote_bda, sizeof(esp_bd_addr_t));
        ESP_LOGI(TAG, "REMOTE BDA:");
        esp_log_buffer_hex(TAG, gl_profile_tab[PROFILE_A_APP_ID].remote_bda, sizeof(esp_bd_addr_t));
        esp_err_t mtu_ret = esp_ble_gattc_send_mtu_req(gattc_if, p_data->connect.conn_id);
        gatc_if = gattc_if;
        if (mtu_ret)
        {
            ESP_LOGE(TAG, "config MTU error, error code = %x", mtu_ret);
        }
        break;
    }
    case ESP_GATTC_OPEN_EVT:
        if (param->open.status != ESP_GATT_OK)
        {
            ESP_LOGE(TAG, "open failed, status %d", p_data->open.status);
            break;
        }
        ESP_LOGI(TAG, "open success");
        break;
    case ESP_GATTC_DIS_SRVC_CMPL_EVT:
        if (param->dis_srvc_cmpl.status != ESP_GATT_OK)
        {
            ESP_LOGE(TAG, "discover service failed, status %d", param->dis_srvc_cmpl.status);
            break;
        }
        ESP_LOGI(TAG, "discover service complete conn_id %d", param->dis_srvc_cmpl.conn_id);
        esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &remote_filter_service_uuid);
        break;
    case ESP_GATTC_CFG_MTU_EVT:
        if (param->cfg_mtu.status != ESP_GATT_OK)
        {
            ESP_LOGE(TAG, "config mtu failed, error status = %x", param->cfg_mtu.status);
        }
        ESP_LOGI(TAG, "ESP_GATTC_CFG_MTU_EVT, Status %d, MTU %d, conn_id %d", param->cfg_mtu.status, param->cfg_mtu.mtu, param->cfg_mtu.conn_id);
        break;
    case ESP_GATTC_SEARCH_RES_EVT:
    {
        ESP_LOGI(TAG, "SEARCH RES: conn_id = %x is primary service %d", p_data->search_res.conn_id, p_data->search_res.is_primary);
        ESP_LOGI(TAG, "start handle %d end handle %d current handle value %d", p_data->search_res.start_handle, p_data->search_res.end_handle, p_data->search_res.srvc_id.inst_id);
        if (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_16 && p_data->search_res.srvc_id.uuid.uuid.uuid16 == REMOTE_SERVICE_UUID)
        {
            ESP_LOGI(TAG, "service found");
            get_server = true;
            gl_profile_tab[PROFILE_A_APP_ID].service_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[PROFILE_A_APP_ID].service_end_handle = p_data->search_res.end_handle;
            ESP_LOGI(TAG, "UUID16: %x", p_data->search_res.srvc_id.uuid.uuid.uuid16);
            deviceReady = true;
        }
        break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT:
        if (p_data->search_cmpl.status != ESP_GATT_OK)
        {
            ESP_LOGE(TAG, "search service failed, error status = %x", p_data->search_cmpl.status);
            break;
        }
        if (p_data->search_cmpl.searched_service_source == ESP_GATT_SERVICE_FROM_REMOTE_DEVICE)
        {
            ESP_LOGI(TAG, "Get service information from remote device");
        }
        else if (p_data->search_cmpl.searched_service_source == ESP_GATT_SERVICE_FROM_NVS_FLASH)
        {
            ESP_LOGI(TAG, "Get service information from flash");
        }
        else
        {
            ESP_LOGI(TAG, "unknown service source");
        }
        ESP_LOGI(TAG, "ESP_GATTC_SEARCH_CMPL_EVT");
        if (get_server)
        {
            uint16_t count = 0;
            esp_gatt_status_t status = esp_ble_gattc_get_attr_count(gattc_if,
                                                                    p_data->search_cmpl.conn_id,
                                                                    ESP_GATT_DB_CHARACTERISTIC,
                                                                    gl_profile_tab[PROFILE_A_APP_ID].service_start_handle,
                                                                    gl_profile_tab[PROFILE_A_APP_ID].service_end_handle,
                                                                    INVALID_HANDLE,
                                                                    &count);
            if (status != ESP_GATT_OK)
            {
                ESP_LOGE(TAG, "esp_ble_gattc_get_attr_count error");
            }

            if (count > 0)
            {
                char_elem_result = (esp_gattc_char_elem_t *)malloc(sizeof(esp_gattc_char_elem_t) * count);
                if (!char_elem_result)
                {
                    ESP_LOGE(TAG, "gattc no mem");
                }
                else
                {
                    status = esp_ble_gattc_get_char_by_uuid(gattc_if,
                                                            p_data->search_cmpl.conn_id,
                                                            gl_profile_tab[PROFILE_A_APP_ID].service_start_handle,
                                                            gl_profile_tab[PROFILE_A_APP_ID].service_end_handle,
                                                            remote_filter_char_uuid,
                                                            char_elem_result,
                                                            &count);
                    if (status != ESP_GATT_OK)
                    {
                        ESP_LOGE(TAG, "esp_ble_gattc_get_char_by_uuid error");
                    }

                    /*  Every service have only one char in our 'ESP_GATTS_DEMO' demo, so we used first 'char_elem_result' */
                    if (count > 0 && (char_elem_result[0].properties & ESP_GATT_CHAR_PROP_BIT_NOTIFY))
                    {
                        gl_profile_tab[PROFILE_A_APP_ID].char_handle = char_elem_result[0].char_handle;
                        esp_ble_gattc_register_for_notify(gattc_if, gl_profile_tab[PROFILE_A_APP_ID].remote_bda, char_elem_result[0].char_handle);
                    }
                }
                /* free char_elem_result */
                free(char_elem_result);
            }
            else
            {
                ESP_LOGE(TAG, "no char found");
            }
        }
        break;
    case ESP_GATTC_REG_FOR_NOTIFY_EVT:
    {
        ESP_LOGI(TAG, "ESP_GATTC_REG_FOR_NOTIFY_EVT");
        if (p_data->reg_for_notify.status != ESP_GATT_OK)
        {
            ESP_LOGE(TAG, "REG FOR NOTIFY failed: error status = %d", p_data->reg_for_notify.status);
        }
        else
        {
            uint16_t count = 0;
            uint16_t notify_en = 1;
            esp_gatt_status_t ret_status = esp_ble_gattc_get_attr_count(gattc_if,
                                                                        gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                                                        ESP_GATT_DB_DESCRIPTOR,
                                                                        gl_profile_tab[PROFILE_A_APP_ID].service_start_handle,
                                                                        gl_profile_tab[PROFILE_A_APP_ID].service_end_handle,
                                                                        gl_profile_tab[PROFILE_A_APP_ID].char_handle,
                                                                        &count);
            if (ret_status != ESP_GATT_OK)
            {
                ESP_LOGE(TAG, "esp_ble_gattc_get_attr_count error");
            }
            if (count > 0)
            {
                descr_elem_result = malloc(sizeof(esp_gattc_descr_elem_t) * count);
                if (!descr_elem_result)
                {
                    ESP_LOGE(TAG, "malloc error, gattc no mem");
                }
                else
                {
                    ret_status = esp_ble_gattc_get_descr_by_char_handle(gattc_if,
                                                                        gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                                                        p_data->reg_for_notify.handle,
                                                                        notify_descr_uuid,
                                                                        descr_elem_result,
                                                                        &count);
                    if (ret_status != ESP_GATT_OK)
                    {
                        ESP_LOGE(TAG, "esp_ble_gattc_get_descr_by_char_handle error");
                    }
    
                    if (count > 0 && descr_elem_result[0].uuid.len == ESP_UUID_LEN_16 && descr_elem_result[0].uuid.uuid.uuid16 == ESP_GATT_UUID_CHAR_CLIENT_CONFIG)
                    {
                        ret_status = esp_ble_gattc_write_char_descr(gattc_if,
                                                                    gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                                                    descr_elem_result[0].handle,
                                                                    sizeof(notify_en),
                                                                    (uint8_t *)&notify_en,
                                                                    ESP_GATT_WRITE_TYPE_RSP,
                                                                    ESP_GATT_AUTH_REQ_NONE);
                    }

                    if (ret_status != ESP_GATT_OK)
                    {
                        ESP_LOGE(TAG, "esp_ble_gattc_write_char_descr error");
                    }

                    /* free descr_elem_result */
                    free(descr_elem_result);
                }
            }
            else
            {
                ESP_LOGE(TAG, "decsr not found");
            }
        }
        break;
    }
    case ESP_GATTC_NOTIFY_EVT:

        decodeJKBMSData(p_data->notify.value, p_data->notify.value_len, &packetChunk, &packetType);


        break;
    case ESP_GATTC_WRITE_DESCR_EVT:
        if (p_data->write.status != ESP_GATT_OK)
        {
            ESP_LOGE(TAG, "write descr failed, error status = %x", p_data->write.status);
            break;
        }
        ESP_LOGI(TAG, "write descr success ");
        uint8_t write_char_data[35];
        for (int i = 0; i < sizeof(write_char_data); ++i)
        {
            write_char_data[i] = i % 256;
        }
        esp_ble_gattc_write_char(gattc_if,
                                 gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                 gl_profile_tab[PROFILE_A_APP_ID].char_handle,
                                 sizeof(write_char_data),
                                 write_char_data,
                                 ESP_GATT_WRITE_TYPE_RSP,
                                 ESP_GATT_AUTH_REQ_NONE);
        break;
    case ESP_GATTC_SRVC_CHG_EVT:
    {
        esp_bd_addr_t bda;
        memcpy(bda, p_data->srvc_chg.remote_bda, sizeof(esp_bd_addr_t));
        ESP_LOGI(TAG, "ESP_GATTC_SRVC_CHG_EVT, bd_addr:");
        esp_log_buffer_hex(TAG, bda, sizeof(esp_bd_addr_t));
        break;
    }
    case ESP_GATTC_WRITE_CHAR_EVT:
        if (p_data->write.status != ESP_GATT_OK)
        {
            ESP_LOGE(TAG, "write char failed, error status = %x", p_data->write.status);
            break;
        }
        ESP_LOGI(TAG, "write char success ");

        break;
    case ESP_GATTC_DISCONNECT_EVT:
        connect = false;
        get_server = false;
        ESP_LOGI(TAG, "ESP_GATTC_DISCONNECT_EVT, reason = %d", p_data->disconnect.reason);
        break;
    default:
        break;
    }
}

static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    uint8_t *adv_name = NULL;
    uint8_t adv_name_len = 0;
    switch (event)
    {
    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
    {
        // scan time 10s
        uint32_t duration = 10;
        esp_ble_gap_start_scanning(duration);
        break;
    }
    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
        // scan start complete event to indicate scan start successfully or failed
        if (param->scan_start_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(TAG, "scan start failed, error status = %x", param->scan_start_cmpl.status);
            break;
        }
        ESP_LOGI(TAG, "scan start success");

        break;
    case ESP_GAP_BLE_SCAN_RESULT_EVT:
    {
        esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;
        switch (scan_result->scan_rst.search_evt)
        {
        case ESP_GAP_SEARCH_INQ_RES_EVT:
            // esp_log_buffer_hex(TAG, scan_result->scan_rst.bda, 6);
            // ESP_LOGI(TAG, "searched Adv Data Len %d, Scan Response Len %d", scan_result->scan_rst.adv_data_len, scan_result->scan_rst.scan_rsp_len);
            adv_name = esp_ble_resolve_adv_data(scan_result->scan_rst.ble_adv,
                                                ESP_BLE_AD_TYPE_NAME_CMPL, &adv_name_len);
            // ESP_LOGI(TAG, "searched Device Name Len %d", adv_name_len);
            // esp_log_buffer_char(TAG, adv_name, adv_name_len);
            if (adv_name_len > 1)
            {
                ESP_LOGI(TAG, "DEVICE NAME: %s", adv_name);
                ESP_LOGI(TAG, "DEVICE NAME LEN: %d", adv_name_len);
                esp_log_buffer_hex(TAG, scan_result->scan_rst.bda, 6);
                ESP_LOGI(TAG, "RSSI: %d", scan_result->scan_rst.rssi);
                ESP_LOGI(TAG, "Device Address: %x:%x:%x:%x:%x:%x", scan_result->scan_rst.bda[0], scan_result->scan_rst.bda[1], scan_result->scan_rst.bda[2], scan_result->scan_rst.bda[3], scan_result->scan_rst.bda[4], scan_result->scan_rst.bda[5]);
                // newline
                ESP_LOGI(TAG, "\n");
                unsigned char addr[6];
                memcpy(addr, scan_result->scan_rst.bda, 6);

                for (int i = 0; i < 20; i++)
                {
                    ESP_LOGI(TAG, "Checking index %i, Got: %s bda0: %x addr0: %x", i, bleScan[i].deviceName, bleScan[i].deviceAddress[0], addr[0]);
                    if (memcmp(bleScan[i].deviceAddress, addr, 6) == 0)
                    {
                        ESP_LOGI(TAG, "%s already in list", adv_name);
                        break;
                    }
                    else
                    {
                        ESP_LOGI(TAG, "Not in list! %x", bleScan[i].deviceAddress[0]);
                        if (bleScan[i].deviceAddress[0] == '\0')
                        {
                            ESP_LOGI(TAG, "Adding %s to list", adv_name);
                            bleScan[i].rssi = scan_result->scan_rst.rssi;
                            memcpy(bleScan[i].deviceAddress, addr, 6);
                            memcpy(bleScan[i].deviceName, adv_name, adv_name_len);
                            // send queue to GUI

                            if (xQueueSend(bleScan_data_queue, &(bleScan), portMAX_DELAY) != pdPASS)
                            {
                                ESP_LOGI(TAG, "Failed to send array to queue");
                            }

                            break;
                        }
                    }
                }
            }

#if CONFIG_EXAMPLE_DUMP_ADV_DATA_AND_SCAN_RESP
            if (scan_result->scan_rst.adv_data_len > 0)
            {
                ESP_LOGI(TAG, "adv data:");
                esp_log_buffer_hex(TAG, &scan_result->scan_rst.ble_adv[0], scan_result->scan_rst.adv_data_len);
            }
            if (scan_result->scan_rst.scan_rsp_len > 0)
            {
                ESP_LOGI(TAG, "scan resp:");
                esp_log_buffer_hex(TAG, &scan_result->scan_rst.ble_adv[scan_result->scan_rst.adv_data_len], scan_result->scan_rst.scan_rsp_len);
            }
#endif
            // ESP_LOGI(TAG, "\n");

            if (adv_name != NULL)
            {
                if (strlen(remote_device_name) == adv_name_len && strncmp((char *)adv_name, remote_device_name, adv_name_len) == 0)
                {
                    ESP_LOGI(TAG, "searched device %s\n", remote_device_name);
                    if (connect == false)
                    {
                        connect = true;
                        ESP_LOGI(TAG, "connect to the remote device.");
                        esp_ble_gap_stop_scanning();
                        esp_ble_gattc_open(gl_profile_tab[PROFILE_A_APP_ID].gattc_if, scan_result->scan_rst.bda, scan_result->scan_rst.ble_addr_type, true);
                    }
                }
            }
            break;
        case ESP_GAP_SEARCH_INQ_CMPL_EVT:
            break;
        default:
            break;
        }
        break;
    }

    case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
        if (param->scan_stop_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(TAG, "scan stop failed, error status = %x", param->scan_stop_cmpl.status);
            break;
        }
        ESP_LOGI(TAG, "stop scan successfully");
        break;

    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(TAG, "adv stop failed, error status = %x", param->adv_stop_cmpl.status);
            break;
        }
        ESP_LOGI(TAG, "stop adv successfully");
        break;
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
        ESP_LOGI(TAG, "update connection params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
                 param->update_conn_params.status,
                 param->update_conn_params.min_int,
                 param->update_conn_params.max_int,
                 param->update_conn_params.conn_int,
                 param->update_conn_params.latency,
                 param->update_conn_params.timeout);
        break;
    default:
        break;
    }
}

static void esp_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    /* If event is register event, store the gattc_if for each profile */
    if (event == ESP_GATTC_REG_EVT)
    {
        if (param->reg.status == ESP_GATT_OK)
        {
            gl_profile_tab[param->reg.app_id].gattc_if = gattc_if;
        }
        else
        {
            ESP_LOGI(TAG, "reg app failed, app_id %04x, status %d",
                     param->reg.app_id,
                     param->reg.status);
            return;
        }
    }

    /* If the gattc_if equal to profile A, call profile A cb handler,
     * so here call each profile's callback */
    do
    {
        int idx;
        for (idx = 0; idx < PROFILE_NUM; idx++)
        {
            if (gattc_if == ESP_GATT_IF_NONE || /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
                gattc_if == gl_profile_tab[idx].gattc_if)
            {
                if (gl_profile_tab[idx].gattc_cb)
                {
                    gl_profile_tab[idx].gattc_cb(event, gattc_if, param);
                }
            }
        }
    } while (0);
}

// Queue for sending data to the GUI
QueueHandle_t ble_data_queue;

void ble_task(void *pvParameters)
{
    bool isConnected = connect;

    // Initialize the BLE scan queue
    bleConnection = xQueueCreate(5, sizeof(isConnected));

    bleScan_data_queue = xQueueCreate(5, sizeof(bleScan));

    // BLE Control struct
    struct BLEControl bleControl;

    bleControl.connect = false;
    bleControl.disconnect = false;
    bleControl.startScan = false;
    bleControl.stopScan = false;

    ble_data_queue = xQueueCreate(5, sizeof(bleControl));

    jkbms_data_queue = xQueueCreate(5, sizeof(struct JKBMSData));

    // Initialize NVS.
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    nvs_handle_t my_handle;
    esp_err_t err;
    err = nvs_open("storage", NVS_READONLY, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        ESP_LOGI(TAG, "Done\n");
    }

    // Try to read ble device name from NVS

    size_t required_len = 0;
    err = nvs_get_str(my_handle, "bleDeviceName", NULL, &required_len);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
    {
        nvs_close(my_handle);
        // return err;
    }

    // Check len
    if (required_len > 60)
    {
        nvs_close(my_handle);
        // return ESP_ERR_NVS_INVALID_LENGTH;
    }
    char *deviceName = malloc(required_len);

    
    err = nvs_get_str(my_handle, "bleDeviceName", deviceName, &required_len);
    if (err == ESP_OK)
    {
        // Ensure null-termination
        ESP_LOGI(TAG, "Read device name: %s", deviceName);
        // Safely copy the name to the global variable
        strncpy(remote_device_name, deviceName, sizeof(remote_device_name) - 1);
        remote_device_name[sizeof(remote_device_name) - 1] = '\0'; // Ensure null termination
    }

    // Close NVS handle
    nvs_close(my_handle);

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret)
    {
        ESP_LOGE(TAG, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret)
    {
        ESP_LOGE(TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_bluedroid_init();
    if (ret)
    {
        ESP_LOGE(TAG, "%s init bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_bluedroid_enable();
    if (ret)
    {
        ESP_LOGE(TAG, "%s enable bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    // register the  callback function to the gap module
    ret = esp_ble_gap_register_callback(esp_gap_cb);
    if (ret)
    {
        ESP_LOGE(TAG, "%s gap register failed, error code = %x\n", __func__, ret);
        return;
    }

    // register the callback function to the gattc module
    ret = esp_ble_gattc_register_callback(esp_gattc_cb);
    if (ret)
    {
        ESP_LOGE(TAG, "%s gattc register failed, error code = %x\n", __func__, ret);
        return;
    }

    ret = esp_ble_gattc_app_register(PROFILE_A_APP_ID);
    if (ret)
    {
        ESP_LOGE(TAG, "%s gattc app register failed, error code = %x\n", __func__, ret);
    }
    esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(500);
    if (local_mtu_ret)
    {
        ESP_LOGE(TAG, "set local  MTU failed, error code = %x", local_mtu_ret);
    }

    bool sentPollCmd = false;

    while (true)
    {
        if (xQueueReceive(ble_data_queue, &(bleControl), (TickType_t)5))
        {
            ESP_LOGI(TAG, "New BLEControl State: StartScan=%i, StopScan=%i, Connect=%i, Disconnect=%i", bleControl.startScan, bleControl.stopScan, bleControl.connect, bleControl.disconnect);
        }

        if (connect)
        {
            if (xQueueSend(jkbms_data_queue, &jkbmsData, portMAX_DELAY) != pdPASS)
            {
                ESP_LOGI(TAG, "Failed to send JKBMS data to queue");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // 1 sec delay

        if (bleControl.startScan)
        {
            ESP_LOGI(TAG, "Start Scan");
            esp_ble_gap_start_scanning(0);
            bleControl.startScan = false;
        }
        if (bleControl.stopScan)
        {
            ESP_LOGI(TAG, "Stop Scan");
            esp_ble_gap_stop_scanning();
            bleControl.stopScan = false;
        }

        if (connect == true && deviceReady && !sentPollCmd)
        {
            sentPollCmd = true;
            if (xQueueSend(bleConnection, &connect, portMAX_DELAY) != pdPASS)
            {
                ESP_LOGI(TAG, "Failed to send connection state to queue");
            }

            uint8_t startPollingCmd[] = {0xaa, 0x55, 0x90, 0xeb, 0x97, 0x00, 0x87, 0x8c, 0xb3, 0xd1, 0x97, 0x18, 0x3e, 0x84, 0x4a, 0xac, 0xc0, 0xa5, 0xf4, 0x68};
            uint8_t getInfoCmd[] = {0xAA, 0x55, 0x90, 0xEB, 0x96, 0x00, 0x26, 0x0B, 0x02, 0xF3, 0xB3, 0xA8, 0x10, 0x62, 0xAF, 0xE8, 0xEC, 0xA0, 0x3C, 0x62};

            // send get info cmd
            esp_ble_gattc_write_char(gatc_if,
                                     gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                     gl_profile_tab[PROFILE_A_APP_ID].char_handle,
                                     sizeof(getInfoCmd),
                                     getInfoCmd,
                                     ESP_GATT_WRITE_TYPE_NO_RSP,
                                     ESP_GATT_AUTH_REQ_NONE);
            ESP_LOGI(TAG, "Sent get info cmd");

            // wait for 1 second
            vTaskDelay(pdMS_TO_TICKS(1000));

            // Send start poll cmd
            esp_ble_gattc_write_char(gatc_if,
                                     gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                     gl_profile_tab[PROFILE_A_APP_ID].char_handle,
                                     sizeof(startPollingCmd),
                                     startPollingCmd,
                                     ESP_GATT_WRITE_TYPE_NO_RSP,
                                     ESP_GATT_AUTH_REQ_NONE);
            ESP_LOGI(TAG, "Sent start poll cmd");
        }
    }
}
