#include "packet_handler.h"

// extern column_data_t g_columns_copy[COLUMN__MAX];

void packet_handler__process(packet_t *received_packet)
{
    uint8_t return_code = 0;

    switch (received_packet->command)
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

        g_bt.obj.write(response_packet, 7);
        break;
    }
    break;

    default:
        break;
    }
}