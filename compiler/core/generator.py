import pathlib
from jinja2 import Environment, FileSystemLoader

from .language import Language
from compiler.protoc_schema.schema import ProtocSchema

class Generator:
    """Handles the generation of code from message definitions.

    Attributes:
        env: The Jinja2 environment used for template rendering.
    """
    def __init__(self, template_dir: pathlib.Path, languages: list[Language]):
        self.env = Environment(loader=FileSystemLoader(template_dir))
        self.languages = languages

    def generate(self, in_file: pathlib.Path, out_dir: pathlib.Path):
        """Generates code from a JSON message definition file.

        This method reads a JSON file containing message definitions, parses them, and then
        generates source and header files for each message in each of the supported languages.

        Args:
            in_file: The path to the input JSON file.
            out_dir: The path to the output directory where the generated files will be saved.
        """
        schema = ProtocSchema.from_json_file(in_file)
        messages = schema.messages

        for lang in self.languages:
            src_template = self.env.get_template(f"{lang.name}/message.{lang.src_ext}.j2")

            header_template = None
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