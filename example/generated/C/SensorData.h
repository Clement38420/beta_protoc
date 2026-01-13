#ifndef SENSORDATA_MSG_H
#define SENSORDATA_MSG_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "beta_protoc.h"
#include "Value.h"

typedef struct {
    uint32_t id;
    char name[32];
    Value value;
} SensorData;

/**
 * @brief Calculates the serialized size of the SensorData message payload.
 * @param data Pointer to the struct to measure.
 * @param size Pointer to a size_t variable to store the calculated size.
 * @return 0 on success, error code otherwise.
 */
int get_sensor_data_size(const SensorData *data, size_t *size);

/**
 * @brief Serializes the SensorData message payload into a buffer.
 * @param data Pointer to the struct to serialize.
 * @param buff Double pointer to the buffer where the payload will be written. The pointer is advanced by the number of bytes written.
 * @param rem_buff Pointer to the remaining buffer size. The value is decremented by the number of bytes written.
 * @return 0 on success, error code otherwise.
 */
int sensor_data_to_buff(const SensorData *data, uint8_t **buff, size_t *rem_buff);

/**
 * @brief Serializes the SensorData message into a complete binary message (header + payload).
 * @param data Pointer to the struct to serialize.
 * @param buff Double pointer to the buffer where the message will be written. The pointer is advanced by the number of bytes written.
 * @param rem_buff Pointer to the remaining buffer size. The value is decremented by the number of bytes written.
 * @return 0 on success, error code otherwise.
 */
int sensor_data_to_message(const SensorData *data, uint8_t **buff, size_t *rem_buff);

/**
 * @brief Deserializes the payload of a SensorData message from a buffer into a struct.
 * @param data Pointer to the struct to populate.
 * @param buff Double pointer to the buffer from which to read the payload. The pointer is advanced by the number of bytes read.
 * @param rem_buff Pointer to the remaining buffer size. The value is decremented by the number of bytes read.
 * @return 0 on success, error code otherwise.
 */
int sensor_data_from_buff(SensorData *data, uint8_t **buff, size_t *rem_buff);

/**
 * @brief Deserializes a complete binary message (header + payload) into a SensorData struct.
 * @param data Pointer to the struct to populate.
 * @param buff Double pointer to the buffer from which to read the message. The pointer is advanced by the number of bytes read.
 * @param rem_buff Pointer to the remaining buffer size. The value is decremented by the number of bytes read.
 * @return 0 on success, error code otherwise.
 */
int sensor_data_from_message(SensorData *data, uint8_t **buff, size_t *rem_buff);

#endif // SENSORDATA_MSG_H