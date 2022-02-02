class Inst:
    def __init__(self):
        self.next = 0
    
    def set_next(self, next):
        self.next = next

class Split(Inst):
    def __init__(self):
        self.next1 = 0
        Inst.__init__(self)
    
    def set_next1(self, next1):
        self.next1 = next1

class Byte(Inst):
    def __init__(self, val):
        assert val <= 255
        self.val = val
        Inst.__init__(self)

class Any(Inst):
    def __init__(self):
        Inst.__init__(self)

class Range(Inst):
    def __init__(self, val_lo, val_hi):
        assert val_lo <= val_hi
        assert val_lo >= 0
        assert val_hi <= 255
        self.val_lo = val_lo
        self.val_hi = val_hi
        Inst.__init__(self)

class Match(Inst):
    def __init__(self):
        Inst.__init__(self)

import string

def printable(ch):
    if chr(ch) in string.printable:
        return "0x%02X ('%s')" % (ch, chr(ch))
    else:
        return "0x%02X" % ch

def dump_program(prog):
    for pc, inst in enumerate(prog):
        print("0x%02X | " % pc, end="")
        if isinstance(inst, Split):
            print("SPLIT -> 0x%02X, 0x%02X" % (inst.next, inst.next1))
        elif isinstance(inst, Byte):
            print("BYTE %s -> 0x%02X" % (printable(inst.val), inst.next))
        elif isinstance(inst, Any):
            print("ANY -> 0x%02X" % inst.next)
        elif isinstance(inst, Match):
            print("MATCH!")
        elif isinstance(inst, Range):
            print("RANGE %s - %s -> 0x%02X" % (printable(inst.val_lo), printable(inst.val_hi), inst.next))

CONT_ONE_MIN = 0x00
CONT_ONE_MAX = 0x3F
CONT_ONE_MASK = 0x3F
CONT_TWO_MIN = 0x00
CONT_TWO_MAX = 0xFFF
CONT_TWO_MASK = 0xFFF
CONT_THREE_MIN = 0x00
CONT_THREE_MAX = 0x3FFFF
CONT_THREE_MASK = 0x3FFFF
CONT_TOP = 0b10000000
ONE_BYTE_MIN = 0x00
ONE_BYTE_MAX = 0x7F
TWO_BYTE_MIN = 0x80
TWO_BYTE_MAX = 0x7FF
TWO_BYTE_TOP = 0b11000000
THREE_BYTE_MIN = 0x800
THREE_BYTE_MAX = 0xFFFF
THREE_BYTE_TOP = 0b11100000
FOUR_BYTE_MIN = 0x10000
FOUR_BYTE_MAX = 0x1FFFFF
FOUR_BYTE_TOP = 0b11110000

def add(prog, inst):
    prog.append(inst)

def link(prog, origin=None, secondary=False):
    src = prog[-1] if origin is None else prog[origin]
    func = src.set_next if not secondary else src.set_next1
    func(len(prog))

def label(prog):
    return len(prog)

def comp_one(prog, lo, hi):
    # 0xxxxxxx
    assert lo <= hi
    assert lo >= ONE_BYTE_MIN
    assert hi <= ONE_BYTE_MAX
    if lo == hi - 1:
        add(Byte(lo))
    else:
        add(Range(lo, hi))
    link(prog)
    add(Match())

def comp_cont_one(prog, lo, hi):
    # 10xxxxxx
    assert lo <= hi
    assert lo >= CONT_ONE_MIN
    assert hi <= CONT_ONE_MAX
    add(Range(lo | CONT_TOP, hi | CONT_TOP))
    link(prog)
    add(Match())

def comp_two(prog, lo, hi):
    # 110yyyyy 10xxxxxx
    assert lo <= hi
    assert lo >= TWO_BYTE_MIN
    assert hi <= TWO_BYTE_MAX
    lo_y = lo >> 6
    hi_y = hi >> 6
    lo_x = lo & CONT_ONE_MASK
    hi_x = hi & CONT_ONE_MASK
    if lo_y == hi_y:
        add(prog, Byte(lo_y | TWO_BYTE_TOP))
        link(prog)
        comp_cont_one(prog, lo_x, hi_x)
    elif lo_y == hi_y - 1:
        first_split = label(prog)
        add(prog, Split())
        link(prog)
        add(prog, Byte(lo_y | TWO_BYTE_TOP))
        link(prog)
        comp_cont_one(prog, lo_x, CONT_ONE_MAX)

        link(prog, origin=first_split, secondary=True)
        add(prog, Byte(hi_y | TWO_BYTE_TOP))
        link(prog)
        comp_cont_one(prog, 0, hi_x)
    elif lo_y < hi_y:
        first_split = label(prog)
        add(prog, Split())
        link(prog)
        add(prog, Byte(lo_y | TWO_BYTE_TOP))
        link(prog)
        comp_cont_one(prog, lo_x, CONT_ONE_MAX)

        second_split = label(prog)
        link(prog, origin=first_split, secondary=True)
        add(prog, Split())
        link(prog)
        add(prog, Range((lo_y + 1) | TWO_BYTE_TOP, (hi_y - 1) | TWO_BYTE_TOP))
        link(prog)
        comp_cont_one(prog, CONT_ONE_MIN, CONT_ONE_MAX)

        link(prog, origin=second_split, secondary=True)
        add(prog, Byte(hi_y | TWO_BYTE_TOP))
        link(prog)
        comp_cont_one(prog, CONT_ONE_MIN, hi_x)

