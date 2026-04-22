#pragma once
#include <Arduino.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define CMD_PAYLOAD_SIZE 50
#define HEADER_LENGTH 3

#define METADATA_LENGTH 5 // header + command + payload_length
#define BUFFER_LENGTH METADATA_LENGTH + CMD_PAYLOAD_SIZE

    inline constexpr uint8_t PACKET_HEADER[] = {0x52, 0x4F, 0x59}; // 'R', 'O', 'Y' in ASCII

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
        INCOMING_PACKET_RETVAL__INVALID_PAYLOAD_LENGTH,
        INCOMING_PACKET_RETVAL__WAITING_FOR_PACKET,
        INCOMING_PACKET_RETVAL__FOUND_PACKET,
    } incoming_packet_retval_e;

    typedef enum
    {
        COMMAND__MIN,

        COMMAND__HAND_SHAKE_BLUETOOTH = 0x01,
        COMMAND__MAX,
    } command_e;

    typedef struct
    {
        uint8_t header[HEADER_LENGTH];
        command_e command;
        uint8_t payload_length;
        uint8_t payload[CMD_PAYLOAD_SIZE];
    } packet_t;

#define CHECK_NULL(variable, label, error_retval) \
    {                                             \
        if (NULL == variable)                     \
        {                                         \
            return_value = error_retval;          \
            goto label;                           \
        }                                         \
    }

#ifdef __cplusplus
}
#endif