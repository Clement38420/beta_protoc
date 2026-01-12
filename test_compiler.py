import pytest
import json
from pathlib import Path
from compiler.protoc_schema.schema import ProtocSchema
from compiler.core.generator import Generator
from compiler.common.errors import JSONParsingErrors
from compiler.core.language import SUPPORTED_LANGUAGES
from compiler import TEMPLATE_DIR

# --- Fixtures ---

@pytest.fixture
def valid_json_file(tmp_path):
    """
    Create a valid JSON message definition file for testing.
    """
    content = {
        "messages": [
            {
                "name": "SensorData",
                "id": 1,
                "fields": [
                    {"name": "id", "id": 1, "type": "uint8"},
                    {"name": "value", "id": 2, "type": "float32"}
                ]
            }
        ]
    }
    f = tmp_path / "valid.json"
    f.write_text(json.dumps(content))
    return f

@pytest.fixture
def invalid_type_json_file(tmp_path):
    """
    Create a JSON file with an undefined data type to trigger validation errors.
    """
    content = {
        "messages": [
            {
                "name": "BadMsg",
                "id": 1,
                "fields": [
                    {"name": "field1", "id": 1, "type": "int128"} # int128 is not supported
                ]
            }
        ]
    }
    f = tmp_path / "invalid_type.json"
    f.write_text(json.dumps(content))
    return f

# --- Test Functions ---

def test_json_validation_success(valid_json_file):
    """
    Test that a correctly formatted JSON is successfully parsed into a ProtocSchema object.
    """
    schema = ProtocSchema.from_json_file(valid_json_file)
    assert len(schema.messages) == 1
    assert schema.messages[0].name == "SensorData"
    assert len(schema.messages[0].fields) == 2

def test_localized_error_reporting(invalid_type_json_file):
    """
    Test that the compiler correctly identifies and locates an invalid type error.
    It should point exactly to the 'type' field of the specific message.
    """
    with pytest.raises(JSONParsingErrors) as excinfo:
        ProtocSchema.from_json_file(invalid_type_json_file)

    error = excinfo.value
    details = error.errors[0]

    # Check if the error message mentions the faulty type
    assert "int128" in details.message
    # Check if the location (loc) matches the JSON structure: messages[0] -> fields[0] -> type
    assert details.loc == ('messages', 0, 'fields', 0, 'type')

def test_invalid_identifier_naming(tmp_path):
    """
    Test that message names starting with a digit are rejected by the validator.
    """
    content = {
        "messages": [{"name": "1InvalidName", "fields": []}]
    }
    f = tmp_path / "bad_name.json"
    f.write_text(json.dumps(content))

    with pytest.raises(JSONParsingErrors) as excinfo:
        ProtocSchema.from_json_file(f)

    assert "is not a valid name" in excinfo.value.errors[0].message

def test_c_code_generation_output(valid_json_file, tmp_path):
    """
    Test the full generation pipeline for the C language.
    Verifies that files are created and type mapping (e.g., uint8 -> uint8_t) is applied.
    """
    output_dir = tmp_path / "generated"

    # Initialize generator with the project's template directory and supported languages
    generator = Generator(TEMPLATE_DIR, SUPPORTED_LANGUAGES)
    generator.generate(valid_json_file, output_dir)

    c_file = output_dir / "C" / "SensorData.c"
    h_file = output_dir / "C" / "SensorData.h"

    # Ensure files were actually written to disk
    assert c_file.exists()
    assert h_file.exists()

    # Verify content of the generated header file
    content = h_file.read_text()
    assert "typedef struct" in content
    assert "uint8_t id;" in content # Check language-specific type conversion
    assert "float value;" in content

def test_missing_dependency_validation(tmp_path):
    """
    Test that referencing a custom message type that hasn't been defined
    raises a validation error during schema analysis.
    """
    content = {
        "messages": [
            {
                "name": "Parent",
                "id": 1,
                "fields": [
                    {"name": "child", "id": 1, "type": "UnknownType"} # UnknownType is neither primitive nor defined
                ]
            }
        ]
    }
    f = tmp_path / "missing_dep.json"
    f.write_text(json.dumps(content))

    with pytest.raises(JSONParsingErrors) as excinfo:
        ProtocSchema.from_json_file(f)

    assert "UnknownType is not a valid type" in excinfo.value.errors[0].message

def test_duplicate_message_id(tmp_path):
    """
    Test that duplicate message IDs are detected and reported.
    """
    content = {
        "messages": [
            {"name": "MessageA", "id": 1, "fields": []},
            {"name": "MessageB", "id": 1, "fields": []}
        ]
    }
    f = tmp_path / "duplicate_msg_id.json"
    f.write_text(json.dumps(content))

    with pytest.raises(JSONParsingErrors) as excinfo:
        ProtocSchema.from_json_file(f)

    error = excinfo.value.errors[0]
    assert '"MessageA", "MessageB" have the same id.' in error.message
    assert error.loc == ('messages',)

def test_duplicate_field_id(tmp_path):
    """
    Test that duplicate field IDs within a single message are detected.
    """
    content = {
        "messages": [
            {
                "name": "MessageWithDupFields",
                "id": 1,
                "fields": [
                    {"name": "field_a", "id": 1, "type": "uint8"},
                    {"name": "field_b", "id": 1, "type": "uint16"}
                ]
            }
        ]
    }
    f = tmp_path / "duplicate_field_id.json"
    f.write_text(json.dumps(content))

    with pytest.raises(JSONParsingErrors) as excinfo:
        ProtocSchema.from_json_file(f)

    error = excinfo.value.errors[0]
    assert '"field_a", "field_b" have the same id.' in error.message
    assert error.loc == ('messages', 0, 'fields')
