from pydantic import BaseModel, Field as PydanticField, AfterValidator
from typing import List, Dict, Annotated

from compiler.common.validators import is_valid_lang, is_valid_extension
from compiler.common.errors import MissingTypeError
from compiler.common.data_types import DataType

class Language(BaseModel):
    """Represents a supported programming language for code generation.

    Attributes:
        name: The name of the language.
        src_ext: The file extension for source files.
        header_ext: The file extension for header files.
        types_mapping: A dictionary mapping `DataType` enums to their string representation in this language.
    """
    name: Annotated[str, AfterValidator(is_valid_lang)] = PydanticField(min_length=1)
    src_ext: Annotated[str, AfterValidator(is_valid_extension)] = PydanticField(min_length=1)
    header_ext: Annotated[str, AfterValidator(is_valid_extension)] = None
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