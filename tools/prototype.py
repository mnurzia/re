from re import L
import string

class Reg:
    def __init__(self, type):
        self.type = type
        self.children = []
    
    def add_child(self, child):
        self.children.append(child)
    
    def get_child(self, index):
        return self.children[index]

    def get_children_len(self):
        return len(self.children)

    def __repr__(self):
        return "Reg<%s,%i>" % (self.type, len(self.children))

class RegChar(Reg):
    def __init__(self, ch):
        Reg.__init__(self, "char")
        if isinstance(ch, int):
            self.ch = chr(ch)
        else:
            self.ch = ch
    
    def get_ch(self):
        return self.ch
    
    def __str__(self):
        if self.ch in string.printable:
            return "Char: ord=0x%X '%s'" % (ord(self.ch), self.ch)
        else:
            return "Char: ord=0x%X" % ord(self.ch)

class RegCharString(Reg):
    def __init__(self, ch):
        Reg.__init__(self, "char_string")
        self.str = ch
    
    def add_char(self, ch):
        self.str = self.str + ch
    
    def get_str(self):
        return self.str
    
    def __str__(self):
        if all(ch in string.printable for ch in self.str):
            return "Str: len=%i '%s'" % (len(self.str), self.str)
        else:
            return "Str: len=%i" % len(self.str)

class RegCharRange(Reg):
    def __init__(self, ch_lo, ch_hi):
        Reg.__init__(self, "char_range")
        self.ch_lo = ch_lo
        self.ch_hi = ch_hi
    
    def __str__(self):
        if self.ch_lo in string.printable and self.ch_hi in string.printable:
            return "CharRange: lo=%i '%s' hi=%i '%s'" % \
                (ord(self.ch_lo), self.ch_lo, ord(self.ch_hi), self.ch_hi)
        else:
            return "CharRange: lo=%i hi=%i" % (ord(self.ch_lo), ord(self.ch_hi))

class RegCharClass(Reg):
    def __init__(self):
        Reg.__init__(self, "char_class")
        self.ranges = []

    def add_range(self, lo, hi):
        self.ranges.append([lo, hi])
    
    def __str__(self):
        return "CharClass: " + ", ".join(["[0x%X-0x%X]" % (ord(a[0]), ord(a[1])) for a in self.ranges])

CHAR_CLASS_DIGITS = RegCharClass()
CHAR_CLASS_DIGITS.add_range("0", "9")

class RegConcat(Reg):
    def __init__(self):
        Reg.__init__(self, "concat")
    
    def __str__(self):
        return "Concat: len=%i" % len(self.children)

class RegAlt(Reg):
    def __init__(self):
        Reg.__init__(self, "alt")
    
    def __str__(self):
        return "Alt: len=%i" % len(self.children)

class RegGreedy(Reg):
    def __init__(self, type):
        Reg.__init__(self, type)
        self.greedy = True
    
    def set_greedy(self, greedy):
        self.greedy = greedy
    
    def get_greedy(self):
        return self.greedy
    
    def __repr__(self):
        return "RegGreedy<%s,%s>" % (Reg.__repr__(self), self.greedy)

class RegKleene(RegGreedy):
    def __init__(self):
        RegGreedy.__init__(self, "kleene")
    
    def __str__(self):
        if self.get_greedy():
            return "Kleene:"
        else:
            return "Kleene: greedy=True"

class RegPlus(RegGreedy):
    def __init__(self):
        RegGreedy.__init__(self, "plus")
    
    def __str__(self):
        if self.get_greedy():
            return "Plus:"
        else:
            return "Plus: greedy=True"

class RegQuestion(RegGreedy):
    def __init__(self):
        RegGreedy.__init__(self, "question")
    
    def __str__(self):
        if self.get_greedy():
            return "Question:"
        else:
            return "Question: greedy=True"

class RegCapture(Reg):
    def __init__(self):
        Reg.__init__(self, "capture")
    
    def __str__(self):
        return "Capture:"

class RegTextStart(Reg):
    def __init__(self):
        Reg.__init__(self, "text_start")
    
    def __str__(self):
        return "TextStart"

class RegTextEnd(Reg):
    def __init__(self):
        Reg.__init__(self, "text_end")
    
    def __str__(self):
        return "TextEnd"

class RegWordBoundary(Reg):
    def __init__(self):
        Reg.__init__(self, "word_boundary")
    
    def __str__(self):
        return "WordBoundary"

