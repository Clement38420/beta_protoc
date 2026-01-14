# Beta Protoc Compiler

**Beta Protoc Compiler** is a command-line interface (CLI) tool designed to generate serialization code and data structures from JSON message definitions. It is specifically built to facilitate serial communication along with the `beta_com` library for framing.

## Features

* **Simple Definition:** Messages are described in a clear, human-readable JSON format.
* **Strong Validation:** Types, names, and structures are rigorously validated before code generation to prevent runtime errors.
* **Multi-language Support:** The tool currently supports C and is designed with a modular architecture to easily add other languages.
* **Dependency Management:** Automatically detects when a message uses another message as a field type and handles the necessary imports or includes.

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

| Argument | Description | Default |
| --- | --- | --- |
| `filepath` | The path to the input JSON file containing message definitions. | *(Required)* |
| `-o`, `--out` | The directory where the generated files will be saved. | `./generated` |
| `--clean` | Deletes the output directory before generating new files. | `False` |

**Example:**

```bash
beta_protoc_compiler my_protocol.json -o ./src/protocol --clean
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

| Category | Types |
| --- | --- |
| **Unsigned Integers** | `uint8`, `uint16`, `uint32`, `uint64` |
| **Signed Integers** | `int8`, `int16`, `int32`, `int64` |
| **Floating Point** | `float32`, `float64` |
| **Text / Logic** | `char`, `string[SIZE]`, `bool` |

`SIZE` must be an integer specifying the maximum length of the string.

### Key Considerations

1. **Naming Conventions:** Message and field names must start with a letter and can only contain letters, digits, or underscores (`_`). In addition, CamelCase is advised to get the right conversion for all languages.
2. **Unique IDs:** Message IDs must be unique across all messages. Field IDs must be unique within a single message.
3. **Dependencies:** If message `A` is used as a field type inside message `B`, message `A` must be defined within the `messages` list. The compiler will automatically generate the required dependencies (e.g., `#include "A.h"`).
4. **Order:** The order in which messages are defined in the JSON file does not matter; the compiler resolves dependencies automatically.

## Supported Languages

Currently, the compiler supports:

