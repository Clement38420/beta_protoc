import json
import argparse
import pathlib
from enum import Enum
from typing import List, Dict, ClassVar, Optional
import sys
from jinja2 import Environment, FileSystemLoader
from pydantic import BaseModel, ValidationError, model_validator, Field as PydanticField
from pydantic_core import ErrorDetails

TEMPLATE_DIR = pathlib.Path(__file__).parent / "templates"

VALIDATION_MESSAGES = {
    "string_too_short": "This field is required.",
    "missing": "This field is required."
}

def loc_to_path(loc: tuple, data: Dict) -> str:
    """Converts a Pydantic error location tuple to a human-readable path string.

    Args:
        loc: A tuple representing the location of an error in the data structure.
        data: The original data dictionary that was validated.

    Returns:
        A string representing the path to the error, e.g., "/messages/MyMessage/fields[0]/name".
    """
    path = ""
    field = loc[-1]
    curr = data
    for index in loc[:-1]:
        try:
            curr = curr[index]
        except (KeyError, IndexError, TypeError):
            break

        if isinstance(curr, dict) and curr.get("name"):
            path = "/".join([path, curr["name"]])
        elif isinstance(index, int):
            path = "".join([path, f"[{str(index)}]"])
        else:
            path = "/".join([path, str(index)])

    return f"{path}/{field}"

class MissingTypeError(Exception):
    """Raised when a type is missing in a language's type mapping."""
    def __init__(self, type_: str, lang: 'Language'):
        self.type = type_
        self.lang = lang

class JSONParsingErrorDetails:
    """Represents the details of a single JSON parsing error."""
    def __init__(self, message: str, loc: tuple):
        self.message = message
        self.loc = loc

class JSONParsingErrors(Exception):
    """Represents a collection of errors found during JSON parsing and validation."""
    def __init__(self, json_data: Dict, errors: List[JSONParsingErrorDetails]):
        self.json_data = json_data
        self.errors = errors

class InvalidTypeError(Exception):
    """Raised when a field's type is not a primitive or a defined message."""
    def __init__(self, type_: str):
        self.type = type_

class DataType(str, Enum):
    """An enumeration of supported data types for message fields."""
    # Unsigned
    UINT8 = "uint8"
    UINT16 = "uint16"
    UINT32 = "uint32"
    UINT64 = "uint64"

    # Signed
    INT8 = "int8"
    INT16 = "int16"
    INT32 = "int32"
    INT64 = "int64"

    # Float
    FLOAT32 = "float32"
    FLOAT64 = "float64"

    # Text and logic
    CHAR = "char"
    STRING = "string"
    BOOL = "bool"

class Language(BaseModel):
    """Represents a supported programming language for code generation.

    Attributes:
        name: The name of the language.
        src_ext: The file extension for source files.
        header_ext: The file extension for header files.
        types_mapping: A dictionary mapping `DataType` enums to their string representation in this language.
    """
    name: str = PydanticField(min_length=1)
    src_ext: str = PydanticField(min_length=1)
    header_ext: str = None
    types_mapping: Dict[DataType, str] = PydanticField(default_factory=dict)

    def convert_type(self, type_: DataType | str) -> str:
        """Converts a beta_protoc data type to its language-specific equivalent.

        Args:
            type_: The data type to convert, which can be a `DataType` enum member or a string
                   representing a custom message type.

        Returns:
            The language-specific type as a string.

        Raises:
            MissingTypeError: If the data type is not defined in the language's `types_mapping`.
        """
        if isinstance(type_, DataType):
            converted_type = self.types_mapping.get(type_)
            if converted_type is not None:
                return converted_type
            else:
                raise MissingTypeError(type_, self)
        else:
            return type_

    @classmethod
    def get_supported_languages_string(cls, supported_langs: List['Language']) -> str:
        """Returns a comma-separated string of supported language names.

        Args:
            supported_langs: A list of `Language` objects.

        Returns:
            A string containing the names of the supported languages, separated by commas.
        """
        return ", ".join([lang.name for lang in supported_langs])


SUPPORTED_LANGUAGES = [
    Language(name="C", src_ext="c", header_ext="h", types_mapping={
        DataType.UINT8: "uint8_t",
        DataType.UINT16: "uint16_t",
        DataType.UINT32: "uint32_t",
        DataType.UINT64: "uint64_t",
        DataType.INT8: "int8_t",
        DataType.INT16: "int16_t",
        DataType.INT32: "int32_t",
        DataType.INT64: "int64_t",
        DataType.FLOAT32: "float",
        DataType.FLOAT64: "double",
        DataType.CHAR: "char",
        DataType.STRING: "char*",
        DataType.BOOL: "bool",
    })
]

class Field(BaseModel):
    """Represents a field in a message.

    Attributes:
        name: The name of the field.
        type: The data type of the field. Can be a `DataType` enum or a string representing a custom message type.
        is_primitive: A boolean indicating whether the field's type is a primitive data type.
    """
    name: str = PydanticField(min_length=1)
    type: DataType | str = PydanticField(min_length=1)
    is_primitive: bool = True


    def check_type(self):
        """Checks if the field's type is valid.

        A type is considered valid if it is either a primitive `DataType` or a
        previously defined message type present in the `Message.registry`.
        If the type is not a primitive, `is_primitive` is set to False.

        Raises:
            InvalidTypeError: If the type is neither a primitive nor a defined message.
        """
        try:
            self.type = DataType(self.type)
        except ValueError:
            if self.type not in Message.registry:
                raise InvalidTypeError(self.type)
            else:
                self.is_primitive = False

