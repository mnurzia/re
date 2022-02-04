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
            "re_main.c",
            "re_parse.c",
            "re_prog.c",
            "re_re.c",
            "re_range.c",
            "re_debug.c"
        ]
        self.cstd = "c89"
        self.config = {}
