#include "lora_wrapper.h"

Lora_settings_t g_lora;

bool setup_Lora_module(Lora_settings_t *lora)
{
#ifdef DEBUG_ENABLED
    Serial.println(F("[LoRa] Starting initialization..."));
#endif

    lora->obj.setPins(LORA_SS_PIN, LORA_RST_PIN, LORA_DIO0_PIN);

    unsigned long startAttemptTime = millis();
    bool initialized = false;
    int attempts = 0;

    while (!initialized && (millis() - startAttemptTime < 5000))
    {
        attempts++;
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

#ifdef DEBUG_ENABLED
    Serial.print(F("\n[LoRa] Success! Frequency: "));
    Serial.print(lora->frequency / 1E6);
    Serial.println(F("MHz"));
#endif

    return true;
}

incoming_packet_retval_e lora_packet_parser(const uint8_t *frame, uint8_t frame_length, packet_t *output_packet)
{
    // check for null pointers and minimum length (header + command + length)
    if (frame == NULL || output_packet == NULL || frame_length < METADATA_LENGTH)
    {
        return INCOMING_PACKET_RETVAL__WAITING_FOR_PACKET;
    }

    // header validation
    if (memcmp(frame, PACKET_HEADER, HEADER_LENGTH) != 0)
    {
        return INCOMING_PACKET_RETVAL__WAITING_FOR_PACKET;
    }

    command_e command = (command_e)frame[HEADER_LENGTH + 1];

    if (command >= COMMAND__MAX || command <= COMMAND__MIN)
    {
        return INCOMING_PACKET_RETVAL__WAITING_FOR_PACKET;
    }

    uint8_t payload_len = frame[HEADER_LENGTH + 2];

    if (payload_len == 0)
    {
        // packet with no payload, just header and command. Still valid
        memset(output_packet, 0, sizeof(packet_t));
        memcpy(output_packet->header, frame, HEADER_LENGTH);
        output_packet->command = command;
        output_packet->payload_length = 0;
        return INCOMING_PACKET_RETVAL__FOUND_PACKET;
    }

    // Validate payload length against frame length and maximum allowed payload size
    if (frame_length != (METADATA_LENGTH + payload_len) || payload_len > CMD_PAYLOAD_SIZE)
    {
        return INCOMING_PACKET_RETVAL__INVALID_PAYLOAD_LENGTH;
    }

    // If we reached this point, the packet is valid. Fill the output structure.
    memset(output_packet, 0, sizeof(packet_t));
    output_packet->command = (command_e)frame[HEADER_LENGTH];
    output_packet->payload_length = payload_len;
    memcpy(output_packet->payload, &frame[METADATA_LENGTH], payload_len);

    return INCOMING_PACKET_RETVAL__FOUND_PACKET;
}

void lora_packet_parser__poll()
{
    int packet_size = LoRa.parsePacket();

    // No packet available
    if (packet_size <= 0)
        return;

    if (packet_size > LORA_MAX_FRAME_SIZE)
    {
        // Discard the packet if it's too large
        while (LoRa.available())
            LoRa.read();
        return;
    }

    uint8_t raw_frame[LORA_MAX_FRAME_SIZE]; // אין צורך ב-memset אם אנחנו דורסים עם readBytes
    packet_t received_packet;

    // קריאת כל החבילה בבת אחת מהצ'יפ ל-RAM
    uint8_t bytes_read = (uint8_t)LoRa.readBytes(raw_frame, packet_size);

    // שליחה לניתוח מבני
    incoming_packet_retval_e status = lora_packet_parser(raw_frame, bytes_read, &received_packet);

    if (status == INCOMING_PACKET_RETVAL__FOUND_PACKET)
    {
        packet_handler__process(&received_packet);
    }
}