import argparse
import enum
import io
import itertools
import json
import pathlib
import urllib3
import shutil
import zipfile

UCD_URL = "https://www.unicode.org/Public/zipped"

class UcdType:
    def __init__(self, name, symbol):
        self.name = name
        self.symbol = symbol

    def parse(self, s):
        raise NotImplementedError

class HexType(UcdType):
    def __init__(self, name):
        UcdType.__init__(self, name, "_Hex")

    def parse(self, s):
        return int(s, 16)

class HexRangeType(UcdType):
    def __init__(self, name):
        UcdType.__init__(self, name, "_HexRange")
    
    def parse(self, s):
        if "." in s:
            return [int(x, 16) for x in s.split("..")]
        else:
            return int(x, 16)

class MiscType(UcdType):
    def __init__(self, name):
        UcdType.__init__(self, name, "M")
    
    def parse(self, s):
        return s

class EnumType(UcdType):
    def __init__(self, name):
        UcdType.__init__(self, name, "E")
        self.uniques = set()
    
    def parse(self, s):
        self.uniques.add(s)
        return s

class NumType(UcdType):
    def __init__(self, name):
        UcdType.__init__(self, name, "N")
    
    def parse(self, s):
        return int(s)

class StringType(UcdType):
    def __init__(self, name):
        UcdType.__init__(self, name, "S")
    
    def parse(self, s):
        return s

class DecompTypeMapType(UcdType):
    def __init__(self, name, map_name):
        UcdType.__init__(self, name, "E, S")
        self.decomp_type = EnumType(map_name)
        self.decomp_map = []
    
    def parse(self, s):
        parts = [x.strip() for x in s.strip().split(" ")]
        enum_part = ""
        map_part = []
        if parts == [""]:
            pass
        elif parts[0].startswith("<") and parts[0].endswith(">"):
            enum_part = parts[0][1:-1]
            map_part = [int(x, 16) for x in parts[1:]]
        else:
            map_part = [int(x, 16) for x in parts]
        return [self.decomp_type.parse(enum_part), map_part]

def read_unicode_data(f, fmts):
    out = []
    for l in f.readlines():
        comment_loc = l.find("#")
        if comment_loc != -1:
            l = l[:comment_loc]
        parts = l.strip().split(";")
        data = {}
        for part, format in zip(parts, fmts):
            data[format.name] = format.parse(part)
        out.append(data)
    return out

def cmd_fetch(args):
    db_dir = pathlib.Path(args.db_dir)
    db_dir.mkdir(exist_ok = True)
    zip_path = db_dir / ("UCD-%s.zip" % args.db_version)
    http = urllib3.PoolManager()
    with http.request("GET", UCD_URL + "/" + args.db_version + "/UCD.zip", \
            preload_content=False) as req, \
        open(zip_path, 'wb') as out_file:
        shutil.copyfileobj(req, out_file)

def cmd_gen_class(args):
    db_dir = pathlib.Path(args.db_dir)
    zip_path = db_dir / ("UCD-%s.zip" % args.db_version)
    zip_file = zipfile.ZipFile(zip_path, 'r')
    udt = io.TextIOWrapper(zip_file.open("UnicodeData.txt"), "utf-8")
    fmt = [
        HexType("Codepoint"), 
        MiscType("Name"), 
        EnumType("General_Category"), 
        NumType("Canonical_Combining_Class"),
        EnumType("Bidi_Class"),
        DecompTypeMapType("Decomposition_Type", "Decomposition_Mapping")
    ]
    data = read_unicode_data(udt, fmt)
    #print(read_unicode_data(udt, fmt))
    #print(sorted(list(fmt[5].decomp_type.uniques)))
    filtered = filter_chars(data, args.criteria)
    if args.format == "c":
        dump_set_c(filtered, args)
    elif args.format == "img-viz":
        dump_set_img_viz(filtered, args)

def filter_chars(data, filter):
    matching_chars = []
    criteria = [x.split("=") for x in filter.split(",")]
    for l in data:
        if all([l[k] == v for k, v in criteria]):
            matching_chars.append(l["Codepoint"])
    return matching_chars

def group_by(i):
    for _, b in itertools.groupby(enumerate(i), lambda pair: pair[1] - pair[0]):
        b = list(b)
        yield b[0][1], b[-1][1]

def dump_set_c(matching_chars, _):
    print("{")
    line = "    "
    groups = group_by(matching_chars)
    for start, end in groups:
        next_part = "{0x%X,0x%X}," % (start, end)
        if len(line) + len(next_part) >= 80:
            print(line)
            line = "    "
        line += next_part
    if line != "    ":
        print(line)
    print("}")

def dump_set_img_viz(matching_chars, _):
    from PIL import Image
    rows = 0x110000 // 256
    img = Image.new("1", (256, rows))
    pix = img.load()
    for r in range(0, rows):
        for c in range(0, 256):
            val = r * 256 + c
            if val in matching_chars:
                pix[c, r] = 1
    img.save("unicode_viz.png")

def cmd_create_db(args):
    db_path = pathlib.Path(args.db_dir) / "unicode_db.json"
    db = {}


if __name__ == "__main__":
    parse = argparse.ArgumentParser()
    parse.add_argument(
        "--db-dir",
        type=str,
        help="directory to store data in",
        default="unicode_data"
    )
    parse.add_argument(
        "--db-version",
        type=str,
        help="version of database to fetch",
        default="latest"
    )

    parse_cmds = parse.add_subparsers(
        help="subcommands",
        required=True
    )
    parse_cmd_fetch = parse_cmds.add_parser(
        "fetch",
        help="fetch unicode database"
    )
    parse_cmd_fetch.set_defaults(func=cmd_fetch)

    parse_cmd_dump = parse_cmds.add_parser(
        "dump_set",
        help="dump set from UnicodeData.txt"
    )
    parse_cmd_dump.set_defaults(func=cmd_gen_class)
    parse_cmd_dump.add_argument(
        "criteria",
        type=str,
        help="filter criteria, name=value separated by commas"
    )
    parse_cmd_dump.add_argument(
        "--format",
        type=str,
        choices=["c", "img-viz"],
        help="format to use for dumping"
    )

    args = parse.parse_args()
    args.func(args)
