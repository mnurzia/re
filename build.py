import pack_v4


class Project(pack_v4.Project):
    def generate(self):
        self.prefix = "re"
        self.version = "0.0.1"
        self.bits = [
            "exports", 
            "cstd", 
            "stringify",
            "hook_malloc", 
            "ds_vector", 
            "ds_string", 
            "ds_string_view",
            "ds_refs",
            "max",
            "debug",
            "unused",
            "hash/murmurhash3",
            "inttypes",
            "zeromem"
        ]
        self.apis = ["re_api.h"]
        self.headers = ["re_internal.h"]
        self.sources = [
            "re_ast.c",
            "re_charclass.c",
            "re_compile.c",
            "re_compile_charclass.c",
            "re_exec.c",
            "re_match_data.c",
            "re_parse.c",
            "re_prog.c",
            "re_re.c",
            "re_range.c",
            "re_debug.c"
        ]
        self.tests = [
            "test_ast.c",
            "test_charclass.c",
            "test_compile.c",
            "test_compile_charclass.c",
            "test_exec.c",
            "test_helpers.c",
            "test_main.c",
            "test_parse.c",
            "test_prog.c",
            "test_range.c"
        ]
        self.test_config = "test_config.h"
        self.cstd = "c89"
        self.config = {}
