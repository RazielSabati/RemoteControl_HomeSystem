#pragma once

#include <Arduino.h>

#include "structs.h"

#define SECURE_CCM_HEADER_SIZE 3
#define SECURE_CCM_COUNTER_SIZE 3
#define SECURE_CCM_METADATA_SIZE 2
#define SECURE_CCM_TAG_SIZE 4
#define SECURE_CCM_NONCE_SIZE 8
#define SECURE_CCM_MAX_PLAINTEXT_SIZE (SECURE_CCM_METADATA_SIZE + CMD_PAYLOAD_SIZE)
#define SECURE_CCM_MAX_PACKET_SIZE (SECURE_CCM_HEADER_SIZE + SECURE_CCM_COUNTER_SIZE + SECURE_CCM_MAX_PLAINTEXT_SIZE + SECURE_CCM_TAG_SIZE)

typedef enum
{
    SECURE_CCM_STATUS__SUCCESS = 0,
    SECURE_CCM_STATUS__INVALID_ARGS,
    SECURE_CCM_STATUS__INVALID_HEADER,
    SECURE_CCM_STATUS__INVALID_LENGTH,
    SECURE_CCM_STATUS__REPLAY_DETECTED,
    SECURE_CCM_STATUS__AUTH_FAILED,
    SECURE_CCM_STATUS__CRYPTO_FAILED
} secure_ccm_status_e;

typedef struct __attribute__((packed))
{
    uint8_t header[SECURE_CCM_HEADER_SIZE];
    uint8_t counter[SECURE_CCM_COUNTER_SIZE];
} secure_ccm_wire_prefix_t;

typedef struct __attribute__((packed))
{
    uint8_t command;
    uint8_t payload_length;
} secure_ccm_metadata_t;

typedef struct __attribute__((packed))
{
    secure_ccm_wire_prefix_t prefix;
    uint8_t ciphertext[SECURE_CCM_MAX_PLAINTEXT_SIZE];
    uint8_t tag[SECURE_CCM_TAG_SIZE];
} secure_ccm_wire_packet_max_t;

secure_ccm_status_e secure_ccm_encrypt_packet(command_e command,
                                              const uint8_t *payload,
                                              uint8_t payload_length,
                                              uint32_t counter24,
                                              uint8_t *output_frame,
                                              uint8_t *output_length);

secure_ccm_status_e secure_ccm_decrypt_packet(const uint8_t *input_frame,
                                              uint8_t input_length,
                                              packet_t *output_packet,
                                              uint32_t *output_counter24);

void secure_ccm_reset_replay_window(uint32_t last_counter24);
uint32_t secure_ccm_get_last_received_counter();
