# Beta Protoc Compiler

**Beta Protoc Compiler** is a command-line interface (CLI) tool designed to generate serialization code and data structures from JSON message definitions. It is specifically built to facilitate serial communication, particularly when working with the `beta_com` library.

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
| **Text / Logic** | `char`, `string`, `bool` |

### Key Considerations

1. **Naming Conventions:** Message and field names must start with a letter and can only contain letters, digits, or underscores (`_`). In addition, CamelCase is advised to get the right conversion for all languages.
2. **Unique IDs:** Message IDs must be unique across all messages. Field IDs must be unique within a single message.
3. **Dependencies:** If message `A` is used as a field type inside message `B`, message `A` must be defined within the `messages` list. The compiler will automatically generate the required dependencies (e.g., `#include "A.h"`).
4. **Order:** The order in which messages are defined in the JSON file does not matter; the compiler resolves dependencies automatically.

## Supported Languages

Currently, the compiler supports:

* **C** (`.c`, `.h`): Generates structs and includes `beta_com.h` and dependent message headers.

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

Once you have generated the code from your JSON schema, you can use it to create and serialize messages. The generated code relies on an underlying communication library (`beta_com`) for low-level protocol handling.

### Generated File Structure

For each message (e.g., `MyMessage`), the following files are created:

*   `MyMessage.h`: The header file defining the data structure and function prototypes.
*   `MyMessage.c`: The implementation of the serialization functions.

### Binary Message Format

The goal is to convert a C struct into a binary message ready to be sent over a serial port or another communication channel. The final format is as follows (all integers are **little-endian**):

```
[COBS_POINTER [FRAME] 0x00]
```

Where `FRAME` consists of:

| Field              | Description                                                                                              | Size                |
|--------------------|----------------------------------------------------------------------------------------------------------|---------------------|
| `PROTOCOL_VERSION` | The version of the communication protocol.                                                               | 1 byte              |
| `MESSAGE_ID`       | The unique identifier of the message (defined in JSON).                                                  | 1 byte              |
| `MESSAGE_LEN`      | The length of the *payload* (all serialized fields).                                                     | 1 byte              |
| `PAYLOAD`          | The message data.                                                                                        | `MESSAGE_LEN` bytes |
| `CRC16`            | A CRC16 Modbus (see `beta_com`) calculated over the entire frame (from `PROTOCOL_VERSION` to `PAYLOAD`). | 2 bytes             |

The `PAYLOAD` itself is a sequence of fields, each encoded as follows:

| Field        | Description                             | Size             |
|--------------|-----------------------------------------|------------------|
| `PROP_ID`    | The field identifier (defined in JSON). | 1 byte           |
| `PROP_LEN`   | The length of the field's value.        | 1 byte           |
| `PROP_VALUE` | The field's value.                      | `PROP_LEN` bytes |

### Generated Functions

For each message, the following functions are generated to facilitate serialization:

1.  **`struct <MessageName>`**
    The C struct representing your message. **You must first populate this struct with the data you want to send.**

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
    This is the main function to use. It takes the populated struct, fully serializes it (header + payload), calculates the CRC, and applies COBS encoding. The result is a complete binary message, ready to be sent.

### Complete Example

```c
#include "Position.h"
#include <stdio.h>

int main() {
    // 1. Populate the message struct
    Position my_pos;
    my_pos.x = 10.5f;
    my_pos.y = -2.3f;

    // 2. Prepare a buffer to receive the encoded message
    uint8_t output_buffer[256];
    uint8_t *p_buffer = output_buffer;
    size_t buffer_len = sizeof(output_buffer);

    // 3. Serialize and encode the message
    int result = Position_to_message(my_pos, &p_buffer, &buffer_len);

    if (result == 0) {
        size_t message_size = sizeof(output_buffer) - buffer_len;
        printf("Message encoded successfully (%zu bytes)!\n", message_size);

        // The 'output_buffer' now contains the complete binary message
        // ready to be sent via UART, TCP, etc.
        // send_binary_data(output_buffer, message_size);
    } else {
        fprintf(stderr, "Error encoding message: %d\n", result);
    }

    return 0;
}
```

