/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <stdint.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tinyusb.h"
#include "sdkconfig.h"
#include "class/vendor/vendor_device.h"

static const char *TAG = "example";

/*
 * TinyUSB callbacks.
 */
void tud_bt_acl_data_sent_cb(uint16_t sent_bytes)
{
    ESP_LOGI(TAG, "called in %s", __func__);
}

void tud_bt_event_sent_cb(uint16_t sent_bytes)
{
    ESP_LOGI(TAG, "called in %s", __func__);
}

void tud_bt_acl_data_received_cb(void *acl_data, uint16_t data_len)
{
    ESP_LOGI(TAG, "called in %s", __func__);
}

void tud_bt_hci_cmd_cb(void *hci_cmd, size_t cmd_len)
{
    ESP_LOGI(TAG, "called in %s", __func__);
}

void app_main(void)
{
    ESP_LOGI(TAG, "USB initialization");
    const tinyusb_config_t tusb_cfg = {
        .device_descriptor = NULL,
        .string_descriptor = NULL,
        .external_phy = false,
        .configuration_descriptor = NULL,
    };

    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

    ESP_LOGI(TAG, "USB initialization DONE");
}