class Message(BaseModel):
    """Represents a message structure.

    Attributes:
        registry: A class-level dictionary to keep track of all defined messages.
        name: The name of the message.
        fields: A list of `Field` objects representing the fields of the message.
        dependencies: A list of other message types that this message depends on.
    """
    registry: ClassVar[Dict[str, Optional['Message']]] = {}

    name: str = PydanticField(min_length=1)
    fields: List[Field]
    dependencies: List[str] = PydanticField(default_factory=list)

    @model_validator(mode='after')
    def add_to_registry(self):
        """Adds the message to the central registry after validation.

        This allows for tracking of all defined messages, which is crucial for
        validating field types that are themselves messages.

        Returns:
            The validated `Message` instance.
        """
        Message.registry[self.name] = self
        return self

    def resolve_dependencies(self):
        """Identifies and records dependencies on other message types.

        This method iterates through the message's fields and adds any non-primitive
        field types to the `dependencies` list. This is used during code generation
        to handle includes or imports of other message definitions.
        """
        for f in self.fields:
            if not f.is_primitive:
                if f.type not in self.dependencies:
                    self.dependencies.append(f.type)

class ProtocSchema(BaseModel):
    """Represents a schema definition."""
    messages: List[Message]

    def validate_schema(self):
        """Validates the entire schema after initial parsing.

        This method iterates through all messages and their fields to perform
        cross-cutting validation that can only happen once the entire schema
        is loaded. Specifically, it checks the validity of field types and
        resolves dependencies between messages.

        Raises:
            JSONParsingErrors: If an invalid type is found in any field.
        """
        for msg_index, message in enumerate(self.messages):
            for field_index, f in enumerate(message.fields):
                try:
                    f.check_type()
                except InvalidTypeError as e:
                    loc = ("messages", msg_index, "fields", field_index, "type")

                    raise JSONParsingErrors(self.model_dump(),
                                            [JSONParsingErrorDetails(
                                                message=f"{e.type} is not a valid type (not a primitive nor a defined message).",
                                                loc=loc
                                            )])
            message.resolve_dependencies()


class Compiler:
    """Handles the generation of code from message definitions.

    Attributes:
        env: The Jinja2 environment used for template rendering.
    """
    def __init__(self, template_dir: pathlib.Path):
        self.env = Environment(loader=FileSystemLoader(template_dir))

    def handle_validation_error(self, e: ValidationError, json_data: Dict):
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

    def parse_json(self, in_file: pathlib.Path) -> ProtocSchema:
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
        Message.registry.clear()

        with open(in_file, "r") as f:
            protoc_json = json.load(f)

        try:
            schema = ProtocSchema.model_validate(protoc_json)
            schema.validate_schema()

            return schema
        except ValidationError as e:
            self.handle_validation_error(e, protoc_json)

    def generate(self, in_file: pathlib.Path, out_dir: pathlib.Path):
        """Generates code from a JSON message definition file.

        This method reads a JSON file containing message definitions, parses them, and then
        generates source and header files for each message in each of the supported languages.

        Args:
            in_file: The path to the input JSON file.
            out_dir: The path to the output directory where the generated files will be saved.
        """
        messages = self.parse_json(in_file).messages

        for lang in SUPPORTED_LANGUAGES:
            src_template = self.env.get_template(f"{lang.name}/message.{lang.src_ext}.j2")

            header_template = None
            gen_header = True if lang.header_ext else False
            if gen_header:
                header_template = self.env.get_template(f"{lang.name}/message.{lang.header_ext}.j2")

            (out_dir / lang.name).mkdir(parents=True, exist_ok=True)

            for message in messages:
                src_content = src_template.render(message=message, lang=lang)
                with open(out_dir / lang.name / (message.name + "." + lang.src_ext), "w") as f:
                    f.write(src_content)

                if gen_header:
                    header_content = header_template.render(message=message, lang=lang)
                    with open(out_dir / lang.name / (message.name + "." + lang.header_ext), "w") as f:
                        f.write(header_content)

def main():
    """The main entry point of the beta_protoc compiler.

    This function parses command-line arguments, validates the input file, creates the output
    directory, and then invokes the `Compiler` to generate the code.
    """
    arg_parser = argparse.ArgumentParser(prog="beta_proc compiler",
                                         description=f"This program uses JSON files to generate {Language.get_supported_languages_string(SUPPORTED_LANGUAGES)} codes which are made to be used in serial communication along with the beta_com library.")
    arg_parser.add_argument("filepath", help="The path to the JSON file to be compiled.")
    arg_parser.add_argument("-o", "--out", default="./generated", help="The output directory for the generated files.")
    args = arg_parser.parse_args()

    protoc_file_path = pathlib.Path(args.filepath).resolve().absolute()

    if not protoc_file_path.exists():
        sys.exit("Error: The specified JSON file does not exist.")

    out_dir = pathlib.Path(args.out).resolve().absolute()
    out_dir.mkdir(parents=True, exist_ok=True)

    compiler = Compiler(TEMPLATE_DIR)

    try:
        compiler.generate(protoc_file_path, out_dir)
    except JSONParsingErrors as e:
        details = "\n".join(
            [f"\t- in {loc_to_path(err.loc, e.json_data)}: {err.message}" for err in e.errors]
        )
        sys.exit(f"Error: JSON parsing error:\n{details}")
    except MissingTypeError as e:
        sys.exit(f"Error: {e.type} is not defined for {e.lang.name} language.")

    print(f"Successfully generated code in {out_dir}")

    return None

if __name__ == '__main__':
  main()