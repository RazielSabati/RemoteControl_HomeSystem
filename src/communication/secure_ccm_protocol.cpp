#include "secure_ccm_protocol.h"

#include <mbedtls/ccm.h>

static const uint8_t k_secure_header[SECURE_CCM_HEADER_SIZE] = {0x52, 0x4F, 0x59};
static const uint8_t k_nonce_prefix[SECURE_CCM_NONCE_SIZE - SECURE_CCM_COUNTER_SIZE] = {0xA1, 0xB2, 0xC3, 0xD4, 0xE5};
static const uint8_t k_aes128_key[16] = {
    0x2b, 0x7e, 0x15, 0x16,
    0xae, 0x2a, 0xd4, 0xa6,
    0x8a, 0xf7, 0x97, 0x75,
    0x40, 0x8e, 0x6a, 0x12};

static uint32_t s_last_received_counter = 0;

static uint32_t counter24_from_bytes(const uint8_t counter[SECURE_CCM_COUNTER_SIZE])
{
    return ((uint32_t)counter[0] << 16) |
           ((uint32_t)counter[1] << 8) |
           ((uint32_t)counter[2]);
}

static void counter24_to_bytes(uint32_t counter24, uint8_t out_counter[SECURE_CCM_COUNTER_SIZE])
{
    out_counter[0] = (uint8_t)((counter24 >> 16) & 0xFF);
    out_counter[1] = (uint8_t)((counter24 >> 8) & 0xFF);
    out_counter[2] = (uint8_t)(counter24 & 0xFF);
}

static void build_nonce(uint32_t counter24, uint8_t nonce[SECURE_CCM_NONCE_SIZE])
{
    memcpy(nonce, k_nonce_prefix, sizeof(k_nonce_prefix));
    counter24_to_bytes(counter24, &nonce[sizeof(k_nonce_prefix)]);
}

secure_ccm_status_e secure_ccm_encrypt_packet(command_e command,
                                              const uint8_t *payload,
                                              uint8_t payload_length,
                                              uint32_t counter24,
                                              uint8_t *output_frame,
                                              uint8_t *output_length)
{
    secure_ccm_status_e status = SECURE_CCM_STATUS__SUCCESS;
    mbedtls_ccm_context ctx;
    uint8_t nonce[SECURE_CCM_NONCE_SIZE] = {0};
    uint8_t plaintext[SECURE_CCM_MAX_PLAINTEXT_SIZE] = {0};
    uint8_t aad[SECURE_CCM_HEADER_SIZE + SECURE_CCM_COUNTER_SIZE] = {0};
    uint8_t frame_length = 0;
    const size_t plain_length = (size_t)(SECURE_CCM_METADATA_SIZE + payload_length);

    if (output_frame == NULL || output_length == NULL)
    {
        return SECURE_CCM_STATUS__INVALID_ARGS;
    }

    if (payload_length > CMD_PAYLOAD_SIZE)
    {
        return SECURE_CCM_STATUS__INVALID_LENGTH;
    }

    if (payload_length > 0 && payload == NULL)
    {
        return SECURE_CCM_STATUS__INVALID_ARGS;
    }

    memcpy(output_frame, k_secure_header, SECURE_CCM_HEADER_SIZE);
    counter24_to_bytes(counter24, &output_frame[SECURE_CCM_HEADER_SIZE]);

    plaintext[0] = (uint8_t)command;
    plaintext[1] = payload_length;
    if (payload_length > 0)
    {
        memcpy(&plaintext[SECURE_CCM_METADATA_SIZE], payload, payload_length);
    }

    build_nonce(counter24, nonce);
    memcpy(aad, output_frame, sizeof(aad)); // authenticate header+counter as AAD

    mbedtls_ccm_init(&ctx);
    if (mbedtls_ccm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, k_aes128_key, 128) != 0)
    {
        status = SECURE_CCM_STATUS__CRYPTO_FAILED;
        goto exit;
    }

    if (mbedtls_ccm_encrypt_and_tag(&ctx,
                                    plain_length,
                                    nonce,
                                    sizeof(nonce),
                                    aad,
                                    sizeof(aad),
                                    plaintext,
                                    &output_frame[SECURE_CCM_HEADER_SIZE + SECURE_CCM_COUNTER_SIZE],
                                    &output_frame[SECURE_CCM_HEADER_SIZE + SECURE_CCM_COUNTER_SIZE + plain_length],
                                    SECURE_CCM_TAG_SIZE) != 0)
    {
        status = SECURE_CCM_STATUS__CRYPTO_FAILED;
        goto exit;
    }

    frame_length = (uint8_t)(SECURE_CCM_HEADER_SIZE + SECURE_CCM_COUNTER_SIZE + plain_length + SECURE_CCM_TAG_SIZE);
    *output_length = frame_length;

