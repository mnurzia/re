# unicode range compiler
# will combine this with a char class compiler quite soon

class Inst:
    def __init__(self):
        self.next = 0
    
    def set_next(self, next):
        self.next = next

class Split(Inst):
    def __init__(self):
        self.next1 = 0
        Inst.__init__(self)
    
    def __str__(self):
        return "SPLIT -> 0x%02X, 0x%02X" % (self.next, self.next1)

    def set_next1(self, next1):
        self.next1 = next1

class Byte(Inst):
    def __init__(self, val):
        assert val <= 255
        self.val = val
        Inst.__init__(self)
    
    def __str__(self):
        return "BYTE %s -> 0x%02X" % (printable(self.val), self.next)

class Any(Inst):
    def __init__(self):
        Inst.__init__(self)
    
    def __str__(self):
        return "ANY -> 0x%02X" % self.next

class Range(Inst):
    def __init__(self, val_lo, val_hi):
        assert val_lo != val_hi
        assert val_lo <= val_hi
        assert val_lo >= 0
        assert val_hi <= 255
        self.val_lo = val_lo
        self.val_hi = val_hi
        Inst.__init__(self)
    
    def __str__(self):
        return "RANGE %s - %s -> 0x%02X" % (printable(self.val_lo), printable(self.val_hi), self.next)

class Match(Inst):
    def __init__(self):
        Inst.__init__(self)
    
    def __str__(self):
        return "MATCH!"

class Fail(Inst):
    def __init__(self):
        Inst.__init__(self)
    
    def __str__(self):
        return "FAIL"

import string
from typing import List

def printable(ch):
    if chr(ch) in string.printable:
        return "0x%02X ('%s')" % (ch, chr(ch))
    else:
        return "0x%02X" % ch

def dump_program(prog):
    for pc, inst in enumerate(prog):
        print("0x%02X | " % pc, end="")
        print(str(inst))

def add(prog, inst):
    prog.append(inst)

def link(prog, origin=None, dest=None, secondary=False):
    src = prog[-1] if origin is None else prog[origin]
    dst = len(prog) if dest is None else dest
    func = src.set_next if not secondary else src.set_next1
    func(dst)

def label(prog):
    return len(prog)

def cache_key(lo, hi, x_bits, y_bits):
    return lo | (hi << 21) | (x_bits << 42) | (y_bits << 63)

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
FOUR_BYTE_BITS = 21
FOUR_BYTE_TOP = 0b11110000


