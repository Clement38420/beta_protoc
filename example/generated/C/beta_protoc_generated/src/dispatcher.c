#include "dispatcher.h"

int protoc_dispatch(uint8_t **buff, size_t *rem_buff, void *ctx) {
    uint8_t *p_buff = *buff;

    // Check for minimum buffer size (version + message ID)
    if (*rem_buff < 3) {
        return DISPATCHER_ERR_INVALID_DATA;
    }
    // Check protocol version
    if (p_buff[0] != PROTOC_VERSION) {
        return DISPATCHER_ERR_INVALID_PROTOC_VERSION;
    }

    // Dispatch based on message ID (little-endian)
    switch (((uint16_t) p_buff[2] << 8) | (uint16_t) p_buff[1]) {
        case 0: {
            SensorData msg;
            int result = sensor_data_from_message(&msg, buff, rem_buff);
            if (result != 0) {
                return result;
            }

            // Call the user-implemented callback for the received message
            if (on_sensor_data_received != NULL) {
                on_sensor_data_received(&msg, ctx);
            }
            return DISPATCHER_SUCCESS;
        }
        case 1: {
            Value msg;
            int result = value_from_message(&msg, buff, rem_buff);
            if (result != 0) {
                return result;
            }

            // Call the user-implemented callback for the received message
            if (on_value_received != NULL) {
                on_value_received(&msg, ctx);
            }
            return DISPATCHER_SUCCESS;
        }
        default:
            return DISPATCHER_ERR_UNKNOWN_MESSAGE_ID;
    }
}