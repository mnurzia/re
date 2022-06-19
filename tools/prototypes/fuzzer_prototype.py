# ^ -> ""
# $ -> ""
# \b -> 
# \B ->
# \A ->
# \Z ->

class RegPart:
    def __init__(self, type):
        self.type = type
        self.children = []
    
    def add_child(self, child):
        self.children.append(child)
    
    def __repr__(self):
        return f"RegPart<{self.type}, {len(self.children)}>"

    def children_to_regexp(self):
        return "".join([c.to_regexp() for c in self.children])

class Star(RegPart):
    def __init__(self):
        super().__init__(self, "Star")
    
    def to_regexp(self):
        if len(self.children) == 1:
            return self.children_to_regexp() + "*"
        else:
            return "(?:" + self.children_to_regexp() + ")*"

class Plus(RegPart):
    def __init__(self):
        super().__init__(self, "Plus")
    
    def to_regexp(self):
        if len(self.children) == 1:
            return self.children_to_regexp() + "+"
        else:
            return "(?:" + self.children_to_regexp() + ")+"

class Question(RegPart):
    def __init__(self):
        super().__init__(self, "Question")
    
    def to_regexp(self):
        if len(self.children) == 1:
            return self.children_to_regexp() + "?"
        else:
            return "(?:" + self.children_to_regexp() + ")?"

class Char(RegPart):
    def __init__(self, ord):
        self.ord = ord
        super().__init__(self, "Char")

class Class(RegPart):
    def __init__(self, ranges):
        self.ranges = ranges
        super().__init__(self, "Class")

class Group(RegPart):
    def __init__(self):
        super().__init__(self, "Group")

class Alt(RegPart):
    def __init__(self):
        super().__init__(self, "Alt")

import random

MIN_CONCATS = 0
MAX_CONCATS = 10
MIN_ALTS = 0
MAX_ALTS = 10
MIN_RANGES = 0
MAX_RANGES = 15

MAX_LVL = 5

TERM_RATIO = 0.8

def generate_random_re(lvl = 0, parent = None):
    term_types = [Char, Class]
    nonterm_types = [Star, Plus, Question, Group, Alt]
    out = []
    for i in range(random.randint(MAX_CONCATS)):
        term = random.random() < TERM_RATIO
        new = None
        if term:
            t = random.choice(term_types)
            if isinstance(t, Char):
                new.append(Char(random.randint(0, 255)))
            elif isinstance(t, Class):
                ranges = []
                for _ in range(random.randint(MIN_RANGES, MAX_RANGES)):
                    ranges.append(
                        (random.randint(0, 255), random.randint(0, 255))
                    )
                new.append(Class(ranges))
        else:
            t = random.choice(nonterm_types)
            new = t()
            if lvl < MAX_LVL:
                generate_random_re(lvl + 1, new)
        if parent is None:
            out.append(new)
        else:
            parent.add_child(new)
