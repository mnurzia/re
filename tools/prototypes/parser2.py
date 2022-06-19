import time

class Reg:
    def __init__(self, type):
        self.type = type
        self.children = 0
    
    def add_child(self):
        if self.children == self.max_children:
            raise Exception("too many kids")
        else:
            self.children += 1
    
    def set_children(self, num):
        if self.max_children != -1:
            if num >= self.max_children:
                raise Exception("too many kids")
        self.children = num
    
    def num_children(self):
        return self.children
    
    def __str__(self):
        return ""

class Group(Reg):
    def __init__(self):
        self.max_children = 1
        Reg.__init__(self, "group")

class Concat(Reg):
    def __init__(self):
        self.max_children = -1
        Reg.__init__(self, "concat")

class Alt(Reg):
    def __init__(self):
        self.max_children = -1
        Reg.__init__(self, "alt")

class Char(Reg):
    def __init__(self, ch):
        self.max_children = 0
        Reg.__init__(self, "char")
        self.ch = ch
    
    def __str__(self):
        return self.ch

class Question(Reg):
    def __init__(self):
        self.max_children = 1
        Reg.__init__(self, "quest")

class Op:
    def __init__(self, type):
        self.type = type

OP_CONCAT = "concat"
OP_ALT = "alt"
OP_GROUP = "group"

order = [OP_GROUP, OP_ALT, OP_CONCAT]

class Builder:
    def __init__(self):
        self.reg_stk = []
        self.op_stk = []
        self.base_ptr = 0
        self.stk_ptr_prev = 0
        self.stk_ptr = 0
        self.push_op_node(OP_GROUP, Group())
    
    def parent(self):
        return self.reg_stk[self.base_ptr]

    def parent_set_children(self, num):
        self.reg_stk[self.base_ptr].set_children(num)

    def parent_add_child(self):
        self.reg_stk[self.base_ptr].add_child()

    def parent_num_children(self):
        return self.reg_stk[self.base_ptr].num_children()

    def has_previous_node(self):
        return self.stk_ptr_prev != self.stk_ptr

    def push_node(self, node):
        self.reg_stk.append(node)
        self.stk_ptr += 1
        if self.stk_ptr_prev != self.stk_ptr:
            self.stk_ptr_prev = self.stk_ptr - 1

    def insert_node(self, node, where):
        self.reg_stk.insert(where, node)
        self.stk_ptr += 1
        if self.stk_ptr_prev != self.stk_ptr:
            self.stk_ptr_prev = self.stk_ptr - 1

    def prev_node(self):
        return self.reg_stk[self.stk_ptr_prev]

    def push_op_node(self, op, node, base=None):
        if base == None:
            self.op_stk.append((op, self.base_ptr, self.stk_ptr_prev))
            self.base_ptr = self.stk_ptr
            self.push_node(node)
            self.stk_ptr_prev = self.stk_ptr
        else:
            self.op_stk.append((op, self.base_ptr, self.stk_ptr_prev))
            self.base_ptr = base
            self.insert_node(node, base)
            self.stk_ptr_prev = self.stk_ptr
    
    def pop_op(self):
        (last_op, last_base, last_stk_prev) = self.op_stk.pop()
        self.base_ptr = last_base
        if last_op != OP_CONCAT:
            self.stk_ptr_prev = last_stk_prev
        else:
            self.stk_ptr_prev = self.stk_ptr
        return last_op
    
    def peek_op(self):
        if len(self.op_stk) > 0:
            return self.op_stk[-1][0]
        else:
            return None

    def parse_reg_in(self, regex):
        for ch in regex:
            self.dump(ch)
            self.wait()
            if ch == '(':
                self.parent_add_child()
                self.push_op_node(OP_GROUP, Group())
            elif ch == ')':
                while True:
                    last_op = self.peek_op()
                    if last_op == OP_ALT:
                        self.parent_add_child()
                    elif last_op == OP_GROUP:
                        break
                    else:
                        self.pop_op()
            elif ch == '|':
                # Pop until we get a group or an alt
                last_op = None
                while True:
                    last_op = self.peek_op()
                    if last_op == OP_GROUP or last_op == OP_ALT:
                        break
                    else:
                        self.pop_op()
                if last_op == OP_GROUP:
                    # Shim in a new alt
                    if self.parent_num_children() == 0:
                        raise Exception("can't alt with nothing")
                    elif self.parent_num_children() == 1:
                        self.push_op_node(OP_ALT, Alt(), self.base_ptr+1)
                        self.parent_add_child()
                    else:
                        raise Exception("group with more than 1 child??")
                elif last_op == OP_ALT:
                    # Add to the existing alt
                    self.parent_add_child()
            else:
                top_op = self.peek_op()
                if top_op == OP_GROUP:
                    if self.parent_num_children() == 0:
                        self.parent_add_child()
                    elif self.parent_num_children() == 1:
                        self.push_op_node(OP_CONCAT, Concat(), self.base_ptr+1)
                        self.parent_set_children(2)
                    else:
                        assert False
                elif top_op == OP_ALT:
                    if self.parent_num_children() == 0:
                        assert False
                    else:
                        if not self.has_previous_node():
                            self.parent_add_child()
                        else:
                            self.push_op_node(OP_CONCAT, Concat(), self.stk_ptr_prev)
                            self.parent_set_children(2)
                else:
                    self.parent_add_child()
                self.push_node(Char(ch))
        self.dump(ch)
        self.wait()

    def dump(self, ch):
        print("\x1b[2J  ch: %s" % ch)
        print("  stk_ptr: %i" % self.stk_ptr)
        print("  stk_ptr_prev: %i" % self.stk_ptr_prev)
        print("  base_ptr: %i" % self.base_ptr)
        print("op_stk:")
        for op, base_ptr, stk_ptr_prev in self.op_stk:
            print(op, base_ptr, stk_ptr_prev)
        print("stk:")
        self.dump_stack()
    
    def dump_stack(self):
        for i, node in enumerate(self.reg_stk):
            print(" ",i, node.type, node.num_children(), str(node))
    
    def wait(self):
        input()

