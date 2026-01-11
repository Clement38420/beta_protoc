from typing import Dict, List, TYPE_CHECKING

if TYPE_CHECKING:
    from compiler.core.language import Language

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