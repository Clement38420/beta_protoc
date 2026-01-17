# Beta Protoc Compiler

**Beta Protoc Compiler** is a command-line interface (CLI) tool designed to generate serialization code and data structures from JSON message definitions. It is specifically built to facilitate serial communication along with the `beta_com` library for framing.

## Features

*   **Optimized Binary Protocol:** Employs Varint and ZigZag encoding for efficient, language-agnostic serialization.
*   **Versatile Array Support:** Natively handles static and dynamic arrays with optimized serialization for primitive types.
*   **JSON Schema Definition:** Uses a clear, human-readable JSON format for defining message structures.
*   **Robust Compile-Time Validation:** Rigorously validates schemas before code generation to prevent runtime errors.
*   **Automatic Dependency Resolution:** Automatically manages dependencies between nested messages.
*   **Modular & Extensible:** A template-based architecture (Jinja2) allows for easy addition of new target languages.
*   **Optional C Dispatcher:** Generates a dispatcher in C for simplified message routing and handling.

## Installation

This project is packaged using `pyproject.toml`. You can install it locally using `pip`:

```bash
# From the project root
pip install .
```

For development or to modify the source code, install it in editable mode:

```bash
pip install -e .
```

## Usage

Once installed, the `beta_protoc_compiler` command is available in your terminal.

### Basic Command

```bash
beta_protoc_compiler path/to/your_schema.json
```

### Options

| Argument | Description | Default                 |
| --- | --- |-------------------------|
| `filepath` | The path to the input JSON file containing message definitions. | *(Required)*            |
| `-l`, `--lang` | The output language(s) for the generated files. Can be one or more. | All supported languages |
| `-o`, `--out` | The directory where the generated files will be saved. | `./generated`           |
| `--clean` | Deletes the output directory before generating new files. | `False`                 |

**Example:**

```bash
beta_protoc_compiler my_protocol.json -l c -o ./src/protocol --clean
```

## Schema Format (JSON)

The input file must follow a specific JSON structure defining a list of messages.

### Global Structure

```json
{
  "messages": [
    {
      "name": "MessageName",
      "id": 1,
      "fields": [
        {
          "name": "field_name",
          "id": 1,
          "type": "FieldType"
        }
      ]
    }
  ]
}
```

### Supported Data Types

You can use the following primitive types or the name of another message defined in your JSON file.

| Category              | Types |
|-----------------------| --- |
| **Unsigned Integers** | `uint8`, `uint16`, `uint32`, `uint64` |
| **Signed Integers**   | `int8`, `int16`, `int32`, `int64` |
| **Floating Point**    | `float32`, `float64` |
| **Other**             | `char`, `bool` |

Additionally, both **static** and **dynamic** arrays are supported for any data type (including nested messages).

*   **Static array:** `type[SIZE]`, for example `int32[10]` for an array of 10 integers.
*   **Dynamic array:** `type[]`, for example `float32[]`.


### Key Considerations

1. **Naming Conventions:** Message and field names must start with a letter and can only contain letters, digits, or underscores (`_`). In addition, CamelCase is advised to get the right conversion for all languages.
2. **Unique IDs:** Message IDs must be unique across all messages. Field IDs must be unique within a single message.
3. **Dependencies:** If message `A` is used as a field type inside message `B`, message `A` must be defined within the `messages` list. The compiler will automatically generate the required dependencies (e.g., `#include "A.h"`).
4. **Order:** The order in which messages are defined in the JSON file does not matter; the compiler resolves dependencies automatically.

## Binary Protocol Specification

The compiler generates code that adheres to a simple, efficient, and language-agnostic binary protocol. The following sections describe the structure and encoding rules of this protocol.

### Binary Message Format

The serializer converts a data structure into a binary message. It's the user's responsibility to handle message framing (e.g., adding start/end bytes) if the communication channel requires it.

All multi-byte integers are encoded in **little-endian** format.

**Message Format:**
`[PROTOCOL_VERSION, MESSAGE_ID, MESSAGE_LEN, PAYLOAD...]`

