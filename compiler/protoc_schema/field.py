from pydantic import BaseModel, Field as PydanticField, AfterValidator, model_validator
from compiler.common.data_types import DataType
from compiler.common.validators import is_valid_name
from typing import Annotated, Optional
from compiler.common.validators import NAME_RE_STRING
import re

class Field(BaseModel):
    """Represents a field in a message.

    Attributes:
        name: The name of the field.
        id: The unique (in the message scope) identifier of the field.
        type: The data type of the field. Representing a string from a 'DataType' member or custom message type.
        size: The size for fixed-size types like 'string[SIZE]'.
        is_primitive: A boolean indicating whether the field's type is a primitive data type.
    """
    name: Annotated[str, AfterValidator(is_valid_name)] = PydanticField(min_length=1)
    id: int = PydanticField(gt=-1)
    type: str = PydanticField(min_length=1)
    is_primitive: bool = True

    is_array: Optional[bool] = False
    is_dynamic: Optional[bool] = False
    array_size: Optional[int] = None


    @model_validator(mode='after')
    def normalize_type(self):
        """Normalize the type of the field.

        If the field is a correct 'DataType' enum, the field type is converted, else it is left unchanged, and it represents a custom message type.
        It also handles special cases like "string[SIZE]".
        """
        array_match = re.match(NAME_RE_STRING + r"\[(\d*)\]$", self.type)
        if array_match:
            self.is_array = True
            self.type = array_match.group(1)
            self.is_dynamic = array_match.group(2) == ""
            self.array_size = None if self.is_dynamic else int(array_match.group(2))

        try:
            self.type = DataType(self.type).value
        except ValueError:
            self.is_primitive = False

        return self

    def get_count_var_name(self):
        """Generates a variable name for the count of elements in an array field."""
        return f"{self.name}_count"

    def get_max_count_var_name(self):
        """Generates a variable name for the maximum count of elements in an array field."""
        return f"{self.name}_max_count"