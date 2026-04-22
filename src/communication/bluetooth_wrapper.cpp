// #include "bluetooth_wrapper.h"

// bt_settings_t g_bt;

// extern communication_protocol_interface_t g_bluetooth_communicator;

// uint8_t bluetooth_serial_read_byte()
// {
//     return g_bt.obj.read();
// }
// void bluetooth_serial_write_byte(uint8_t byte)
// {
//     g_bt.obj.write(byte);
// }
// bool bluetooth_serial_is_ready()
// {
//     return g_bt.obj.available();
// }

// void bluetooth_communicator__setup()
// {
//     communication_protocol__setup(&g_bluetooth_communicator, bluetooth_serial_read_byte, bluetooth_serial_write_byte, bluetooth_serial_is_ready);
// }

// bool bluetooth__setup(bt_settings_t *bt)
// {

//     if (!bt->obj.begin(DEVICE_NAME))
//     {
//         Serial.println("Bluetooth begin failed");
//         return false;
//     }
//     bluetooth_communicator__setup();

//     bt->setup_successful = true;
//     return true;
// }

#include "bluetooth_wrapper.h"
bt_settings_t g_bt;
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "graphics/image_loader.h"
// #include "graphics/pages/menu.h"

// #define BLUETOOTH_TIMEOUT_MS 40
// #define BT_TX_QUEUE_SAFE_THRESHOLD 3

// volatile unsigned long bt_last_packet_duration_ms = 0;

// static volatile bool s_connection_check_flag = false;

//  //Accessing the internal FreeRTOS queue handle from the BluetoothSerial library -  possible because the variable isnt 'static'
// extern xQueueHandle _spp_tx_queue;

// //This volatile variable mirrors the internal 'SPP_CONGESTED' bit.
// static volatile bool s_is_congested = false;

// /**
//  * @brief Internal callback for ESP32 SPP (Serial Port Profile) events.
//  * This function is triggered by the BT stack whenever congestion state changes.
//  * * @param event The type of SPP event (e.g., Data received, Congestion, etc.)
//  * @param param Pointer to event parameters containing the congestion status.
//  */
// void IRAM_ATTR bluetooth_internal_callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
// {
//     if (event == ESP_SPP_CONG_EVT)
//     {
//         /* * param->cong.cong == true  -> Link is full/congested (Stop sending)
//          * param->cong.cong == false -> Link is free (Resume sending)
//          */
//         s_is_congested = param->cong.cong;
//     }
// }

// /**
//  * @brief Logic check: Is the Bluetooth link currently able to accept more data?
//  * This is a high-performance check of the congestion flag.
//  * * @return true if the link is free to send, false if it is congested.
//  */
// bool SPP_CAN_SEND()
// {
//     /* * If s_is_congested is false, it means we are NOT congested,
//      * which corresponds to the SPP_CONGESTED bit being SET in the library.
//      */
//     return !s_is_congested;
// }

// /**
//  * @brief Checks the available space in the internal transmission queue.
//  * Useful to avoid overflow before calling write().
//  * * @return Number of free packets (slots) available in the TX queue.
//  */
// int txQueueFree()
// {
//     if (_spp_tx_queue == NULL)
//     {
//         return 0;
//     }
//     /* Standard FreeRTOS call to check remaining spaces in a queue */
//     return (int)uxQueueSpacesAvailable(_spp_tx_queue);
// }

// void BT_connection_check()
// {
//     static volatile uint8_t s_counter = 0;
//     if (s_counter++ >= CONNECTION_CHECK_INTERVAL && !s_connection_check_flag)
//     {
//         s_connection_check_flag = true;
//         s_counter = 0;
//     }
// }

// void bluetooth__BOOT_setup(bt_settings_t *bt)
// {
//     bt->icon_state = BT_ICON_HIDDEN;
//     bt->state = BT_OFF;
//     bt->user_cmd = BT_DISABLE;
//     bt->tx_task_handle = NULL;
// }

// return_code_e bluetooth__setup(bt_settings_t *bt)
// {
//     return_code_e status = RETURN_CODE__UNINITIALISED;
//     bt->error_code = RETURN_CODE__UNINITIALISED;

//     if (!bt->obj.begin(DEVICE_NAME))
//     {
//         bt->setup_successful = false;
//         status = RETURN_CODE__BT__BEGIN_FAILED;
//         goto exit;
//     }