**Overall Message Structure:**

| Field              | Description                                          | Size                |
|--------------------|------------------------------------------------------|---------------------|
| `PROTOCOL_VERSION` | The version of the serialization protocol.           | 1 byte              |
| `MESSAGE_ID`       | The unique identifier for the message (from JSON).   | 2 bytes             |
| `MESSAGE_LEN`      | The length of the `PAYLOAD` in bytes.                | Varint (1-10 bytes)  |
| `PAYLOAD`          | The serialized data fields.                          | `MESSAGE_LEN` bytes |

**Payload Field Structure:**

The `PAYLOAD` consists of a sequence of fields, where each field is encoded as follows:

`[FIELD_ID, FIELD_LEN, FIELD_VALUE...]`

| Field        | Description                                  | Size                |
|--------------|----------------------------------------------|---------------------|
| `FIELD_ID`   | The unique identifier for the field (from JSON). | Varint (1-10 bytes) |
| `FIELD_LEN`  | The length of the `FIELD_VALUE` in bytes.    | Varint (1-10 bytes) |
| `FIELD_VALUE`| The binary value of the field.               | `FIELD_LEN` bytes   |

### Data Type Encoding

The way data types are serialized into `FIELD_VALUE` depends on their type:

*   **Integers (8-bit and 16-bit):**
    *   `int8`, `uint8`, `int16`, `uint16` are written directly in little-endian format. They are not converted to varints, as the overhead would negate any potential space savings. For 16-bit integers, the average size is roughly equivalent, and using varints would add computational cost for minimal gain.

*   **Integers (32-bit and 64-bit):**
    *   `uint32`, `uint64`: Encoded as standard **varints**. This is efficient for small, non-negative numbers.
    *   `int32`, `int64`: Encoded using **ZigZag** encoding first, and then the result is encoded as a **varint**. ZigZag re-maps signed integers to unsigned integers so that small negative numbers (like -1) are encoded as small varints, which is highly efficient.

*   **Floating-Point Numbers:**
    *   `float32`, `float64`: Written directly in IEEE 754 binary format (little-endian).

*   **Other Types:**
    *   `bool`: Encoded as a single byte (`0x00` for `false`, `0x01` for `true`).
    *   `char`: Encoded as a single byte.
    *   **Nested Messages:** The field value is the serialized sub-message itself (following the same `[PROTOCOL_VERSION, MESSAGE_ID, ...]` structure).
    *   **Arrays:** The encoding of arrays depends on the type of their elements.
        *   **Arrays of Nested Messages:** For arrays of complex types (other messages), each element is serialized as a separate `[FIELD_ID, FIELD_LEN, FIELD_VALUE]` block. This allows for lists of different-sized objects.
        *   **Arrays of Primitive Types (Optimization):** For arrays of primitive types (e.g., `int32`, `float32`), a significant optimization is applied. The entire array is treated as a single field. The `FIELD_ID` is written once, followed by a `FIELD_LEN` that represents the total byte size of *all elements combined*. The `FIELD_VALUE` then consists of the raw, concatenated values of the array elements. This reduces overhead by removing the need for repeated ID and length tags for each element. For example, an array of 10 `uint32` integers will be encoded as one field, not ten.

To create a null-terminated string, you can use an array of `char` (e.g., `char[64]`). The deserializer will automatically add a null terminator `\0` at the end of the data. Furthermore, during serialization, if a `\0` character is found before the end of the array's specified size, the serialization will stop at that point, saving space in the final message.

### ID and Size Limitations

*   **Message ID:** The `MESSAGE_ID` is a 2-byte integer, allowing for up to **65,536 unique messages**.
*   **Field ID:** The `FIELD_ID` is encoded as a variable-length integer (varint), allowing for up to **2^64 unique fields** (depending on the architecture) per message.
*   **Message and Field Size:** The `MESSAGE_LEN` and `FIELD_LEN` are also encoded as varints, allowing for payloads and fields a theoretical **maximum length of 2^64 bytes** (depending on the architecture).