def comp_three(prog, lo, hi):
    # 1110zzzz 10yyyyyy 10xxxxxx
    assert lo <= hi
    assert lo >= THREE_BYTE_MIN
    assert hi <= THREE_BYTE_MAX
    lo_z = lo >> 12
    hi_z = hi >> 12
    lo_yx = lo & CONT_TWO_MASK
    hi_yx = hi & CONT_TWO_MASK
    if lo_z == hi_z:
        add(prog, Byte(lo_z | THREE_BYTE_TOP))
        link(prog)
        comp_cont_two(prog, lo_yx, hi_yx)
    elif lo_z == hi_z - 1:
        first_split = label(prog)
        add(prog, Split())
        link(prog)
        add(prog, Byte(lo_z | THREE_BYTE_TOP))
        link(prog)
        comp_cont_two(prog, lo_yx, CONT_TWO_MAX)

        link(prog, origin=first_split, secondary=True)
        add(prog, Byte(hi_z | THREE_BYTE_TOP))
        link(prog)
        comp_cont_two(prog, 0, hi_yx)
    elif lo_z < hi_z:
        first_split = label(prog)
        add(prog, Split())
        link(prog)
        add(prog, Byte(lo_z | THREE_BYTE_TOP))
        link(prog)
        comp_cont_two(prog, lo_z, CONT_TWO_MAX)

        second_split = label(prog)
        link(prog, origin=first_split, secondary=True)
        add(prog, Split())
        link(prog)
        add(prog, Range((lo_z + 1) | THREE_BYTE_TOP, (hi_z - 1) | THREE_BYTE_TOP))
        link(prog)
        comp_cont_two(prog, CONT_TWO_MIN, CONT_TWO_MAX)

        link(prog, origin=second_split, secondary=True)
        add(prog, Byte(hi_z | THREE_BYTE_TOP))
        link(prog)
        comp_cont_two(prog, CONT_TWO_MIN, hi_yx)

def comp_range(lo, hi):
    prog = []
    assert lo <= hi
    assert lo >= 0
    assert lo <= TWO_BYTE_MAX
    assert hi <= TWO_BYTE_MAX
    min = -1
    max = -1
    num_bytes = 0
    last_split = 0
    if lo <= ONE_BYTE_MAX:
        min = lo
        if hi <= ONE_BYTE_MAX:
            max = hi
        else:
            max = ONE_BYTE_MAX
            lo = TWO_BYTE_MIN
        num_bytes = 1
    if lo >= TWO_BYTE_MIN and lo <= TWO_BYTE_MAX:
        if min != -1: # already made range
            last_split = label(prog)
            add(prog, Split())
            link(prog)
            comp_one(prog, min, max)
        min = lo
        if hi <= TWO_BYTE_MAX:
            max = hi
        else:
            max = TWO_BYTE_MAX
            lo = THREE_BYTE_MIN
        num_bytes = 2
    if lo >= THREE_BYTE_MIN and lo <= THREE_BYTE_MAX:
        if min != -1:
            if last_split != 0:
                link(prog, origin=last_split, secondary=True)
            last_split = label(prog)
            add(prog, Split())
            link(prog)
            comp_two(prog, min, max)
        min = lo
        if hi <= THREE_BYTE_MAX:
            max = hi
        else:
            max = THREE_BYTE_MAX
            lo = FOUR_BYTE_MIN
        num_bytes = 3
    if lo >= FOUR_BYTE_MIN and lo <= FOUR_BYTE_MAX:
        if min != -1:
            if last_split != 0:
                link(prog, origin=last_split, secondary=True)
            last_split = label(prog)
            add(prog, Split())
            link(prog)
            comp_three(prog, min, max)
        min = lo
        if hi <= FOUR_BYTE_MAX:
            max = hi
        else:
            max = FOUR_BYTE_MAX
    if min != -1:
        if last_split != 0:
            link(prog, origin=last_split, secondary=True)
        [comp_one, comp_two, comp_three, comp_four][num_bytes - 1](prog, min, max)

def follow(prog, thrds, new_thrds=[]):
    for thrd in thrds:
        inst = prog[thrd]
        if isinstance(inst, Split):
            if inst.next not in new_thrds:
                follow(prog, [inst.next], new_thrds)
            if inst.next1 not in new_thrds:
                follow(prog, [inst.next1], new_thrds)
        else:
            new_thrds.append(thrd)
    return new_thrds

def run_match(prog, str):
    thrds = [0]
    for ch in str.encode("utf-8"):
        new_thrds = []
        thrds = follow(prog, thrds)
        for thrd in thrds:
            inst = prog[thrd]
            if isinstance(inst, Byte):
                if ch == inst.val:
                    new_thrds.append(inst.next)
            elif isinstance(inst, Range):
                if ch >= inst.val_lo and ch <= inst.val_hi:
                    new_thrds.append(inst.next)
        thrds = new_thrds
    return any([isinstance(prog[pc], Match) for pc in thrds])

def test_range(lo, hi):
    prog = comp_range(lo, hi)
    for i in range(lo, hi+1):
        print("Test %s-%s: %s" % tuple(hex(n) for n in [lo, hi, i]),end="\r")
        if not run_match(prog, chr(i)):
            print()
            print("String: %s" % repr(chr(i).encode("utf-8")))
            dump_program(prog)
            break
    print()

test_range(0x80, 0x7FF)
dump_program(comp_range(0x80, 0x7FF))

#print(run_match(comp_range(0x80, 0x7FF), chr(0xC0)))
