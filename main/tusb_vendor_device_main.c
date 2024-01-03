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
#include "tusb_cdc_acm.h"
#include "sdkconfig.h"
#include "soc/lp_system_reg.h"
#include "soc/hp_system_reg.h"
#include "esp_rom_uart.h"
#include "esp_sleep.h"
#include "esp_private/esp_pmu.h"

static const char *TAG = "example";
static uint8_t buf[CONFIG_TINYUSB_CDC_RX_BUFSIZE + 1];
esp_err_t esp_sleep_set_wakeup_source(uint32_t wakeup_mask);

void tinyusb_cdc_rx_callback(int itf, cdcacm_event_t *event)
{
    /* initialization */
    size_t rx_size = 0;

    /* read */
    esp_err_t ret = tinyusb_cdcacm_read(itf, buf, CONFIG_TINYUSB_CDC_RX_BUFSIZE, &rx_size);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Data from channel %d:", itf);
        ESP_LOG_BUFFER_HEXDUMP(TAG, buf, rx_size, ESP_LOG_INFO);
    } else {
        ESP_LOGE(TAG, "Read error");
    }

    /* write back */
    tinyusb_cdcacm_write_queue(itf, buf, rx_size);
    tinyusb_cdcacm_write_flush(itf, 0);
}

void tinyusb_cdc_line_state_changed_callback(int itf, cdcacm_event_t *event)
{
    int dtr = event->line_state_changed_data.dtr;
    int rts = event->line_state_changed_data.rts;
    ESP_LOGI(TAG, "Line state changed on channel %d: DTR:%d, RTS:%d", itf, dtr, rts);
}

// Invoked when usb bus is suspended
void tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
    //blink_interval_ms = BLINK_SUSPENDED;
    ESP_LOGI(TAG, "USB suspend callback");
#if 0
    REG_SET_BIT(HP_SYSTEM_USBOTG20_CTRL_REG, HP_SYSTEM_PHY_SUSPEND_FORCE_EN);
    REG_CLR_BIT(HP_SYSTEM_USBOTG20_CTRL_REG, HP_SYSTEM_PHY_SUSPENDM);
#endif

    REG_SET_BIT(LP_SYSTEM_REG_USB_CTRL_REG, LP_SYSTEM_REG_USBOTG20_IN_SUSPEND);
    esp_rom_uart_tx_wait_idle(0);
    esp_light_sleep_start();
    ESP_LOGI(TAG, "USB wakeup");
    REG_CLR_BIT(LP_SYSTEM_REG_USB_CTRL_REG, LP_SYSTEM_REG_USBOTG20_IN_SUSPEND);
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
    //blink_interval_ms = BLINK_MOUNTED;
    ESP_LOGI(TAG, "USB resume callback");
}

void app_main(void)
{
    esp_sleep_set_wakeup_source(PMU_USB_WAKEUP_EN); //PMU_USB_WAKEUP_EN
    ESP_LOGI(TAG, "USB initialization");
    const tinyusb_config_t tusb_cfg = {
        .device_descriptor = NULL,
        .string_descriptor = NULL,
        .external_phy = false,
        .configuration_descriptor = NULL,
    };

    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

    tinyusb_config_cdcacm_t acm_cfg = {
        .usb_dev = TINYUSB_USBDEV_0,
        .cdc_port = TINYUSB_CDC_ACM_0,
        .rx_unread_buf_sz = 512,
        .callback_rx = &tinyusb_cdc_rx_callback, // the first way to register a callback
        .callback_rx_wanted_char = NULL,
        .callback_line_state_changed = NULL,
        .callback_line_coding_changed = NULL
    };

    ESP_ERROR_CHECK(tusb_cdc_acm_init(&acm_cfg));
    /* the second way to register a callback */
    ESP_ERROR_CHECK(tinyusb_cdcacm_register_callback(
                        TINYUSB_CDC_ACM_0,
                        CDC_EVENT_LINE_STATE_CHANGED,
                        &tinyusb_cdc_line_state_changed_callback));

#if (CONFIG_TINYUSB_CDC_COUNT > 1)
    acm_cfg.cdc_port = TINYUSB_CDC_ACM_1;
    ESP_ERROR_CHECK(tusb_cdc_acm_init(&acm_cfg));
    ESP_ERROR_CHECK(tinyusb_cdcacm_register_callback(
                        TINYUSB_CDC_ACM_1,
                        CDC_EVENT_LINE_STATE_CHANGED,
                        &tinyusb_cdc_line_state_changed_callback));
#endif

    ESP_LOGI(TAG, "USB initialization DONE");
}
