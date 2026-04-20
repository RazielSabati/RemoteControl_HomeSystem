#include "communication_protocol.h"

static incoming_packet_retval_e parse_incoming_byte(communication_protocol_interface_t *communicator, packet_t *output_packet)
{
    incoming_packet_retval_e return_value = INCOMING_PACKET_RETVAL__WAITING_FOR_PACKET;
    uint8_t incoming_byte = 0;

    CHECK_NULL(communicator, exit, INCOMING_PACKET_RETVAL__WAITING_FOR_PACKET);
    CHECK_NULL(communicator->read_byte, exit, INCOMING_PACKET_RETVAL__WAITING_FOR_PACKET);
    CHECK_NULL(output_packet, exit, INCOMING_PACKET_RETVAL__WAITING_FOR_PACKET);

    incoming_byte = communicator->read_byte();

    switch (communicator->current_state)
    {
    case BUFFER_STATUS__SEARCH_HEADER:
        communicator->packet.header[communicator->index] = incoming_byte;
        if (communicator->packet.header[communicator->index] != PACKET_HEADER[communicator->index])
        {
            communicator->index = 0;
            break;
        }
        communicator->index++;
        // found valid packet header
        if (communicator->index == HEADER_LENGTH)
        {
            communicator->current_state = BUFFER_STATUS__GET_COMMAND;
        }
        break;

    case BUFFER_STATUS__GET_COMMAND:
        // read command value
        communicator->packet.command = (command_e)incoming_byte;
        communicator->current_state = BUFFER_STATUS__GET_LENGTH;
        break;

    case BUFFER_STATUS__GET_LENGTH:
        if (incoming_byte > CMD_PAYLOAD_SIZE)
        {
            // found packet with invalid payload length. Drop packet and continue
            return_value = INCOMING_PACKET_RETVAL__INVALID_PAYLOAD_LENGTH;
            communicator->index = 0;
            communicator->current_state = BUFFER_STATUS__SEARCH_HEADER;
            break;
        }
        communicator->packet.payload_length = incoming_byte;
        communicator->index = 0;
        if (communicator->packet.payload_length > 0)
        {
            // there is payload to read
            communicator->current_state = BUFFER_STATUS__READ_PAYLOAD;
        }
        else
        {
            // found packet with no payload. load it and finish
            memcpy(output_packet, &(communicator->packet), METADATA_LENGTH);
            communicator->current_state = BUFFER_STATUS__SEARCH_HEADER;
            return_value = INCOMING_PACKET_RETVAL__FOUND_PACKET;
        }
        break;

    case BUFFER_STATUS__READ_PAYLOAD:
        communicator->packet.payload[communicator->index] = incoming_byte;
        communicator->index++;
        if (communicator->index >= communicator->packet.payload_length)
        {
            // finished reading the packet's payload. load it and finish

            memcpy(output_packet, &(communicator->packet), METADATA_LENGTH + communicator->packet.payload_length);

            communicator->index = 0;
            communicator->current_state = BUFFER_STATUS__SEARCH_HEADER;
            return_value = INCOMING_PACKET_RETVAL__FOUND_PACKET;
        }
        break;

    default:
        break;
    }
exit:
    return return_value;
}

communication_protocol_retval_e communication_protocol__setup(communication_protocol_interface_t *communicator, read_byte_fn read_byte, write_byte_fn write_byte, is_ready_fn is_ready)
{
    communication_protocol_retval_e return_value = COMMUNICATION_PROTOCOL_RETVAL__UNINITIALISED;

    CHECK_NULL(communicator, exit, COMMUNICATION_PROTOCOL_RETVAL__NULL_PARAMETERS)
    CHECK_NULL(read_byte, exit, COMMUNICATION_PROTOCOL_RETVAL__NULL_PARAMETERS)
    CHECK_NULL(write_byte, exit, COMMUNICATION_PROTOCOL_RETVAL__NULL_PARAMETERS)
    CHECK_NULL(is_ready, exit, COMMUNICATION_PROTOCOL_RETVAL__NULL_PARAMETERS)

    communicator->read_byte = read_byte;
    communicator->write_byte = write_byte;
    communicator->is_ready = is_ready;
    communicator->current_state = BUFFER_STATUS__SEARCH_HEADER;
    communicator->index = 0;

    return_value = COMMUNICATION_PROTOCOL_RETVAL__SUCCESS;
exit:
    return return_value;
}

communication_protocol_retval_e communication_protocol__run_main_logic(communication_protocol_interface_t *communicator, process_packet_fn process_packet)
{
    communication_protocol_retval_e return_value = COMMUNICATION_PROTOCOL_RETVAL__SUCCESS;

    uint8_t index = 0;
    packet_t recevied_packet = {0};

    CHECK_NULL(communicator, exit, COMMUNICATION_PROTOCOL_RETVAL__NULL_PARAMETERS)
    CHECK_NULL(communicator->is_ready, exit, COMMUNICATION_PROTOCOL_RETVAL__NULL_PARAMETERS)

    // MAX_SEQUENT_BYTES prevents from infinite loop
    while (index <= MAX_SEQUENT_BYTES && communicator->is_ready())
    {
        if (INCOMING_PACKET_RETVAL__FOUND_PACKET == parse_incoming_byte(communicator, &recevied_packet))
        {
            process_packet(&recevied_packet);
            memset(&recevied_packet, 0, BUFFER_LENGTH);
        }
        index++;
    }
exit:
    return return_value;
}

communication_protocol_retval_e communication_protocol__send_packet(communication_protocol_interface_t *communicator, command_e command, uint8_t *packet, uint8_t length)
{
    communication_protocol_retval_e return_value = COMMUNICATION_PROTOCOL_RETVAL__SUCCESS;

    CHECK_NULL(communicator, exit, COMMUNICATION_PROTOCOL_RETVAL__NULL_PARAMETERS)
    CHECK_NULL(communicator->write_byte, exit, COMMUNICATION_PROTOCOL_RETVAL__NULL_PARAMETERS)

    if (length > 0)
    {
        CHECK_NULL(packet, exit, COMMUNICATION_PROTOCOL_RETVAL__NULL_PARAMETERS)
    }

    for (size_t i = 0; i < HEADER_LENGTH; i++)
    {
        communicator->write_byte(PACKET_HEADER[i]);
    }
    communicator->write_byte(command);
    communicator->write_byte(length);

    for (size_t i = 0; i < length; i++)
    {
        communicator->write_byte(packet[i]);
    }
exit:
    return return_value;
}