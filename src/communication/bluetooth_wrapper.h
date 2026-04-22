#pragma once

#undef dump
#include <BluetoothSerial.h>
#include "return_code.h"
#include "structs.h"

#define DEVICE_NAME "MADAN-1"

typedef void (*process_packet_fn)(packet_t);

typedef struct
{
    bool setup_successful = false;
    return_code_e error_code = RETURN_CODE__UNINITIALISED;

    BluetoothSerial obj;

    uint8_t incoming_byte_del1;
    uint8_t incoming_byte_del2;

    uint8_t rx_buffer_index = 0;
    incoming_data_state_e incoming_data_state = INCOMING_DATA__WAIT_FOR_PACKET;
    packet_t received_packet;

    bool stop_sending_data = false;
} bt_settings_t;

extern bt_settings_t g_bt;

return_code_e bluetooth__setup(bt_settings_t *bt);
bool bluetooth__poll_packet(bt_settings_t *bt, packet_t *output_packet);
