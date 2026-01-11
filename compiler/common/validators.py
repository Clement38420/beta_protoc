import re
from pydantic_core import PydanticCustomError

NAME_RE = re.compile(r'^[A-Za-z_][A-Za-z0-9_]*$')
def is_valid_name(name: str) -> str:
    if not NAME_RE.match(name):
        raise PydanticCustomError(
            "invalid_name",
            '"{name}" is not a valid name (only letters, digits and \'_\' allowed, cannot start with a digit)',
            {"name": name},
        )
    return name

LANG_RE = re.compile(r'^[A-Za-z][A-Za-z0-9+\-#]*$')
def is_valid_lang(name: str) -> str:
    if not LANG_RE.match(name):
        raise PydanticCustomError(
            "invalid_lang",
            '"{lang_name}" is not a valid language name (can only starts with a letter and cannot contain spaces)',
            {"lang_name": name},
        )
    return name

EXTENSION_RE = re.compile(r'^[a-z][a-z0-9]*$')
def is_valid_extension(extension: str) -> str:
    if not EXTENSION_RE.match(extension):
        raise PydanticCustomError(
            "invalid_extension",
            '"{extension}" is not a valid extension name (can only start with a letter and cannot contain spaces, upper case letters, and dots)"',
            {"extension": extension},
        )
    return extension