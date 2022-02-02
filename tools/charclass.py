ranges = []

RUNE_MAX = 1 << 32

# output all ranges
def ranges_normalize_v0(ranges: list):
    ranges.sort(key=lambda r: r[0])
    min = -1
    max = -1
    out = []
    for r in ranges:
        assert r[0] < r[1]
        if min == -1:
            min = r[0]
            max = r[1]
        elif r[0] < max:
            if r[1] > max:
                # Extend range
                max = r[1]
            # Otherwise do nothing.
        elif r[0] == max:
            # Concatenate range.
            max = r[1]
        elif r[0] >= max:
            # New range.
            out.append([min, max])
            min = r[0]
            max = r[1]
    if min == -1 and max == -1:
        raise Exception("empty class")
    else:
        out.append([min, max])
    if len(out) == 0:
        raise Exception("empty class")
    return out

# print("Single", ranges_normalize_v0([[0, 5]]))
# print("Two non-overlapping", ranges_normalize_v0([[0, 10], [12, 15]]))
# print("Three non-overlapping", ranges_normalize_v0([[0, 10], [12, 15], [25, 30]]))
# print("Two overlapping", ranges_normalize_v0([[5, 10], [7, 15]]))
# print("Three overlapping", ranges_normalize_v0([[5, 10], [7, 15], [14, 25]]))
# print("Three with two overlapping", ranges_normalize_v0([[5, 10], [11, 15], [14, 20]]))
# print("Two next to each other", ranges_normalize_v0([[5, 10], [10, 15]]))
# print("Three next to each other", ranges_normalize_v0([[5, 10], [10, 15], [15, 20]]))
# print("Two with same start", ranges_normalize_v0([[5, 10], [5, 10]]))
# print("Two with same start and different ends", ranges_normalize_v0([[5, 10], [5, 15]]))
# print("Two with one encompassing the other", ranges_normalize_v0([[5, 10], [6, 7]]))

# output all ranges, optionally inverting
def ranges_normalize_v1(ranges: list, invert=False):
    ranges.sort(key=lambda r: r[0])
    min = -1
    max = -1
    last_max = -1
    out = []
    for r in ranges:
        assert r[0] < r[1]
        if min == -1:
            min = r[0]
            max = r[1]
        elif r[0] < max:
            if r[1] > max:
                # Extend range
                max = r[1]
            # Otherwise do nothing.
        elif r[0] == max:
            # Concatenate range.
            max = r[1]
        elif r[0] >= max:
            # New range.
            if not invert:
                out.append([min, max])
            else:
                if last_max == -1:
                    start = 0
                    end = min
                else:
                    start = last_max
                    end = min
                assert start <= end
                if start < end:
                    out.append([start, end])
                    last_max = max
            min = r[0]
            max = r[1]
    if min == -1 and max == -1:
        raise Exception("empty class")
    else:
        if not invert:
            out.append([min, max])
        else:
            if last_max == -1:
                start = 0
                end = min
            else:
                start = last_max
                end = min
            assert start <= end
            if start < end:
                out.append([start, end])
            if max < RUNE_MAX:
                out.append([max, RUNE_MAX])
    if len(out) == 0:
        raise Exception("empty class")
    return out

# print("Single", ranges_normalize_v1([[0, 5]], invert=True))
# print("Two non-overlapping", ranges_normalize_v1([[0, 10], [12, 15]], invert=True))
# print("Three non-overlapping", ranges_normalize_v1([[0, 10], [12, 15], [25, 30]], invert=True))
# print("Two overlapping", ranges_normalize_v1([[5, 10], [7, 15]], invert=True))
# print("Three overlapping", ranges_normalize_v1([[5, 10], [7, 15], [14, 25]], invert=True))
# print("Three with two overlapping", ranges_normalize_v1([[5, 10], [11, 15], [14, 20]], invert=True))
# print("Two next to each other", ranges_normalize_v1([[5, 10], [10, 15]], invert=True))
# print("Three next to each other", ranges_normalize_v1([[5, 10], [10, 15], [15, 20]], invert=True))
# print("Two with same start", ranges_normalize_v1([[5, 10], [5, 10]], invert=True))
# print("Two with same start and different ends", ranges_normalize_v1([[5, 10], [5, 15]], invert=True))
# print("Two with one encompassing the other", ranges_normalize_v1([[5, 10], [6, 7]], invert=True))
# print("Two with same start and different ends II", ranges_normalize_v1([[5, 10], [5, 9]]))

ASCII_CHARCLASSES = {
    "alnum": [['0', '9'], ['A', 'Z'], ['a', 'z']],
    "alpha": [['A', 'Z'], ['a', 'z']],
    "ascii": [[0, 0x7F]],
    "blank": ['\t', ' '],
    "cntrl": [[0, 0x1F], 0x7F],
    "digit": [['0', '9']],
    "graph": [[0x21, 0x7E]],
    "lower": [['a', 'z']],
    "print": [[0x20, 0x7E]],
    "punct": [[0x21, 0x2F], [0x3A, 0x40], [0x5B, 0x60], [0x7B, 0x7E]],
    "space": [0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x20],
    "perl_space": [0x09, 0x0A, 0x0C, 0x0D, 0x20],
    "upper": [['A', 'Z']],
    "word": [['0', '9'], ['A', 'Z'], ['a', 'z'], '_'],
    "xdigit": [['0', '9'], ['A', 'F'], ['a', 'f']]
}

def conv_ord(ch):
    if isinstance(ch, str):
        return ord(ch)
    elif isinstance(ch, int):
        return ch

def conv_range(r):
    if isinstance(r, list):
        return [conv_ord(r[0]), conv_ord(r[1]) + 1]
    else:
        return [conv_ord(r), conv_ord(r) + 1]

def encode_ranges(rs):
    out = []
    for r in rs:
        assert r[0] < 128
        out.append(r[0])
        out.append(r[1] - 1)
    return out

print("typedef enum re__charclass_ascii_type {")
for i, (cc, rs) in enumerate(sorted(ASCII_CHARCLASSES.items(), key=lambda k: k[0])):
    print("    RE__CHARCLASS_ASCII_TYPE_%s%s" % (cc.upper(), "," if i != len(ASCII_CHARCLASSES)-1 else ""))
print("} re__charclass_ascii_type;")

for i, (cc, rs) in enumerate(sorted(ASCII_CHARCLASSES.items(), key=lambda k: k[0])):
    ranges = [conv_range(r) for r in rs]
    norm = ranges_normalize_v1(ranges)
    encoded = encode_ranges(norm)
    arr = '(unsigned char*)"'+ "".join(["\\x%02X" % b for b in encoded]) + '"'
    print("{%i, %s, %i, %s}%s" % (len(cc), '"' + cc + '"', len(encoded) / 2, arr,  "," if i != len(ASCII_CHARCLASSES)-1 else ""))

