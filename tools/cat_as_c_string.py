import sys

with open(sys.argv[1], 'rb') as f:
    s = '"'
    for c in f.read():
        if c == 0x00:
            s += "\\0"
        elif c == 0x09:
            s += "\\t"
        elif c == 0x0A:
            s += "\\n"
        elif c == 0x0D:
            s += "\\r"
        elif c == 0x5C:
            s += "\\\\"
        elif c == 0x22:
            s += "\\\""
        elif c < 0x20 or c >= 0x7F:
            s += "\\x%02X" % c
        else:
            s += chr(c)
    s += '"'
    print(s)
