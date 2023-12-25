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


#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_log.h"

static const char *TAG = "example";

#define BLE_HCI_UART_H4_NONE        0x00
#define BLE_HCI_UART_H4_CMD         0x01
#define BLE_HCI_UART_H4_ACL         0x02
#define BLE_HCI_UART_H4_SCO         0x03
#define BLE_HCI_UART_H4_EVT         0x04


#define BLE_HCI_EVENT_HDR_LEN               (2)

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
    uint16_t len;
    uint8_t *data;

    ESP_LOGI(TAG, "called in %s", __func__);

    if (!esp_vhci_host_check_send_available()) {
        ESP_LOGE(TAG, "Controller not ready to receive packets");
    }

    len = data_len + 1;
    data = malloc(len);
    data[0] = BLE_HCI_UART_H4_ACL;
    memcpy(&data[1], acl_data, data_len);
    esp_vhci_host_send_packet(data, len);
    free(data);
}

void tud_bt_hci_cmd_cb(void *hci_cmd, size_t cmd_len)
{
    uint16_t len;
    uint8_t *cmd;

    ESP_LOGI(TAG, "called in %s", __func__);

    if (!esp_vhci_host_check_send_available()) {
        ESP_LOGE(TAG, "Controller not ready to receive packets");
    }

    len = cmd_len + 1;
    cmd = malloc(len);
    *cmd = 0x01;
    memcpy(&cmd[1], hci_cmd, cmd_len);
    esp_vhci_host_send_packet(cmd, len);
    free(cmd);
}


/*
 * @brief: BT controller callback function, used to notify the upper layer that
 *         controller is ready to receive command
 */
static void controller_rcv_pkt_ready(void)
{

}

/*
 * @brief: BT controller callback function, to transfer data packet to the host
 */
static int host_rcv_pkt(uint8_t *data, uint16_t len)
{
    if (data[0] == BLE_HCI_UART_H4_EVT) {
        tud_bt_event_send(&data[1], len -1);
    } else if (data[0] == BLE_HCI_UART_H4_ACL) {
        tud_bt_acl_data_send(&data[1], len -1);
    }

    return 0;
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

    // ******************* Init BT controller *************************
    esp_err_t ret;

    /* Initialize NVS — it is used to store PHY calibration data */
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

    ret = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Bluetooth Controller initialize failed: %s", esp_err_to_name(ret));
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Bluetooth Controller initialize failed: %s", esp_err_to_name(ret));
        return;
    }

    // ********************** Register HCI **************************
    static const esp_vhci_host_callback_t vhci_host_cb = {
        .notify_host_send_available = controller_rcv_pkt_ready,
        .notify_host_recv = host_rcv_pkt,
    };
    assert(esp_vhci_host_register_callback(&vhci_host_cb) == ESP_OK);
}
