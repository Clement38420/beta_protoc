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

1. **Naming Conventions:** Message and field names must start with a letter and can only contain letters, digits, or underscores (`_`).
2. **Unique IDs:** Message IDs must be unique across all messages. Field IDs must be unique within a single message.
3. **Dependencies:** If message `A` is used as a field type inside message `B`, message `A` must be defined within the `messages` list. The compiler will automatically generate the required dependencies (e.g., `#include "A.h"`).
4. **Order:** The order in which messages are defined in the JSON file does not matter; the compiler resolves dependencies automatically.

## Supported Languages

Currently, the compiler supports:

* **C** (`.c`, `.h`): Generates structs and includes `beta_com.h` and dependent message headers.

## Adding a New Language

The architecture is modular. To add support for a new language (e.g., Python, C++), follow these two steps:

### 1. Define the Language

Open `compiler/core/language.py` and add an entry to the `SUPPORTED_LANGUAGES` list. You must provide the mapping between internal types (`DataType`) and the target language types.

```python
# Example for Python
Language(
    name="Python",
    src_ext="py",
    header_ext=None, # Python does not use header files
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