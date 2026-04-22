#include "bluetooth_wrapper.h"

bt_settings_t g_bt;

static void bluetooth__reset_parser(bt_settings_t *bt)
{
    if (bt == NULL)
    {
        return;
    }

    bt->incoming_data_state = INCOMING_DATA__WAIT_FOR_PACKET;
    bt->rx_buffer_index = 0;
    memset(&bt->received_packet, 0, sizeof(packet_t));
}

return_code_e bluetooth__setup(bt_settings_t *bt)
{
    return_code_e status = RETURN_CODE__UNINITIALISED;

    if (bt == NULL)
    {
        return RETURN_CODE__INVALID_PARAMETERS;
    }

    bt->error_code = RETURN_CODE__UNINITIALISED;

    if (!bt->obj.begin(DEVICE_NAME))
    {
        bt->setup_successful = false;
        status = RETURN_CODE__BT__BEGIN_FAILED;
        bt->error_code = status;
        return status;
    }

    bluetooth__reset_parser(bt);
    bt->incoming_byte_del1 = 0;
    bt->incoming_byte_del2 = 0;
    bt->setup_successful = true;
    status = RETURN_CODE__SUCCESS;
    bt->error_code = status;
    return status;
}

bool bluetooth__poll_packet(bt_settings_t *bt, packet_t *output_packet)
{
    if (bt == NULL || output_packet == NULL)
    {
        return false;
    }

    while (bt->obj.available())
    {
        int incoming = bt->obj.read();
        if (incoming < 0)
        {
            return false;
        }

        uint8_t incoming_byte = (uint8_t)incoming;

        switch (bt->incoming_data_state)
        {
        case INCOMING_DATA__WAIT_FOR_PACKET:
            if ((incoming_byte == PACKET_HEADER[2]) &&
                (bt->incoming_byte_del1 == PACKET_HEADER[1]) &&
                (bt->incoming_byte_del2 == PACKET_HEADER[0]))
            {
                memcpy(bt->received_packet.header, PACKET_HEADER, HEADER_LENGTH);
                bt->incoming_data_state = INCOMING_DATA__GET_COMMAND;
            }
            break;

        case INCOMING_DATA__GET_COMMAND:
            bt->received_packet.command = (command_e)incoming_byte;
            bt->incoming_data_state = INCOMING_DATA__GET_LENGTH;
            break;

        case INCOMING_DATA__GET_LENGTH:
            if (incoming_byte > CMD_PAYLOAD_SIZE)
            {
                bluetooth__reset_parser(bt);
            }
            else
            {
                bt->received_packet.payload_length = incoming_byte;
                bt->rx_buffer_index = 0;
                if (incoming_byte == 0)
                {
                    memcpy(output_packet, &bt->received_packet, METADATA_LENGTH);
                    bluetooth__reset_parser(bt);
                    return true;
                }
                bt->incoming_data_state = INCOMING_DATA__READ_PAYLOAD;
            }
            break;

        case INCOMING_DATA__READ_PAYLOAD:
            if (bt->rx_buffer_index < CMD_PAYLOAD_SIZE)
            {
                bt->received_packet.payload[bt->rx_buffer_index++] = incoming_byte;
            }

            if (bt->rx_buffer_index >= bt->received_packet.payload_length)
            {
                memcpy(output_packet, &bt->received_packet, METADATA_LENGTH + bt->received_packet.payload_length);
                bluetooth__reset_parser(bt);
                return true;
            }
            break;

        default:
            bluetooth__reset_parser(bt);
            break;
        }

        bt->incoming_byte_del2 = bt->incoming_byte_del1;
        bt->incoming_byte_del1 = incoming_byte;
    }

    return false;
}
