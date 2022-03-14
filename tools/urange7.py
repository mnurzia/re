class BRange:
    def __init__(self, min, max):
        assert min >= 0
        assert max <= 255
        assert min <= max
        self.min = min
        self.max = max

    def __repr__(self):
        return "B[%02X-%02X]" % (self.min, self.max)
    
    def __str__(self):
        return repr(self)
    
    def __hash__(self):
        return hash((self.min, self.max))
    
    def __eq__(self, other):
        return self.min == other.min and self.max == other.max
    
    def adjacent(self, other):
        return other.min == self.max + 1
    
    def merge(self, other):
        assert self.adjacent(other)
        self.max = other.max

class RRange:
    def __init__(self, min, max):
        assert min >= 0
        assert min <= max
        self.min = min
        self.max = max
    
    def __repr__(self):
        return "R[%X-%X]" % (self.min, self.max)
    
    def __str__(self):
        return repr(self)
    
    def __hash__(self):
        assert False

class BRangeTree:
    def __init__(self, br):
        self.br = br
        self.prev = None
        self.next = None
        self.down = None
        self.down_last = None
        self.hash = None
    
    def invariants(self):
        if self.down != None:
            assert self.down.prev == None
            assert self.down_last.next == None

    def get_ref(self, br):
        self.invariants()
        if self.down == None:
            self.down = BRangeTree(br)
            self.down.prev = None
            self.down_last = self.down
            self.invariants()
            return self.down
        else:
            obj = self.down
            while obj.next != None:
                if obj.br == br:
                    self.invariants()
                    return obj
                obj = obj.next
            new = BRangeTree(br)
            obj.next = new
            new.prev = obj
            self.down_last = new
            self.invariants()
            return obj.next
    
    def merge(self, other):
        assert other != None
        self.next = other.next
        self.br.merge(other.br)

    def __hash__(self):
        assert self.hash != None
        return self.hash

    def hash_and_link(self, last):
        obj = last
        while obj != None:
            if obj.down:
                obj.down.hash_and_link(obj.down_last)
                if obj.next != None:
                    if obj.br.adjacent(obj.next.br):
                        if obj.next.down == None and obj.down == None:
                            obj.merge(obj.next)
                        elif obj.next.down.hash == obj.down.hash:
                            if obj.next.down == obj.down:
                                obj.merge(obj.next)
            obj.hash = hash((obj.br, obj.down, obj.next))
            obj = obj.prev

    def merge_all(self):
        obj = self.down_last
        while obj != None:
            obj.merge_all()
            if obj.next != None:
                if obj.br.adjacent(obj.next.br):
                    if obj.next.down == None:
                        if obj.down == None:
                            obj.merge(obj.next)
                    else:
                        if obj.down != None:
                            if obj.down.hash == obj.next.down.hash:
                                if obj.down == obj.next.down:
                                    obj.merge(obj.next)
            obj = obj.prev

    def hash_no_link(self, last):
        obj = last
        while obj != None:
            if obj.down:
                obj.down.hash_no_link(obj.down_last)
            obj.hash = hash((obj.br, obj.down, obj.next))
            obj = obj.prev

    def hash_all(self):
        obj = self.down_last
        while obj != None:
            obj.hash_all()
            obj = obj.prev
        self.hash = hash((self.br, self.down, self.next))

    def __eq__(self, other):
        if other == None:
            return False
        return self.next == other.next and self.down == other.down and self.br == other.br
        
import enum
from re import L

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

def clamp(r, b):
    out = RRange(r.min, r.max)
    if r.min < b.min:
        out.min = b.min
    if r.max > b.max:
        out.max = b.max
    return out

def intersects(r, b):
    return r.min <= b.max and b.min <= r.max

def split_range(tree, r):
    begin_classes = [UClass.ASCII, UClass.BYTES2, UClass.BYTES3, UClass.BYTES4]
    min_value = 0
    for cls in begin_classes:
        y_bits, x_bits = yx_bits[cls]
        max_value = (1 << (y_bits + x_bits)) - 1
        bounds = RRange(min_value, max_value)
        if intersects(r, bounds):
            compile_range(tree, cls, clamp(r, bounds))
        min_value = max_value+1

def next_ucls(ucls):
    return [None, UClass.CONT1, UClass.CONT2, UClass.CONT3, None, UClass.CONT1, UClass.CONT2][ucls]

