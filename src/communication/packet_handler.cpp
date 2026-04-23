#include "packet_handler.h"
#include "bluetooth_wrapper.h"
#include "lora_wrapper.h"

static bool packet_handler__serialize_packet(command_e command,
                                             const uint8_t *payload,
                                             uint8_t payload_length,
                                             uint8_t *output_frame,
                                             uint8_t *output_length)
{
    if (output_frame == NULL || output_length == NULL)
    {
        return false;
    }

    if (payload_length > CMD_PAYLOAD_SIZE)
    {
        return false;
    }

    if ((payload_length > 0) && (payload == NULL))
    {
        return false;
    }

    output_frame[0] = PACKET_HEADER[0];
    output_frame[1] = PACKET_HEADER[1];
    output_frame[2] = PACKET_HEADER[2];
    output_frame[3] = (uint8_t)command;
    output_frame[4] = payload_length;

    if (payload_length > 0)
    {
        memcpy(&output_frame[METADATA_LENGTH], payload, payload_length);
    }

    *output_length = (uint8_t)(METADATA_LENGTH + payload_length);
    return true;
}

bool packet_handler__send_packet(packet_source_e destination,
                                 command_e command,
                                 const uint8_t *payload,
                                 uint8_t payload_length)
{
    uint8_t plain_frame[METADATA_LENGTH + CMD_PAYLOAD_SIZE] = {0};
    uint8_t plain_length = 0;

    if (!packet_handler__serialize_packet(command, payload, payload_length, plain_frame, &plain_length))
    {
        return false;
    }

    if (destination == PACKET_SOURCE__LORA)
    {
        if (g_lora.encryption_enabled)
        {
            uint8_t encrypted_frame[LORA_MAX_FRAME_SIZE] = {0};
            uint8_t encrypted_length = 0;
            if (!lora__build_encrypted_frame(&g_lora, command, payload, payload_length, encrypted_frame, &encrypted_length))
            {
                return false;
            }

            if (!g_lora.obj.beginPacket())
            {
                return false;
            }

            size_t written = g_lora.obj.write(encrypted_frame, encrypted_length);
            if (written != encrypted_length)
            {
                return false;
            }

            return (g_lora.obj.endPacket() == 1);
        }

        if (!g_lora.obj.beginPacket())
        {
            return false;
        }

        size_t written = g_lora.obj.write(plain_frame, plain_length);
        if (written != plain_length)
        {
            return false;
        }

        return (g_lora.obj.endPacket() == 1);
    }

    size_t written = g_bt.obj.write(plain_frame, plain_length);
    return (written == plain_length);
}

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
        static const uint8_t handshake_response_payload[] = {0xAA, 0xBB};
        (void)packet_handler__send_packet(message->source,
                                          COMMAND__HAND_SHAKE_BLUETOOTH,
                                          handshake_response_payload,
                                          sizeof(handshake_response_payload));
        break;
    }

    default:
        break;
    }
}
