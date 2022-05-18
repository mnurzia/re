# quick (actually not quick) 'n' dirty code to generate optimized dfa for utf8
# charclasses

import itertools

ACCEPT = True
SENTINEL = False

class DState:
    def __init__(self):
        self.transitions = [None] * 256
        self.transitions_int = [0] * 256
        self.outgoing = None
        self.sentinel = None
        self.accept = False
        self.hash = None
        self.id = None
    
    def __setitem__(self, key, val):
        self.transitions[key] = val
    
    def __getitem__(self, key):
        return self.transitions[key]
    
    def __hash__(self):
        if self.hash is None:
            self.hash = hash(tuple(self.transitions))
        return self.hash

    def __eq__(self, other):
        return self.transitions == other.transitions and self.accept == other.accept

    def reduce(self, cache):
        if self is None:
            return
        map(DState.reduce, self.transitions)
        for i in range(len(self.transitions)):
            self.transitions_int[i] = cache[self.transitions[i]]
    
    def compress(self, cache):
        if self.outgoing is None:
            groups = itertools.groupby(self.transitions_int)
            self.outgoing = []
            begin = 0
            end = 0
            amt = 0
            for state_idx, out_idx in groups:
                begin += amt
                amt = len(list(out_idx))
                end = begin + amt
                if state_idx != 0:
                    self.outgoing.append(((begin, end), state_idx))
                    cache.ids[state_idx].compress(cache)

    def make_accept(self):
        self.accept = True

INST_CHAR = "CHAR"
INST_RANGE = "RANGE"
INST_SPLIT = "SPLIT"

class DStateCache:
    def __init__(self):
        self.cache = {None: 0}
        self.ids = {}
        self.pcs = {}
        self.insts = ["X"]
    
    def __getitem__(self, dstate):
        if dstate not in self.cache:
            self.cache[dstate] = len(self.cache)
        return self.cache[dstate]
    
    def reduce(self, dstate):
        for trans in dstate.transitions:
            if trans is None:
                continue
            else:
                self.reduce(trans)
        for i, trans in enumerate(dstate.transitions):
            if trans is not None:
                dstate.transitions_int[i] = trans.id
        if dstate not in self.cache:
            id = len(self.cache)
            self.cache[dstate] = id
            self.ids[id] = dstate
        dstate.id = self.cache[dstate]
    
    def emit(self, state, a):
        self.insts.append(a)

    def compile(self, dstate):
        self.pcs[dstate.id] = len(self.insts)
        if dstate.accept:
            self.emit(dstate, [INST_SPLIT, len(self.insts) + 1, 0])
        state_groups = dstate.outgoing
        last_split = None
        for i, ((begin, end), state_idx) in enumerate(state_groups):
            if state_idx == 0:
                continue
            if last_split is not None:
                self.insts[last_split][2] = len(self.insts)
            if i != len(state_groups) - 1:
                self.emit(dstate, [INST_SPLIT, len(self.insts) + 1, 0])
                last_split = len(self.insts) - 1
            next_loc = len(self.insts) + 1
            should_comp_next = True
            if self.ids[state_idx].accept:
                if len(self.ids[state_idx].outgoing) == 0:
                    next_loc = 0
                    should_comp_next = False
                else:
                    if state_idx in self.pcs:
                        next_loc = self.pcs[state_idx]
                        should_comp_next = False
                    else:
                        next_loc = len(self.insts) + 1
            else:
                if state_idx in self.pcs:
                    next_loc = self.pcs[state_idx]
                    should_comp_next = False
            if begin + 1 == end:
                self.emit(dstate, [INST_CHAR, begin, next_loc])
            else:
                self.emit(dstate, [INST_RANGE, begin, end - 1, next_loc])
            if should_comp_next:
                self.compile(self.ids[state_idx])
    
    def compress(self, dstate):
        dstate.compress(self)
            
def build_seq_recursive(seq, state):
    if len(seq) > 0:
        if state[seq[0]] is None:
            state[seq[0]] = DState()
        build_seq_recursive(seq[1:], state[seq[0]])
    else:
        state.make_accept()

def simple_enc_utf8(ord):
    if ord < 0x80:
        return [ord]
    elif ord < 0x800:
        return [((ord >> 6) & 0x1F) | 0xC0, (ord & 0x3F) | 0x80]
    elif ord < 0x10000:
        return [
            ((ord >> 12) & 0x0F) | 0xE0, 
            ((ord >> 6) & 0x3F) | 0x80,
            (ord & 0x3F) | 0x80
        ]
    elif ord < 0x110000:
        return [
            ((ord >> 18) & 0x07) | 0xF0,
            ((ord >> 12) & 0x3F) | 0x80,
            ((ord >> 6) & 0x3F) | 0x80,
            (ord & 0x3F) | 0x80
        ]

def encode_pc(offs):
    bytes = []
    while offs > 0x7F:
        bytes.append(offs & 0x7F)
        offs >>= 7
    bytes.append(offs)
    return bytes

def fmt_out(offs):
    if offs == 0:
        return "out"
    else:
        return "0x%02X" % offs

def encode_insts(insts):
    total_listing = []
    total_disasm = []
    for i, inst in enumerate(insts):
        if i == 0:
            continue
        listing = []
        op = inst[0]
        if op == INST_CHAR:
            listing.append(0)
            listing.append(inst[1])
            listing.extend(encode_pc(inst[2]))
            disasm = "BYTE  0x%02X      -> %s" % (inst[1], fmt_out(inst[2]))
        elif op == INST_RANGE:
            listing.append(1)
            listing.append(inst[1])
            listing.append(inst[2])
            listing.extend(encode_pc(inst[3]))
            disasm = "RANGE 0x%02X-0x%02X -> %s" % (inst[1], inst[2], fmt_out(inst[3]))
        elif op == INST_SPLIT:
            listing.append(2)
            listing.extend(encode_pc(inst[1]))
            listing.extend(encode_pc(inst[2]))
            disasm = "SPLIT           -> %s, %s" % (fmt_out(inst[1]), fmt_out(inst[2]))
        total_listing.append(listing)
        total_disasm.append(disasm)
    for pc, (listing, disasm) in enumerate(zip(total_listing, total_disasm)):
        pc = pc + 1
        enc = (" ".join(["0x%02X," % x for x in listing])).ljust(23)
        print("/* %04X */ %s /* %s */" % (pc, enc, disasm))

if __name__ == "__main__":
    import sys
    av = sys.argv
    if len(sys.argv) == 1:
        av = ["", "fwd", "0-D7FF", "E000-10FFFF"]
    ranges = []
    forwards = av[1] == "fwd"
    for arg in av[2:]:
        spl = arg.split("-")
        if len(spl) == 0:
            ranges.append((int(spl[0], 16), int(spl[0], 16) + 1))
        else:
            ranges.append((int(spl[0], 16), int(spl[1], 16) + 1))
    print("generating ranges ", *ranges)
    state = DState()
    for ub in itertools.chain(*[range(a, b) for a, b in ranges]):
        text = simple_enc_utf8(ub)
        if not forwards:
            text = list(reversed(text))
        build_seq_recursive(text, state)
        if ub % 0x100 == 0:
            print("generating %08X..." % ub, end="\r")
    print()

cache = DStateCache()
print("reducing...")
cache.reduce(state)
cache.compress(state)
print("compiling...")
cache.compile(state)
encode_insts(cache.insts)