class RegWordBoundaryNot(Reg):
    def __init__(self):
        Reg.__init__(self, "word_boundary_not")
    
    def __str__(self):
        return "WordBoundaryNot"

def build_reg_post(post_in):
    frag_stack = []
    for i, ch in enumerate(post_in):
        if ch == ".":
            e2 = frag_stack.pop()
            e1 = frag_stack.pop()
            out = RegConcat()
            out.add_child(e1)
            out.add_child(e2)
            frag_stack.append(out)
        elif ch == "*":
            e1 = frag_stack.pop()
            out = RegKleene()
            out.add_child(e1)
            frag_stack.append(out)
        elif ch == "|":
            e2 = frag_stack.pop()
            e1 = frag_stack.pop()
            out = RegAlt()
            out.add_child(e1)
            out.add_child(e2)
            frag_stack.append(out)
        elif ch == "?":
            e1 = frag_stack.pop()
            out = RegQuestion()
            out.add_child(e1)
            frag_stack.append(out)
        elif ch == "+":
            e1 = frag_stack.pop()
            out = RegPlus()
            out.add_child(e1)
            frag_stack.append(out)
        else:
            frag_stack.append(RegCharRange(ch, chr(ord(ch) + 1)))
    return frag_stack[0]

def concat_stk(reg_stk, reg_stk_begin):
    while len(reg_stk) > reg_stk_begin + 1:
        e2 = reg_stk.pop()
        e1 = reg_stk.pop()
        if type(e1) is RegChar and type(e2) is RegChar:
            out = RegCharString(e1.get_ch())
            out.add_char(e2.get_ch())
        elif type(e1) is RegCharString and type(e2) is RegChar:
            out = e1
            out.add_char(e2.get_ch())
        elif type(e1) is RegConcat:
            out = e1
            out.add_child(e2)
        else:
            out = RegConcat()
            out.add_child(e1)
            out.add_child(e2)
        reg_stk.append(out)

def alt_stk(reg_stk, reg_stk_begin):
    e2 = reg_stk.pop()
    e1 = reg_stk.pop()
    if type(e2) is RegAlt:
        if type(e1) is RegAlt:
            out = e1
            out.children.extend(e2.children)
        else:
            out = e2
            out.children.insert(0, e1)
    else:
        out = RegAlt()
        out.add_child(e1)
        out.add_child(e2)
    reg_stk.append(out)

def build_char_class(char_classes):
    if len(char_classes) == 0:
        raise Exception("empty char class")
    else:
        if len(char_classes) == 1 and len(char_classes[0]) == 1:
            return RegChar(char_classes[0])
        elif len(char_classes) == 1 and len(char_classes[0]) == 2:
            return RegCharRange(char_classes[0][0], char_classes[0][1])
        else:
            out_alt = RegAlt()
            for char_class in char_classes:
                if len(char_class) == 1:
                    out_alt.add_child(RegChar(char_class[0]))
                else:
                    out_alt.add_child(RegCharRange(char_class[0], char_class[1]))
            return out_alt

DEC_DIGITS = ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9"]
MAX_OCTAL_DIGITS = 8

