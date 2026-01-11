import argparse
import pathlib
import shutil
import sys

from .compiler.language import Language, SUPPORTED_LANGUAGES
from .compiler.generator import Generator
from .common import loc_to_path, JSONParsingErrors, MissingTypeError
from compiler import TEMPLATE_DIR

def main():
    """The main entry point of the beta_protoc compiler.

    This function parses command-line arguments, validates the input file, creates the output
    directory, and then invokes the `Compiler` to generate the code.
    """
    arg_parser = argparse.ArgumentParser(prog="beta_proc compiler",
                                         description=f"This program uses JSON files to generate {Language.get_supported_languages_string(SUPPORTED_LANGUAGES)} codes which are made to be used in serial communication along with the beta_com library.")
    arg_parser.add_argument("filepath", help="The path to the JSON file to be compiled.")
    arg_parser.add_argument("-o", "--out", default="./generated", help="The output directory for the generated files.")
    args = arg_parser.parse_args()

    protoc_file_path = pathlib.Path(args.filepath).resolve().absolute()

    if not protoc_file_path.exists():
        sys.exit("Error: The specified JSON file does not exist.")

    out_dir = pathlib.Path(args.out).resolve().absolute()
    if out_dir.exists():
        shutil.rmtree(out_dir)

    out_dir.mkdir(parents=True, exist_ok=True)

    compiler = Generator(TEMPLATE_DIR)

    try:
        compiler.generate(protoc_file_path, out_dir)
    except JSONParsingErrors as e:
        details = "\n".join(
            [f"\t- in {loc_to_path(err.loc, e.json_data)}: {err.message}" for err in e.errors]
        )
        sys.exit(f"Error: JSON parsing error:\n{details}")
    except MissingTypeError as e:
        sys.exit(f"Error: {e.type} is not defined for {e.lang.name} language.")

    print(f"Successfully generated code in {out_dir}")

    return None