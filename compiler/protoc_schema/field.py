from pydantic import BaseModel, Field as PydanticField, AfterValidator
from compiler.common.data_types import DataType
from compiler.common.validators import is_valid_name
from typing import Annotated

class Field(BaseModel):
    """Represents a field in a message.

    Attributes:
        name: The name of the field.
        type: The data type of the field. Can be a `DataType` enum or a string representing a custom message type.
        is_primitive: A boolean indicating whether the field's type is a primitive data type.
    """
    name: Annotated[str, AfterValidator(is_valid_name)] = PydanticField(min_length=1)
    type: DataType | str = PydanticField(min_length=1)
    is_primitive: bool = True


    def normalize_type(self):
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
            self.is_primitive = False