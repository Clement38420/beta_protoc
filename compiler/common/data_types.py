from enum import Enum

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