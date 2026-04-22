#pragma once

#include "structs.h"

typedef enum
{
    PACKET_SOURCE__BLUETOOTH,
    PACKET_SOURCE__LORA
} packet_source_e;

typedef struct
{
    packet_t packet;
    packet_source_e source;
} received_packet_message_t;

void packet_handler__process(packet_t *received_packet);
void packet_handler__process_message(const received_packet_message_t *message);
void process_incoming_packet(packet_t received_packet);
