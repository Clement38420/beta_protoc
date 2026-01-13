from pydantic import BaseModel, Field as PydanticField, AfterValidator, model_validator
from pydantic_core import PydanticCustomError

from compiler.common.data_types import DataType
from compiler.common.validators import is_valid_name
from typing import Annotated, Optional
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
    size: Optional[int] = None
    is_primitive: bool = True


    @model_validator(mode='after')
    def normalize_type(self):
        """Normalize the type of the field.

        If the field is a correct 'DataType' enum, the field type is converted, else it is left unchanged, and it represents a custom message type.
        It also handles special cases like "string[SIZE]".

        Raises:
            PydanticCustomError: If the type is "string" without a specified size.
        """
        # Handle "string[SIZE]" format
        string_match = re.match(r"string\[(\d+)]$", self.type)
        if string_match:
            self.size = int(string_match.group(1))
            self.type = "string"
        elif self.type == "string":
            raise PydanticCustomError(
                "invalid_string_type",
                "Type 'string' must have a specified size, e.g., 'string[255]'.",
            )

        try:
            self.type = DataType(self.type).value
        except ValueError:
            self.is_primitive = False

        return self
