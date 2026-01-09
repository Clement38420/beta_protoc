#ifndef HELLO_MSG_H
#define HELLO_MSG_H

#include <stdint.h>
#include <stdbool.h>

#include "beta_com.h"

typedef struct {
    uint32_t sender;
    char* message;
} Hello;

#endif