# Caching is DEFINITELY "worth it."
# Gives us a very substantial size reduction.
def comp_generic_cached(prog, lo, hi, x_bits, y_bits, cache={}):
    # 110yyyyy 10xxxxxx
    key = cache_key(lo, hi, x_bits, y_bits)
    if key in cache:
        return cache[key]
    max_val = (1 << (y_bits + x_bits)) - 1
    assert lo <= hi
    assert lo >= 0
    assert hi <= max_val
    mask = (0b11111110 << y_bits) & 0b11111111
    begin = label(prog)
    cache[key] = begin
    if x_bits == 0:
        if lo == hi:
            add(prog, Byte(lo | mask))
        else:
            add(prog, Range(lo | mask, hi | mask))
        link(prog)
        add(prog, Match())
    else:
        x_mask = ((1 << x_bits) - 1)
        lo_y = lo >> x_bits
        hi_y = hi >> x_bits
        lo_x = lo & x_mask
        hi_x = hi & x_mask
        if lo_y == hi_y:
            linkfrom = label(prog)
            add(prog, Byte(lo_y | mask))
            linkto = comp_generic_cached(prog, lo_x, hi_x, x_bits - 6, 6, cache)
            link(prog, origin=linkfrom, dest=linkto)
        else:
            if lo_x == 0 and hi_x == x_mask:
                # if these are maximal, we can do away with the partial ranges
                linkfrom = label(prog)
                add(prog, Range(lo_y | mask, hi_y | mask))
                linkto = comp_generic_cached(prog, 0, x_mask, x_bits - 6, 6, cache)
                link(prog, origin=linkfrom, dest=linkto)
            else:
                if lo_y == hi_y - 1:
                    spl_loc = label(prog)
                    add(prog, Split())
                    link(prog)
                    linkfrom = label(prog)
                    add(prog, Byte(lo_y | mask))
                    linkto = comp_generic_cached(prog, lo_x, x_mask, x_bits - 6, 6, cache)
                    link(prog, origin=linkfrom, dest=linkto)

                    link(prog, origin=spl_loc, secondary=True)
                    linkfrom = label(prog)
                    add(prog, Byte(hi_y | mask))
                    linkto = comp_generic_cached(prog, 0, hi_x, x_bits - 6, 6, cache)
                    link(prog, origin=linkfrom, dest=linkto)
                elif lo_y < hi_y:
                    spl_loc_0 = label(prog)
                    add(prog, Split())
                    link(prog)
                    linkfrom = label(prog)
                    add(prog, Byte(lo_y | mask))
                    linkto = comp_generic_cached(prog, lo_x, x_mask, x_bits - 6, 6, cache)
                    link(prog, origin=linkfrom, dest=linkto)

                    link(prog, origin=spl_loc_0, secondary=True)
                    if x_mask == hi_x:
                        # can further optimize if x_mask == hi_x
                        linkfrom = label(prog)
                        add(prog, Range((lo_y + 1) | mask, hi_y | mask))
                        linkto = comp_generic_cached(prog, 0, hi_x, x_bits - 6, 6, cache)
                        link(prog, origin=linkfrom, dest=linkto)
                    else:
                        spl_loc_1 = label(prog)
                        add(prog, Split())
                        link(prog)
                        linkfrom = label(prog)
                        if lo_y + 1 == hi_y - 1:
                            add(prog, Byte((lo_y + 1) | mask))
                        else:
                            add(prog, Range((lo_y + 1) | mask, (hi_y - 1) | mask))
                        linkto = comp_generic_cached(prog, 0, x_mask, x_bits - 6, 6, cache)
                        link(prog, origin=linkfrom, dest=linkto)

                        link(prog, origin=spl_loc_1, secondary=True)
                        linkfrom = label(prog)
                        add(prog, Byte(hi_y | mask))
                        linkto = comp_generic_cached(prog, 0, hi_x, x_bits - 6, 6, cache)
                        link(prog, origin=linkfrom, dest=linkto)
    return begin

def comp_range_cached(prog, cache, lo, hi):
    assert lo <= hi
    assert lo >= 0
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
            comp_generic_cached(prog, min, max, 0, 7, cache)
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
            comp_generic_cached(prog, min, max, 6, 5, cache)
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
            comp_generic_cached(prog, min, max, 12, 4, cache)
        min = lo
        if hi <= FOUR_BYTE_MAX:
            max = hi
        else:
            max = FOUR_BYTE_MAX
        num_bytes = 4
    if min != -1:
        if last_split != 0:
            link(prog, origin=last_split, secondary=True)
        if num_bytes == 1:
            comp_generic_cached(prog, min, max, 0, 7, cache)
        elif num_bytes == 2:
            comp_generic_cached(prog, min, max, 6, 5, cache)
        elif num_bytes == 3:
            comp_generic_cached(prog, min, max, 12, 4, cache)
        elif num_bytes == 4:
            comp_generic_cached(prog, min, max, 18, 3, cache)

def require_normalization(ranges):
    prev_lo = -1
    prev_hi = -1
    for range in ranges:
        if range[0] >= range[1]:
            raise Exception("range not normalized")
        if prev_lo == -1:
            prev_lo = range[0]
            prev_hi = range[1]
        else:
            if range[0] <= prev_hi:
                raise Exception("range not normalized")
            else:
                prev_lo = range[0]
                prev_hi = range[1]

