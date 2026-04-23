#include "packet_handler.h"
#include "bluetooth_wrapper.h"
#include "lora_wrapper.h"

static int8_t packet_handler__clamp_to_i8(int value)
{
    if (value > 127)
    {
        return 127;
    }
    if (value < -128)
    {
        return -128;
    }
    return (int8_t)value;
}

static bool packet_handler__validate_lora_link_request(const received_packet_message_t *message)
{
    if (message == NULL)
    {
        return false;
    }

    // TAG/counter validation is already enforced upstream in lora__poll_packet()
    // when secure mode is enabled (secure_ccm_decrypt_packet()).
    // Handler performs command-specific validation here.
    return (message->packet.payload_length == 0);
}

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

    case COMMAND__LORA_LINK_REQUEST:
    {
        if (message->source == PACKET_SOURCE__BLUETOOTH)
        {
            // Forward the request from phone to remote LoRa side.
            (void)packet_handler__send_packet(PACKET_SOURCE__LORA,
                                              COMMAND__LORA_LINK_REQUEST,
                                              message->packet.payload,
                                              message->packet.payload_length);
            break;
        }

        // Request originated from LoRa side.
        if (!packet_handler__validate_lora_link_request(message))
        {
            static const uint8_t validation_error_payload[] = {0x01};
            (void)packet_handler__send_packet(PACKET_SOURCE__BLUETOOTH,
                                              COMMAND__GENERAL_ERROR,
                                              validation_error_payload,
                                              sizeof(validation_error_payload));
            break;
        }

        // Link result payload (compact):
        // [0] RSSI in dBm as int8
        // [1] SNR in dB as int8 (rounded)
        uint8_t lora_link_result_payload[2] = {0};
        int rssi_dbm = g_lora.obj.packetRssi();
        float snr_db = g_lora.obj.packetSnr();
        int snr_db_rounded = (snr_db >= 0.0f) ? (int)(snr_db + 0.5f) : (int)(snr_db - 0.5f);

        lora_link_result_payload[0] = (uint8_t)packet_handler__clamp_to_i8(rssi_dbm);
        lora_link_result_payload[1] = (uint8_t)packet_handler__clamp_to_i8(snr_db_rounded);

        (void)packet_handler__send_packet(PACKET_SOURCE__BLUETOOTH,
                                          COMMAND__LORA_LINK_RESULT,
                                          lora_link_result_payload,
                                          sizeof(lora_link_result_payload));
        break;
    }

    case COMMAND__SET_LIGHT:
    case COMMAND__SET_LIGHT_RESULT:
    case COMMAND__SET_SOUND_STATE:
    case COMMAND__SET_SOUND_RESULT:
    case COMMAND__SET_HEAT_STATE:
    case COMMAND__SET_HEAT_RESULT:
    case COMMAND__SET_SCENARIO:
    case COMMAND__START_SCENARIO_RESULT:
    {
        if (message->source == PACKET_SOURCE__BLUETOOTH)
        {
            // Forward the request from phone to remote LoRa side.
            (void)packet_handler__send_packet(PACKET_SOURCE__LORA,
                                              message->packet.command,
                                              message->packet.payload,
                                              message->packet.payload_length);
            break;
        }

        if (message->source == PACKET_SOURCE__BLUETOOTH)
        {
            // Request originated from LoRa side.
            (void)packet_handler__send_packet(PACKET_SOURCE__BLUETOOTH,
                                              message->packet.command,
                                              NULL,
                                              0);
            break;
        }
        break;
    }

    case COMMAND__SYSTEM_DATA:
    {
        // System data report from remote LoRa side. Forward to phone.
        if (message->source == PACKET_SOURCE__LORA)
        {
            // insert the data into the struct and send it when timer expires
            break;
        }
        break;
    }

    default:
        break;
    }
}
