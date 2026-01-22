#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <stdint.h>
#include "beta_protoc.h"

// Include all message headers
#include "SensorData.h"
#include "Value.h"

#ifdef __cplusplus
extern "C" {
#endif

// Error codes for the dispatcher
typedef enum {
    DISPATCHER_SUCCESS = 0,
    DISPATCHER_ERR_INVALID_DATA = -100,
    DISPATCHER_ERR_INVALID_PROTOC_VERSION = -101,
    DISPATCHER_ERR_UNKNOWN_MESSAGE_ID = -102,
} dispatcher_err_t;

// Weak callback function declarations to be implemented by the user
/**
 * @brief Weak callback function to be implemented by the user.
 *
 * This function is called by the dispatcher when a SensorData message is
 * successfully received and deserialized.
 *
 * @param msg Pointer to the deserialized SensorData message struct.
 * @param ctx Pointer to user-defined context (if needed).
 */
void on_sensor_data_received(SensorData *msg, void *ctx) __attribute__((weak));
/**
 * @brief Weak callback function to be implemented by the user.
 *
 * This function is called by the dispatcher when a Value message is
 * successfully received and deserialized.
 *
 * @param msg Pointer to the deserialized Value message struct.
 * @param ctx Pointer to user-defined context (if needed).
 */
void on_value_received(Value *msg, void *ctx) __attribute__((weak));

/**
 * @brief Dispatches an incoming binary message.
 *
 * It reads the message header, finds the corresponding message type,
 * deserializes it, and calls the appropriate `on_<MessageName>_received` callback if it exists.
 *
 * @param buff Double pointer to the buffer containing the binary message.
 *             The pointer is advanced past the processed message.
 * @param rem_buff Pointer to the remaining buffer size.
 *                 The value is decremented by the size of the processed message.
 * @param ctx Pointer to user-defined context, which will be transmitted to callbacks (if needed).
 * @return DISPATCHER_SUCCESS on success, or an error code on failure.
 */
int protoc_dispatch(uint8_t **buff, size_t *rem_buff, void *ctx);

#ifdef __cplusplus
}
#endif

#endif // DISPATCHER_H