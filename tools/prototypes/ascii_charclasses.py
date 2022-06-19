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
        return [conv_ord(r[0]), conv_ord(r[1])]
    else:
        return [conv_ord(r), conv_ord(r)]
