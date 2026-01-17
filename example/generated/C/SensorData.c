#include "SensorData.h"

int32_t get_sensor_data_size(const SensorData *data) {
    if (data == NULL) {
        return BETA_PROTOC_ERR_INVALID_ARGS;
    }

    int32_t size = 0;
    // Field: id
    {
        size_t field_size = 0;
        
        // Primitive type size calculation
        field_size += uint32_size(data->id);

        // Add size of field length (varint) and field ID (varint)
        field_size += varint_size(field_size);
        field_size += varint_size((uint64_t) 0);
        if (size + field_size > SIZE_MAX) {
            return BETA_PROTOC_VALUE_EXCEEDS_ARCH_LIMIT;
        }
        size += field_size;
    }
    // Field: name
    {
        size_t field_size = 0;
        
        for (size_t i = 0; i < data->name_count; i++) {
        
        // Special case for char type to avoid counting after null-terminator
        if (data->name[i] == '\0') {
            break;
        }
        
        // Primitive type size calculation
        field_size += char_size(data->name[i]);
        }

        // Add size of field length (varint) and field ID (varint)
        field_size += varint_size(field_size);
        field_size += varint_size((uint64_t) 1);
        if (size + field_size > SIZE_MAX) {
            return BETA_PROTOC_VALUE_EXCEEDS_ARCH_LIMIT;
        }
        size += field_size;
    }
    // Field: value
    {
        size_t field_size = 0;
        
        
        // Nested message size calculation
        int32_t nested_size = get_value_size(&(data->value));
        if (nested_size < 0) {
            return nested_size;
        }
        field_size += nested_size;
        

        // Add size of field length (varint) and field ID (varint)
        field_size += varint_size(field_size);
        field_size += varint_size((uint64_t) 2);
        if (size + field_size > SIZE_MAX) {
            return BETA_PROTOC_VALUE_EXCEEDS_ARCH_LIMIT;
        }
        size += field_size;
    }
    return size;
}

beta_protoc_err_t sensor_data_to_buff(const SensorData *data, uint8_t **buff, size_t *rem_buff) {
    if (buff == NULL || *buff == NULL || rem_buff == NULL || data == NULL) {
        return BETA_PROTOC_ERR_INVALID_ARGS;
    }
    // Field: id
    {
        // Serialize field ID
        beta_protoc_err_t id_varint_err = varint_to_buff((uint64_t) 0, buff, rem_buff);
        if (id_varint_err != 0) {
            return id_varint_err;
        }
        
        beta_protoc_err_t len_varint_err = varint_to_buff(uint32_size(data->id), buff, rem_buff);
        if (len_varint_err != 0) {
            return len_varint_err;
        }

        // Serialize value
        beta_protoc_err_t field_err = uint32_to_buff(data->id, buff, rem_buff);
        if (field_err != 0) {
            return field_err;
        }
    }
    // Field: name
    {
        if (data->name_count > 32) {
            return BETA_PROTOC_ERR_ARRAY_SIZE_EXCEEDED;
        }
        // Serialize field ID
        beta_protoc_err_t id_varint_err = varint_to_buff((uint64_t) 1, buff, rem_buff);
        if (id_varint_err != 0) {
            return id_varint_err;
        }
        
        // Serialize field length (sum of all elements size for primitive arrays)
        size_t array_size = 0;
        for (size_t i = 0; i < data->name_count; i++) {
            
            // Special case for char type to avoid counting after null-terminator
            if (data->name[i] == '\0') {
                break;
            }
            
            array_size += char_size(data->name[i]);
        }
        beta_protoc_err_t len_varint_err = varint_to_buff(array_size, buff, rem_buff);
        if (len_varint_err != 0) {
            return len_varint_err;
        }

        for (size_t i = 0; i < data->name_count; i++) {

        
        // Special case for char type to avoid writing after null-terminator
        if (data->name[i] == '\0') {
            break;
        }
        
        

        // Serialize value
        beta_protoc_err_t field_err = char_to_buff(data->name[i], buff, rem_buff);
        if (field_err != 0) {
            return field_err;
        }
        }
    }
    // Field: value
    {
        // Serialize field ID
        beta_protoc_err_t id_varint_err = varint_to_buff((uint64_t) 2, buff, rem_buff);
        if (id_varint_err != 0) {
            return id_varint_err;
        }
        
        // Serialize field length
        int32_t nested_size = get_value_size(&(data->value));
        if (nested_size < 0) {
            return nested_size;
        }
        beta_protoc_err_t len_varint_err = varint_to_buff(nested_size, buff, rem_buff);
        if (len_varint_err != 0) {
            return len_varint_err;
        }

        // Serialize value
        beta_protoc_err_t field_err = value_to_buff(&(data->value), buff, rem_buff);
        if (field_err != 0) {
            return field_err;
        }
    }
    return BETA_PROTOC_SUCCESS;
}

beta_protoc_err_t sensor_data_to_message(const SensorData *data, uint8_t **buff, size_t *rem_buff) {
    if (buff == NULL || *buff == NULL || rem_buff == NULL || data == NULL) {
        return BETA_PROTOC_ERR_INVALID_ARGS;
    }

    // Write protocol version
    if (*rem_buff < 1) {
        return BETA_PROTOC_ERR_BUFFER_TOO_SMALL;
    }
    **buff = (uint8_t) PROTOC_VERSION;
    (*buff)++;
    (*rem_buff)--;

    // Write message ID
    if (*rem_buff < 2) {
        return BETA_PROTOC_ERR_BUFFER_TOO_SMALL;
    }
    **buff = (uint16_t) 0;
    (*buff)++;
    **buff = (((uint16_t) 0 >> 8) & 0x00FF);
    (*buff)++;
    *rem_buff -= 2;

    // Write payload size
    int32_t payload_size = get_sensor_data_size(data);
    if (payload_size < 0) {
        return payload_size;
    }
    beta_protoc_err_t len_varint_err = varint_to_buff(payload_size, buff, rem_buff);
    if (len_varint_err != 0) {
        return len_varint_err;
    }

    // Write payload
    beta_protoc_err_t msg_err = sensor_data_to_buff(data, buff, rem_buff);
    if (msg_err != 0) {
        return msg_err;
    }

    return BETA_PROTOC_SUCCESS;
}

