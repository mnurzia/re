class Byte:
    def __init__(self, lo):
        self.lo = lo
    
    def __repr__(self):
        return "[%02X]" % self.lo

class Range:
    def __init__(self, lo, hi):
        self.lo = lo
        self.hi = hi
    
    def __repr__(self):
        return "[%02X-%02X]" % (self.lo, self.hi)

class Concat:
    def __init__(self, *children):
        self.children = children
    
    def __repr__(self):
        return "(" + ", ".join([repr(c) for c in self.children]) + ")"

class Split(Concat):
    def __repr__(self):
        return "{" + " | ".join([repr(c) for c in self.children]) + "}"

max_rune_per_bytes = [
    2 ** 7,
    2 ** 11,
    2 ** 16,
    2 ** 21
]

max_continuation_byte = 64

continuation_mask = 0x3F

def compile_range(lo, hi):
    def make_cont(b):
        return b | 0b10000000

    def compile_continuation_0(lo, hi):
        # Last continuation byte:
        # 10xxxxxx
        if lo == hi:
            return Byte(make_cont(lo))
        else:
            return Range(make_cont(lo), make_cont(hi))
    
    def compile_continuation_1(lo, hi):
        # Second to last continuation byte:
        # 10yyyyyy 10xxxxxx
        lo_y = lo >> 6
        hi_y = hi >> 6
        lo_x = lo & 0x3F
        hi_x = hi & 0x3F
        if lo_y == hi_y:
            return Concat(
                Byte(make_cont(lo_y)), 
                compile_continuation_0(lo_x, hi_x)
            )
        elif lo_y == hi_y - 1:
            return Split(
                Concat(
                    Byte(make_cont(lo_y)), 
                    compile_continuation_0(lo_x, 0x40)
                ),
                Concat(
                    Byte(make_cont(hi_y)), 
                    compile_continuation_0(0, hi_x)
                )
            )
        elif lo_y < hi_y:
            return Split(
                Concat(
                    Byte(make_cont(lo_y)), 
                    compile_continuation_0(lo_x, 0x40)
                ),
                Concat(
                    Range(
                        make_cont(lo_y + 1), 
                        make_cont(hi_y - 1)
                    ), 
                    compile_continuation_0(0, 0x40)
                ),
                Concat(
                    Byte(make_cont(hi_y)), 
                    compile_continuation_0(0, hi_y)
                )
            )
    print(repr(compile_continuation_1(lo, hi)))

    def continuation_byte(b):
        return (b & 0x3F) | 0b10000000

    def compile_continuations(lo, hi, index):
        shift_amt = index * 6
        min_y = (lo >> shift_amt) & continuation_mask
        max_y = (hi >> shift_amt) & continuation_mask
        if index == 0:
            return Range(min_y, max_y)

    # Compile a one-byte sequence in range [0x0, 0x80)
    def compile_one_byte_seq(lo, hi):
        # One byte sequence:
        # 0000000000000000000000000xxxxxxx
        # 0xxxxxxx
        return Range(lo, hi)
    
    # Compile a two-byte sequence in range [0x80, 0x800)
    def compile_two_byte_seq(lo, hi):
        # Two byte sequence:
        # 000000000000000000000yyyyyxxxxxx
        # 110yyyyy 10xxxxxx
        # Determine minimum and maximum values of yyyyy:
        # Three distinct cases:
        # if (min_y == max_y) {
        #   byte(min_y)
        #   byte(min_x, max_x)
        # } else if (min_y == max_y - 1) {
        #   split {
        #       byte(min_y)
        #       range(min_x, MOST)
        #   } {
        #       byte(max_y)
        #       range(LEAST, max_x)
        #   }
        # } else if (min_y < max_y) {
        #   split {
        #       byte(min_y)
        #       range(min_x, MOST)
        #   } {
        #       range(min_y + 1, max_y - 1)
        #       range(LEAST, MOST)
        #   } {
        #       byte(max_y)
        #       range(LEAST, max_x)
        #   }
        # }
        min_y = lo >> 6
        max_y = hi >> 6
        min_x = lo & 0x3F
        max_x = hi & 0x3F
        if min_y == max_y:
            return Concat(Byte(min_y), Range(min_x, max_x))
        elif min_y == max_y - 1:
            return Split(
                Concat(
                    Byte(min_y), 
                    Range(
                        continuation_byte(min_x),
                        continuation_byte(max_continuation_byte)
                    )
                ),
                Concat(
                    Byte(max_y),
                    Range(
                        continuation_byte(0),
                        continuation_byte(max_x)
                    )
                )
            )
        elif min_y < max_y:
            return Split(
                Concat(
                    Byte(min_y),
                    Range(
                        continuation_byte(min_x),
                        continuation_byte(max_continuation_byte)
                    )
                ),
                Concat(
                    Range(min_y + 1, max_y - 1),
                    Range(
                        continuation_byte(0),
                        continuation_byte(max_continuation_byte)
                    ),
                ),
                Concat(
                    Byte(max_y),
                    Range(
                        continuation_byte(0),
                        continuation_byte(max_x)
                    )
                )
            )
    
    # Compile a three-byte sequence in range [0x800, 0x10000]
    def compile_three_byte_seq(lo, hi):
        pass
        # Three byte sequence:
        # 0000000000000000zzzzyyyyyyxxxxxx
        # 1110zzzz 10yyyyyy 10xxxxxx
        # if (min_z == max_z) {
        #   byte(min_z)
        #   if (min_y == max_y) {
        #       byte(min_y)
        #       range(min_x, max_x)
        #   } else if (min_y == max_y - 1) {
        #       split {
        #           byte(min_y)
        #           range(min_x, MOST)
        #       } {
        #           byte(max_y)
        #           range(LEAST, max_x)
        #       }
        #   } else if (min_y < max_y) {
        #       split {
        #           byte(min_y)
        #           range(min_x, MOST)
        #       } {
        #           range(min_y + 1, max_y - 1)
        #           range(LEAST, MOST)
        #       } {
        #           byte(max_y)
        #           range(LEAST, max_x)
        #       }
        #   }
        # } else if (min_z == max_z - 1) {
        #   
        # }

compile_range(0x90, 0x6F0)