def compile_range(tree, cls, r):
    y_bits, x_bits = yx_bits[cls]
    y_mask = (1 << y_bits) - 1
    x_mask = (1 << x_bits) - 1
    byte_mask = 0b11111111
    u_mask = (0b11111110 << y_bits) & byte_mask
    if x_bits == 0:
        byte_min = (r.min & byte_mask) | u_mask
        byte_max = (r.max & byte_mask) | u_mask
        br = BRange(byte_min, byte_max)
        tree.get_ref(br)
    else:
        y_min = r.min >> x_bits
        y_max = r.max >> x_bits
        x_min = r.min & x_mask
        x_max = r.max & x_mask
        byte_min = (y_min & byte_mask) | u_mask
        byte_max = (y_max & byte_mask) | u_mask
        if y_min == y_max:
            y_tree_0 = tree.get_ref(BRange(byte_min, byte_max))
            compile_range(y_tree_0, next_ucls(cls), RRange(x_min, x_max))
        elif x_min == 0 and x_max == x_mask:
            y_tree_0 = tree.get_ref(BRange(byte_min, byte_max))
            compile_range(y_tree_0, next_ucls(cls), RRange(x_min, x_max))
        elif x_min == 0:
            y_tree_0 = tree.get_ref(BRange(byte_min, byte_max - 1))
            compile_range(y_tree_0, next_ucls(cls), RRange(0, x_mask))
            y_tree_1 = tree.get_ref(BRange(byte_max, byte_max))
            compile_range(y_tree_1, next_ucls(cls), RRange(0, x_max))
        elif x_max == x_mask:
            y_tree_0 = tree.get_ref(BRange(byte_min, byte_min))
            compile_range(y_tree_0, next_ucls(cls), RRange(x_min, x_mask))
            y_tree_1 = tree.get_ref(BRange(byte_min + 1, byte_max))
            compile_range(y_tree_1, next_ucls(cls), RRange(0, x_mask))
        else:
            y_tree_0 = tree.get_ref(BRange(byte_min, byte_min))
            compile_range(y_tree_0, next_ucls(cls), RRange(x_min, x_mask))
            y_tree_1 = tree.get_ref(BRange(byte_min + 1, byte_max - 1))
            compile_range(y_tree_1, next_ucls(cls), RRange(0, x_mask))
            y_tree_2 = tree.get_ref(BRange(byte_max, byte_max))
            compile_range(y_tree_2, next_ucls(cls), RRange(0, x_max))

def split_range(tree, r):
    begin_classes = [UClass.ASCII, UClass.BYTES2, UClass.BYTES3, UClass.BYTES4]
    min_value = 0
    for cls in begin_classes:
        y_bits, x_bits = yx_bits[cls]
        max_value = (1 << (y_bits + x_bits)) - 1
        bounds = RRange(min_value, max_value)
        if intersects(r, bounds):
            compile_range(tree, cls, clamp(r, bounds))
        min_value = max_value+1

def launder_ptr(tree):
    return hex(id(tree)) if tree != None else "None"

def dump_tree(tree, lvl=0):
    indent = "  "*lvl
    if tree == None:
        print(indent, "<term>")
    obj = tree
    while obj != None:
        tree_ptr = launder_ptr(obj)
        next_ptr = launder_ptr(obj.next)
        prev_ptr = launder_ptr(obj.prev)
        hash_val = "<no hash>" if obj.hash == None else hex(obj.hash)
        print(indent, tree_ptr, obj.br, next_ptr, prev_ptr, hash_val)
        dump_tree(obj.down, lvl+1)
        obj = obj.next

def link(prog, origin=None, dest=None, secondary=False):
    src = prog[-1] if origin is None else prog[origin]
    dst = len(prog) if dest is None else dest
    func = src.set_next if not secondary else src.set_next1
    func(dst)

def compile_tree(prog, cache, tree):
    start_pc = len(prog)
    if tree == None:
        prog.append(Match())
        return start_pc
    if tree in cache:
        return cache[tree]
    split_from = -1
    if tree.next is not None:
        split_from = len(prog)
        prog.append(Split())
        link(prog)
    br = tree.br
    link_from = len(prog)
    if br.min == br.max:
        prog.append(Byte(br.min))
    else:
        prog.append(Range(br.min, br.max))
    next_pc = compile_tree(prog, cache, tree.down)
    link(prog, origin=link_from, dest=next_pc)
    if tree.next is not None:
        split_to = compile_tree(prog, cache, tree.next)
        link(prog, origin=split_from, dest=split_to, secondary=True)
    cache[tree] = start_pc
    return start_pc

def compile_tree_nonrec(prog, cache, tree):
    if tree == None:
        start_pc = len(prog)
        prog.append(Match())
        return start_pc
    split_from = -1
    start_pc = len(prog)
    while tree is not None:
        br = tree.br
        if tree in cache:
            if split_from != -1:
                link(prog, origin=split_from, dest=cache[tree], secondary=True)
            break
        else:
            next_pc = len(prog)
            if split_from != -1:
                link(prog, origin=split_from, dest=next_pc, secondary=True)
        if tree.next is not None:
            split_from = len(prog)
            prog.append(Split())
            link(prog)
        link_from = len(prog)
        if br.min == br.max:
            prog.append(Byte(br.min))
        else:
            prog.append(Range(br.min, br.max))
        if tree.down in cache:
            link_to = cache[tree.down]
        else:
            link_to = compile_tree_nonrec(prog, cache, tree.down)
            cache[tree.down] = link_to
        link(prog, origin=link_from, dest=link_to)
        tree = tree.next
    return start_pc

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

tree = BRangeTree(BRange(0x00, 0xFF))
split_range(tree, RRange(0x00, 0x700))
split_range(tree, RRange(0x702, 0x800))
tree.hash_all()
dump_tree(tree)
tree.merge_all()
dump_tree(tree)
prog = [Fail()]
compile_tree_nonrec(prog, {}, tree)
dump_prog_graph(prog)
