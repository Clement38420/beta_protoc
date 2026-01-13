#include "Value.h"

int get_value_size(const Value *data, size_t *size) {
    *size = 0;
    {
        size_t field_size = 0;
        field_size = sizeof(uint8_t); // Field ID
        field_size += sizeof(uint8_t); // Length byte
        
        field_size += sizeof(uint32_t); // Actual data
        
        *size += field_size;
    }
    {
        size_t field_size = 0;
        field_size = sizeof(uint8_t); // Field ID
        field_size += sizeof(uint8_t); // Length byte
        
        field_size += strlen(data->unit); // Actual string data
        
        *size += field_size;
    }

    return 0;
}

int value_to_buff(const Value *data, uint8_t **buff, size_t *rem_buff) {
    {
        if (*rem_buff < 2) return BETA_PROTOC_ERR_BUFFER_TOO_SMALL;
        **buff = (uint8_t)0; // Field ID
        (*buff)++;
        
        **buff = (uint8_t)sizeof(uint32_t); // Length byte
        (*buff)++;
        *rem_buff -= 2;

        int result = uint32_to_buff(data->value, buff, rem_buff);
        
        if (result != 0) return result;
    }
    {
        if (*rem_buff < 2) return BETA_PROTOC_ERR_BUFFER_TOO_SMALL;
        **buff = (uint8_t)1; // Field ID
        (*buff)++;
        
        **buff = (uint8_t)strlen(data->unit); // Length byte
        (*buff)++;
        *rem_buff -= 2;

        int result = string_to_buff(data->unit, strlen(data->unit), buff, rem_buff);
        
        if (result != 0) return result;
    }

    return 0;
}

int value_to_message(const Value *data, uint8_t **buff, size_t *rem_buff) {
    if (*rem_buff < MESSAGE_HEADER_SIZE) return BETA_PROTOC_ERR_BUFFER_TOO_SMALL;
    **buff = (uint8_t)PROTOC_VERSION; // Protoc version
    (*buff)++;
    **buff = (uint8_t)1; // Message ID
    (*buff)++;
    uint8_t *p_payload_len = *buff; // Pointer to message length byte
    **buff = 0;
    (*buff)++;
    *rem_buff -= MESSAGE_HEADER_SIZE;

    size_t initial_rem_buff = *rem_buff;

    int result = value_to_buff(data, buff, rem_buff);
    if (result != 0) return result;

    *p_payload_len = initial_rem_buff - *rem_buff;

    return 0;
}

int value_from_buff(Value *data, uint8_t **buff, size_t *rem_buff) {
    while (*rem_buff > 0) {
        if (*rem_buff < 2) return BETA_PROTOC_ERR_INVALID_DATA;
        uint8_t field_id = **buff;
        (*buff)++;
        size_t field_len = **buff;
        (*buff)++;
        *rem_buff -= 2;

        switch (field_id) {
            case 0:
            {
                
                int result = uint32_from_buff(&(data->value), buff, rem_buff);
                
                if (result != 0) return result;
                break;
            }
            case 1:
            {
                
                if (field_len >= 32) return BETA_PROTOC_ERR_INVALID_DATA;
                int result = string_from_buff(data->unit, field_len, buff, rem_buff);
                data->unit[field_len] = '\0';
                
                if (result != 0) return result;
                break;
            }

            default:
                if (field_len > *rem_buff) return BETA_PROTOC_ERR_INVALID_DATA;
                (*buff) += field_len;
                *rem_buff -= field_len;
        }
    }

    return 0;
}

int value_from_message(Value *data, uint8_t **buff, size_t *rem_buff) {
    if (*rem_buff < MESSAGE_HEADER_SIZE) return BETA_PROTOC_ERR_INVALID_DATA;
    if (**buff != PROTOC_VERSION) return BETA_PROTOC_ERR_INVALID_PROTOC_VERSION;
    (*buff)++;
    if (**buff != 1) return BETA_PROTOC_ERR_INVALID_ID;
    (*buff)++;
    size_t payload_len = **buff;
    (*buff)++;
    *rem_buff -= MESSAGE_HEADER_SIZE;

    if (*rem_buff < payload_len) return BETA_PROTOC_ERR_INVALID_DATA;
    size_t rem_payload = payload_len;

    int result = value_from_buff(data, buff, &rem_payload);
    if (result != 0) return result;

    *rem_buff -= payload_len;

    return 0;
}