beta_protoc_err_t sensor_data_from_buff(SensorData *data, uint8_t **buff, size_t *rem_buff) {
    if (buff == NULL || *buff == NULL || rem_buff == NULL || data == NULL) {
        return BETA_PROTOC_ERR_INVALID_ARGS;
    }

    // Initialize array counts
    data->name_count = 0;

    while (*rem_buff > 0) {
        // Deserialize field ID
        uint64_t field_id;
        beta_protoc_err_t id_varint_err = varint_from_buff(&field_id, buff, rem_buff);
        if (id_varint_err != 0) {
            return id_varint_err;
        }

        // Deserialize field length
        size_t field_len;
        {
            uint64_t tmp;
            beta_protoc_err_t len_varint_err = varint_from_buff(&tmp, buff, rem_buff);
            if (len_varint_err != 0) {
                return len_varint_err;
            }
            if (tmp > SIZE_MAX) {
                return BETA_PROTOC_VALUE_EXCEEDS_ARCH_LIMIT;
            }
            field_len = (size_t) tmp;
        }

        switch (field_id) {
            // Field: id
            case 0: {
                uint8_t *field_start_buff = *buff;

                // Deserialize field value
                
                beta_protoc_err_t field_err = uint32_from_buff(&(data->id), buff, rem_buff);
                if (field_err != 0) {
                    return field_err;
                }

                // Check if the correct number of bytes were read
                if ((size_t)(*buff - field_start_buff) != field_len) {
                    return BETA_PROTOC_ERR_INVALID_DATA;
                }

                break;
            }
            // Field: name
            case 1: {
                uint8_t *field_start_buff = *buff;

                // Deserialize field value
                
                while (*buff - field_start_buff < field_len) {
                    beta_protoc_err_t field_err = char_from_buff(&(data->name[data->name_count]), buff, rem_buff);
                    if (field_err != 0) {
                        return field_err;
                    }
                    data->name_count++;
                    if (data->name_count > 32) {
                        return BETA_PROTOC_ERR_ARRAY_SIZE_EXCEEDED;
                    }
                }

                // Check if the correct number of bytes were read
                if ((size_t)(*buff - field_start_buff) != field_len) {
                    return BETA_PROTOC_ERR_INVALID_DATA;
                }

                break;
            }
            // Field: value
            case 2: {
                uint8_t *field_start_buff = *buff;

                // Deserialize field value
                size_t rem_nested = field_len;
                beta_protoc_err_t field_err = value_from_buff(&(data->value), buff, &rem_nested);
                *rem_buff -= field_len;
                if (field_err != 0) {
                    return field_err;
                }

                // Check if the correct number of bytes were read
                if ((size_t)(*buff - field_start_buff) != field_len) {
                    return BETA_PROTOC_ERR_INVALID_DATA;
                }

                break;
            }
            default:
                // Skip unknown fields
                if (field_len > *rem_buff) {
                    return BETA_PROTOC_ERR_INVALID_DATA;
                }
                (*buff) += field_len;
                *rem_buff -= field_len;
        }
    }

    // Null-terminate strings
    data->name[data->name_count] = '\0';

    return BETA_PROTOC_SUCCESS;
}

beta_protoc_err_t sensor_data_from_message(SensorData *data, uint8_t **buff, size_t *rem_buff) {
    if (buff == NULL || *buff == NULL || rem_buff == NULL || data == NULL) {
        return BETA_PROTOC_ERR_INVALID_ARGS;
    }

    // Read and check protocol version
    if (*rem_buff < 1) {
        return BETA_PROTOC_ERR_INVALID_DATA;
    }
    if (**buff != PROTOC_VERSION) {
        return BETA_PROTOC_ERR_INVALID_PROTOC_VERSION;
    }
    (*buff)++;
    (*rem_buff)--;

    // Read and check message ID
    if (*rem_buff < 2) {
        return BETA_PROTOC_ERR_INVALID_DATA;
    }
    if ((((uint16_t)(*(*buff + 1)) << 8) | (uint16_t)(**buff)) != 0) {
        return BETA_PROTOC_ERR_INVALID_ID;
    }
    *buff += 2;
    *rem_buff -= 2;

    // Read payload length
    size_t payload_len;
    {
        uint64_t tmp;
        beta_protoc_err_t len_varint_err = varint_from_buff(&tmp, buff, rem_buff);
        if (len_varint_err != 0) {
            return len_varint_err;
        }
        if (tmp > SIZE_MAX) {
            return BETA_PROTOC_VALUE_EXCEEDS_ARCH_LIMIT;
        }
        payload_len = (size_t) tmp;
    }

    if (*rem_buff < payload_len) {
        return BETA_PROTOC_ERR_INVALID_DATA;
    }
    size_t rem_payload = payload_len;

    // Read payload
    beta_protoc_err_t msg_err = sensor_data_from_buff(data, buff, &rem_payload);
    if (msg_err != 0) {
        return msg_err;
    }

    *rem_buff -= payload_len;

    return BETA_PROTOC_SUCCESS;
}