class Builder2:
    def __init__(self):
        self.op_stk = []
        self.reg_stk = [Group()] 
        self.stk_ptr = 1
        self.stk_ptr_prev = 1
        self.base_ptr = 0
        self.inserts = 0
        self.adds = 0
        self.depth = 1
        self.depth_max = 1
        self.depth_prev = 1
    
    def push_node(self, node):
        self.reg_stk.append(node)
        if self.stk_ptr == self.stk_ptr_prev:
            self.stk_ptr += 1
        else:
            self.stk_ptr += 1
            self.stk_ptr_prev = self.stk_ptr - 1
            self.depth_prev = self.depth
        self.adds += 1

    def push_node_concat(self, node):
        base_type = self.parent().type
        if base_type == "group" or base_type == "alt":
            if self.stk_ptr == self.stk_ptr_prev:
                # No children
                self.push_node(node)
                self.parent().add_child()
            else:
                # At least one child
                self.op_stk.append((self.base_ptr, self.stk_ptr_prev, self.depth, self.depth_max))
                self.base_ptr = self.stk_ptr_prev
                self.wrap_node(Concat())
                self.depth += 1
                self.push_node(node)
                self.parent().add_child()
        else:
            self.push_node(node)
            self.parent().add_child()

    def wrap_node(self, node):
        self.depth_prev += 1
        self.depth_max = max(self.depth_prev, self.depth_max)
        self.reg_stk.insert(self.stk_ptr_prev, node)
        self.reg_stk[self.stk_ptr_prev].add_child()
        self.stk_ptr += 1
        self.inserts += 1

    def parent(self):
        return self.reg_stk[self.base_ptr]

    def calc_max_depth(self, idx, lvl):
        self.depth_calc = max(lvl, self.depth_calc)
        next = idx + 1
        for i in range(self.reg_stk[idx].children):
            next += self.calc_max_depth(next, lvl+1)
        return next - idx

    def parse_reg_in(self, regex):
        for i in range(len(regex) + 1):
            if i == len(regex):
                ch = "last"
            else:
                ch = regex[i]
            self.dump(ch)
            self.wait()
            if ch == "last":
                while True:
                    peek_type = self.reg_stk[self.base_ptr].type
                    if peek_type == "concat" or peek_type == "alt":
                        # can pop safely
                        self.depth_prev = self.depth_max
                        (self.base_ptr, self.stk_ptr_prev, self.depth, dmax) = self.op_stk.pop()
                        self.depth_max = max(self.depth_max, dmax)
                    elif peek_type == "group":
                        if self.base_ptr == 0:
                            break
                        # Not ok.
                        raise Exception("unmatched (")
                print("inserts:", self.inserts)
                print("adds:", self.adds)
                print("max depth:", self.depth_max)
                self.depth_calc = 0
                self.calc_max_depth(0, 0)
                print("calc max depth:", self.depth_calc)
            elif ch == '(':
                self.push_node_concat(Group())
                self.op_stk.append((self.base_ptr, self.stk_ptr_prev, self.depth, self.depth_max))
                self.depth += 1
                self.depth_prev = self.depth
                self.depth_max = self.depth
                self.base_ptr = self.stk_ptr - 1
                self.stk_ptr_prev = self.stk_ptr
            elif ch == ')':
                while True:
                    peek_type = self.reg_stk[self.base_ptr].type
                    self.depth_prev = self.depth_max
                    (self.base_ptr, self.stk_ptr_prev, self.depth, dmax) = self.op_stk.pop()
                    self.depth_max = max(self.depth_max, dmax)
                    if peek_type == "group":
                        break
            elif ch == '?':
                if self.stk_ptr == self.stk_ptr_prev:
                    raise Exception("can't ? nothing")
                else:
                    self.wrap_node(Question())
            elif ch == '|':
                peek_type = None
                while True:
                    peek_type = self.reg_stk[self.base_ptr].type
                    if peek_type == "concat":
                        self.depth_prev = self.depth_max
                        (self.base_ptr, self.stk_ptr_prev, self.depth, dmax) = self.op_stk.pop()
                        self.depth_max = max(self.depth_max, dmax)
                    elif peek_type == "group" or peek_type == "alt":
                        break
                if peek_type == "group":
                    # Shim in alt
                    self.wrap_node(Alt())
                    self.op_stk.append((self.base_ptr, self.stk_ptr_prev, self.depth, self.depth_max))
                    self.depth += 1
                    self.depth_prev = self.depth
                    self.depth_max = max(self.depth_max, self.depth)
                    self.base_ptr = self.stk_ptr_prev
                    self.stk_ptr_prev = self.stk_ptr
                elif peek_type == "alt":
                    self.stk_ptr_prev = self.stk_ptr
                    self.depth_max = max(self.depth_prev, self.depth_max)
                    self.depth_prev = self.depth
            else:
                self.push_node_concat(Char(ch))
    
    def dump(self, ch):
        print("\x1b[2J  ch: %s" % ch)
        print("  stk_ptr: %i" % self.stk_ptr)
        print("  stk_ptr_prev: %i" % self.stk_ptr_prev)
        print("  base_ptr: %i" % self.base_ptr)
        print("  depth_max: %i" % self.depth_max)
        print("  depth: %i" % self.depth)
        print("  depth_prev: %i" % self.depth_prev)
        print("op_stk:")
        for i, (base_ptr, stk_ptr_prev, depth, depth_prev) in enumerate(self.op_stk):
            print(i, base_ptr, stk_ptr_prev, self.reg_stk[base_ptr].type, depth, depth_prev)
        print("stk:")
        self.dump_stack()

    def get_flags(self, i):
        flags = ""
        if i == self.base_ptr:
            flags += "B"
        if i == self.stk_ptr_prev:
            flags += "P"
        if i == self.stk_ptr:
            flags += "S"
        return flags.ljust(3)

    def dump_stack(self):
        for i, node in enumerate(self.reg_stk):
            print(self.get_flags(i),i, node.type, node.num_children(), str(node))
        print(self.get_flags(len(self.reg_stk)))
    
    def wait(self):
        input()

b = Builder2()
b.parse_reg_in("(ab)(ab)((a)ab)(ab)")
