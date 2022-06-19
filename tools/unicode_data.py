import argparse
import enum
import io
import itertools
import json
import pathlib
from re import A
import sys
import urllib3
import shutil
import zipfile
import copy
import math

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

class HexSeqType(UcdType):
    def __init__(self, name):
        UcdType.__init__(self, name, "_HexSeq")
    
    def parse(self, s):
        return [int(x, 16) for x in s.strip().split(" ")]

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
        self.uniques.add(s.strip())
        return s.strip()

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

def read_unicode_data(name, fmts):
    db_dir = pathlib.Path(args.db_dir)
    zip_path = db_dir / ("UCD-%s.zip" % args.db_version)
    zip_file = zipfile.ZipFile(zip_path, 'r')
    f = io.TextIOWrapper(zip_file.open(name), "utf-8")
    out = []
    for l in f.readlines():
        comment_loc = l.find("#")
        if comment_loc != -1:
            l = l[:comment_loc]
        if l.strip() == "":
            continue
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
    fmt = [
        HexType("Codepoint"), 
        MiscType("Name"), 
        EnumType("General_Category"), 
        NumType("Canonical_Combining_Class"),
        EnumType("Bidi_Class"),
        DecompTypeMapType("Decomposition_Type", "Decomposition_Mapping")
    ]
    data = read_unicode_data("UnicodeData.txt", fmt)
    #print(read_unicode_data(udt, fmt))
    #print(sorted(list(fmt[5].decomp_type.uniques)))
    filtered = filter_chars(data, args.criteria)
    if args.format == "c":
        dump_set_c(filtered, args)
    elif args.format == "img-viz":
        dump_set_img_viz(filtered, args)
    elif args.format == "c-dzz":
        dump_set_c_dzz(filtered, args)

def cmd_make_prop_data(args):
    fmt = [
        HexType("Codepoint"), 
        MiscType("Name"), 
        EnumType("General_Category"), 
        NumType("Canonical_Combining_Class"),
        EnumType("Bidi_Class"),
        DecompTypeMapType("Decomposition_Type", "Decomposition_Mapping")
    ]
    data = read_unicode_data("UnicodeData.txt", fmt)
    categories = fmt[2].uniques
    out_data = bytearray()
    cat_offset_length = {}
    for cat in sorted(categories):
        matching = filter_chars(data, "General_Category="+cat)
        encoded = encode_property(matching)
        cat_offset_length[cat] = (len(out_data), len(encoded))
        out_data += encoded
    print("re__rune_data_prop_compressed re__rune_data_prop_refs[] = {")
    for cat, (offset, length) in sorted(cat_offset_length.items()):
        print('  {"%s", %i},' % (cat, offset))
    print('  {MN_NULL, 0},')
    print("};")
    print()
    print("mn_uint8 re__rune_data_prop_bytes[] = ")
    out_uint8arr(out_data)
    if args.debug:
        print("len", len(out_data))

def leadnum(l):
    for i, x in enumerate(l):
        if x != l[0]:
            return (l[0],i)
    return (l[0], len(l))

def trailnum(l):
    for i, x in enumerate(reversed(l)):
        if x != l[-1]:
            return (l[-1],i)
    return (l[0], len(l))

def cmd_make_casefold_data(args):
    fmt = [
        HexType("code"),
        EnumType("status"),
        HexSeqType("mapping")
    ]
    data = read_unicode_data("CaseFolding.txt", fmt)
    map_to = [set([x]) for x in range(0x110000)]
    for c in data:
        if c["status"] == "C" or c["status"] == "S":
            assert len(c["mapping"]) == 1
            new_set = map_to[c["mapping"][0]] | set([c["code"]]) | map_to[c["code"]]
            map_to[c["mapping"][0]] = new_set
            map_to[c["code"]] = new_set
    loops = [-1] * 0x110000
    for codep, s in enumerate(map_to):
        if loops[codep] != -1:
            continue
        folds = sorted(list(s))
        for i, fold in enumerate(folds):
            if i < len(folds) - 1:
                next = folds[i+1]
            else:
                next = folds[0]
            loops[fold] = next
    deltas = [l - i for i, l in enumerate(loops)]
    if args.explore:
        try_sizes_new(deltas, args.explore_amt)
    else:
        sizes = [int(x) for x in args.sizes.split(",")]
        render_casefold(sizes, deltas)

def bytewidth(arr, signed=False):
    maxval = max([abs(x) for x in arr])
    if signed:
        if maxval < 128:
            return 1
        elif maxval < 32768:
            return 2
        else:
            return 4
    else:
        if maxval < 256:
            return 1
        elif maxval < 65536:
            return 2
        else:
            return 4

