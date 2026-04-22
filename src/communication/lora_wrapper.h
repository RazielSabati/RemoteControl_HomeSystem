#pragma once

#include <SPI.h>
#include <LoRa.h>
#include <pins.h>
#include <Arduino.h>
#include <AESLib.h>
#include "return_code.h"
#include "packet_handler.h"

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

    bool stop_sending_data = false;
} Lora_settings_t;

extern Lora_settings_t g_lora;
bool setup_Lora_module(Lora_settings_t *lora);

void lora_packet_parser__poll();
incoming_packet_retval_e lora_packet_parser(const uint8_t *frame, uint8_t frame_length, packet_t *output_packet);