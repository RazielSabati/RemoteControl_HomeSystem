#pragma once

#include <string.h>
#include "commands.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define PACKET_PAYLOAD_SIZE 50

//*******************************************************************
#define MAX_SEQUENT_BYTES 200 // must be less then uint8_t

    typedef enum
    {
        BUFFER_STATUS__SEARCH_HEADER,
        BUFFER_STATUS__GET_COMMAND,
        BUFFER_STATUS__GET_LENGTH,
        BUFFER_STATUS__READ_PAYLOAD,
    } buffer_status_e;

    typedef enum
    {
        INCOMING_PACKET_RETVAL__INVALID_PAYLOAD_LENGTH,
        INCOMING_PACKET_RETVAL__WAITING_FOR_PACKET,
        INCOMING_PACKET_RETVAL__FOUND_PACKET,
    } incoming_packet_retval_e;

    typedef uint8_t (*read_byte_fn)(void);
    typedef void (*write_byte_fn)(uint8_t byte);
    typedef bool (*is_ready_fn)(void);
    typedef void (*process_packet_fn)(packet_t *packet);

    typedef struct
    {
        read_byte_fn read_byte;
        write_byte_fn write_byte;
        is_ready_fn is_ready;
        packet_t packet;
        uint8_t index;
        buffer_status_e current_state;
    } communication_protocol_interface_t;

    typedef enum
    {
        COMMUNICATION_PROTOCOL_RETVAL__UNINITIALISED,
        COMMUNICATION_PROTOCOL_RETVAL__SUCCESS,
        COMMUNICATION_PROTOCOL_RETVAL__NULL_PARAMETERS,
    } communication_protocol_retval_e;

    communication_protocol_retval_e communication_protocol__setup(communication_protocol_interface_t *communicator, read_byte_fn read_byte, write_byte_fn write_byte, is_ready_fn is_ready);
    communication_protocol_retval_e communication_protocol__run_main_logic(communication_protocol_interface_t *communicator, process_packet_fn process_packet);
    communication_protocol_retval_e communication_protocol__send_packet(communication_protocol_interface_t *communicator, command_e command, uint8_t *packet, uint8_t length);

#ifdef __cplusplus
}
#endif