#ifndef VALUE_MSG_H
#define VALUE_MSG_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "beta_protoc.h"

typedef struct {
    uint32_t value;
    char unit[32];
} Value;

/**
 * @brief Calculates the serialized size of the Value message payload.
 * @param data Pointer to the struct to measure.
 * @param size Pointer to a size_t variable to store the calculated size.
 * @return 0 on success, error code otherwise.
 */
int get_value_size(const Value *data, size_t *size);

/**
 * @brief Serializes the Value message payload into a buffer.
 * @param data Pointer to the struct to serialize.
 * @param buff Double pointer to the buffer where the payload will be written. The pointer is advanced by the number of bytes written.
 * @param rem_buff Pointer to the remaining buffer size. The value is decremented by the number of bytes written.
 * @return 0 on success, error code otherwise.
 */
int value_to_buff(const Value *data, uint8_t **buff, size_t *rem_buff);

/**
 * @brief Serializes the Value message into a complete binary message (header + payload).
 * @param data Pointer to the struct to serialize.
 * @param buff Double pointer to the buffer where the message will be written. The pointer is advanced by the number of bytes written.
 * @param rem_buff Pointer to the remaining buffer size. The value is decremented by the number of bytes written.
 * @return 0 on success, error code otherwise.
 */
int value_to_message(const Value *data, uint8_t **buff, size_t *rem_buff);

/**
 * @brief Deserializes the payload of a Value message from a buffer into a struct.
 * @param data Pointer to the struct to populate.
 * @param buff Double pointer to the buffer from which to read the payload. The pointer is advanced by the number of bytes read.
 * @param rem_buff Pointer to the remaining buffer size. The value is decremented by the number of bytes read.
 * @return 0 on success, error code otherwise.
 */
int value_from_buff(Value *data, uint8_t **buff, size_t *rem_buff);

/**
 * @brief Deserializes a complete binary message (header + payload) into a Value struct.
 * @param data Pointer to the struct to populate.
 * @param buff Double pointer to the buffer from which to read the message. The pointer is advanced by the number of bytes read.
 * @param rem_buff Pointer to the remaining buffer size. The value is decremented by the number of bytes read.
 * @return 0 on success, error code otherwise.
 */
int value_from_message(Value *data, uint8_t **buff, size_t *rem_buff);

#endif // VALUE_MSG_H