def make_arrays(sizes, deltas, cache):
    max_rune = 0x110000
    arrays = []
    prev_refs = deltas
    prev_size = 1
    i = len(sizes)
    while True:
        if i == 0:
            break
        if tuple(sizes[0:i]) in cache:
            arrays, prev_refs = cache[tuple(sizes[0:i])]
            arrays = copy.deepcopy(arrays)
            prev_refs = copy.deepcopy(prev_refs)
            break
        i -= 1
    while True:
        if i == len(sizes):
            break
        size = sizes[i]
        blocks = [prev_refs[i * size:(i+1)*size] for i in range(len(prev_refs) // size)]
        blocks_uniques = []
        blocks_refs = []
        for block in blocks:
            if block not in blocks_uniques:
                blocks_uniques.append(block)
            blocks_refs.append(blocks_uniques.index(block))
        array, locs = heuristic_squish(blocks_uniques)
        arrays.append(array)
        prev_refs = list([locs[x] for x in blocks_refs])
        i += 1
        cache[tuple(sizes[0:i])] = (copy.deepcopy(arrays), copy.deepcopy(prev_refs))
    arrays.append(prev_refs)
    return arrays

def try_sizes_new(deltas, nsize):
    import math
    max_rune = 0x110000
    max_bits = (max_rune).bit_length()
    pow2 = list([2**x for x in range(1, max_bits)])
    cache = {}
    outs = []
    for szi in itertools.product(*([pow2]*nsize)):
        sizes = list(szi)
        if math.prod(sizes) > max_rune:
            continue
        arrays = make_arrays(sizes, deltas, cache)
        uncomp_size = 0
        dzz_size = 0
        arrsize = []
        for i, arr in enumerate(arrays):
            arrsize.append(len(arr) * bytewidth(arr, signed = i == 0))
            uncomp_size += arrsize[-1]
            dzz_size += len(dzz_encode(arr))
        out_csv = [uncomp_size, dzz_size] + arrsize + [len(sizes)] + sizes
        outs.append([uncomp_size, sizes])
        s = ",".join([str(x) for x in out_csv])
        print(s)
        sys.stderr.write(s+"\n")
    lowest = sorted(outs)[0]
    sys.stderr.write("lowest " + (",".join([str(x) for x in lowest])+"\n"))

def render_casefold(sizes, deltas):
    arrays = make_arrays(sizes, deltas, {})
    for i, (array, size) in enumerate(zip(arrays, sizes + [0x110000 // math.prod(sizes)])):
        bitwidth = bytewidth(array, signed = i == 0) * 8
        inttype = "uint" if i != 0 else "int"
        print(f"MN_INTERNAL_DATA mn_{inttype}{bitwidth} re__rune_data_casefold_{i}[] = ")
        digs = int(math.ceil(math.log(max([abs(x) for x in array]), 16)))
        out_intarr(array, digs)
        print()
    print("MN_INTERNAL re_rune re__rune_data_casefold_next(re_rune r) {")
    print("  return ")
    shifts_masks = []
    for i, size in enumerate(sizes):
        bl = math.prod(sizes[0:i]).bit_length()
        shifts_masks.append((bl - 1, size - 1))
    shifts_masks.append((math.prod(sizes).bit_length() - 1, -1))
    for i in range(len(sizes) + 1):
        print(f"re__rune_data_casefold_{i}[")
    for shift_amt, mask in reversed(shifts_masks):
        shift_expr = f"r >> {shift_amt}" if shift_amt != 0 else "r"
        if mask != -1:
            print(f"+ (({shift_expr}) & 0x{mask:02X})]")
        else:
            print(f"{shift_expr}]")
    print(";")
    print("}")

def try_sizes(deltas):
    max_rune = 0x110000
    pow2 = list([2**x for x in range(1, 21)])
    for size_a in pow2:
        for size_b in pow2:
            if size_a * size_b > max_rune or size_b <= size_a:
                continue
            size_c = max_rune // size_b
            blocks_a = [deltas[i * size_a:(i+1)*size_a] for i in range(max_rune // size_a)]
            blocks_a_uniques = []
            blocks_a_refs = []
            for block in blocks_a:
                if block not in blocks_a_uniques:
                    blocks_a_uniques.append(block)
                blocks_a_refs.append(blocks_a_uniques.index(block))
            a_array, a_locs = heuristic_squish(blocks_a_uniques)
            a_per_b = size_b // size_a
            mapped_refs = [a_locs[x] for x in blocks_a_refs]
            blocks_b = [mapped_refs[i * a_per_b:(i+1)*a_per_b] for i in range(max_rune // size_b)]
            blocks_b_uniques = []
            blocks_b_refs = []
            for block in blocks_b:
                if block not in blocks_b_uniques:
                    blocks_b_uniques.append(block)
                blocks_b_refs.append(blocks_b_uniques.index(block))
            b_array, b_locs = heuristic_squish(blocks_b_uniques)
            c_array = [b_locs[x] for x in blocks_b_refs]
            uncomp_size = len(a_array) * bytewidth(max(deltas)) + len(b_array) * bytewidth(max(b_array)) \
                + len(c_array) * bytewidth(max(c_array))
            dzz_size = len(dzz_encode(a_array)) + len(dzz_encode(b_array)) \
                + len(dzz_encode(c_array))
            print("A: %8i B: %8i C: %8i Uncompressed: %8i Compressed: %8i" % (size_a, size_b, size_c, uncomp_size, dzz_size))

def arraydat(arr):
    return f"len: {len(arr)}, bytewidth: {bytewidth(max(arr))}, leadzero: {leadzeros(arr)}, trailzero: {trailzeros(arr)}"

def perform_squish(arrangement, leads, trails, blocks):
    out = []
    locs = []
    lastidx = None
    for idx in arrangement:
        if lastidx == None:
            out.extend(blocks[idx])
            locs.append(0)
        else:
            trail = trails[lastidx]
            lead = leads[idx]
            if trail[0] == lead[0]:
                moveup = min(trail[1], lead[1])
            else:
                moveup = 0
            locs.append(len(out) - moveup)
            out.extend(blocks[idx][moveup:])
        lastidx = idx
    return out, locs

def squish_factor(leads, trails, arrangement):
    fac = 0
    for i in range(1,len(arrangement)):
        if trails[arrangement[i]][0] == leads[arrangement[i]][0]:
            fac += min(trails[arrangement[i-1]][1], leads[arrangement[i]][1])
    return fac

def find_best_pair(leads, trails):
    maxsq = -1
    maxsqidx = None
    for i, t in enumerate(trails):
        for j, l in enumerate(leads):
            if j == i:
                continue
            sq = squish_factor(leads, trails, (i, j))
            if sq > maxsq:
                maxsq = sq
                maxsqidx = (i, j)
    return maxsqidx

def find_best_prepend(leads, trails, arrangement):
    maxsq = -1
    maxsqidx = None
    for i in range(len(trails)):
        if i in arrangement:
            continue
        sq = squish_factor(leads, trails, [i] + arrangement)
        if sq > maxsq:
            maxsq = sq
            maxsqidx = i
    return (maxsqidx, maxsq)

def find_best_append(leads, trails, arrangement):
    maxsq = -1
    maxsqidx = None
    for i in range(len(leads)):
        if i in arrangement:
            continue
        sq = squish_factor(leads, trails, arrangement + [i])
        if sq > maxsq:
            maxsq = sq
            maxsqidx = i
    return (maxsqidx, maxsq)

def heuristic_squish_loop(leads, trails, workingset):
    if len(workingset) == len(leads):
        return False
    # find max index
    if len(leads) == 1:
        workingset.append(0)
        return False
    if len(workingset) == 0:
        trailidx, leadidx = find_best_pair(leads, trails)
        workingset.append(trailidx)
        workingset.append(leadidx)
       # print("START", trailidx, leadidx)
    else:
        # check whether to prepend or append
        trailidx, trailsq = find_best_prepend(leads, trails, workingset)
        leadidx, leadsq = find_best_append(leads, trails, workingset)
        if leadsq < trailsq:
            workingset.insert(0, trailidx)
            #print("PREPEND", trailidx)
        else:
            workingset.append(leadidx)
            #print("APPEND", leadidx)
    return True

def heuristic_squish_slice(leads, trails, arrangement):
    while True:
        fac = squish_factor(leads, trails, arrangement)
        maxsq = -1
        maxsqidx = -1
        maxslice = []
        for slice_idx in range(1, len(leads)):
            sliced = arrangement[slice_idx:] + arrangement[:slice_idx]
            sq = squish_factor(leads, trails, sliced)
            if sq > maxsq:
                maxsq = sq
                maxsqidx = slice_idx
                maxslice = sliced
        if maxsq > fac:
            arrangement = maxslice
            #print("SLICE", maxsqidx)
        else:
            break
    return arrangement

def heuristic_squish_swap(leads, trails, arrangement):
    while True:
        fac = squish_factor(leads, trails, arrangement)
        maxsq = -1
        maxswap = []
        maxswapidx = -1
        for src_idx in range(len(leads)):
            for dst_idx in range(len(trails)):
                swapped = arrangement.copy()
                src = swapped[src_idx]
                swapped[src_idx] = swapped[dst_idx]
                swapped[dst_idx] = src
                sq = squish_factor(leads, trails, swapped)
                if sq > maxsq:
                    maxsq = sq
                    maxswap = swapped
                    maxswapidx = (src_idx, dst_idx)
        if maxsq > fac:
            arrangement = maxswap
            #print("SWAP", maxswapidx[0], maxswapidx[1])
        else:
            break
    return arrangement

def heuristic_squish(blocks):
    leads = list(map(leadnum, blocks))
    trails = list(map(trailnum, blocks))
    arrangement = []
    while heuristic_squish_loop(leads, trails, arrangement):
        continue
    arrangement = heuristic_squish_slice(leads, trails, arrangement)
    arrangement = heuristic_squish_swap(leads, trails, arrangement)
    array, locs = perform_squish(arrangement, leads, trails, blocks)
    locs = map(lambda a: a[1], sorted(zip(arrangement, locs), key=lambda f: f[0]))
    return (array, list(locs))

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

def dump_set_c(matching_chars, args):
    print("{")
    line = "    "
    l = 0
    groups = group_by(matching_chars)
    for start, end in groups:
        next_part = "{0x%X,0x%X}," % (start, end)
        l += 2
        if len(line) + len(next_part) >= 80:
            print(line)
            line = "    "
        line += next_part
    if line != "    ":
        print(line)
    print("}")
    if args.debug:
        print("len %i" % l)

def decompose_groups(groups):
    for r in groups:
        yield r[0]
        yield r[1]

def encode_property(matching_chars):
    array_out = bytearray()
    groups = list(group_by(matching_chars))
    #print("groups_len=",len(groups),"chars_len=",len(matching_chars))
    #print(list(decompose_groups(groups)))
    uleb128(array_out, len(groups))
    array_out += dzz_encode(list(decompose_groups(groups)))
    return array_out

def uleb128(array_out, value):
    while True:
        masked = value & 0x7F
        value >>= 7
        if value:
            array_out.append(masked | 0x80)
        else:
            array_out.append(masked)
            break

def rle_encode(array_in):
    array_out = bytearray()
    for k, v in itertools.groupby(array_in):
        uleb128(array_out, len(list(v)))
        array_out.append(k)
    return array_out

def dzz_encode(array_in):
    def zig(value):
        mask = (2 << value.bit_length()) - 1
        return ((value & (mask >> 1)) << 1) ^ (mask if value < 0 else 0)
    def zig_uleb128(array_out, value):
        uleb128(array_out, zig(value))
    array_out = bytearray()
    previous = 0
    for ch in array_in:
        zig_uleb128(array_out, ch - previous)
        previous = ch
    return array_out

def out_uint8arr(arr_in):
    out_intarr(arr_in, 2)

def out_intarr(arr_in, dec):
    print("{",end="\n  ")
    for i, ch in enumerate(arr_in):
        sgn = "-" if ch < 0 else " "
        print(f"{sgn}0x%0{str(dec)}X," % abs(ch), end="")
        if i % 16 == 15:
            print("\n  ",end="")
    print("\n};")

def dump_set_c_dzz(matching_chars, args):
    # https://justine.lol/sizetricks/
    array_out = dzz_encode(matching_chars)
    out_uint8arr(array_out)
    if args.debug:
        print("len %i" % len(array_out))

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
    parse.add_argument(
        "--debug",
        action="store_const",
        const=True,
        default=False,
        help="show debug info"
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
        choices=["c", "c-dzz", "img-viz"],
        help="format to use for dumping"
    )

    parse_cmd_dump_vals = parse_cmds.add_parser(
        "make_prop_data",
        help="make property data arrays"
    )
    parse_cmd_dump_vals.set_defaults(func=cmd_make_prop_data)

    parse_cmd_make_casefold = parse_cmds.add_parser(
        "make_casefold_data",
        help="make casefolding data"
    )
    parse_cmd_make_casefold.set_defaults(func=cmd_make_casefold_data)
    parse_cmd_make_casefold.add_argument("--explore",action="store_true")
    parse_cmd_make_casefold.add_argument("--explore-amt",type=int)
    parse_cmd_make_casefold.add_argument("--sizes",type=str)

    args = parse.parse_args()
    args.func(args)
