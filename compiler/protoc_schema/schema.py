from pydantic import ValidationError, BaseModel
from pydantic_core import ErrorDetails
from typing import List, Dict
from .message import Message
from compiler.common.errors import JSONParsingErrors, InvalidTypeError, JSONParsingErrorDetails
import pathlib
import json

VALIDATION_MESSAGES = {
    "string_too_short": "This field is required.",
    "missing": "This field is required."
}

class ProtocSchema(BaseModel):
    """Represents a protoc_schema definition."""
    messages: List[Message]

    @staticmethod
    def handle_validation_error(e: ValidationError, json_data: Dict):
        """Handles Pydantic validation errors and transforms them into custom `JSONParsingErrors`.

        This method processes a `ValidationError` from Pydantic, applies custom error messages
        if available, and then raises a `JSONParsingErrors` exception containing the details.

        Args:
            e: The `ValidationError` instance caught during Pydantic model validation.
            json_data: The original JSON data that was being validated.

        Raises:
            JSONParsingErrors: Always raises this exception with the processed error details.
        """
        new_errors: list[ErrorDetails] = []
        for error in e.errors():
            custom_message = VALIDATION_MESSAGES.get(error['type'])
            if custom_message:
                ctx = error.get('ctx')
                error['msg'] = (
                    custom_message.format(**ctx) if ctx else custom_message
                )
            new_errors.append(error)

        raise JSONParsingErrors(json_data, [JSONParsingErrorDetails(err.get("msg"), err.get("loc")) for err in new_errors])

    @classmethod
    def from_json_file(cls, in_file: pathlib.Path) -> 'ProtocSchema':
        """Parses and validates a JSON file into a `ProtocSchema` object.

        This method reads the specified JSON file, validates its structure against
        the `ProtocSchema` model, and then performs additional validation checks.

        Args:
            in_file: The path to the input JSON file.

        Returns:
            A validated `ProtocSchema` object.

        Raises:
            JSONParsingErrors: If any validation errors occur during parsing.
        """
        with open(in_file, "r") as f:
            data = json.load(f)

        try:
            schema = cls.model_validate(data)
            schema.validate_schema()
            return schema

        except ValidationError as e:
            raise cls.handle_validation_error(e, data)

    def validate_schema(self):
        """Validates the entire protoc_schema after initial parsing.

        This method iterates through all messages and their fields to perform
        cross-cutting validation that can only happen once the entire protoc_schema
        is loaded. Specifically, it checks the validity of field types and
        resolves dependencies between messages.

        Raises:
            JSONParsingErrors: If an invalid type is found in any field.
        """
        messages_name = {msg.name for msg in self.messages}

        errors_to_raise = []

        for msg_index, message in enumerate(self.messages):
            for field_index, f in enumerate(message.fields):
                    f.normalize_type()
                    if not f.is_primitive:
                        if f.type not in messages_name:
                            loc = ("messages", msg_index, "fields", field_index, "type")
                            errors_to_raise.append(JSONParsingErrorDetails(
                                message=f"{f.type} is not a valid type (not a primitive nor a defined message).",
                                loc=loc
                            ))

            message.resolve_dependencies()

        if len(errors_to_raise) > 0:
            raise JSONParsingErrors(self.model_dump(), errors_to_raise)