def build_reg_in(in_in):
    PARSE_GND = 0
    PARSE_ESCAPE = 1
    PARSE_ESCAPE_OCTAL = 2
    PARSE_CHARCLASS_INITIAL = 3
    PARSE_CHARCLASS_INITIAL2 = 4
    PARSE_CHARCLASS_AFTERFIRST = 5
    PARSE_CHARCLASS_AFTERHYPHEN = 6
    op_stk = []
    reg_stk = []
    reg_stk_begin = 0
    state = PARSE_GND
    char_classes = []
    escape_char_ord = 0
    escape_char_digits = 0
    delay = 0
    index = 0
    last = None
    while index != len(in_in) + 1:
        if index == len(in_in):
            ch = None
            last = True
        else:
            ch = in_in[index]
            last = None
        if state == PARSE_GND:
            if ch == "\\":
                concat_stk(reg_stk, reg_stk_begin)
                op_stk.append(("\\", reg_stk_begin, state))
                reg_stk_begin = len(reg_stk)
                state = PARSE_ESCAPE
            elif ch == "(":
                concat_stk(reg_stk, reg_stk_begin)
                op_stk.append(("(", reg_stk_begin, state))
                reg_stk_begin = len(reg_stk)
            elif ch == ")":
                concat_stk(reg_stk, reg_stk_begin)
                while 1:
                    if len(op_stk) == 0:
                        raise Exception("unmatched right parentheses")
                    op, reg_stk_begin, _ = op_stk.pop()
                    if op == "(":
                        break
                    elif op == "|":
                        if len(reg_stk) < reg_stk_begin + 2:
                            raise Exception("can't alternate with empty value")
                        alt_stk(reg_stk, reg_stk_begin)
                e1 = reg_stk.pop()
                out = RegCapture()
                out.add_child(e1)
                reg_stk.append(out)
            elif ch == "|":
                concat_stk(reg_stk, reg_stk_begin)
                if len(reg_stk) - reg_stk_begin < 1:
                    raise Exception("can't alternate with empty value")
                op_stk.append(("|", reg_stk_begin, state))
                reg_stk_begin = len(reg_stk)
            elif ch == "*":
                if len(reg_stk) == reg_stk_begin:
                    raise Exception("can't kleene nothing")
                out = RegKleene()
                out.add_child(reg_stk.pop())
                reg_stk.append(out)
            elif ch == "+":
                if len(reg_stk) == reg_stk_begin:
                    raise Exception("can't plus nothing")
                out = RegPlus()
                out.add_child(reg_stk.pop())
                reg_stk.append(out)
            elif ch == "?":
                if len(reg_stk) == reg_stk_begin:
                    raise Exception("can't question nothing")
                top = reg_stk.pop()
                if type(top) in [RegKleene, RegPlus, RegQuestion]:
                    top.set_greedy(False)
                    reg_stk.append(top)
                else:
                    out = RegQuestion()
                    out.add_child(reg_stk.pop())
                    reg_stk.append(out)
            elif ch == "^":
                concat_stk(reg_stk, reg_stk_begin)
                reg_stk.append(RegTextStart())
            elif ch == "$":
                concat_stk(reg_stk, reg_stk_begin)
                reg_stk.append(RegTextEnd())
            elif ch == "[":
                state = PARSE_CHARCLASS_INITIAL
            elif last:
                # Condense stack to one value
                concat_stk(reg_stk, reg_stk_begin)
                while 1:
                    if len(op_stk) == 0:
                        break
                    (op, reg_stk_begin, _) = op_stk.pop()
                    if op == "(":
                        raise Exception("unmatched left parentheses")
                    elif op == "|":
                        if len(reg_stk) < reg_stk_begin + 2:
                            raise Exception("can't alternate with empty value")
                        alt_stk(reg_stk, reg_stk_begin)
            else:
                concat_stk(reg_stk, reg_stk_begin)
                reg_stk.append(RegChar(ch))
        elif state == PARSE_ESCAPE:
            finish = False
            if ch in DEC_DIGITS: 
                # Octal escape
                # Because we don't do backreferences, no disambiguation is
                # needed, we just treat them like octals. 
                escape_char_ord = 0
                escape_char_ord += DEC_DIGITS.index(ch)
                escape_char_digits = 1
                state = PARSE_ESCAPE_OCTAL
            elif ch == 'a':
                # Alarm / bell
                reg_stk.append(RegChar("\x07")) # bell character
                finish = True
            elif ch == 'A':
                # Beginning of string, when not in []
                if op_stk[-1][0] == "\\":
                    reg_stk.append(RegTextStart())
                    finish = True
                else:
                    raise Exception("cannot use '\A' within []")
            elif ch == 'b':
                # Word boundary.
                if op_stk[-1][0] == "\\":
                    reg_stk.append(RegWordBoundary())
                else:
                    # Backspace when [].
                    reg_stk.append(RegChar("\x08"))
                finish = True
            elif ch == 'B':
                # Not word boundary.
                if op_stk[-1][0] == "\\":
                    reg_stk.append(RegWordBoundaryNot())
                    finish = True
                else:
                    raise Exception("cannot use '\B' within []")
            elif ch == 'c':
                # Control-X, std control character
                # Don't support for now
                raise Exception("cannot use \c")
            elif ch == 'd':
                # Digits.
                cl = RegCharClass()
                cl.add_range(ord("0"), ord)
                reg_stk.append(RegCharClass())
            elif last:
                raise Exception("empty escape")
            else:
                raise Exception("invalid character in escape")
            if finish:
                _, reg_stk_begin, state = op_stk.pop()
        elif state == PARSE_ESCAPE_OCTAL:
            # Octal, after first digit
            if ch in DEC_DIGITS:
                if escape_char_digits < MAX_OCTAL_DIGITS:
                    escape_char_ord *= 8
                    escape_char_ord += DEC_DIGITS.index(ch)
                    escape_char_digits += 1
                else:
                    raise Exception("octal literal too long")
            else: # includes last 
                # Defer parsing of next char to previous state, as there is
                # no way to disambiguate without lookahead
                reg_stk.append(RegChar(escape_char_ord))
                # Go back 
                _, reg_stk_begin, state = op_stk.pop()
                escape_char_ord = 0
                escape_char_digits = 0
                delay += 1
        #Initial (after [)
        # - -> add -> Initial2
        # ] -> finish -> Gnd
        # ^ -> negate -> Initial2
        # * -> add -> AfterFirst
        #Initial2 (after ^)
        # - -> add -> Initial2
        # ] -> finish -> Gnd
        # * -> add -> AfterFirst
        #AfterFirst (after first character in range)
        # - -> nothing -> AfterHyphen
        # ] -> finish -> Gnd
        # * -> add -> AfterFirst
        #AfterHyphen (after hyphen)
        # - -> push -> Initial2
        # ] -> add -> Gnd
        # * -> push -> Initial2
        elif state == PARSE_CHARCLASS_INITIAL:
            if ch == "-":
                char_classes.append(["-"])
                state = PARSE_CHARCLASS_INITIAL2
            elif ch == "]":
                concat_stk(reg_stk, reg_stk_begin)
                reg_stk.append(build_char_class(char_classes))
                state = PARSE_GND
            elif ch == "^":
                negate_char_class = True
                state = PARSE_CHARCLASS_INITIAL2
            elif last:
                raise Exception("unterminated char class")
            else:
                char_classes.append([ch])
                state = PARSE_CHARCLASS_AFTERFIRST
        elif state == PARSE_CHARCLASS_INITIAL2:
            if ch == "-":
                char_classes.append(["-"])
                state = PARSE_CHARCLASS_INITIAL2
            elif ch == "]":
                concat_stk(reg_stk, reg_stk_begin)
                reg_stk.append(build_char_class(char_classes))
                state = PARSE_GND
            elif last:
                raise Exception("unterminated char class")
            else:
                char_classes.append([ch])
                state = PARSE_CHARCLASS_AFTERFIRST
        elif state == PARSE_CHARCLASS_AFTERFIRST:
            if ch == "-":
                state = PARSE_CHARCLASS_AFTERHYPHEN
            elif ch == "]":
                concat_stk(reg_stk, reg_stk_begin)
                reg_stk.append(build_char_class(char_classes))
                state = PARSE_GND
            elif last:
                raise Exception("unterminated char class")
            else:
                char_classes.append([ch])
                state = PARSE_CHARCLASS_AFTERFIRST
        elif state == PARSE_CHARCLASS_AFTERHYPHEN:
            if ch == "-":
                char_classes[-1].append(ch)
                state = PARSE_CHARCLASS_INITIAL2
            elif ch == "]":
                concat_stk(reg_stk, reg_stk_begin)
                reg_stk.append(build_char_class(char_classes))
                state = PARSE_GND
            elif last:
                raise Exception("unterminated char class")
            else:
                char_classes[-1].append(ch)
                state = PARSE_CHARCLASS_INITIAL2
        if delay > 0:
            delay -= 1
        else:
            index += 1
    return reg_stk[0]