def comp_ranges(ranges):
    prog = []
    cache = {}
    add(prog, Fail())
    last_split = None
    require_normalization(ranges)
    for i, range in enumerate(ranges):
        if last_split is not None:
            link(prog, origin=last_split, secondary=True)
        if i != len(ranges) - 1:
            last_split = label(prog)
            add(prog, Split())
            link(prog)
        comp_range_cached(prog, cache, range[0], range[1] - 1)
    return prog
        

def follow(prog, start, new_thrds):
    inst = prog[start]
    if isinstance(inst, Split):
        if inst.next not in new_thrds:
            new_thrds.append(inst.next)
            follow(prog, inst.next, new_thrds)
        if inst.next1 not in new_thrds:
            new_thrds.append(inst.next1)
            follow(prog, inst.next1, new_thrds)
    else:
        if inst not in new_thrds:
            new_thrds.append(start)

def run_match(prog, str):
    thrds = []
    follow(prog, 1, thrds)
    for ch in str.encode("utf-8",'surrogatepass'):
        new_thrds = []
        for thrd in thrds:
            inst = prog[thrd]
            if isinstance(inst, Byte):
                if ch == inst.val:
                    new_thrds.append(inst.next)
            elif isinstance(inst, Range):
                if ch >= inst.val_lo and ch <= inst.val_hi:
                    new_thrds.append(inst.next)
        thrds = []
        for new in new_thrds:
            follow(prog, new, thrds)
    return any([isinstance(prog[pc], Match) for pc in thrds])

import time

def test_range(clo, chi, lo, hi):
    prog = comp_range_cached(clo, chi)
    times = []
    dump_program(prog)
    for i in range(lo, hi+1):
        print("Test %s-%s: %s" % tuple(hex(n) for n in [lo, hchild_start_idx, child_end_idx]),end="\r")
        start = time.time_ns()
        res = run_match(prog, chr(i))
        end = time.time_ns()
        if not res:
            print()
            print("String: %s" % repr(chr(i).encode("utf-8")))
            dump_program(prog)
            break
        times.append(end - start)
    print()
    #print(times)

# pipe output of this to csv
# results: seems to asymptotically reduce 82%(!!!) of instructions
# on average, reduces 79% of instructions (cached is 21% of size)
# worth it!
def test_reduction():
    for i in range(0, 0x200000):
        prog0 = comp_range(0, i)
        prog1 = comp_range_cached(0, i)
        uncached_size = len(prog0)
        cached_size = len(prog1)
        print(str(i)+","+str(cached_size / uncached_size) + ",")

def analyze_reduction():
    import csv, statistics
    f = csv.reader(open("comp.csv", "r"))
    print(statistics.mean([float(a[1]) for a in f]))

def dump_prog_graph(prog):
    import graphviz
    dot = graphviz.Digraph(comment="NFA Program")

    nodes = list(range(len(prog)))
    edges = []
    for i, inst in enumerate(prog):
        if isinstance(inst, Split):
            edges.append((i, inst.next, "red"))
            edges.append((i, inst.next1, "red"))
        elif isinstance(inst, Byte):
            edges.append((i, inst.next, "black"))
        elif isinstance(inst, Range):
            edges.append((i, inst.next, "black"))
    
    for node in nodes:
        shape = "box"
        if node == 1:
            shape = "box"
        if type(prog[node]) is Match:
            shape = "box"
        color = "white"
        dot.node(str(node), shape=shape, label=str(prog[node]), style="filled", fillcolor=color)
    
    for begin, end, color in edges:
        dot.edge(str(begin), str(end), color=color, taillabel=str(begin), headlabel=str(end))
    
    dot.render(filename="graph", format="png", cleanup=True)

def clip_lo(lo, bound):
    return bound if lo < bound else lo

def clip_hi(hi, bound):
    return bound if hi > bound else hi

