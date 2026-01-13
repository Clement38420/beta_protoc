from pydantic import BaseModel, Field as PydanticField, AfterValidator
from typing import List, Dict, Annotated
from enum import Enum
import re
from compiler.common.validators import is_valid_lang, is_valid_extension
from compiler.common.errors import MissingTypeError
from compiler.common.data_types import DataType

def camel_to_snake(string: str) -> str:
    name = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', string)
    return re.sub('([a-z0-9])([A-Z])', r'\1_\2', name).lower()

def camel_to_camel(string: str) -> str:
    return string

class Case(Enum):
    """An enumeration of case conversion functions."""
    SNAKE = 0
    CAMEL = 1

CaseConversionFuncs = {
    Case.SNAKE: camel_to_snake,
    Case.CAMEL: camel_to_camel
}

class Language(BaseModel):
    """Represents a supported programming language for code generation."""
    name: Annotated[str, AfterValidator(is_valid_lang)] = PydanticField(min_length=1)
    case: Case
    src_ext: Annotated[str, AfterValidator(is_valid_extension)] = PydanticField(min_length=1)
    header_ext: Annotated[str, AfterValidator(is_valid_extension)] | None = None
    types_mapping: Dict[DataType, str] = PydanticField(default_factory=dict)

    def convert_type(self, type_: str) -> str:
        try:
            converted_type = self.types_mapping.get(DataType(type_))
            if converted_type is not None:
                return converted_type
            else:
                raise MissingTypeError(type_, self)
        except ValueError:
            return type_

    def camel_to_proper_case(self, string: str) -> str:
        """Converts a CamelCase string to its language-specific equivalent."""
        return CaseConversionFuncs[self.case](string)

    @classmethod
    def get_supported_languages_string(cls, supported_langs: List['Language']) -> str:
        return ", ".join([lang.name for lang in supported_langs])

SUPPORTED_LANGUAGES = [
    Language(
        name="C",
        case=Case.SNAKE,
        src_ext="c",
        header_ext="h",
        types_mapping={
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
            DataType.STRING: "char",
            DataType.BOOL: "bool",
        }
    )
]