//     /** * CRITICAL STEP: Register our callback to listen for hardware events.
//      * Without this, SPP_CAN_SEND() will never update its state.
//      */
//     bt->obj.register_callback(bluetooth_internal_callback);

//     bt->setup_successful = true;
//     bt->state = BT_STARTING;
//     status = RETURN_CODE__SUCCESS;
// exit:
//     bt->error_code = status;
//     return status;
// }

// return_code_e bluetooth_logic(bt_settings_t *bt)
// {
//     return_code_e status = RETURN_CODE__SUCCESS;

//     switch (bt->state)
//     {
//     case BT_STARTING:
//     {
//         status = bluetooth__setup(bt);
//         if (status == RETURN_CODE__SUCCESS)
//         {
//             bluetooth_update_icon(bt, BT_ICON_ENABLED, false);
//             bt->state = BT_WAITING_FOR_CLIENT;
//         }
//     }
//     break;
//     case BT_WAITING_FOR_CLIENT:
//     {
//         if (s_connection_check_flag && bt->obj.hasClient())
//         {
//             bluetooth_update_icon(bt, BT_ICON_CONNECTED, false);
//             status = bluetooth_start_tx_task(bt);
//             if (status == RETURN_CODE__SUCCESS)
//             {
//                 bt->state = BT_SESSION;
//             }
//         }
//     }
//     break;
//     case BT_SESSION:
//     {
//         if (s_connection_check_flag && !bt->obj.hasClient())
//         {
//             bluetooth_update_icon(bt, BT_ICON_ENABLED, false);
//             bt->state = BT_WAITING_FOR_CLIENT;
//             break;
//         }
//         bluetooth__handler_received_byte(bt,&process_incoming_packet);
//     }
//     break;

//     case BT_RECOVERY:
//     {
//     }
//     break;

//     case BT_SHUTTING_DOWN:
//     {
//         if (bt->obj.hasClient())
//         {
//             bt->obj.disconnect();
//             vTaskDelay(pdMS_TO_TICKS(50));
//         }
//         _spp_tx_queue = NULL;
//         s_is_congested = false;
//         bt->obj.end();

//         bluetooth_update_icon(bt, BT_ICON_HIDDEN, false);

//         bt->state = BT_OFF;
//     }
//     break;

//     case BT_OFF:
//     {
//         // Idle: do nothing
//     }
//     break;

//     default:
//         status = RETURN_CODE__INVALID_PARAMETERS;
//         bt->state = BT_OFF;
//         break;
//     }

//     bt->error_code = status;
//     return status;
// }

// void bluetooth_update_icon(bt_settings_t *bt, bt_icon_state_e new_icon_state, bool force_redraw)
// {
//     if (bt == NULL)
//         return;

//     // Skip redraw only if state is same AND force_redraw is false
//     if (!force_redraw && (bt->icon_state == new_icon_state))
//         return;

//     // Always clear the icon area before drawing the new icon
//     g_tft.fillRect(15, 13, 25, 32, g_tft.color565(17, 17, 17)); // Clear with background color

//     switch (new_icon_state)
//     {
//     case BT_ICON_HIDDEN:
//         // Bluetooth OFF → icon hidden
//         break;

//     case BT_ICON_ENABLED:
//         // Bluetooth ON, no client
//         draw_png_from_SPIFFS(LANGUAGE_FOLDER "/BTOn.png", 15, 13, true);
//         break;

//     case BT_ICON_CONNECTED:
//         // Bluetooth connected
//         draw_png_from_SPIFFS(LANGUAGE_FOLDER "/BTConnected.png", 15, 13, true);
//         break;

//     default:
//         // Unknown state → do nothing (keep previous icon_state)
//         return;
//     }

//     // Store the new icon state after drawing
//     bt->icon_state = new_icon_state;
// }

// void bluetooth_request_send(bt_settings_t *bt,system_values *struct_to_send)
// {
//     if (bt->state == BT_SESSION)
//     {
//         bt->live_values = *(struct_to_send);
//         if (bt->tx_task_handle != NULL && g_reading_file.state != FILE_STATE__STREAMING && bt->stop_sending_data == false)
//         {
//             xTaskNotifyGive(bt->tx_task_handle);
//         }
//     }
// }

// static void bt_tx_task(void *arg)
// {
//     bt_settings_t *bt = (bt_settings_t *)arg;

//     // Payload buffer (adjust size if needed)
//     static constexpr size_t BT_PAYLOAD_BUFFER_SIZE = 1024;
//     char payload[BT_PAYLOAD_BUFFER_SIZE];