class RangeComp:
    def __init__(self, ranges):
        self.prog = []
        add(self.prog, Fail())
        self.ranges = ranges
        self.ranges = [(r[0], r[1] - 1) for r in self.ranges]
        self.cache = {}
    
    def label(self):
        return len(self.prog)

    def add(self, inst):
        self.prog.append(inst)

    def link(self, origin=None, dest=None, secondary=False):
        src = self.prog[-1] if origin is None else self.prog[origin]
        dst = len(self.prog) if dest is None else dest
        func = src.set_next if not secondary else src.set_next1
        func(dst)

    def compile(self):
        # min: 0x0 max: 0x800
        start_idx = 0
        end_idx = 0
        next_start_idx = 0
        for i, r in enumerate(self.ranges[start_idx:]):
            lo = r[0]
            hi = r[1]
            if lo <= ONE_BYTE_MAX:
                if hi >= TWO_BYTE_MIN:
                    next_start_idx = end_idx
                else:
                    next_start_idx = end_idx + 1
                end_idx += 1
            else:
                break
        if end_idx - start_idx > 0:
            self.compile_one_byte(start_idx, end_idx)
        start_idx = next_start_idx
        end_idx = start_idx
        for i, r in enumerate(self.ranges[start_idx:]):
            lo = r[0]
            hi = r[1]
            if lo <= TWO_BYTE_MAX:
                if hi >= THREE_BYTE_MIN:
                    next_start_idx = end_idx
                else:
                    next_start_idx = end_idx + 1
                end_idx += 1
            else:
                break
        if end_idx - start_idx > 0:
            self.compile_two_byte(start_idx, end_idx)

    def compile_one_byte(self, start_idx, end_idx):
        # 0yyyyyyy
        assert end_idx - start_idx > 0
        if start_idx == end_idx - 1:
            # One range, compiles to either a Byte or Range
            lo = clip_lo(self.ranges[start_idx][0], ONE_BYTE_MIN)
            hi = clip_hi(self.ranges[start_idx][1], ONE_BYTE_MAX)
            if lo == hi:
                self.add(Byte(lo & ONE_BYTE_MAX))
            else:
                self.add(Range(lo & ONE_BYTE_MAX, hi & ONE_BYTE_MAX))
            self.link()
            self.add(Match())
        else:
            # Multiple ranges
            last_split = None
            for i, r in enumerate(self.ranges[start_idx:end_idx]):
                if last_split != None:
                    self.link(origin=last_split, secondary=True)
                if i != ((end_idx - start_idx) - 1):
                    last_split = self.label()
                    self.add(Split())
                    self.link()
                lo = clip_lo(r[0], ONE_BYTE_MIN)
                hi = clip_hi(r[1], ONE_BYTE_MAX)
                if lo == hi:
                    self.add(Byte(lo & ONE_BYTE_MAX))
                else:
                    self.add(Range(lo & ONE_BYTE_MAX, hi & ONE_BYTE_MAX))
                self.link()
                self.add(Match())

    def compile_two_byte(self, start_idx, end_idx):
        # 110yyyyy # 10xxxxxx
        # Find common yyyyy
        assert end_idx - start_idx > 0
        child_start_idx = start_idx
        child_next_start_idx = start_idx
        child_end_idx = end_idx
        running_y = -1
        last_split = None
        for i, r in enumerate(self.ranges[start_idx:end_idx]):
            assert r[1] >= TWO_BYTE_MIN
            lo = clip_lo(r[0], TWO_BYTE_MIN)
            hi = clip_hi(r[1], TWO_BYTE_MAX)
            if running_y == -1:
                running_y = (r[0] >> 6) & ((1 << 5) - 1)
            next_lo_y = (lo >> 6) & ((1 << 5) - 1)
            next_hi_y = (hi >> 6) & ((1 << 5) - 1)
            if next_lo_y == running_y:
                if next_hi_y > running_y:
                    child_next_start_idx = child_end_idx
                else:
                    child_next_start_idx = child_end_idx + 1
                child_end_idx += 1
            else:
                # compile found common ys
                if last_split != None:
                    self.link(origin=last_split, secondary=True)
                if child_end_idx != end_idx:
                    last_split = self.label()
                    self.add(Split())
                    self.link()
                assert child_end_idx - child_start_idx > 0
                if child_start_idx == child_end_idx - 1:
                    # One range, compiles to either a Byte or Range
                    lo_x = (clip_lo(self.ranges[child_start_idx][0], (running_y << 6)) & CONT_ONE_MAX) | 0x80
                    hi_x = (clip_hi(self.ranges[child_start_idx][1], (running_y << 6) | CONT_ONE_MAX) & CONT_ONE_MAX) | 0x80
                    if lo_x == hi_x:
                        self.add(Byte(lo_x))
                    else:
                        self.add(Range(lo_x, hi_x))
                    self.link()
                    self.add(Match())
                else:
                    # Multiple ranges
                    last_split_2 = None
                    for j, r2 in enumerate(self.ranges[child_start_idx:child_end_idx]):
                        if last_split_2 != None:
                            self.link(origin=last_split_2, secondary=True)
                        if j != ((child_end_idx - child_start_idx) - 1):
                            last_split = self.label()
                            self.add(Split())
                            self.link()
                        lo_x = (clip_lo(r2[0], (running_y << 6)) & CONT_ONE_MAX) | 0x80
                        hi_x = (clip_hi(r2[1], (running_y << 6) | CONT_ONE_MAX) & CONT_ONE_MAX) | 0x80
                        if lo_x == hi_x:
                            self.add(Byte(lo_x))
                        else:
                            self.add(Range(lo_x, hi_x))
                        self.link()
                        self.add(Match())
                child_start_idx = child_next_start_idx
                child_end_idx = child_start_idx
                running_y = next_lo_y

    def compile_recursive(self, start_idx, end_idx, y_bits, x_bits, prev_clip_lo, prev_clip_hi):
        # 0yyyyyyy
        # 110yyyyy 10xxxxxx
        # 1110yyyy 10xxxxxx 10xxxxxx
        # 11110yyy 10xxxxxx 10xxxxxx 10xxxxxx
        # yyyyyyy
        #   yyyyyxxxxxx
        #    yyyyxxxxxxxxxxxx
        #     yyyxxxxxxxxxxxxxxxxxx
        assert (end_idx - start_idx) > 0
        out_mask = (0b11111110 << y_bits) & 0b11111111
        y_mask = ((1 << y_bits) - 1)
        x_pow = ((1 << x_bits))
        x_mask = (x_pow - 1)
        if x_bits == 0:
            last_split = None
            for i, r in enumerate(self.ranges[start_idx:end_idx]):
                if last_split != None:
                    self.link(origin=last_split, secondary=True)
                if i != end_idx - 1:
                    last_split = self.label()
                    self.add(Split())
                    self.link()
                lo_byte = ((clip_lo(r[0], prev_clip_lo) >> x_bits) & y_mask) | out_mask
                hi_byte = ((clip_hi(r[1], prev_clip_hi) >> x_bits) & y_mask) | out_mask
                if lo_byte == hi_byte:
                    self.add(Byte(lo_byte))
                else:
                    self.add(Range(lo_byte, hi_byte))
                self.link()
                self.add(Match())
        else:
            child_start_idx = start_idx
            child_end_idx = start_idx
            child_next_start_idx = start_idx
            last_split = None
            current_byte = -1
            current_clip = prev_clip_lo
            while child_start_idx != end_idx:
                if last_split != None:
                    self.link(origin=last_split, secondary=True)
                r = self.ranges[child_end_idx]
                lo_clip = clip_lo(r[0], current_clip)
                hi_clip = clip_hi(r[1], prev_clip_hi)
                lo_byte = ((lo_clip >> x_bits) & y_mask) | out_mask
                hi_byte = ((hi_clip >> x_bits) & y_mask) | out_mask
                if lo_byte == current_byte:
                    if hi_byte > lo_byte:
                        child_next_start_idx = child_end_idx
                    else:
                        child_next_start_idx = child_end_idx + 1
                    child_end_idx += 1
                else:
                    if current_byte == -1:
                        current_byte = lo_byte
                    else:
                        self.add(Byte(lo_byte))
                        self.link()
                        self.compile_recursive(child_start_idx, child_end_idx, 6, x_bits - 6, lo_clip, hi_clip)
                        current_clip += (1 << x_bits)
                        child_start_idx = child_next_start_idx
                        child_end_idx = child_start_idx
    
    def compile_recursive_new(self, y_bits, x_bits, ranges):
        # 0yyyyyyy
        # 110yyyyy 10xxxxxx
        # 1110yyyy 10xxxxxx 10xxxxxx
        # 11110yyy 10xxxxxx 10xxxxxx 10xxxxxx
        # yyyyyyy
        #   yyyyyxxxxxx
        #    yyyyxxxxxxxxxxxx
        #     yyyxxxxxxxxxxxxxxxxxx
        out_mask = (0b11111110 << y_bits) & 0b11111111
        last_split = None
        x_mask = (1 << x_bits) - 1
        y_mask = (1 << y_bits) - 1
        if x_bits == 0:
            for i, r in enumerate(ranges):
                assert r[0] <= r[1]
                assert r[0] <= y_mask
                assert r[1] <= y_mask
                if last_split != None:
                    self.link(origin=last_split, secondary=True)
                if i != len(ranges) - 1:
                    last_split = self.label()
                    self.add(Split())
                    self.link()
                lo_byte = (r[0] >> x_bits) | out_mask
                hi_byte = (r[1] >> x_bits) | out_mask
                if lo_byte == hi_byte:
                    self.add(Byte(lo_byte))
                else:
                    self.add(Range(lo_byte, hi_byte))
                self.link()
                self.add(Match())
        else:
            sets = []
            current_ranges = []
            prev_y = -1
            for i, r in enumerate(ranges):
                lo_y = (r[0] >> x_bits) | out_mask
                hi_y = (r[1] >> x_bits) | out_mask
                lo_x = r[0] & x_mask
                hi_x = r[1] & x_mask
                if prev_y == -1:
                    prev_y = lo_y
                if prev_y != lo_y:
                    sets.append((prev_y, prev_y, current_ranges))
                    current_ranges = []
                    prev_y = lo_y
                if hi_y == lo_y:
                    current_ranges.append([lo_x, hi_x])
                elif hi_y == lo_y + 1:
                    current_ranges.append([lo_x, x_mask])
                    sets.append((prev_y, prev_y, current_ranges))
                    current_ranges = [[0, hi_x]]
                    prev_y = hi_y
                else:
                    current_ranges.append([lo_x, x_mask])
                    sets.append((prev_y, prev_y, current_ranges))
                    current_ranges = [[0, x_mask]]
                    sets.append((prev_y + 1, hi_y - 1, current_ranges))
                    current_ranges = [[0, hi_x]]
                    prev_y = hi_y
            assert len(current_ranges) != 0
            sets.append((prev_y, prev_y, current_ranges))
            last_split = None
            for i, (lo_y, hi_y, new_ranges) in enumerate(sets):
                if last_split is not None:
                    self.link(last_split, secondary=True)
                if i != len(sets) - 1:
                    last_split = self.label()
                    self.add(Split())
                    self.link()
                if lo_y == hi_y:
                    self.add(Byte(lo_y))
                else:
                    self.add(Range(lo_y, hi_y))
                self.link()
                self.compile_recursive_new(6, x_bits - 6, new_ranges)