exit:
    mbedtls_ccm_free(&ctx);
    return status;
}

secure_ccm_status_e secure_ccm_decrypt_packet(const uint8_t *input_frame,
                                              uint8_t input_length,
                                              packet_t *output_packet,
                                              uint32_t *output_counter24)
{
    secure_ccm_status_e status = SECURE_CCM_STATUS__SUCCESS;
    mbedtls_ccm_context ctx;
    uint8_t nonce[SECURE_CCM_NONCE_SIZE] = {0};
    uint8_t aad[SECURE_CCM_HEADER_SIZE + SECURE_CCM_COUNTER_SIZE] = {0};
    uint8_t plaintext[SECURE_CCM_MAX_PLAINTEXT_SIZE] = {0};
    uint32_t incoming_counter = 0;
    size_t cipher_length = 0;
    uint8_t payload_length = 0;
    command_e command = COMMAND__MIN;
    const uint8_t *ciphertext = NULL;
    const uint8_t *tag = NULL;

    if (input_frame == NULL || output_packet == NULL)
    {
        return SECURE_CCM_STATUS__INVALID_ARGS;
    }

    if (input_length < (SECURE_CCM_HEADER_SIZE + SECURE_CCM_COUNTER_SIZE + SECURE_CCM_METADATA_SIZE + SECURE_CCM_TAG_SIZE))
    {
        return SECURE_CCM_STATUS__INVALID_LENGTH;
    }

    if (memcmp(input_frame, k_secure_header, SECURE_CCM_HEADER_SIZE) != 0)
    {
        return SECURE_CCM_STATUS__INVALID_HEADER;
    }

    incoming_counter = counter24_from_bytes(&input_frame[SECURE_CCM_HEADER_SIZE]);
    if (incoming_counter <= s_last_received_counter)
    {
        return SECURE_CCM_STATUS__REPLAY_DETECTED;
    }

    cipher_length = (size_t)(input_length - SECURE_CCM_HEADER_SIZE - SECURE_CCM_COUNTER_SIZE - SECURE_CCM_TAG_SIZE);
    if (cipher_length < SECURE_CCM_METADATA_SIZE || cipher_length > SECURE_CCM_MAX_PLAINTEXT_SIZE)
    {
        return SECURE_CCM_STATUS__INVALID_LENGTH;
    }

    build_nonce(incoming_counter, nonce);
    memcpy(aad, input_frame, sizeof(aad));

    ciphertext = &input_frame[SECURE_CCM_HEADER_SIZE + SECURE_CCM_COUNTER_SIZE];
    tag = &input_frame[SECURE_CCM_HEADER_SIZE + SECURE_CCM_COUNTER_SIZE + cipher_length];

    mbedtls_ccm_init(&ctx);
    if (mbedtls_ccm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, k_aes128_key, 128) != 0)
    {
        status = SECURE_CCM_STATUS__CRYPTO_FAILED;
        goto exit;
    }

    if (mbedtls_ccm_auth_decrypt(&ctx,
                                 cipher_length,
                                 nonce,
                                 sizeof(nonce),
                                 aad,
                                 sizeof(aad),
                                 ciphertext,
                                 plaintext,
                                 tag,
                                 SECURE_CCM_TAG_SIZE) != 0)
    {
        status = SECURE_CCM_STATUS__AUTH_FAILED;
        goto exit;
    }

    command = (command_e)plaintext[0];
    payload_length = plaintext[1];
    if (payload_length > CMD_PAYLOAD_SIZE || cipher_length != (size_t)(SECURE_CCM_METADATA_SIZE + payload_length))
    {
        status = SECURE_CCM_STATUS__INVALID_LENGTH;
        goto exit;
    }

    memset(output_packet, 0, sizeof(packet_t));
    memcpy(output_packet->header, k_secure_header, SECURE_CCM_HEADER_SIZE);
    output_packet->command = command;
    output_packet->payload_length = payload_length;
    if (payload_length > 0)
    {
        memcpy(output_packet->payload, &plaintext[SECURE_CCM_METADATA_SIZE], payload_length);
    }

    s_last_received_counter = incoming_counter;
    if (output_counter24 != NULL)
    {
        *output_counter24 = incoming_counter;
    }

exit:
    mbedtls_ccm_free(&ctx);
    return status;
}

void secure_ccm_reset_replay_window(uint32_t last_counter24)
{
    s_last_received_counter = (last_counter24 & 0x00FFFFFFu);
}

uint32_t secure_ccm_get_last_received_counter()
{
    return s_last_received_counter;
}