//     for (;;)
//     {
//         // Wait until someone notifies "send now"
//         ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

//         // Send only during active session
//         if (_spp_tx_queue == NULL || bt->state != BT_SESSION)
//             continue;

//         // Must have a connected client
//         if (!bt->obj.hasClient())
//         {
//             bt->error_code = RETURN_CODE__BT__ERR_NO_CLIENT;
//             continue;
//         }

//         // Must not be congested
//         if ( !SPP_CAN_SEND() )
//         {
//             bt->error_code = RETURN_CODE__BT__ERR_CONGESTED;
//             continue;
//         }

//         // Must have enough free slots in internal TX queue
//         if (txQueueFree() < BT_TX_QUEUE_SAFE_THRESHOLD)
//         {
//             bt->error_code = RETURN_CODE__BT__ERR_TX_QUEUE_LOW;
//             vTaskDelay(pdMS_TO_TICKS(10));
//             continue;
//         }

//         // Build payload from snapshot
//         int len = snprintf(payload, sizeof(payload),
//                            "Crueso_data"
//                            "\tstatus byte\t%u"
//                            "\tsystem mode(1->ccba)\t%u"
//                            "\tpapr s-v\t%u"
//                            "\thub s-v\t%u"
//                            "\tunite s-v\t%u"
//                            "\tdisplay s-v\t%u"
//                            "\tflow meter\t%.2f"
//                            "\tchecked flow\t%d"
//                            "\tunite pressure\t%.2f"
//                            "\tmin pressure\t%.2f"
//                            "\tmax pressure\t%.2f"
//                            "\tchecked peak pressure\t%.2f"
//                            "\tprogram state\t%u"
//                            "\tunite temp\t%.2f"
//                            "\tblower 2S\t%u"
//                            "\tblower-C\t%u"
//                            "\tCO2\t%u"
//                            "\tbreath state\t%u"
//                            "\tO2 Bar\t%u"
//                            "\tbar per min\t%.2f"
//                            "\tO2 temp\t%d"
//                            "\tHUB BATTERY\t%u"
//                            "\tfan 1\t%u"
//                            "\tfan 2\t%u"
//                            "\tpapr blower\t%u"
//                            "\tpapr pressure\t%.2f"
//                            "\tpapr temp\t%.2f"
//                            "\tPAPR BATTERY\t%u\n",
//                            (unsigned)bt->live_values.status_byte,
//                            (unsigned)bt->live_values.system_mode_is_ccba,
//                            (unsigned)bt->live_values.papr_software_version,
//                            (unsigned)bt->live_values.hub_software_version,
//                            (unsigned)bt->live_values.unite_software_version,
//                            (unsigned)SOFTWARE_VERSION,
//                            bt->live_values.unite_flow_meter / 64.0f,
//                            (int)bt->live_values.unite_checked_flow,
//                            bt->live_values.unite_pressure / 128.0f,
//                            bt->live_values.unite_min_pressure / 8.0f,
//                            bt->live_values.unite_max_pressure / 8.0f,
//                            bt->live_values.unite_checked_peak_pressure / 128.0f,
//                            (unsigned)bt->live_values.unite_program_state,
//                            bt->live_values.unite_temp / 64.0f,
//                            (unsigned)(bt->live_values.unite_blower_1 * 10u),
//                            (unsigned)(bt->live_values.unite_blower_2 * 10u),
//                            (unsigned)bt->live_values.unite_co2_ppm,
//                            (unsigned)bt->live_values.unite_breath_state,
//                            (unsigned)bt->live_values.o2_bar,
//                            (bt->live_values.bar_per_min),
//                            (int)bt->live_values.o2_temp,
//                            (unsigned)bt->live_values.hub_battery_percent,
//                            (unsigned)bt->live_values.fan1_speed,
//                            (unsigned)bt->live_values.fan2_speed,
//                            (unsigned)bt->live_values.papr_blower_rpm,
//                            bt->live_values.papr_pressure / 128.0f,
//                            bt->live_values.papr_temperature / 128.0f,
//                            (unsigned)bt->live_values.papr_battery_percent);

//         if (len < 0 || (size_t)len >= sizeof(payload))
//         {
//             // Payload overflow / formatting error
//             bt->error_code = RETURN_CODE__BT__ERR_WRITE_FAILED;
//             continue;
//         }

//         // Enqueue packet to BTSerial internal TX queue
//         size_t sent = bt->obj.write((const uint8_t *)payload, (size_t)len);
//         if (sent != (size_t)len)
//         {
//             bt->error_code = RETURN_CODE__BT__ERR_WRITE_FAILED;
//             continue;
//         }

