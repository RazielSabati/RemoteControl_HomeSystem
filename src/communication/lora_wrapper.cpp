#include "lora_wrapper.h"

#include "packet_handler.h"

Lora_settings_t g_lora;

bool setup_Lora_module(Lora_settings_t *lora)
{
#ifdef DEBUG_ENABLED
    Serial.println(F("[LoRa] Starting initialization..."));
#endif

    if (lora == NULL)
    {
        return false;
    }

    lora->obj.setPins(LORA_SS_PIN, LORA_RST_PIN, LORA_DIO0_PIN);

    unsigned long startAttemptTime = millis();
    bool initialized = false;

    while (!initialized && (millis() - startAttemptTime < 5000))
    {
        if (lora->obj.begin(lora->frequency))
        {
            initialized = true;
        }
        else
        {
#ifdef DEBUG_ENABLED
            Serial.print(F("."));
#endif
            delay(200);
        }
    }

    if (!initialized)
    {
#ifdef DEBUG_ENABLED
        Serial.println(F("\n[LoRa] ERROR: Chip not found or SPI communication failed!"));
#endif
        return false;
    }

    lora->obj.setSignalBandwidth(lora->bandwidth);
    lora->obj.setSpreadingFactor(lora->spreadingFactor);
    lora->obj.setCodingRate4(lora->codingRate);
    lora->obj.setTxPower(lora->txPower, PA_OUTPUT_PA_BOOST_PIN);
    lora->obj.enableCrc();
    lora->obj.setSyncWord(0xF3);

    lora->setup_successful = true;
    lora->error_code = RETURN_CODE__SUCCESS;

#ifdef DEBUG_ENABLED
    Serial.print(F("\n[LoRa] Success! Frequency: "));
    Serial.print(lora->frequency / 1E6);
    Serial.println(F("MHz"));
#endif

    return true;
}

incoming_packet_retval_e lora_packet_parser(const uint8_t *frame, uint8_t frame_length, packet_t *output_packet)
{
    if (frame == NULL || output_packet == NULL || frame_length < METADATA_LENGTH)
    {
        return INCOMING_PACKET_RETVAL__WAITING_FOR_PACKET;
    }

    if (memcmp(frame, PACKET_HEADER, HEADER_LENGTH) != 0)
    {
        return INCOMING_PACKET_RETVAL__WAITING_FOR_PACKET;
    }

    command_e command = (command_e)frame[HEADER_LENGTH];
    if (command >= COMMAND__MAX || command <= COMMAND__MIN)
    {
        return INCOMING_PACKET_RETVAL__WAITING_FOR_PACKET;
    }

    uint8_t payload_len = frame[HEADER_LENGTH + 1];
    if (payload_len > CMD_PAYLOAD_SIZE)
    {
        return INCOMING_PACKET_RETVAL__INVALID_PAYLOAD_LENGTH;
    }

    if (frame_length != (uint8_t)(METADATA_LENGTH + payload_len))
    {
        return INCOMING_PACKET_RETVAL__INVALID_PAYLOAD_LENGTH;
    }

    memset(output_packet, 0, sizeof(packet_t));
    memcpy(output_packet->header, frame, HEADER_LENGTH);
    output_packet->command = command;
    output_packet->payload_length = payload_len;

    if (payload_len > 0)
    {
        memcpy(output_packet->payload, &frame[METADATA_LENGTH], payload_len);
    }

    return INCOMING_PACKET_RETVAL__FOUND_PACKET;
}

bool lora__poll_packet(Lora_settings_t *lora, packet_t *output_packet)
{
    if (lora == NULL || output_packet == NULL)
    {
        return false;
    }

    int packet_size = lora->obj.parsePacket();
    if (packet_size <= 0)
    {
        return false;
    }

    if (packet_size > LORA_MAX_FRAME_SIZE)
    {
        while (lora->obj.available())
        {
            (void)lora->obj.read();
        }
        return false;
    }

    uint8_t raw_frame[LORA_MAX_FRAME_SIZE] = {0};
    uint8_t bytes_read = (uint8_t)lora->obj.readBytes(raw_frame, packet_size);

    return (lora_packet_parser(raw_frame, bytes_read, output_packet) == INCOMING_PACKET_RETVAL__FOUND_PACKET);
}
