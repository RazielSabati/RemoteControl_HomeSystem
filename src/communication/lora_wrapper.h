#pragma once

#include <SPI.h>
#include <LoRa.h>
#include <pins.h>
#include <Arduino.h>
#include "return_code.h"
#include "structs.h"
#include "secure_ccm_protocol.h"

#define LORA_MAX_FRAME_SIZE 255
typedef struct
{
    bool setup_successful = false;
    return_code_e error_code = RETURN_CODE__UNINITIALISED;

    LoRaClass obj;

    long frequency = 433E6;
    float bandwidth = 31.25E3;
    int spreadingFactor = 8;
    int codingRate = 5;
    int txPower = 20;

    bool encryption_enabled = false;
    uint32_t tx_counter24 = 1;
    uint32_t last_rx_counter24 = 0;

    bool stop_sending_data = false;
} Lora_settings_t;

extern Lora_settings_t g_lora;
bool setup_Lora_module(Lora_settings_t *lora);

bool lora__poll_packet(Lora_settings_t *lora, packet_t *output_packet);
bool lora__build_encrypted_frame(Lora_settings_t *lora, command_e command, const uint8_t *payload, uint8_t payload_length, uint8_t *output_frame, uint8_t *output_length);
incoming_packet_retval_e lora_packet_parser(const uint8_t *frame, uint8_t frame_length, packet_t *output_packet);
