#include "packet_handler.h"
#include "bluetooth_wrapper.h"
#include "lora_wrapper.h"

// extern column_data_t g_columns_copy[COLUMN__MAX];

void packet_handler__process(packet_t *received_packet)
{
    received_packet_message_t message = {0};
    if (received_packet == NULL)
    {
        return;
    }

    memcpy(&message.packet, received_packet, sizeof(packet_t));
    message.source = PACKET_SOURCE__BLUETOOTH;
    packet_handler__process_message(&message);
}

void process_incoming_packet(packet_t received_packet)
{
    received_packet_message_t message = {0};
    memcpy(&message.packet, &received_packet, sizeof(packet_t));
    message.source = PACKET_SOURCE__LORA;
    packet_handler__process_message(&message);
}

void packet_handler__process_message(const received_packet_message_t *message)
{
    if (message == NULL)
    {
        return;
    }

    switch (message->packet.command)
    {
    case COMMAND__HAND_SHAKE_BLUETOOTH:
    {
        Serial.println(F("Handshake received, sending response..."));

        uint8_t response_packet[7];

        response_packet[0] = PACKET_HEADER[0];              // 0x52
        response_packet[1] = PACKET_HEADER[1];              // 0x4F
        response_packet[2] = PACKET_HEADER[2];              // 0x59
        response_packet[3] = COMMAND__HAND_SHAKE_BLUETOOTH; // 0x01
        response_packet[4] = 0x02;
        response_packet[5] = 0xAA;
        response_packet[6] = 0xBB;

        if (message->source == PACKET_SOURCE__LORA)
        {
            g_lora.obj.beginPacket();
            g_lora.obj.write(response_packet, 7);
            (void)g_lora.obj.endPacket();
        }
        else
        {
            g_bt.obj.write(response_packet, 7);
        }
        break;
    }
    break;

    default:
        break;
    }
}
