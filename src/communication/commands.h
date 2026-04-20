
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

    #define CHECK_NULL(variable, label, error_retval) \
    {                                             \
        if (NULL == variable)                     \
        {                                         \
            return_value = error_retval;          \
            goto label;                           \
        }                                         \
    }


    extern const uint8_t PACKET_HEADER[HEADER_LENGTH];

    typedef enum
    {
        COMMAND__DEBUG,
      
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


#ifdef __cplusplus
}
#endif