### Deserialization Process

The deserialization process performs the reverse operation of serialization:
1.  It reads the message header (`PROTOCOL_VERSION`, `MESSAGE_ID`, `MESSAGE_LEN`) to validate and identify the message.
2.  It iterates through the `PAYLOAD`, reading each field's header (`FIELD_ID`, `FIELD_LEN`).
3.  It extracts the `FIELD_VALUE` and populates the corresponding member of the data structure.
4.  Multi-byte values are converted from little-endian back to the host's native byte order.

## Supported Languages

Currently, the compiler supports:

* **C** (`.c`, `.h`): Generates structs and dependent message headers.

## Language: C

This section describes how to use the C code generated by the compiler.

### Array Handling in C

The way arrays are handled in the generated C code depends on whether they are static or dynamic.

#### Static Arrays

A field defined with a fixed size, like `"type": "uint8[16]"` in JSON, will be generated in the C struct as follows:

```c
// In your message struct:
uint8_t my_field[16];
size_t my_field_count;
```

*   `my_field[16]`: A standard C array with the specified size.
*   `my_field_count`: A `size_t` variable indicating how many elements are actually in use. When serializing, the compiler will only write `my_field_count` elements. When deserializing, this field will be populated with the number of elements read from the buffer.

#### Dynamic Arrays

A field defined as a dynamic array, like `"type": "MyMessage[]"`, requires you to manage the memory. The generated C struct will contain:

```c
// In your message struct:
MyMessage* my_field;
size_t my_field_count;
size_t my_field_max_count;
```

*   `my_field`: A pointer to a block of memory that you must allocate.
*   `my_field_count`: The number of elements currently stored in the allocated memory.
*   `my_field_max_count`: The total capacity of the allocated memory block (i.e., the maximum number of elements it can hold).

Before serializing or deserializing, you are responsible for allocating the memory for the dynamic array and setting `my_field_max_count` to the capacity of your buffer. The compiler will check that `my_field_count` does not exceed `my_field_max_count` to prevent buffer overflows.

### Project Integration

Once you have generated the code from your JSON schema, you can use it to create, serialize, and deserialize messages.

#### Generated File Structure

For each message (e.g., `MyMessage`), the following files are created:

*   `MyMessage.h`: The header file defining the data structure and function prototypes.
*   `MyMessage.c`: The implementation of the serialization and deserialization functions.

#### Common Code Dependency

The generated C code has an external dependency that must be included in your project's build system (e.g., `CMakeLists.txt`) to compile correctly.

1.  **`beta_protoc` Common code:**
    The core serialization helpers are located in the `protoc_common_code/C/` directory of this repository. You must add `beta_protoc.c` and `beta_protoc.h` to your project.

### The Dispatcher (Optional)

To simplify message handling, the compiler also generates a `dispatcher`. It is a set of files (`dispatcher.c` and `dispatcher.h`) that can automatically read an incoming byte stream, identify a message, deserialize it, and call a user-defined callback function.

**Features:**

*   **Automatic Message Identification:** Reads the message header and determines the message type.
*   **Callback System:** For each message `MyMessage`, it calls a weak function `on_MyMessage_received(MyMessage *msg)` that you can implement in your application.
*   **Stream-Safe:** The dispatcher can be fed bytes one by one or in chunks, and it will find messages in the stream.

To use it, include `dispatcher.h` in your project and implement the `on_<MessageName>_received` functions for the messages you want to handle. Then, feed your incoming data stream to the `protoc_dispatch` function.

### Generated Functions

For each message, the following functions are generated to facilitate serialization and deserialization:

1.  **`struct <MessageName>`**
    The C struct representing your message. You must first populate this struct with the data you want to send, or use it to receive deserialized data.

    ```c
    // Example for a "Position" message
    Position my_pos;
    my_pos.x = 12.34;
    my_pos.y = -56.78;
    ```

