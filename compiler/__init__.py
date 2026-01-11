from .compiler.generator import Generator
import pathlib

TEMPLATE_DIR = pathlib.Path(__file__).parent / "templates"

__all__ = [
    "Generator",
    "TEMPLATE_DIR"
]