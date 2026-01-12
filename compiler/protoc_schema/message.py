from typing import List, Annotated
from .field import Field
from pydantic import BaseModel, Field as PydanticField, AfterValidator
from compiler.common.validators import is_valid_name

class Message(BaseModel):
    """Represents a message structure.

    Attributes:
        name: The name of the message.
        id: The unique identifier of the message.
        fields: A list of `Field` objects representing the fields of the message.
        dependencies: A list of other message types that this message depends on.
    """
    name: Annotated[str, AfterValidator(is_valid_name)] = PydanticField(min_length=1)
    id: int = PydanticField(gt=-1)
    fields: List[Field]
    dependencies: List[str] = PydanticField(default_factory=list)

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