def dump_reg(reg, lvl=0):
    print("\t"*lvl + str(reg))
    for child in reg.children:
        dump_reg(child, lvl+1)

class Inst:
    def __init__(self, type, next):
        self.type = type
        self.next = next
    
    def set_next(self, next):
        self.next = next

    def __repr__(self):
        return "Inst<%s,%i>" % (self.type, self.next)

class InstCharRange(Inst):
    def __init__(self, lo, hi, next):
        Inst.__init__(self, "range", next)
        self.lo = lo
        self.hi = hi
    
    def __str__(self):
        return "range[%02X-%02X] -> %i" % (self.lo, self.hi, self.next)

class InstSplit(Inst):
    def __init__(self, next, next1):
        Inst.__init__(self, "split", next)
        self.next1 = next1
    
    def set_next1(self, next1):
        self.next1 = next1
    
    def __str__(self):
        return "split -> %i and -> %i" % (self.next, self.next1)

class InstMatch(Inst):
    def __init__(self, index=0):
        Inst.__init__(self, "match", 0)
        self.index = index
    
    def __str__(self):
        return "match id %i" % self.index

class InstFail(Inst):
    def __init__(self):
        Inst.__init__(self, "fail", 0)
    
    def __str__(self):
        return "fail"

class InstSave(Inst):
    def __init__(self, index, next):
        Inst.__init__(self, "save", next)
        self.index = index
    
    def __str__(self):
        return "save to %i -> %i" % (self.index, self.next)

