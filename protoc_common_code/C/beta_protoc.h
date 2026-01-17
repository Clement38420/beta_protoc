#ifndef BETA_PROTOC_H
#define BETA_PROTOC_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PROTOC_VERSION 1

typedef enum {
    BETA_PROTOC_SUCCESS = 0,
    BETA_PROTOC_ERR_INVALID_ARGS = -1, // NULL pointers passed as parameters
    BETA_PROTOC_ERR_BUFFER_TOO_SMALL = -2, // Output buffer too small
    BETA_PROTOC_ERR_INVALID_ID = -3, // Message ID does not match the struct
    BETA_PROTOC_ERR_INVALID_PROTOC_VERSION = -4, // Protoc version does not match
    BETA_PROTOC_VALUE_EXCEEDS_ARCH_LIMIT = -5, // Value exceeds architecture limits (e.g., varint too large for 32 bits size_t)
    BETA_PROTOC_ERR_INVALID_DATA = -6, // General data error
    BETA_PROTOC_ERR_ARRAY_SIZE_EXCEEDED = -7, // Array size exceeded for fixed-size arrays
    BETA_PROTOC_ERR_NULL_ARRAY_POINTER = -8 // NULL pointer passed for an array field
} beta_protoc_err_t;

uint32_t zigzag_encode_32(int32_t value);
int32_t zigzag_decode_32(uint32_t value);
uint64_t zigzag_encode_64(int64_t value);
int64_t zigzag_decode_64(uint64_t value);

size_t safe_strlen(const char *str, size_t max_len);

size_t int8_size(int8_t data);
size_t int16_size(int16_t data);
size_t uint8_size(uint8_t data);
size_t uint16_size(uint16_t data);

size_t int32_size(int32_t data);
size_t int64_size(int64_t data);
size_t uint32_size(uint32_t data);
size_t uint64_size(uint64_t data);

size_t float32_size(float data);
size_t float64_size(double data);

size_t char_size(char data);
size_t bool_size(bool data);

beta_protoc_err_t varint_to_buff(uint64_t data, uint8_t **buff, size_t *rem_buff);
beta_protoc_err_t varint_from_buff(uint64_t *data, uint8_t **buff, size_t *rem_buff);
size_t varint_size(uint64_t data);

beta_protoc_err_t int8_to_buff(int8_t data, uint8_t **buff, size_t *rem_buff);
beta_protoc_err_t int16_to_buff(int16_t data, uint8_t **buff, size_t *rem_buff);
beta_protoc_err_t int32_to_buff(int32_t data, uint8_t **buff, size_t *rem_buff);
beta_protoc_err_t int64_to_buff(int64_t data, uint8_t **buff, size_t *rem_buff);

beta_protoc_err_t uint8_to_buff(uint8_t data, uint8_t **buff, size_t *rem_buff);
beta_protoc_err_t uint16_to_buff(uint16_t data, uint8_t **buff, size_t *rem_buff);
beta_protoc_err_t uint32_to_buff(uint32_t data, uint8_t **buff, size_t *rem_buff);
beta_protoc_err_t uint64_to_buff(uint64_t data, uint8_t **buff, size_t *rem_buff);

beta_protoc_err_t float32_to_buff(float data, uint8_t **buff, size_t *rem_buff);
beta_protoc_err_t float64_to_buff(double data, uint8_t **buff, size_t *rem_buff);

beta_protoc_err_t string_to_buff(const char *data, size_t data_len, uint8_t **buff, size_t *rem_buff);
beta_protoc_err_t char_to_buff(char data, uint8_t **buff, size_t *rem_buff);
beta_protoc_err_t bool_to_buff(bool data, uint8_t **buff, size_t *rem_buff);

beta_protoc_err_t int8_from_buff(int8_t *data, uint8_t **buff, size_t *rem_buff);
beta_protoc_err_t int16_from_buff(int16_t *data, uint8_t **buff, size_t *rem_buff);
beta_protoc_err_t int32_from_buff(int32_t *data, uint8_t **buff, size_t *rem_buff);
beta_protoc_err_t int64_from_buff(int64_t *data, uint8_t **buff, size_t *rem_buff);

beta_protoc_err_t uint8_from_buff(uint8_t *data, uint8_t **buff, size_t *rem_buff);
beta_protoc_err_t uint16_from_buff(uint16_t *data, uint8_t **buff, size_t *rem_buff);
beta_protoc_err_t uint32_from_buff(uint32_t *data, uint8_t **buff, size_t *rem_buff);
beta_protoc_err_t uint64_from_buff(uint64_t *data, uint8_t **buff, size_t *rem_buff);

beta_protoc_err_t float32_from_buff(float *data, uint8_t **buff, size_t *rem_buff);
beta_protoc_err_t float64_from_buff(double *data, uint8_t **buff, size_t *rem_buff);

beta_protoc_err_t bool_from_buff(bool *data, uint8_t **buff, size_t *rem_buff);
beta_protoc_err_t char_from_buff(char *data, uint8_t **buff, size_t *rem_buff);
beta_protoc_err_t string_from_buff(char *data, size_t data_len, uint8_t **buff, size_t *rem_buff);

#ifdef __cplusplus
}
#endif

#endif //BETA_PROTOC_H