from pydantic import BaseModel, Field as PydanticField, AfterValidator
from compiler.common.data_types import DataType
from compiler.common.validators import is_valid_name
from typing import Annotated

class Field(BaseModel):
    """Represents a field in a message.

    Attributes:
        name: The name of the field.
        id: The unique (in the message scope) identifier of the field.
        type: The data type of the field. Can be a `DataType` enum or a string representing a custom message type.
        is_primitive: A boolean indicating whether the field's type is a primitive data type.
    """
    name: Annotated[str, AfterValidator(is_valid_name)] = PydanticField(min_length=1)
    id: int = PydanticField(gt=-1)
    type: DataType | str = PydanticField(min_length=1)
    is_primitive: bool = True


    def normalize_type(self):
        """Normalize the type of the field.

        If the field is a correct 'DataType' enum, the field type is converted, else it is left unchanged, and it represents a custom message type.
        """
        try:
            self.type = DataType(self.type)
        except ValueError:
            self.is_primitive = False