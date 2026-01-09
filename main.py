import json
import argparse
import pathlib
from dataclasses import dataclass, field
from enum import Enum
from typing import List, Dict, ClassVar, Optional
import sys
from jinja2 import Environment, FileSystemLoader

TEMPLATE_DIR = pathlib.Path(__file__).parent / "templates"

class DataType(str, Enum):
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

@dataclass
class Language:
    name: str = None
    src_ext: str = None
    header_ext: str = None
    types_mapping: Dict[DataType, str] = field(default_factory=dict)

    def get_lang_type(self, type_: DataType | str) -> str:
        if isinstance(type_, DataType):
            return self.types_mapping[type_]
        else:
            return type_

    @classmethod
    def get_supported_languages_string(cls, supported_langs: List['Language']) -> str:
        return ", ".join([lang.name for lang in supported_langs])


SUPPORTED_LANGUAGES = [
    Language("C", "c", "h", {
        DataType.UINT8: "uint8_t",
        DataType.UINT16: "uint16_t",
        DataType.UINT32: "uint32_t",
        DataType.UINT64: "uint64_t",
        DataType.INT8: "int8_t",
        DataType.INT16: "int16_t",
        DataType.INT32: "int32_t",
        DataType.INT64: "int64_t",
        DataType.FLOAT32: "float",
        DataType.FLOAT64: "double",
        DataType.CHAR: "char",
        DataType.STRING: "char*",
        DataType.BOOL: "bool",
    })
]

@dataclass
class Field:
    name: str
    type: DataType | str
    is_primitive: bool = True


    def __post_init__(self):
        if isinstance(self.type, str):
            try:
                self.type = DataType(self.type)
            except ValueError:
                if self.type not in Message.registry:
                    sys.exit(f"Error: Type '{self.type}' is unknown (not a primitive nor a defined message).")
                else:
                    self.is_primitive = False

    @classmethod
    def from_dict(cls, dict_: Dict) -> 'Field':
        return cls(dict_["name"], dict_["type"])

@dataclass
class Message:
    registry: ClassVar[Dict[str, Optional['Message']]] = {}

    name: str
    fields: List[Field]
    dependencies: List[str] = field(default_factory=list)

    def __post_init__(self):
        for f in self.fields:
            if not f.is_primitive:
                if f.type not in self.dependencies:
                    self.dependencies.append(f.type)

        Message.registry[self.name] = self

    @classmethod
    def from_dict(cls, dict_: Dict) -> 'Message':
        return cls(dict_["name"], [Field.from_dict(field_) for field_ in dict_["fields"]])

class Compiler:
    def __init__(self, template_dir: pathlib.Path):
        self.env = Environment(loader=FileSystemLoader(template_dir))

    def generate(self, in_file: pathlib.Path, out_dir: pathlib.Path):
        with open(in_file, "r") as f:
            protoc_json = json.load(f)

        # Pre-filling to know which types are correct in Message instanciation
        Message.registry.clear()
        for msg_data in protoc_json["messages"]:
            Message.registry[msg_data["name"]] = None

        messages = [Message.from_dict(message) for message in protoc_json["messages"]]

        for lang in SUPPORTED_LANGUAGES:
            src_template = self.env.get_template(f"{lang.name}/message.{lang.src_ext}.j2")

            gen_header = True if lang.header_ext else False
            if gen_header:
                header_template = self.env.get_template(f"{lang.name}/message.{lang.header_ext}.j2")

            (out_dir / lang.name).mkdir(parents=True, exist_ok=True)

            for message in messages:
                src_content = src_template.render(message=message, lang=lang)
                with open(out_dir / lang.name / (message.name + "." + lang.src_ext), "w") as f:
                    f.write(src_content)

                if gen_header:
                    header_content = header_template.render(message=message, lang=lang)
                    with open(out_dir / lang.name / (message.name + "." + lang.header_ext), "w") as f:
                        f.write(header_content)

def main():
    arg_parser = argparse.ArgumentParser(prog="beta_proc compiler",
                                         description=f"This program uses json files to generate {Language.get_supported_languages_string(SUPPORTED_LANGUAGES)} codes which are made to be used in serial communication along with the beta_com library.")
    arg_parser.add_argument("filepath", help="The path to the json file to be compiled.")
    arg_parser.add_argument("-o", "--out", default="./generated", help="The output directory for the generated files.")
    args = arg_parser.parse_args()

    protoc_file_path = pathlib.Path(args.filepath).resolve().absolute()

    if not protoc_file_path.exists():
        sys.exit("Error: The specified json file does not exist.")

    out_dir = pathlib.Path(args.out).resolve().absolute()
    out_dir.mkdir(parents=True, exist_ok=True)

    compiler = Compiler(TEMPLATE_DIR)
    compiler.generate(protoc_file_path, out_dir)

    return None

if __name__ == '__main__':
  main()