class InstAssert(Inst):
    TEXT_START = "text_start"
    TEXT_END = "text_end"

    def __init__(self, looking_for, next):
        Inst.__init__(self, "Assert", next)
        self.looking_for = looking_for
    
    def __str__(self):
        out = " ".join(self.looking_for)
        return "assert " + out + " -> %i" % self.next

class Expr:
    def __init__(self, start):
        self.start = start
        self.end = self.start
        self.patches = []
    
    def get_start(self):
        return self.start
    
    def set_end(self, end):
        self.end = end

    def get_end(self):
        return self.end

    def add_patch(self, pc, id):
        self.patches.append((pc, id))
    
    def add_patches(self, patches):
        self.patches.extend(patches)
    
    def get_patches(self):
        return self.patches

    def __repr__(self):
        return "Expr<%i, %i, %i>" % (self.start, self.end, len(self.patches))

#  |
# a b

# | a | b | 

# a*b*|

#   |
# *  *
# a  b

# |       |       |
#   *   *   *   *
#     a       b

class Compiler:
    def __init__(self):
        self.program = [InstFail()]

    def compile(self, in_in):
        reg = build_reg_in(in_in)
        dump_reg(reg)
        self.start_search = len(self.program)
        # add non-greedy dotstar for searches
        self.program.append(InstSplit(3, 2))
        self.program.append(InstCharRange(0, 256, 1))
        self.start = len(self.program)
        self.save_ctr = 0
        out_expr = self.compile_reg_new(reg)
        pc = len(self.program)
        self.program.append(InstMatch())
        self.patch(out_expr, pc)
        return self.program
    
    def patch(self, expr, in_pc):
        for pc, id in expr.patches:
            if id == 0:
                self.program[pc].set_next(in_pc)
            elif id == 1:
                self.program[pc].set_next1(in_pc)

    def compile_reg(self, reg):
        if type(reg) is RegChar:
            pc = len(self.program)
            self.program.append(InstCharRange(ord(reg.get_ch()), ord(reg.get_ch())+1, 0))
            out_expr = Expr(pc)
            out_expr.add_patch(pc, 0)
            return out_expr
        elif type(reg) is RegTextStart:
            pc = len(self.program)
            self.program.append(InstAssert(set([InstAssert.TEXT_START]), 0))
            out_expr = Expr(pc)
            out_expr.add_patch(pc, 0)
            return out_expr
        elif type(reg) is RegTextEnd:
            pc = len(self.program)
            self.program.append(InstAssert(set([InstAssert.TEXT_END]), 0))
            out_expr = Expr(pc)
            out_expr.add_patch(pc, 0)
            return out_expr
        elif type(reg) is RegCharString:
            pc = len(self.program)
            start_pc = pc
            for ch in reg.get_str()[:-1]:
                self.program.append(InstCharRange(ord(ch), ord(ch)+1, pc+1))
                pc += 1
            self.program.append(InstCharRange(ord(reg.get_str()[-1]), ord(reg.get_str()[-1])+1, 0))
            out_expr = Expr(start_pc)
            out_expr.add_patch(pc, 0)
            return out_expr
        elif type(reg) is RegCharRange:
            pc = len(self.program)
            self.program.append(InstCharRange(ord(reg.get_ch_lo()), ord(reg.get_ch_hi()), 0))
            out_expr = Expr(pc)
            out_expr.add_patch(pc, 0)
            return out_expr
        elif type(reg) is RegConcat:
            pc = len(self.program)
            sub_expr_0 = self.compile_reg(reg.get_child(0))
            last = sub_expr_0
            for child in reg.children[1:]:
                next = self.compile_reg(child)
                self.patch(last, next.start)
                last = next
            out_expr = Expr(pc)
            out_expr.add_patches(last.get_patches())
            return out_expr
        elif type(reg) is RegAlt:
            start_pc = len(self.program)
            last_pc = start_pc
            out_expr = Expr(start_pc)
            for i, child in enumerate(reg.children[:-1]):
                last_pc = len(self.program)
                self.program.append(InstSplit(last_pc+1, 0))
                sub_expr = self.compile_reg(child)
                out_expr.add_patches(sub_expr.get_patches())
                self.program[last_pc].set_next1(len(self.program))
            final_pc = len(self.program)
            sub_expr = self.compile_reg(reg.children[-1])
            out_expr.add_patches(sub_expr.get_patches())
            self.program[last_pc].set_next1(final_pc)
            return out_expr
        elif type(reg) is RegKleene:
            pc = len(self.program)
            self.program.append(InstSplit(pc+1, 0))
            sub_expr_0 = self.compile_reg(reg.get_child(0))
            out_expr = Expr(pc)
            self.patch(sub_expr_0, pc)
            out_expr.add_patch(pc, 1)
            return out_expr
        elif type(reg) is RegPlus:
            pc = len(self.program)
            sub_expr_0 = self.compile_reg(reg.get_child(0))
            end_pc = len(self.program)
            self.program.append(InstSplit(pc, 0))
            self.patch(sub_expr_0, end_pc)
            out_expr = Expr(pc)
            out_expr.add_patch(end_pc, 1)
            return out_expr
        elif type(reg) is RegQuestion:
            pc = len(self.program)
            self.program.append(InstSplit(pc+1, 0))
            sub_expr_0 = self.compile_reg(reg.get_child(0))
            out_expr = Expr(pc)
            out_expr.add_patches(sub_expr_0.get_patches())
            out_expr.add_patch(pc, 1)
            return out_expr
        elif type(reg) is RegCapture:
            pc = len(self.program)
            old_save_ctr = self.save_ctr
            self.program.append(InstSave(old_save_ctr, pc+1))
            self.save_ctr += 2
            sub_expr_0 = self.compile_reg(reg.get_child(0))
            end_pc = len(self.program)
            self.program.append(InstSave(old_save_ctr+1, 0))
            self.patch(sub_expr_0, end_pc)
            out_expr = Expr(pc)
            out_expr.add_patch(end_pc, 0)
            return out_expr
    
    def compile_reg_new(self, reg_in):
        stk = [(reg_in, 0, Expr(len(self.program)))]
        ret = None
        while len(stk) > 0:
            reg, idx, expr = stk.pop()
            print("  "*len(stk), reg, idx, ret, stk)
            next_pc = len(self.program)
            if type(reg) == RegChar:
                self.program.append(InstCharRange(ord(reg.get_ch()), ord(reg.get_ch())+1, 0))
                expr.set_end(next_pc + 1)
                expr.add_patch(next_pc, 0)
            elif type(reg) is RegCharString:
                for i, ch in enumerate(reg.get_str()):
                    # may not be needed in c impl
                    next_instruction_pc = len(self.program)+1
                    if i == len(reg.get_str()):
                        next_instruction_pc = 0
                    self.program.append(InstCharRange(ord(ch), ord(ch)+1, next_instruction_pc))
                expr.set_end(len(self.program) + 1)
                expr.add_patch(len(self.program), 0)
            elif type(reg) is RegCharRange:
                self.program.append(InstCharRange(ord(reg.ch_lo()), ord(reg.ch_hi()), 0))
                expr.set_end(next_pc + 1)
                expr.add_patch(next_pc, 0)
            elif type(reg) == RegConcat:
                length = reg.get_children_len()
                if idx < length:
                    if idx == 0:
                        # before first child
                        pass
                    else:
                        # before intermediate children and last child
                        expr.set_end(next_pc)
                        self.patch(ret, next_pc)
                    stk.append((reg, idx+1, expr))
                    stk.append((reg.get_child(idx), 0, Expr(next_pc)))
                else:
                    # after last child
                    expr.add_patches(ret.get_patches())
                    expr.set_end(next_pc)
            elif type(reg) == RegAlt:
                length = reg.get_children_len()
                if idx < length:
                    if idx == 0:
                        # before first child
                        self.program.append(InstSplit(next_pc+1, 0))
                    elif idx < length - 1:
                        # before intermediate children
                        self.program[expr.get_end()].set_next1(next_pc)
                        expr.add_patches(ret.get_patches())
                        expr.set_end(next_pc)
                        self.program.append(InstSplit(next_pc+1, 0))
                    elif idx == length - 1:
                        # before last child
                        self.program[expr.get_end()].set_next1(next_pc)
                        expr.add_patches(ret.get_patches())
                        expr.set_end(next_pc)
                    stk.append((reg, idx+1, expr))
                    stk.append((reg.get_child(idx), 0, Expr(next_pc+1)))
                else:
                    # after last child
                    expr.add_patches(ret.get_patches())
                    expr.set_end(next_pc)
            elif type(reg) == RegKleene:
                if idx == 0:
                    # before child
                    if reg.get_greedy():
                        self.program.append(InstSplit(next_pc+1, 0))
                    else:
                        self.program.append(InstSplit(0, next_pc+1))
                    stk.append((reg, idx+1, expr))
                    stk.append((reg.get_child(0), 0, Expr(next_pc+1)))
                elif idx == 1:
                    # after child
                    self.patch(ret, expr.start)
                    if reg.get_greedy():
                        expr.add_patch(expr.start, 1)
                    else:
                        expr.add_patch(expr.start, 0)
                    expr.set_end(next_pc)
            elif type(reg) == RegPlus:
                if idx == 0:
                    # before child
                    stk.append((reg, idx+1, expr))
                    stk.append((reg.get_child(0), 0, Expr(next_pc+1)))
                elif idx == 1:
                    # after child
                    self.patch(ret, next_pc)
                    if reg.get_greedy():
                        self.program.append(InstSplit(expr.start, 0))
                        expr.add_patch(next_pc, 1)
                    else:
                        self.program.append(InstSplit(0, expr.start))
                        expr.add_patch(next_pc, 0)
                    expr.set_end(next_pc+1)
            elif type(reg) == RegQuestion:
                if idx == 0:
                    # before child
                    if reg.get_greedy():
                        self.program.append(InstSplit(next_pc+1, 0))
                    else:
                        self.program.append(InstSplit(0, next_pc+1))
                    stk.append((reg, idx+1, expr))
                    stk.append((reg.get_child(0), 0, Expr(next_pc+1)))
                elif idx == 1:
                    # after child
                    expr.add_patches(ret.get_patches())
                    if reg.get_greedy():
                        expr.add_patch(expr.start, 1)
                    else:
                        expr.add_patch(expr.start, 0)
                    expr.set_end(next_pc)
            elif type(reg) == RegCapture:
                if idx == 0:
                    # before child
                    self.program.append(InstSave(self.save_ctr, next_pc+1))
                    self.save_ctr += 2
                    stk.append((reg, idx+1, expr))
                    stk.append((reg.get_child(0), 0, Expr(next_pc+1)))
                elif idx == 1:
                    # after child
                    old_save_ctr = self.program[expr.get_start()].get_index()
                    self.program.append(InstSave(old_save_ctr+1, 0))
                    self.patch(ret, next_pc)
                    expr.add_patch(next_pc, 0)
                    expr.set_end(next_pc+1)
            elif type(reg) == RegTextStart:
                self.program.append(InstAssert(set([InstAssert.TEXT_START]), 0))
                expr.add_patch(next_pc, 0)
                expr.set_end(next_pc + 1)
            elif type(reg) == RegTextEnd:
                self.program.append(InstAssert(set([InstAssert.TEXT_END]), 0))
                expr.add_patch(next_pc, 0)
                expr.set_end(next_pc + 1)
            ret = expr
        return ret
    
    def evaluate_asserts(self, pos, text):
        out = set()
        if pos == 0:
            out |= set([InstAssert.TEXT_START])
        elif pos == len(text):
            out |= set([InstAssert.TEXT_END])
        return out

    def add_threads_new(self, origin, thrd_list, pos, text):
        stk = [origin]
        while len(stk):
            top = stk.pop()
            if top in thrd_list:
                continue
            else:
                thrd_list.append(top)
            op = self.program[top]
            if type(op) is InstSplit:
                stk.append(op.next1)
                stk.append(op.next)
            elif type(op) is InstSave:
                stk.append(op.next)
            elif type(op) is InstAssert:
                if op.looking_for.issubset(self.evaluate_asserts(pos, text)):
                    stk.append(op.next)

    def dump_program_graph(self, out, thrds=[]):
        import graphviz
        dot = graphviz.Digraph(comment="NFA Program")

        nodes = list(range(len(self.program)))
        edges = []
        for i, inst in enumerate(self.program):
            if type(inst) is not InstCharRange:
                if type(inst) is not InstMatch:
                    edges.append((i, inst.next, "red"))
                if type(inst) is InstSplit:
                    edges.append((i, inst.next1, "red"))
            else:
                edges.append((i, inst.next, "black"))
        
        for node in nodes:
            shape = "box"
            if node == 1:
                shape = "box"
            if type(self.program[node]) is InstMatch:
                shape = "box"
            color = "white"
            if node in thrds:
                color = "lightyellow"
            dot.node(str(node), shape=shape, label=str(self.program[node]), style="filled", fillcolor=color)
        
        for begin, end, color in edges:
            dot.edge(str(begin), str(end), color=color, taillabel=str(begin), headlabel=str(end))
        
        dot.render(filename=out, format="png", cleanup=True)

    def dump_program(self):
        for i, p in enumerate(self.program):
            print("%04i | %s" % (i, p))

    # match re     -> ^re.*$    "begin"
    # fullmatch re -> ^re$      "both"
    # search re    -> ^.*re.*$  "neither"

    # if anchor is "begin", then
      # on the first iteration
        # explore as usual
      # in between
        # if a match state is entered, finish with match early
      # at the end of the text
        # if we are in a match state finish with match
        # if we are in an EOT assertion finish with match
    # if anchor is "both" then
      # on the first iteration
        # explore as usual
      # in between
        # if a match state is entered, continue
      # at the end of the text
        # if we are in a match state finish with match
        # if we are in an EOT assertion finish with match
    # if anchor is "neither" then
      # on the first iteration
        # explore until we have some states
      # in between
        # if a match state is entered, finish with match early
      # at the end of the text
        # if we are in a match state finish with match
        # if we are in an EOT assertion finish with match
    
    def match_anchored(self, str, anchortype):
        thrds_a = []
        thrds_b = []
        if anchortype == "begin" or anchortype == "both":
            origin = self.start
        elif anchortype == "neither":
            origin = self.start_search
        self.add_threads_new(origin, thrds_a, 0, str)
        for i in range(len(str)+1):
            if i < len(str):
                ch = ord(str[i])
            else:
                ch = END_OF_TEXT
            print(i, ch, thrds_a)
            for thrd in thrds_a:
                op = prog[thrd]
                if type(op) is InstCharRange:
                    if ch >= op.lo and ch < op.hi:
                        self.add_threads_new(op.next, thrds_b, i, str)
                elif type(op) is InstMatch:
                    if anchortype == "begin" or anchortype == "neither":
                        # found a match early, return true early
                        return True
                    elif anchortype == "both":
                        if ch == END_OF_TEXT:
                            thrds_b.append(thrd)
                elif type(op) is InstAssert:
                    if ch == END_OF_TEXT:
                        if op.looking_for.issubset(self.evaluate_asserts(i, str)):
                            self.add_threads_new(op.next, thrds_b, i, str)
            thrds_a = thrds_b
            thrds_b = []
            print(i, ch, thrds_a)
        for thrd in thrds_a:
            if type(prog[thrd]) is InstMatch:
                return True
        return False