* **C** (`.c`, `.h`): Generates structs and dependent message headers.

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
        DataType.UINT8: "int",
        DataType.STRING: "str",
        # ... define mappings for all DataType enum members
    }
)
```

### 2. Create Templates

Create a directory with the **exact name** of the language in `compiler/templates/` (e.g., `compiler/templates/Python/`).
Add the Jinja2 templates corresponding to the extensions defined in the `Language` object:

* `message.py.j2` (if `src_ext="py"`)

The compiler will automatically locate and use these templates during generation.

## Example

**Input (`msg.json`):**

```json
{
  "messages": [
    {
      "name": "Position",
      "id": 1,
      "fields": [
        { "name": "x", "id": 1, "type": "float32" },
        { "name": "y", "id": 2, "type": "float32" }
      ]
    },
    {
      "name": "RobotStatus",
      "id": 2,
      "fields": [
        { "name": "id", "id": 1, "type": "uint8" },
        { "name": "pos", "id": 2, "type": "Position" }
      ]
    }
  ]
}
```

**Command:**

```bash
beta_protoc_compiler msg.json
```

**Output (C):**

The compiler will generate `Position.h`, `Position.c`, `RobotStatus.h` (which includes `Position.h`), and `RobotStatus.c`.

## Using the Generated Code (C Example)

Once you have generated the code from your JSON schema, you can use it to create, serialize, and deserialize messages.

### Generated File Structure

For each message (e.g., `MyMessage`), the following files are created:

*   `MyMessage.h`: The header file defining the data structure and function prototypes.
*   `MyMessage.c`: The implementation of the serialization and deserialization functions.

### Project Integration

The generated C code has an external dependency that must be included in your project's build system (e.g., `CMakeLists.txt`) to compile correctly.

1.  **`beta_protoc` Common code:**
    The core serialization helpers are located in the `protoc_common_code/C/` directory of this repository. You must add `beta_protoc.c` and `beta_protoc.h` to your project.

### Binary Message Format

The serializer converts a C struct into a binary message. It's the user's responsibility to handle message framing (e.g., adding start/end bytes) if the communication channel requires it.

All multi-byte integers are encoded in **little-endian** format.

**Message Format:**
`[PROTOCOL_VERSION, MESSAGE_ID, MESSAGE_LEN, PAYLOAD...]`

**Overall Message Structure:**

| Field              | Description                                          | Size                |
|--------------------|------------------------------------------------------|---------------------|
| `PROTOCOL_VERSION` | The version of the serialization protocol.           | 1 byte              |
| `MESSAGE_ID`       | The unique identifier for the message (from JSON).   | 1 byte              |
| `MESSAGE_LEN`      | The length of the `PAYLOAD` in bytes.                | 1 byte              |
| `PAYLOAD`          | The serialized data fields.                          | `MESSAGE_LEN` bytes |

**Payload Field Structure:**

The `PAYLOAD` consists of a sequence of fields, where each field is encoded as follows:

`[FIELD_ID, FIELD_LEN, FIELD_VALUE...]`

| Field        | Description                                  | Size             |
|--------------|----------------------------------------------|------------------|
| `FIELD_ID`   | The unique identifier for the field (from JSON). | 1 byte           |
| `FIELD_LEN`  | The length of the `FIELD_VALUE` in bytes.    | 1 byte           |
| `FIELD_VALUE`| The binary value of the field.               | `FIELD_LEN` bytes|

### ID and Size Limitations

*   **Message ID:** The `MESSAGE_ID` is currently a 1-byte integer, limiting the protocol to a maximum of **256 unique messages**.
*   **Field ID:** Similarly, the `FIELD_ID` within each message is a 1-byte integer, allowing for a maximum of **256 unique fields per message**.

These limitations are planned to be addressed in future updates:
- The `MESSAGE_ID` will be expanded to 2 bytes, increasing the total number of possible messages to 65,536.
- The `FIELD_ID` will be encoded using varints, removing the 256-field limit and optimizing space for smaller IDs.

### From Binary to Struct

The deserialization process, handled by the `_from_message` functions, performs the reverse operation:
1.  It reads the message header (`PROTOCOL_VERSION`, `MESSAGE_ID`, `MESSAGE_LEN`) to validate the message.
2.  It iterates through the `PAYLOAD`, reading each field's header (`FIELD_ID`, `FIELD_LEN`).
3.  It extracts the `FIELD_VALUE` and copies it into the corresponding member of the C struct.
4.  Multi-byte values are converted from little-endian back to the host's byte order.

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
void on_Position_received(Position *msg) {
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
    int result = Position_to_message(&my_pos, &p_buffer, &buffer_len);

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
            dispatcher_err_t dispatch_result = protoc_dispatch(&p_read_buffer, &read_buffer_len);
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


## Future Improvements

This compiler is under active development. Here are some ideas for future enhancements:

### Array Support

Handling arrays is a common need in communication protocols. Here’s how it could be implemented:

*   **Fixed-size arrays:** For fields that are always of the same length (e.g., a `float32[3]` for a 3D vector).
    *   **Schema:** Could be defined in JSON as `"type": "float32[3]"`.
    *   **Generated Code (C):** Would generate a simple array in the struct (e.g., `float x[3];`). Serialization would involve iterating through the array.

*   **Dynamic arrays:** For fields of variable length (e.g., a list of sensor readings).
    *   **Schema:** Could be defined as `"type": "uint16[]"`.
    *   **Generated Code (C):** This is more complex. It would require a pointer and a size field in the struct (e.g., `uint16_t* values; size_t num_values;`).
    *   **Serialization:** The binary format would need to encode the number of elements followed by the elements themselves. This would require careful memory management during deserialization (e.g., using `malloc`).

### Other Potential Features

*   **Enumerations (Enums):** Add support for defining enums in the JSON schema to create named constants, which improves code readability.
*   **Field Constraints:** Allow specifying constraints in the JSON schema (e.g., min/max values for numbers, max length for strings) and generate validation code.
*   **More Languages:** Add support for other popular languages like Python or C++.
*   **Automated Build System Integration:** Generate `CMake` or `Makefile` snippets to simplify the integration of the generated code into larger projects.
*   **Oneof / Union Support:** Implement a `oneof` keyword to allow a message to contain one of a set of fields, which is useful for saving memory when only one field is used at a time.

