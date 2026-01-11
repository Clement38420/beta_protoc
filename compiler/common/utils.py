from typing import Dict

def loc_to_path(loc: tuple, data: Dict) -> str:
    """Converts a Pydantic error location tuple to a human-readable path string.

    Args:
        loc: A tuple representing the location of an error in the data structure.
        data: The original data dictionary that was validated.

    Returns:
        A string representing the path to the error, e.g., "/messages/MyMessage/fields[0]/name".
    """
    path = ""
    field = loc[-1]
    curr = data
    for index in loc[:-1]:
        try:
            curr = curr[index]
        except (KeyError, IndexError, TypeError):
            break

        if isinstance(curr, dict) and curr.get("name"):
            path = "/".join([path, curr["name"]])
        elif isinstance(index, int):
            path = "".join([path, f"[{str(index)}]"])
        else:
            path = "/".join([path, str(index)])

    return f"{path}/{field}"