import argparse
import pathlib
import shutil
import sys

from .core.language import Language, SUPPORTED_LANGUAGES
from .core.generator import Generator
from .common import loc_to_path, JSONParsingErrors, MissingTypeError
from compiler import TEMPLATE_DIR

def main():
    """The main entry point of the beta_protoc compiler.

    This function parses command-line arguments, validates the input file, creates the output
    directory, and then invokes the `Compiler` to generate the code.
    """
    arg_parser = argparse.ArgumentParser(prog="beta_protoc_compiler",
                                         description=f"This program uses JSON files to generate {Language.get_supported_languages_string(SUPPORTED_LANGUAGES)} codes which are made to be used in serial communication along with the beta_com library.")
    arg_parser.add_argument("filepath", help="The path to the JSON file to be compiled.")
    arg_parser.add_argument("-o", "--out", default="./generated", help="The output directory for the generated files.")
    arg_parser.add_argument("-l", "--lang",
                                help="The target languages for the generated files.",
                                nargs='+',
                                choices=[lang.name for lang in SUPPORTED_LANGUAGES])
    arg_parser.add_argument("--clean", action="store_true", help="Delete the output directory before regenerating code.")
    args = arg_parser.parse_args()

    protoc_file_path = pathlib.Path(args.filepath).resolve().absolute()

    if not protoc_file_path.exists():
        sys.exit(f"Error: File '{protoc_file_path}' does not exist.")

    if (not args.lang) or (len(args.lang) == 0):
        print("Info: No language specified, defaulting to all.")
        selected_languages = SUPPORTED_LANGUAGES
    else:
        selected_languages = [lang for lang in SUPPORTED_LANGUAGES if lang.name in args.lang]

    compiler = Generator(TEMPLATE_DIR, selected_languages)

    out_dir = pathlib.Path(args.out).resolve().absolute()

    for lang in selected_languages:
        lang_out_dir = out_dir / lang.name
        if lang_out_dir.exists() and args.clean:
            shutil.rmtree(lang_out_dir)

    out_dir.mkdir(parents=True, exist_ok=True)

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