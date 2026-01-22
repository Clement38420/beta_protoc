#include "../include/beta_protoc.h"

#include <string.h>

uint32_t zigzag_encode_32(int32_t value) {
    return (uint32_t)((value << 1) ^ (value >> 31));
}

int32_t zigzag_decode_32(uint32_t value) {
    return (int32_t)((value >> 1) ^ -(int32_t)(value & 1));
}

uint64_t zigzag_encode_64(int64_t value) {
    return (uint64_t)((value << 1) ^ (value >> 63));
}

int64_t zigzag_decode_64(uint64_t value) {
    return (int64_t)((value >> 1) ^ -(int64_t)(value & 1));
}

size_t safe_strlen(const char *str, size_t max_len) {
    size_t len = 0;
    while (len < max_len && str[len] != '\0') {
        len++;
    }
    return len;
}

beta_protoc_err_t varint_to_buff(uint64_t data, uint8_t **buff, size_t *rem_buff) {
    if (buff == NULL || *buff == NULL || rem_buff == NULL) {
        return BETA_PROTOC_ERR_INVALID_ARGS;
    }

    do {
        if (*rem_buff < 1) return BETA_PROTOC_ERR_BUFFER_TOO_SMALL;

        uint8_t byte = data & 0x7F;

        data >>= 7;
        if (data != 0) {
            byte |= 0x80; // More bytes to come
        }

        **buff = byte;
        (*buff)++;
        (*rem_buff)--;

    } while (data != 0);

    return BETA_PROTOC_SUCCESS;
}

beta_protoc_err_t varint_from_buff(uint64_t *data, uint8_t **buff, size_t *rem_buff) {
    if (data == NULL || buff == NULL || *buff == NULL || rem_buff == NULL) {
        return BETA_PROTOC_ERR_INVALID_ARGS;
    }

    *data = 0;
    uint8_t shift = 0;

     while (1) {
        if (*rem_buff < 1) return BETA_PROTOC_ERR_BUFFER_TOO_SMALL;

        uint8_t byte = **buff;
        (*buff)++;
        (*rem_buff)--;

        *data |= ((uint64_t) (byte & 0x7F)) << shift;

        if ((byte & 0x80) == 0) {
            return BETA_PROTOC_SUCCESS;
        }

        shift += 7;

        if (shift >= 64) return BETA_PROTOC_ERR_INVALID_DATA;
    }
}

size_t varint_size(uint64_t data) {
    size_t out_size = 0;

    do {
        (out_size)++;
        data >>= 7;
    } while (data != 0);

    return out_size;
}

size_t int8_size(int8_t data) {
    (void)data;
    return 1;
}

size_t int16_size(int16_t data) {
    (void)data;
    return 2;
}

size_t uint8_size(uint8_t data) {
    (void)data;
    return 1;
}

size_t uint16_size(uint16_t data) {
    (void)data;
    return 2;
}

size_t int32_size(int32_t data) {
    return varint_size(zigzag_encode_32(data));
}

size_t int64_size(int64_t data) {
    return varint_size(zigzag_encode_64(data));
}

size_t uint32_size(uint32_t data) {
    return varint_size(data);
}

size_t uint64_size(uint64_t data) {
    return varint_size(data);
}

size_t float32_size(float data) {
    (void)data;
    return 4;
}

size_t float64_size(double data) {
    (void)data;
    return 8;
}

size_t char_size(char data) {
    (void)data;
    return 1;
}

size_t bool_size(bool data) {
    (void)data;
    return 1;
}

static beta_protoc_err_t _write_unsigned(uint64_t data, size_t size, uint8_t **buff, size_t *rem_buff) {
    if (buff == NULL || *buff == NULL || rem_buff == NULL) {
        return BETA_PROTOC_ERR_INVALID_ARGS;
    }

    if (*rem_buff < size) return BETA_PROTOC_ERR_BUFFER_TOO_SMALL;

    for (size_t i = 0; i < size; i++) {
        **buff = (uint8_t)(data & 0xFF);
        (*buff)++;
        data >>= 8;
    }
    *rem_buff -= size;
    return BETA_PROTOC_SUCCESS;
}

