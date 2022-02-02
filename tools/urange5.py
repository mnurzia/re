import enum

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

def printable(ch):
    if chr(ch) in string.printable:
        return "0x%02X ('%s')" % (ch, chr(ch))
    else:
        return "0x%02X" % ch

def dump_program(prog):
    for pc, inst in enumerate(prog):
        print("0x%02X | " % pc, end="")
        print(str(inst))

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
        dot.edge(str(begin), str(end), color=color, taillabel="0x%02X" % begin, headlabel="0x%02X" % end)
    
    dot.render(filename="graph", format="png", cleanup=True)

class UClass(enum.IntEnum):
    ASCII = 0
    BYTES2 = 1
    BYTES3 = 2
    BYTES4 = 3
    CONT1 = 4
    CONT2 = 5
    CONT3 = 6
    OVERLAP_TOP = 7

yx_bits = (
    (7, 0),
    (5, 6),
    (4, 12),
    (3, 18),
    (6, 0),
    (6, 6),
    (6, 12)
)

class RangeComp:
    def __init__(self):
        self.prog = []
        self.suffix_cache = {}
        self.add(Fail())
        self.max_depth = 0
        self.depth = 0
    
    def label(self):
        return len(self.prog)

    def add(self, inst):
        self.prog.append(inst)

    def link(self, origin=None, dest=None, secondary=False):
        src = self.prog[-1] if origin is None else self.prog[origin]
        dst = len(self.prog) if dest is None else dest
        func = src.set_next if not secondary else src.set_next1
        func(dst)

    def next_ucls(self, ucls):
        return [None, UClass.CONT1, UClass.CONT2, UClass.CONT3, None, UClass.CONT1, UClass.CONT2][ucls]

    def comp_range(self, min, max, ucls):
        self.depth += 1
        self.max_depth = self.max_depth if self.depth < self.max_depth else self.depth
        if (min, max, ucls) in self.suffix_cache:
            self.depth -= 1
            return self.suffix_cache[(min, max, ucls)]
        # First split range into common first bytes.
        y_bits, x_bits = yx_bits[ucls]
        y_mask = (1 << y_bits) - 1
        min_y_byte = (min >> x_bits) & y_mask
        max_y_byte = (max >> x_bits) & y_mask
        x_mask = (1 << x_bits) - 1
        min_x = min & x_mask
        max_x = max & x_mask
        ucls_mask = (0b11111110 << y_bits) & 0b11111111
        begin = self.label()
        self.suffix_cache[(min, max, ucls)] = begin
        if self.next_ucls(ucls) == None:
            if min_y_byte == max_y_byte:
                self.add(Byte(min_y_byte | ucls_mask))
            else:
                self.add(Range(min_y_byte | ucls_mask, max_y_byte | ucls_mask))
            self.link()
            self.add(Match())
        else:
            if min_x == 0 and max_x == x_mask:
                last_inst = self.label()
                if min_y_byte == max_y_byte:
                    self.add(Byte(min_y_byte | ucls_mask))
                else:
                    self.add(Range(min_y_byte | ucls_mask, max_y_byte | ucls_mask))
                link_to = self.comp_range(0, x_mask, self.next_ucls(ucls))
                self.link(last_inst, link_to)
            else:
                if min_y_byte == max_y_byte:
                    last_inst = self.label()
                    self.add(Byte(min_y_byte | ucls_mask))
                    link_to = self.comp_range(min_x, max_x, self.next_ucls(ucls))
                    self.link(last_inst, link_to)
                else:
                    spl_loc = self.label()
                    self.add(Split())
                    if min_x == 0:
                        link_to = self.comp_range(min, (max_y_byte << x_bits) - 1, ucls)
                        self.link(spl_loc, link_to)
                        link_to = self.comp_range(max_y_byte << x_bits, max, ucls)
                        self.link(spl_loc, link_to, secondary=True)
                    else:
                        link_to = self.comp_range(min, (min_y_byte << x_bits) | x_mask, ucls)
                        self.link(spl_loc, link_to)
                        link_to = self.comp_range((min_y_byte + 1) << x_bits, max, ucls)
                        self.link(spl_loc, link_to, secondary=True)
        return begin

    def comp_ranges(self, ucls, bounds, ranges_slice):
        def intersects(r, b):
            return r[0] <= b[1] and b[0] <= r[1] 
        def clamp(r, b):
            out = [r[0], r[1]]
            if r[0] < b[0]:
                out[0] = b[0]
            if r[1] > b[1]:
                out[1] = b[1]
            return tuple(out)
        assert len(ranges_slice) > 0
        assert all([intersects(r, bounds) for r in ranges_slice])
        y_bits, x_bits = yx_bits[ucls]
        byte_mask = 0b11111111
        u_mask = (0b11111110 << y_bits) & byte_mask
        if x_bits == 0:
            last_split = None
            for i, r in enumerate(ranges_slice):
                if last_split is not None:
                    self.link(last_split, self.label(), True)
                if i != len(ranges_slice) - 1:
                    last_split = self.label()
                    self.add(Split())
                    self.link()
                clamped = clamp(r, bounds)
                if clamped[0] == clamped[1]:
                    self.add(Byte((clamped[0] | u_mask) & byte_mask))
                else:
                    self.add(Range((clamped[0] | u_mask) & byte_mask, (clamped[1] | u_mask) & byte_mask))
                self.link()
                self.add(Match())
        else:
            x_mask = (1 << x_bits) - 1
            y_mask = ((1 << y_bits) - 1) << x_bits
            y_max = ((1 << y_bits) - 1)
            y_num = (1 << y_bits)
            r_begin = 0
            r_end = 0
            y_idx = (clamp(ranges_slice[0], bounds)[0] & y_mask) >> x_bits
            defer = None
            last_split = None
            while r_end != len(ranges_slice):
                bound_min = bounds[0] | (y_idx << x_bits)
                bound_max = bounds[0] | (((y_idx + 1) << x_bits) - 1)
                while r_end < len(ranges_slice):
                    if intersects(clamp(ranges_slice[r_end], bounds), (bound_min, bound_max)):
                        r_end += 1
                    else:
                        break
                r_end_next = r_end
                if r_end - r_begin > 0:
                    this_split = last_split
                    if defer is not None:
                        this_split = self.label()
                        self.add(Split())
                        self.link()
                        d_y_idx, d_begin, d_end = defer
                        self.add(Byte((d_y_idx | u_mask) & byte_mask))
                        self.link()
                        old_bound_min = bounds[0] | (d_y_idx << x_bits)
                        old_bound_max = bounds[0] | (((d_y_idx + 1) << x_bits) - 1)
                        self.comp_ranges(self.next_ucls(ucls), (old_bound_min, old_bound_max), ranges_slice[d_begin:d_end])
                    if last_split is not None:
                        self.link(last_split, self.label(), True)
                    last_split = this_split
                    defer = (y_idx, r_begin, r_end)
                r_begin = r_end_next
                r_end = r_begin
                y_idx += 1
            if defer is not None:
                if last_split is not None:
                    self.link(last_split, self.label(), True)
                d_y_idx, d_begin, d_end = defer
                self.add(Byte((d_y_idx | u_mask) & byte_mask))
                self.link()
                old_bound_min = bounds[0] | (d_y_idx << x_bits)
                old_bound_max = bounds[0] | (((d_y_idx + 1) << x_bits) - 1)
                self.comp_ranges(self.next_ucls(ucls), (old_bound_min, old_bound_max), ranges_slice[d_begin:d_end])

    def comp_ranges_new(self, ucls, bounds, ranges_slice):
        def intersects(r, b):
            return r[0] <= b[1] and b[0] <= r[1] 
        def clamp(r, b):
            out = [r[0], r[1]]
            if r[0] < b[0]:
                out[0] = b[0]
            if r[1] > b[1]:
                out[1] = b[1]
            return tuple(out)
        assert len(ranges_slice) > 0
        assert all([intersects(r, bounds) for r in ranges_slice])
        y_bits, x_bits = yx_bits[ucls]
        byte_mask = 0b11111111
        u_mask = (0b11111110 << y_bits) & byte_mask
        if x_bits == 0:
            last_split = None
            for i, r in enumerate(ranges_slice):
                if last_split is not None:
                    self.link(last_split, self.label(), True)
                if i != len(ranges_slice) - 1:
                    last_split = self.label()
                    self.add(Split())
                    self.link()
                clamped = clamp(r, bounds)
                if clamped[0] == clamped[1]:
                    self.add(Byte((clamped[0] | u_mask) & byte_mask))
                else:
                    self.add(Range((clamped[0] | u_mask) & byte_mask, (clamped[1] | u_mask) & byte_mask))
                self.link()
                self.add(Match())
        else:
            r_begin = 0
            r_end_full = 0
            r_end = 0
            y_mask = (1 << y_bits) - 1
            x_mask = (1 << x_bits) - 1
            y_mask_shifted = y_mask << x_bits
            y_idx = (clamp(ranges_slice[0], bounds)[0] >> x_bits) & y_mask
            defer = None
            last_split = None
            while r_end != len(ranges_slice):
                while r_end != len(ranges_slice):
                    bound_min = bounds[0] | (y_idx << x_bits)
                    bound_max = bounds[0] | ((y_idx << x_bits) - 1)
                    clamped = clamp(ranges_slice[r_end], bounds)
                    if not intersects(clamped, (bound_min, bound_max)):
                        break
                    if clamp(clamped, (bound_min, bound_max)) == (bound_min, bound_max):
                        

c = RangeComp()
c.comp_ranges_new(UClass.BYTES4, (0, 0x10FFFF), ((0, 0x10FFFF),))
dump_prog_graph(c.prog)
print(c.max_depth)

