# clang -Xclang -ast-dump -fsyntax-only -fno-color-diagnostics ../test/recover_desugar/main.c > testAST.txt ; python3 ../ASTInfoGenerator/ast_info_generator.py testAST.txt testASTOutput.txt

# python3 ../run_pt.py recover_desugar

import sys
sys.path.insert(0, '..')

from AnnoStructure.AnnotationManager import *
from Specifiers.FunctionStructure.Function import *
from Specifiers.StructStructure.Struct import *
from Specifiers.SpecifierManager import *
from StructVariables.StructVarManager import *
from ASTInfoGenerator.Debug import *

from DeclParser.DeclParser import *

file_to_read = sys.argv[1]
output_file = sys.argv[2]

with open(file_to_read) as ast:
    cur_spec = None  # Function or Struct or None
    cur_top_decl = None  # FunctionDecl or RecordDecl (strings)
    cur_mid_decl = None  # ParmVarDecl or FieldDecl (strings)
    param_index = None
    field_index = None

    ast_lines = ast.readlines()
    specifier_manager = SpecifierManager()
    annotation_manager = AnnotationManager()
    struct_var_manager = StructVarManager()
    decl_parser = DeclParser()

    for expr in ast_lines:
        expr = expr.strip()

        first_letter_index = expr.find(next(filter(str.isalpha, expr)))
        start_decl_index = first_letter_index
        decl = ""
        cur_decl_char = expr[start_decl_index]

        try:
            while (cur_decl_char != " "):
                decl += cur_decl_char
                start_decl_index += 1
                cur_decl_char = expr[start_decl_index]
        except IndexError:
            # might be a null statement (looks like "<<<NULL>>>" in AST)

            # TODO: write explicit check for null statements; lack of
            # documentation on the formatting of
            # null statements makes writing explicit checks difficult

            field_index = None
            continue

        type_parsed = decl_parser.raw_ast_to_decl_type(expr, specifier_manager)
        found_type = type(type_parsed)

        if found_type is FunctionDecl:
            specifier_manager.add_function(
                type_parsed.get_fn_name(), type_parsed.get_ret_type())

        elif found_type is RecordDecl:
            specifier_manager.add_struct(type_parsed.get_struct_name())

        elif found_type is FieldDecl:
            spec = specifier_manager.get_specifiers()[-1]
            if type(spec) is Struct:
                field = Field(type_parsed.get_field_name(),
                              type_parsed.get_field_index())
                spec.add_field(field)

        elif found_type is ParmVarDecl:
            spec = specifier_manager.get_specifiers()[-1]
            if type(spec) is Function:
                param = Parameter(type_parsed.get_parm_index(),
                                  type_parsed.get_parm_type())
                spec.add_parameter(param)

        elif found_type is TypedefDecl:
            specifier_manager.get_struct(type_parsed.get_original_type_name()).add_typedef(
                type_parsed.get_alias_name())

        elif found_type is StructVarDecl:
            struct_var_manager.add_struct_var(
                type_parsed.get_struct_var_name(), type_parsed.get_struct_type_name())

        elif found_type is AnnotateAttr:
            anno = Annotation(type_parsed.get_annotation_type(
            ), type_parsed.get_annotation_target(), type_parsed.get_annotation_methods())
            annotation_manager.add_annotation(anno)

    if DEBUG:
        for spec in specifier_manager.get_specifiers():
            if type(spec) is Function:
                print(f"*FUNCTION")
                print(f"@NAME ({spec.get_name()})")
                print(f"@RETURN_TYPE ({spec.get_return_type().strip()})")

                # @PARAMETERS [type one,type two,type three]

                param_str = ""

                for (i, parameter) in enumerate(spec.get_parameters()):
                    param_str += parameter.get_param_type()
                    if (i != len(spec.get_parameters()) - 1):
                        param_str += ","

                if param_str != "":
                    print(f"@PARAMETERS [{param_str.strip()}]")

                print()

        for spec in specifier_manager.get_specifiers():
            # *STRUCT
            # @NAME (struct_name)
            # @FIELDS [name1,name2,name3]

            if type(spec) is Struct:
                print(f"*STRUCT")
                print(f"@NAME ({spec.get_name()})")

                field_str = ""

                for (i, field) in enumerate(spec.get_fields()):
                    field_str += field.get_field_name()
                    if (i != len(spec.get_fields()) - 1):
                        field_str += ","

                if field_str != "":
                    print(f"@FIELDS [{field_str.strip()}]")

                print()

        for struct_var in struct_var_manager.struct_vars:
            print("*STRUCT_VARIABLE")
            print(f"@NAME ({struct_var.get_var_name()})")
            print(f"@TYPE ({struct_var.get_type_name()})")
            print()

        for anno in annotation_manager.get_annotations():
            print("*ANNOTATION")
            print(f"@STRING ({anno.to_str()})")
            print()

    with open(output_file, "w") as output:
        for spec in specifier_manager.get_specifiers():
            if type(spec) is Function:
                output.write(f"*FUNCTION\n")
                output.write(f"@NAME ({spec.get_name()})\n")
                output.write(
                    f"@RETURN_TYPE ({spec.get_return_type().strip()})\n")

                param_str = ""

                for (i, parameter) in enumerate(spec.get_parameters()):
                    param_str += parameter.get_param_type()
                    if (i != len(spec.get_parameters()) - 1):
                        param_str += ","

                if param_str != "":
                    output.write(f"@PARAMETERS [{param_str.strip()}]\n")

                output.write("\n")

        for spec in specifier_manager.get_specifiers():
            if type(spec) is Struct:
                output.write(f"*STRUCT\n")
                output.write(f"@NAME ({spec.get_name()})\n")

                field_str = ""

                for (i, field) in enumerate(spec.get_fields()):
                    field_str += field.get_field_name()
                    if (i != len(spec.get_fields()) - 1):
                        field_str += ","

                if field_str != "":
                    output.write(f"@FIELDS [{field_str.strip()}]\n")

                output.write("\n")

        for struct_var in struct_var_manager.struct_vars:
            output.write("*STRUCT_VARIABLE\n")
            output.write(f"@NAME ({struct_var.get_var_name()})\n")
            output.write(f"@TYPE ({struct_var.get_type_name()})\n\n")

        for anno in annotation_manager.get_annotations():
            output.write("*ANNOTATION\n")
            output.write(f"@STRING ({anno.to_str()})\n\n")
