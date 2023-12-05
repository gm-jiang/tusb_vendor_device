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
static uint8_t buf[CONFIG_TINYUSB_VENDOR_RX_BUFSIZE + 1];

void tud_vendor_rx_cb(uint8_t itf)
{
    /* initialization */
    size_t rx_size = 0;

    /* read */
    rx_size = tud_vendor_n_read(itf, buf, CONFIG_TINYUSB_VENDOR_RX_BUFSIZE);
    ESP_LOGI(TAG, "Data from channel %d:", itf);
    ESP_LOG_BUFFER_HEXDUMP(TAG, buf, rx_size, ESP_LOG_INFO);
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