//         bt->error_code = RETURN_CODE__SUCCESS;
//     }
// }

// return_code_e bluetooth_start_tx_task(bt_settings_t *bt)
// {
//     if (!bt)
//         return RETURN_CODE__INVALID_PARAMETERS;

//     // Task already created → nothing to do
//     if (bt->tx_task_handle != NULL)
//         return RETURN_CODE__SUCCESS;

//     // Create TX task
//     BaseType_t result = xTaskCreatePinnedToCore(
//         bt_tx_task,          // Task function
//         "bt_tx_task",        // Task name
//         4096,                // Stack size
//         bt,                  // Task parameter
//         2,                   // Priority (above main loop)
//         &bt->tx_task_handle, // Task handle
//         0                    // Core 0 (same core as BT stack)
//     );

//     if (result != pdPASS)
//     {
//         bt->tx_task_handle = NULL;
//         return RETURN_CODE__BT__ERR_WRITE_FAILED;
//     }

//     return RETURN_CODE__SUCCESS;
// }

#include "bluetooth_wrapper.h"
bt_settings_t g_bt;

return_code_e bluetooth__setup(bt_settings_t *bt)
{
    return_code_e status = RETURN_CODE__UNINITIALISED;
    bt->error_code = RETURN_CODE__UNINITIALISED;

    if (!bt->obj.begin(DEVICE_NAME))
    {
        bt->setup_successful = false;
        status = RETURN_CODE__BT__BEGIN_FAILED;
        goto exit;
    }

    bt->setup_successful = true;
    status = RETURN_CODE__SUCCESS;
exit:
    bt->error_code = status;
    return status;
}
static void bluetooth__reset_parser(bt_settings_t *bt)
{
    if (bt == NULL)
        return;

    // עכשיו הפונקציה באמת מחזירה את המנתח למצב "נקי" לגמרי
    bt->incoming_data_state = INCOMING_DATA__WAIT_FOR_PACKET;
    bt->rx_buffer_index = 0;
    memset(&bt->received_packet, 0, sizeof(packet_t));
}

void bluetooth__poll_and_process_packets(bt_settings_t *bt, process_packet_fn process_packet)
{
    while (bt->obj.available())
    {
        int incoming = bt->obj.read();
        if (incoming < 0)
            return;

        uint8_t incoming_byte = (uint8_t)incoming;

        switch (bt->incoming_data_state)
        {
        case INCOMING_DATA__WAIT_FOR_PACKET:
            // Check for packet header sequence: we need to see the three header bytes in order (0x52, 0x4F, 0x59)
            if ((incoming_byte == PACKET_HEADER[2]) && (bt->incoming_byte_del1 == PACKET_HEADER[1]) && (bt->incoming_byte_del2 == PACKET_HEADER[0]))
            {
                bt->incoming_data_state = INCOMING_DATA__GET_COMMAND;
            }
            break;

        case INCOMING_DATA__GET_COMMAND:
            bt->received_packet.command = (command_e)incoming_byte;
            bt->incoming_data_state = INCOMING_DATA__GET_LENGTH;
            break;

        case INCOMING_DATA__GET_LENGTH:
            if (incoming_byte > BUFFER_SIZE || incoming_byte == 0)
            {
                if (incoming_byte == 0)
                {
                    process_packet(bt->received_packet); // valid packet with no payload
                }
                bluetooth__reset_parser(bt);
            }
            else
            {
                bt->received_packet.payload_length = incoming_byte;
                bt->incoming_data_state = INCOMING_DATA__READ_PAYLOAD;
                bt->rx_buffer_index = 0;
            }
            break;

        case INCOMING_DATA__READ_PAYLOAD:
            // write byte to payload buffer if there is space (should always be the case due to previous length check)
            if (bt->rx_buffer_index < BUFFER_SIZE)
            {
                bt->received_packet.payload[bt->rx_buffer_index++] = incoming_byte;
            }

            // does the packet have the full payload?
            if (bt->rx_buffer_index >= bt->received_packet.payload_length)
            {
                process_packet(bt->received_packet);
                bluetooth__reset_parser(bt);
            }
            break;

        default:
            bluetooth__reset_parser(bt);
            break;
        }

        bt->incoming_byte_del2 = bt->incoming_byte_del1;
        bt->incoming_byte_del1 = incoming_byte;
    }
}