def follow(nfa):
    stk = [nfa]
    l = []
    while len(stk):
        top = stk[-1]
        for out in reversed(top.outs):
            if out[1] == "":
                if out[0] not in l:
                    stk.append(out[0])
        if stk[-1] not in l:
            l.append(stk.pop())
    return tuple(l)

def add_threads(prog, origin, l, asserts=set()):
    stk = [origin]
    while len(stk):
        top = stk[-1]
        if type(prog[top]) is InstSplit:
            if prog[top].next1 not in l:
                stk.append(prog[top].next1)
            if prog[top].next not in l:
                stk.append(prog[top].next)
        elif type(prog[top]) is InstSave:
            if prog[top].next not in l:
                stk.append(prog[top].next)
        elif type(prog[top]) is InstAssert:
            if prog[top].looking_for.issubset(asserts):
                if prog[top].next not in l:
                    stk.append(prog[top].next)
        if stk[-1] not in l:
            l.append(stk.pop())

END_OF_TEXT = -1

c = Compiler()

reg = r"\44"
test_string = "text...blabh"

dump_reg(build_reg_in(reg))

#prog = c.compile(reg)
#c.dump_program()
#print(c.match_anchored(test_string, "neither"))

#dump_reg(build_reg_in(reg))
#prog = c.compile(reg)
#dump_program(prog)

#print(full_match_new(prog, "aaaaabbbbbbcccccc"))
