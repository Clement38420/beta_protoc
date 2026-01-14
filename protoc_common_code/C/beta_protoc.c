#include "beta_protoc.h"

#include <string.h>

static beta_protoc_err_t _write_unsigned(uint64_t data, size_t size, uint8_t **buff, size_t *rem_buff) {
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
    return _write_unsigned((uint8_t)data, 1, buff, rem_buff);
}

beta_protoc_err_t int16_to_buff(int16_t data, uint8_t **buff, size_t *rem_buff) {
    return _write_unsigned((uint16_t)data, 2, buff, rem_buff);
}

beta_protoc_err_t int32_to_buff(int32_t data, uint8_t **buff, size_t *rem_buff) {
    return _write_unsigned((uint32_t)data, 4, buff, rem_buff);
}

beta_protoc_err_t int64_to_buff(int64_t data, uint8_t **buff, size_t *rem_buff) {
    return _write_unsigned((uint64_t)data, 8, buff, rem_buff);
}

beta_protoc_err_t uint8_to_buff(uint8_t data, uint8_t **buff, size_t *rem_buff) {
    return _write_unsigned(data, 1, buff, rem_buff);
}

beta_protoc_err_t uint16_to_buff(uint16_t data, uint8_t **buff, size_t *rem_buff) {
    return _write_unsigned(data, 2, buff, rem_buff);
}

beta_protoc_err_t uint32_to_buff(uint32_t data, uint8_t **buff, size_t *rem_buff) {
    return _write_unsigned(data, 4, buff, rem_buff);
}

beta_protoc_err_t uint64_to_buff(uint64_t data, uint8_t **buff, size_t *rem_buff) {
    return _write_unsigned(data, 8, buff, rem_buff);
}

beta_protoc_err_t float_to_buff(float data, uint8_t **buff, size_t *rem_buff) {
    union { float f; uint32_t u; } converter;
    converter.f = data;

    return _write_unsigned(converter.u, 4, buff, rem_buff);
}

beta_protoc_err_t double_to_buff(double data, uint8_t **buff, size_t *rem_buff) {
    union { double f; uint64_t u; } converter;
    converter.f = data;

    return _write_unsigned(converter.u, 8, buff, rem_buff);
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
    beta_protoc_err_t err = _read_unsigned(&temp, 4, buff, rem_buff);
    if (err == BETA_PROTOC_SUCCESS) *data = (int32_t)temp;
    return err;
}

beta_protoc_err_t int64_from_buff(int64_t *data, uint8_t **buff, size_t *rem_buff) {
    uint64_t temp;
    beta_protoc_err_t err = _read_unsigned(&temp, 8, buff, rem_buff);
    if (err == BETA_PROTOC_SUCCESS) *data = (int64_t)temp;
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
    beta_protoc_err_t err = _read_unsigned(&temp, 4, buff, rem_buff);
    if (err == BETA_PROTOC_SUCCESS) *data = (uint32_t)temp;
    return err;
}

beta_protoc_err_t uint64_from_buff(uint64_t *data, uint8_t **buff, size_t *rem_buff) {
    return _read_unsigned(data, 8, buff, rem_buff);
}

beta_protoc_err_t float_from_buff(float *data, uint8_t **buff, size_t *rem_buff) {
    uint64_t temp;
    beta_protoc_err_t err = _read_unsigned(&temp, 4, buff, rem_buff);
    if (err == BETA_PROTOC_SUCCESS) {
        union { uint32_t u; float f; } converter;
        converter.u = (uint32_t)temp;
        *data = converter.f;
    }
    return err;
}

beta_protoc_err_t double_from_buff(double *data, uint8_t **buff, size_t *rem_buff) {
    uint64_t temp;
    beta_protoc_err_t err = _read_unsigned(&temp, 8, buff, rem_buff);
    if (err == BETA_PROTOC_SUCCESS) {
        union { uint64_t u; double f; } converter;
        converter.u = temp;
        *data = converter.f;
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