beta_protoc_err_t int8_to_buff(int8_t data, uint8_t **buff, size_t *rem_buff) {
    return _write_unsigned(data, 1, buff, rem_buff);
}

beta_protoc_err_t int16_to_buff(int16_t data, uint8_t **buff, size_t *rem_buff) {
    return _write_unsigned(data, 2, buff, rem_buff);
}

beta_protoc_err_t int32_to_buff(int32_t data, uint8_t **buff, size_t *rem_buff) {
    uint32_t temp = zigzag_encode_32(data);
    return varint_to_buff(temp, buff, rem_buff);
}

beta_protoc_err_t int64_to_buff(int64_t data, uint8_t **buff, size_t *rem_buff) {
    uint64_t temp = zigzag_encode_64(data);
    return varint_to_buff(temp, buff, rem_buff);
}

beta_protoc_err_t uint8_to_buff(uint8_t data, uint8_t **buff, size_t *rem_buff) {
    return _write_unsigned(data, 1, buff, rem_buff);
}

beta_protoc_err_t uint16_to_buff(uint16_t data, uint8_t **buff, size_t *rem_buff) {
    return _write_unsigned(data, 2, buff, rem_buff);
}

beta_protoc_err_t uint32_to_buff(uint32_t data, uint8_t **buff, size_t *rem_buff) {
    return varint_to_buff(data, buff, rem_buff);
}

beta_protoc_err_t uint64_to_buff(uint64_t data, uint8_t **buff, size_t *rem_buff) {
    return varint_to_buff(data, buff, rem_buff);
}

beta_protoc_err_t float32_to_buff(float data, uint8_t **buff, size_t *rem_buff) {
    uint32_t u_val;
    memcpy(&u_val, &data, sizeof(uint32_t));

    return _write_unsigned(u_val, 4, buff, rem_buff);
}

beta_protoc_err_t float64_to_buff(double data, uint8_t **buff, size_t *rem_buff) {
    uint64_t u_val;
    memcpy(&u_val, &data, sizeof(uint64_t));

    return _write_unsigned(u_val, 8, buff, rem_buff);
}

beta_protoc_err_t string_to_buff(const char *data, size_t data_len, uint8_t **buff, size_t *rem_buff) {
    if (buff == NULL || *buff == NULL || rem_buff == NULL) {
        return BETA_PROTOC_ERR_INVALID_ARGS;
    }

    if (*rem_buff < data_len) return BETA_PROTOC_ERR_BUFFER_TOO_SMALL;

    memcpy(*buff, data, data_len);
    *buff += data_len;
    *rem_buff -= data_len;

    return BETA_PROTOC_SUCCESS;
}

beta_protoc_err_t char_to_buff(char data, uint8_t **buff, size_t *rem_buff) {
    return _write_unsigned((uint8_t)data, 1, buff, rem_buff);
}

beta_protoc_err_t bool_to_buff(bool data, uint8_t **buff, size_t *rem_buff) {
    return _write_unsigned((uint8_t)data, 1, buff, rem_buff);
}

static beta_protoc_err_t _read_unsigned(uint64_t *data, size_t size, uint8_t **buff, size_t *rem_buff) {
    if (buff == NULL || *buff == NULL || rem_buff == NULL || data == NULL) {
        return BETA_PROTOC_ERR_INVALID_ARGS;
    }
    if (*rem_buff < size) return BETA_PROTOC_ERR_BUFFER_TOO_SMALL;

    *data = 0;
    for (size_t i = 0; i < size; i++) {
        *data |= ((uint64_t)(**buff) << (8 * i));
        (*buff)++;
    }
    *rem_buff -= size;
    return BETA_PROTOC_SUCCESS;
}

beta_protoc_err_t int8_from_buff(int8_t *data, uint8_t **buff, size_t *rem_buff) {
    uint64_t temp;
    beta_protoc_err_t err = _read_unsigned(&temp, 1, buff, rem_buff);
    if (err == BETA_PROTOC_SUCCESS) *data = (int8_t)temp;
    return err;
}

