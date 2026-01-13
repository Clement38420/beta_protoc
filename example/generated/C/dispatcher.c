#include "dispatcher.h"

dispatcher_err_t protoc_dispatch(uint8_t **buff, size_t *rem_buff) {
    uint8_t *p_buff = *buff;

    if (*rem_buff < MESSAGE_HEADER_SIZE) return DISPATCHER_ERR_INVALID_DATA;
    if (p_buff[0] != PROTOC_VERSION) {
            (*buff)++; // Move forward to try to identify a message
            (*rem_buff)--;
        return DISPATCHER_ERR_INVALID_PROTOC_VERSION;
    }

    switch(p_buff[1]) {
    
        case 0: {
            SensorData msg;
            int result = sensor_data_from_message(&msg, buff, rem_buff);
            if (result != 0) return result;

            on_sensor_data_received(&msg);
            return DISPATCHER_SUCCESS;
        }
    
        case 1: {
            Value msg;
            int result = value_from_message(&msg, buff, rem_buff);
            if (result != 0) return result;

            on_value_received(&msg);
            return DISPATCHER_SUCCESS;
        }
    
    default:
        (*buff)++; // Move forward to try to identify a message
        (*rem_buff)--;
        return DISPATCHER_ERR_UNKNOWN_ID;
    }
}