2.  **`int get_<MessageName>_size(<MessageName> data, size_t *size)`**
    Calculates the total size in bytes that the message *payload* will occupy once serialized.

3.  **`int <MessageName>_to_buff(<MessageName> data, uint8_t **buff, size_t *buff_len)`**
    Serializes only the *payload* (the fields) of the struct into a provided buffer. This function is mainly used internally by `<MessageName>_to_message`.

4.  **`int <MessageName>_to_message(<MessageName> data, uint8_t **buff, size_t *buff_len)`**
    This is the main function to use for serialization. It takes the populated struct and serializes it into a binary message format (header + payload).

5.  **`int <MessageName>_from_buff(<MessageName> *data, uint8_t **buff, size_t *rem_buff)`**
    Deserializes the *payload* from a buffer and populates the provided struct. This function is mainly used internally by `<MessageName>_from_message`.

6.  **`int <MessageName>_from_message(<MessageName> *data, uint8_t **buff, size_t *rem_buff)`**
    This is the main function to use for deserialization. It takes a buffer containing a binary message, validates the header, and deserializes the payload into the provided struct.

### Complete Example

Here is an example demonstrating serialization and deserialization using the dispatcher.

```c
#include "Position.h"
#include "dispatcher.h"
#include <stdio.h>

// --- User-defined callback ---
// This function is called by the dispatcher when a Position message is received.
void on_position_received(Position *msg) {
    printf("Callback triggered!\n");
    printf("Received Position: x=%.2f, y=%.2f\n", msg->x, msg->y);
}

int main() {
    // --- Serialization ---

    // 1. Populate the message struct
    Position my_pos;
    my_pos.x = 10.5f;
    my_pos.y = -2.3f;

    // 2. Prepare a buffer to receive the encoded message
    uint8_t output_buffer[256];
    uint8_t *p_buffer = output_buffer;
    size_t buffer_len = sizeof(output_buffer);

    // 3. Serialize the message
    int result = position_to_message(my_pos, &p_buffer, &buffer_len);

    if (result == 0) {
        size_t message_size = sizeof(output_buffer) - buffer_len;
        printf("Message encoded successfully (%zu bytes)!\n", message_size);

        // The 'output_buffer' now contains the binary message.
        // In a real application, you would receive this data from a serial port, socket, etc.

        // --- Deserialization with Dispatcher ---

        // 4. Feed the buffer to the dispatcher
        uint8_t *p_read_buffer = output_buffer;
        size_t read_buffer_len = message_size;
        
        while(read_buffer_len > 0) {
            int dispatch_result = protoc_dispatch(&p_read_buffer, &read_buffer_len);
            if (dispatch_result == DISPATCHER_SUCCESS) {
                printf("Dispatcher found and processed a message.\n");
            }
            // The dispatcher advances the buffer pointer automatically.
        }

    } else {
        fprintf(stderr, "Error encoding message: %d\n", result);
    }

    return 0;
}
```

## Adding a New Language

The architecture is modular. To add support for a new language (e.g., Python, C++), follow these two steps:

### 1. Define the Language

Open `compiler/core/language.py` and add an entry to the `SUPPORTED_LANGUAGES` list. You must provide:
- The mapping between internal types (`DataType`) and the target language types.
- The naming convention (`case`) to use for generating names (e.g., function names like `read_<message_name>`).

```python
# Example for Python
from compiler.core.language import Language, Case
from compiler.common.data_types import DataType

Language(
    name="Python",
    case=Case.SNAKE,  # Use snake_case for names (e.g., my_function)
    src_ext="py",
    header_ext=None,  # Python does not use header files
    types_mapping={
        DataType.UINT8: "",
        # ... define mappings for all DataType enum members
    }
)
```

### 2. Create Templates

Create a directory with the **exact name** of the language in `compiler/templates/` (e.g., `compiler/templates/Python/`).
Add the Jinja2 templates corresponding to the extensions defined in the `Language` object:

* `message.py.j2` (if `src_ext="py"`)

The compiler will automatically locate and use these templates during generation.
