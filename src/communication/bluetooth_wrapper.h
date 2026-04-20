#pragma once

#undef dump
#include <BluetoothSerial.h>
#include "return_code.h"

#define BUFFER_SIZE 100
#define HEADER_LENGTH 3
#define DEVICE_NAME "MADAN-1"

// int menuType = -1; 
// int actionIndex = -1; 

  typedef enum
    {
        INCOMING_DATA__WAIT_FOR_PACKET,
        INCOMING_DATA__GET_COMMAND,
        INCOMING_DATA__GET_LENGTH,
        INCOMING_DATA__READ_PAYLOAD,
        INCOMING_DATA__PROCESS_PACKET
    } incoming_data_state_e;

    typedef enum
    {
        COMMAND_HAND_SHAKE = 0x01,
        COMMAND_CHANGE_SERVO_CROSSING = 0x97,
        COMMAND_SET_IDLE = 0x60,
        COMMAND_ID__MAX_COMMAND,
    } command_id_e;

    typedef struct command_s
    {
        command_id_e command_id;
        uint8_t payload_length;
        uint8_t data[BUFFER_SIZE];
    } command_t;

    typedef void (*process_packet_fn)(command_t);


    
typedef struct
{
    bool setup_successful = false;
    return_code_e error_code = RETURN_CODE__UNINITIALISED;

    BluetoothSerial obj;

    bool stop_sending_data = false;
} bt_settings_t;

extern bt_settings_t g_bt;

return_code_e bluetooth__setup(bt_settings_t *bt);

void process_incoming_packet(command_t received_command);
void bluetooth__handler_received_byte(bt_settings_t *bt,process_packet_fn process_packet);




// #pragma once
// #undef dump
// #include <BluetoothSerial.h>
// #include "communication_protocol.h"
// #include "return_code.h"

// #define DEVICE_NAME "MADAN-1"



// typedef struct
// {
//     bool setup_successful = false;
//     return_code_e error_code = RETURN_CODE__UNINITIALISED;

//     BluetoothSerial obj;

//     bool stop_sending_data = false;
// } bt_settings_t;

// extern bt_settings_t g_bt;


// void bluetooth_communicator__setup();
// bool bluetooth__setup(bt_settings_t *bt);