def uclass(y_bits, x_bits):
    if y_bits == 3:
        return "byte4"
    elif y_bits == 4:
        return "byte3"
    elif y_bits == 5:
        return "byte2"
    elif y_bits == 6:
        if x_bits == 0:
            return "cont1"
        elif x_bits == 6:
            return "cont2"
        elif x_bits == 12:
            return "cont3"
    elif y_bits == 7:
        return "ascii"

class RangeCacheKey:
    def __init__(self, y_bits, x_bits, r):
        self.y_bits = y_bits
        self.x_bits = x_bits
        self.r = r

    def __hash__(self):
        return hash((self.y_bits, self.x_bits, self.r))
    
    def __eq__(self, other):
        return (self.x_bits, self.y_bits, self.r) == (other.x_bits, other.y_bits, other.r)

    def __repr__(self):
        return uclass(self.y_bits, self.x_bits)+("[%02X-%02X]" % (self.r[0], self.r[1]))

class ListCacheKey:
    def __init__(self, y_bits, x_bits, rs):
        self.y_bits = y_bits
        self.x_bits = x_bits
        self.rs = rs

    def __hash__(self):
        return hash((self.y_bits, self.x_bits, self.rs))
    
    def __eq__(self, other):
        return (self.x_bits, self.y_bits, self.rs) == (other.x_bits, other.y_bits, other.rs)

    def __repr__(self):
        rstr = "("+",".join(["[%02X-%02X]" % (r[0], r[1]) for r in self.rs])+")"
        return uclass(self.y_bits, self.x_bits)+rstr

