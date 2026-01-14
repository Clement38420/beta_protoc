#ifndef BETA_PROTOC_H
#define BETA_PROTOC_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define PROTOC_VERSION 1
#define MESSAGE_HEADER_SIZE 3 // 1 byte for protocol version, 1 byte for message ID, 1 byte for message length
#define STRING_MAX_SIZE 32 // Array allocation for strings

typedef enum {
    BETA_PROTOC_SUCCESS = 0, // Operation successful
    BETA_PROTOC_ERR_INVALID_ARGS = -10, // NULL pointers passed as parameters
    BETA_PROTOC_ERR_BUFFER_TOO_SMALL = -11, // Output or work buffer too small
    BETA_PROTOC_ERR_INVALID_ID = -12, // Message ID does not match the struct
    BETA_PROTOC_ERR_INVALID_PROTOC_VERSION = -13, // Protoc version does not match
    BETA_PROTOC_ERR_INVALID_DATA = -14, // General data error
} beta_protoc_err_t;

beta_protoc_err_t int8_to_buff(int8_t data, uint8_t **buff, size_t *buff_len);
beta_protoc_err_t int16_to_buff(int16_t data, uint8_t **buff, size_t *buff_len);
beta_protoc_err_t int32_to_buff(int32_t data, uint8_t **buff, size_t *buff_len);
beta_protoc_err_t int64_to_buff(int64_t data, uint8_t **buff, size_t *buff_len);

beta_protoc_err_t int8_to_buff(int8_t data, uint8_t **buff, size_t *rem_buff);
beta_protoc_err_t int16_to_buff(int16_t data, uint8_t **buff, size_t *rem_buff);
beta_protoc_err_t int32_to_buff(int32_t data, uint8_t **buff, size_t *rem_buff);
beta_protoc_err_t int64_to_buff(int64_t data, uint8_t **buff, size_t *rem_buff);

beta_protoc_err_t uint8_to_buff(uint8_t data, uint8_t **buff, size_t *rem_buff);
beta_protoc_err_t uint16_to_buff(uint16_t data, uint8_t **buff, size_t *rem_buff);
beta_protoc_err_t uint32_to_buff(uint32_t data, uint8_t **buff, size_t *rem_buff);
beta_protoc_err_t uint64_to_buff(uint64_t data, uint8_t **buff, size_t *rem_buff);

beta_protoc_err_t float_to_buff(float data, uint8_t **buff, size_t *rem_buff);
beta_protoc_err_t double_to_buff(double data, uint8_t **buff, size_t *rem_buff);

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

beta_protoc_err_t float_from_buff(float *data, uint8_t **buff, size_t *rem_buff);
beta_protoc_err_t double_from_buff(double *data, uint8_t **buff, size_t *rem_buff);

beta_protoc_err_t bool_from_buff(bool *data, uint8_t **buff, size_t *rem_buff);
beta_protoc_err_t char_from_buff(char *data, uint8_t **buff, size_t *rem_buff);
beta_protoc_err_t string_from_buff(char *data, size_t data_len, uint8_t **buff, size_t *rem_buff);

#endif //BETA_PROTOC_H