beta_protoc_err_t int16_from_buff(int16_t *data, uint8_t **buff, size_t *rem_buff) {
    uint64_t temp;
    beta_protoc_err_t err = _read_unsigned(&temp, 2, buff, rem_buff);
    if (err == BETA_PROTOC_SUCCESS) *data = (int16_t)temp;
    return err;
}

beta_protoc_err_t int32_from_buff(int32_t *data, uint8_t **buff, size_t *rem_buff) {
    uint64_t temp;
    beta_protoc_err_t err = varint_from_buff(&temp, buff, rem_buff);
    if (err == BETA_PROTOC_SUCCESS) *data = zigzag_decode_32(temp);
    return err;
}

beta_protoc_err_t int64_from_buff(int64_t *data, uint8_t **buff, size_t *rem_buff) {
    uint64_t temp;
    beta_protoc_err_t err = varint_from_buff(&temp, buff, rem_buff);
    if (err == BETA_PROTOC_SUCCESS) *data = zigzag_decode_64(temp);
    return err;
}

beta_protoc_err_t uint8_from_buff(uint8_t *data, uint8_t **buff, size_t *rem_buff) {
    uint64_t temp;
    beta_protoc_err_t err = _read_unsigned(&temp, 1, buff, rem_buff);
    if (err == BETA_PROTOC_SUCCESS) *data = (uint8_t)temp;
    return err;
}

beta_protoc_err_t uint16_from_buff(uint16_t *data, uint8_t **buff, size_t *rem_buff) {
    uint64_t temp;
    beta_protoc_err_t err = _read_unsigned(&temp, 2, buff, rem_buff);
    if (err == BETA_PROTOC_SUCCESS) *data = (uint16_t)temp;
    return err;
}

beta_protoc_err_t uint32_from_buff(uint32_t *data, uint8_t **buff, size_t *rem_buff) {
    uint64_t temp;
    beta_protoc_err_t err = varint_from_buff(&temp, buff, rem_buff);
    if (err == BETA_PROTOC_SUCCESS) *data = (uint32_t)temp;
    return err;
}

beta_protoc_err_t uint64_from_buff(uint64_t *data, uint8_t **buff, size_t *rem_buff) {
    return varint_from_buff(data, buff, rem_buff);
}

beta_protoc_err_t float32_from_buff(float *data, uint8_t **buff, size_t *rem_buff) {
    uint64_t temp;
    beta_protoc_err_t err = _read_unsigned(&temp, 4, buff, rem_buff);
    if (err == BETA_PROTOC_SUCCESS) {
        uint32_t u_val = (uint32_t) temp;
        memcpy(data, &u_val, sizeof(float));
    }
    return err;
}

beta_protoc_err_t float64_from_buff(double *data, uint8_t **buff, size_t *rem_buff) {
    uint64_t temp;
    beta_protoc_err_t err = _read_unsigned(&temp, 8, buff, rem_buff);
    if (err == BETA_PROTOC_SUCCESS) {
        memcpy(data, &temp, sizeof(double));
    }
    return err;
}

beta_protoc_err_t string_from_buff(char *data, size_t data_len, uint8_t **buff, size_t *rem_buff) {
    if (buff == NULL || *buff == NULL || rem_buff == NULL || data == NULL) {
        return BETA_PROTOC_ERR_INVALID_ARGS;
    }

    if (*rem_buff < data_len) return BETA_PROTOC_ERR_BUFFER_TOO_SMALL;

    memcpy(data, *buff, data_len);
    *buff += data_len;
    *rem_buff -= data_len;

    return BETA_PROTOC_SUCCESS;
}

beta_protoc_err_t char_from_buff(char *data, uint8_t **buff, size_t *rem_buff) {
    uint64_t temp;
    beta_protoc_err_t err = _read_unsigned(&temp, 1, buff, rem_buff);
    if (err == BETA_PROTOC_SUCCESS) *data = (char)temp;
    return err;
}

beta_protoc_err_t bool_from_buff(bool *data, uint8_t **buff, size_t *rem_buff) {
    uint64_t temp;
    beta_protoc_err_t err = _read_unsigned(&temp, 1, buff, rem_buff);
    if (err == BETA_PROTOC_SUCCESS) *data = (bool)temp;
    return err;
}