class ListCacheVal:
    def __init__(self, r, next):
        self.r = r
        self.next = next
    
    def __repr__(self):
        nstr = ""
        if self.next is not None:
            rstr = "("+",".join(["[%02X-%02X]" % (r[0], r[1]) for r in self.next])+")"
            nstr = " -> " + rstr
        return ("[%02X-%02X]" % (self.r[0], self.r[1])) + nstr

class RangeCompNew:
    def __init__(self):
        self.range_cache = {}
        self.list_cache = {}
        self.prog = []
        self.add(Fail())

    def label(self):
        return len(self.prog)

    def add(self, inst):
        self.prog.append(inst)

    def link(self, origin=None, dest=None, secondary=False):
        src = self.prog[-1] if origin is None else self.prog[origin]
        dst = len(self.prog) if dest is None else dest
        func = src.set_next if not secondary else src.set_next1
        func(dst)

    def build_ranges(self, y_bits, x_bits, ranges):
        for i in range(len(ranges)):
            reverse_idx = len(ranges) - i - 1
            r = ranges[reverse_idx]
            range_cache_key = RangeCacheKey(y_bits, x_bits, r)
            range_contents = None
            if x_bits != 0:
                y_mask = ((1 << y_bits) - 1)
                x_mask = ((1 << x_bits) - 1)
                min_y = r[0] & (~x_mask) # yyyyy000000
                max_y = r[1] & (~x_mask)
                min_y_shifted = min_y >> x_bits
                max_y_shifted = max_y >> x_bits
                min_x = r[0] & x_mask    # 00000xxxxxx
                max_x = r[1] & x_mask
                child_y_bits = 6
                child_x_bits = x_bits - 6
                if min_x == 0 and max_x == x_mask:
                    # maximal value
                    child_ranges = ((0, x_mask),)
                    range_contents = ListCacheKey(child_y_bits, child_x_bits, child_ranges)
                    self.build_ranges(child_y_bits, child_x_bits, child_ranges)
                else:
                    if min_y_shifted == max_y_shifted:
                        child_ranges = ((min_x, max_x),)
                        range_contents = ListCacheKey(child_y_bits, child_x_bits, child_ranges)
                        self.build_ranges(child_y_bits, child_x_bits, child_ranges)
                    elif min_y_shifted == max_y_shifted - 1:
                        child_ranges = ((min_y, min_y | x_mask), (max_y, r[1]))
                        range_contents = ListCacheKey(y_bits, x_bits, child_ranges)
                        self.build_ranges(y_bits, x_bits, child_ranges)
                    else:
                        child_ranges = ((min_y, min_y | x_mask), ((min_y | x_mask)+1, (max_y - 1) | x_mask), (max_y, r[1]))
                        range_contents = ListCacheKey(y_bits, x_bits, child_ranges)
                        self.build_ranges(y_bits, x_bits, child_ranges)
            list_cache_key = ListCacheKey(y_bits, x_bits, ranges[reverse_idx:])
            if cache_key not in self.range_cache:
                self.range_cache[range_cache_key] = range_contents
            #print(r, reverse_idx, len(ranges))
            if reverse_idx == len(ranges) - 1:
                #print(r, reverse_idx)
                self.list_cache[list_cache_key] = ListCacheVal(r, None)
            else:
                self.list_cache[list_cache_key] = ListCacheVal(r, ranges[reverse_idx+1:])

    def clip(self, r, clip_range):
        out = [r[0], r[1]]
        if r[0] < clip_range[0]:
            out[0] = clip_range[0]
        if r[1] > clip_range[1]:
            out[1] = clip_range[1]
        return tuple(out)

    def compile_ranges(self, ranges):
        def intersects(r, r2):
            return r[0] <= r2[1] and r2[0] <= r[1]
        self.inst_cache = {}
        start_idx = 0
        end_idx = 0
        range_bits = [[0, 7], [7, 11], [11, 16], [16, 21]]
        xy_bits = [[0, 7], [6, 5], [12, 4], [18, 3]]
        last_split = None
        rlists = []
        for bl, r2 in enumerate(range_bits):
            rmin = (1 << r2[0])
            if rmin == 1:
                rmin = 0
            rmax = (1 << r2[1]) - 1
            filt_rs = list(filter(lambda r3: intersects(r3, (rmin, rmax)), ranges))
            filt_rs = tuple([self.clip(r, (rmin, rmax)) for r in filt_rs])
            if len(filt_rs) > 0:
                rlists.append((bl, filt_rs))
        for i, (bl, filt_rs) in enumerate(rlists):
            #print(i, bl, filt_rs)
            if last_split != None:
                self.link(origin=last_split, secondary=True)
            if i != len(rlists) - 1:
                last_split = self.label()
                self.add(Split())
                self.link()
            self.build_ranges(xy_bits[bl][1], xy_bits[bl][0], filt_rs)
            self.compile_ranges_rec(xy_bits[bl][1], xy_bits[bl][0], filt_rs)
    
    def compile_ranges_rec(self, y_bits, x_bits, ranges):
        if len(ranges) == 0:
            return
        last_split = None
        lck = ListCacheKey(y_bits, x_bits, ranges)
        start = self.list_cache[lck]
        while start != None:
            if last_split != None:
                self.link(origin=last_split, secondary=True)
            if start.next != None:
                last_split = self.label()
                self.add(Split())
                self.link()
            rc = self.range_cache[RangeCacheKey(y_bits, x_bits, start.r)]
            out_mask = (0b11111110 << y_bits) & 0b11111111
            min_byte = (start.r[0] >> x_bits) | out_mask
            max_byte = (start.r[1] >> x_bits) | out_mask
            if rc is not None:
                if rc.x_bits == x_bits and rc.y_bits == y_bits:
                    self.compile_ranges_rec(y_bits, x_bits, rc.rs)
                else:
                    if min_byte == max_byte:
                        self.add(Byte(min_byte))
                    else:
                        self.add(Range(min_byte, max_byte))
                    self.link()
                    self.compile_ranges_rec(rc.y_bits, rc.x_bits, rc.rs)
            else:
                if min_byte == max_byte:
                    self.add(Byte(min_byte))
                else:
                    self.add(Range(min_byte, max_byte))
                self.link()
                self.add(Match())
            if start.next != None:
                next_key = ListCacheKey(y_bits, x_bits, start.next)
                start = self.list_cache[next_key]
            else:
                start = None

            
#comp = RangeComp([[0x85, 0x89]])
#comp.compile_recursive(0, 1, 5, 6, 0)
comp = RangeCompNew()
comp.build_ranges(5, 6, ((0x80, 0xC1),))
print("list cache:")
for k, v in comp.list_cache.items():
    print("  ", repr(k), ":", repr(v))
print("range cache:")
for k, v in comp.range_cache.items():
    print("  ", repr(k), ":", repr(v))
comp.compile_ranges(((0, 0x10FFFF),))
dump_prog_graph(comp.prog)
