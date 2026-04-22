#include <Arduino.h>
#include <SPI.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include <pins.h>

#include "./communication/bluetooth_wrapper.h"
#include "./communication/lora_wrapper.h"
#include "./communication/packet_handler.h"

extern bt_settings_t g_bt;
extern Lora_settings_t g_lora;

static QueueHandle_t g_packet_queue = NULL;
static const uint8_t PACKET_QUEUE_LENGTH = 10;

static void bluetooth_error_blink_forever()
{
    digitalWrite(STATUS_LED_BLUE_PIN, LOW);
    while (1)
    {
        digitalWrite(STATUS_LED_RED_PIN, HIGH);
        vTaskDelay(pdMS_TO_TICKS(300));
        digitalWrite(STATUS_LED_RED_PIN, LOW);
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}

static void bluetooth_rx_task(void *parameter)
{
    (void)parameter;
    packet_t packet = {0};

    for (;;)
    {
        packet = {0};
        if (bluetooth__poll_packet(&g_bt, &packet))
        {
            received_packet_message_t msg = {0};
            memcpy(&msg.packet, &packet, sizeof(packet_t));
            msg.source = PACKET_SOURCE__BLUETOOTH;
            (void)xQueueSend(g_packet_queue, &msg, 0);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void lora_rx_task(void *parameter)
{
    (void)parameter;

    packet_t packet = {0};

    for (;;)
    {
        packet = {0};
        if (lora__poll_packet(&g_lora, &packet))
        {
            received_packet_message_t msg = {0};
            memcpy(&msg.packet, &packet, sizeof(packet_t));
            msg.source = PACKET_SOURCE__LORA;
            (void)xQueueSend(g_packet_queue, &msg, 0);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void packet_handler_task(void *parameter)
{
    (void)parameter;

    received_packet_message_t msg = {0};
    for (;;)
    {
        if (xQueueReceive(g_packet_queue, &msg, portMAX_DELAY) == pdTRUE)
        {
            packet_handler__process_message(&msg);
        }
    }
}

void setup()
{
    Serial.begin(9600);
    SPI.begin();

    pinMode(STATUS_LED_RED_PIN, OUTPUT);
    pinMode(STATUS_LED_BLUE_PIN, OUTPUT);
    digitalWrite(STATUS_LED_RED_PIN, LOW);
    digitalWrite(STATUS_LED_BLUE_PIN, LOW);

    g_packet_queue = xQueueCreate(PACKET_QUEUE_LENGTH, sizeof(received_packet_message_t));
    if (g_packet_queue == NULL)
    {
        while (1)
        {
            delay(1000);
        }
    }

    if (bluetooth__setup(&g_bt) != RETURN_CODE__SUCCESS)
    {
        bluetooth_error_blink_forever();
    }

    if (!setup_Lora_module(&g_lora))
    {
        while (1)
        {
            delay(1000);
        }
    }

    digitalWrite(STATUS_LED_RED_PIN, LOW);
    digitalWrite(STATUS_LED_BLUE_PIN, HIGH);

    (void)xTaskCreatePinnedToCore(bluetooth_rx_task, "bt_rx_task", 4096, NULL, 1, NULL, 1);
    (void)xTaskCreatePinnedToCore(lora_rx_task, "lora_rx_task", 4096, NULL, 1, NULL, 1);
    (void)xTaskCreatePinnedToCore(packet_handler_task, "pkt_handler_task", 4096, NULL, 2, NULL, 1);
}

void loop()
{
    vTaskDelay(pdMS_TO_TICKS(1000));
}
