import re
import re2
import pcre
import pcre2

def ok(name):
    print("\x1b[32m%s\x1b[0m " % name, end="")

def bad(name):
    print("\x1b[31m%s\x1b[0m " % name, end="")

class PCRE2Wrap:
    def __init__(self, reg):
        self.reg = pcre2.PCRE2(reg.encode())
    
    def match(self, text):
        return self.reg.search(text.encode())

def compall(regex):
    outs = {}
    try:
        outs["python"] = (True, re.compile(regex))
    except Exception as e:
        outs["python"] = (False, str(e))
    
    try:
        outs["pcre"] = (True, pcre.compile(regex))
    except Exception as e:
        outs["pcre"] = (False, str(e))
    
    try:
        outs["re2"] = (True, re2.compile(regex))
    except Exception as e:
        outs["re2"] = (False, str(e))
    
    try:
        outs["pcre2"] = (True, PCRE2Wrap(regex))
    except Exception as e:
        outs["pcre2"] = (False, str(e))
    
    return outs

def matchall(comp_outs, text):
    outs = {}
    for name, (is_good, comp) in comp_outs.items():
        if is_good:
            if comp.match(text) is not None:
                outs[name] = True
            else:
                outs[name] = False
        else:
            outs[name] = False
    return outs

# parse test
def ptest(name, regex):
    outs = compall(regex)
    regheader(name, regex)
    for name, (is_good, _) in outs.items():
        if is_good:
            ok(name)
        else:
            bad(name)
    print()
    for name, (is_good, e) in outs.items():
        if not is_good:
            print("  "+name+":", e)

def regheader(name, regex):
    print("\x1b[36m%s\x1b[0m: \x1b[35m%s\x1b[0m " % (name, repr(regex)), end="")

def mtest(name, regex, text):
    outs = matchall(compall(regex), text)
    print("\x1b[36m%s\x1b[0m: \x1b[35m%s\x1b[0m -> \x1b[33m%s\x1b[0m " % (name, repr(regex), repr(text)), end="")
    for name, matched in outs.items():
        if matched:
            ok(name)
        else:
            bad(name)
    print()

# parse test, multiple
def ptest_multiple(fmtstr, substs, regexes):
    print("\x1b[36m%s\x1b[0m:" % fmtstr)
    outs = {}
    for i, reg in enumerate(regexes):
        outs = compall(reg)
        print("    ",end="")
        regheader(fmtstr.format(substs[i]), reg)
        for name, (is_good, _) in outs.items():
            if is_good:
                ok(name)
            else:
                bad(name)
        print()
        if not all([a[0] for a in outs.values()]):
            for name, (is_good, e) in outs.items():
                if not is_good:
                    print("      "+name+":", e)
    print()
ptest("Nested groups", "(abc(abcdef))")
ptest("Named groups, ?P<", "(?P<name>.*)")
ptest("Named groups, ?P<", "(?<name>.*)")
ptest("Parens with )", "())")
ptest("Parens with (", "(()")
ptest("\\e supported", "\\e")
mtest("\\e matches escape character", r"\e", "\x1b")
ptest("\\* supported", r"\*")
ptest("\\+ supported", r"\+")
ptest("\\? supported", r"\?")
ptest("\\[ supported", r"\[")
ptest("\\] supported", r"\]")
ptest_multiple("\\{} supported", 
    [chr(x) for x in range(0, 127)], 
    ["\\"+chr(x) for x in range(0, 127)])
ptest_multiple("[\\{}] supported", 
    [chr(x) for x in range(0, 127)], 
    ["[\\"+chr(x)+"]" for x in range(0, 127)])
ptest("] can occur on its own", r"]")
mtest("\\Q...\\E matches backslashes", r"\Q\t\E", "\t")
mtest("\\Q...\\E doesnt match backslashes", r"\Q\t\E", "\\t")
ptest("\\Q works until end of string", r"\Qabcdef\t")
mtest("\\Q until end of string still matches", r"\Qabcdef\t", "abcdef\\t")
ptest("\\Q double escapes \\E", r"\Q\\E\E")
mtest("\\Q double escaped \\E matches \E", r"\Q\\E\E", "\\E")
ptest("\\Q works with \\ at end", "\\Q\\")
mtest("\\Q with \\ at end matches \\", "\\Q\\", "\\")
ptest("\\Q works with \\\\ at end", "\\Q\\")
mtest("\\Q with \\\\ at end matches \\\\", "\\Q\\\\", "\\\\")
ptest("\\c without anything supported", r"\c")
ptest("\\cK control character supported", r"\cA")
ptest("\\x without anything supported", r"\x")
ptest("\\x with one character supported", r"\x1")
ptest("\\x with two characters supported", r"\x1F")
ptest("\\x with two different cased characters", r"\xaA")
mtest("\\x with two different cased characters matches", r"\xaA", "\xaa")
ptest("\\x{} supported", r"\x{}")
ptest("\\x{00} supported", r"\x{00}")
ptest("\\x{} with one digit supported", r"\x{1}")
ptest("\\x{} with two digits supported", r"\x{12}")
ptest("\\x{} with three digits supported", r"\x{123}")
ptest("\\x{} with four digits supported", r"\x{1234}")
ptest("\\x{} with five digits supported", r"\x{12345}")
ptest("\\x{} with six digits supported", r"\x{123456}")
ptest("\\x{} with seven digits supported", r"\x{1234567}")
ptest("\\x{} with eight digits supported", r"\x{12345678}")
ptest("\\x{} with nine digits supported", r"\x{123456789}")
ptest("\\x{} with ten digits supported", r"\x{123456789a}")
ptest("\\x{} with eleven digits supported", r"\x{123456789ab}")
ptest("\\x{} with twelve digits supported", r"\x{123456789abc}")
ptest("\\x{} with maximal unicode value supported", r"\x{10FFFF}")
ptest("\\x{} with out-of-range Unicode value supported", r"\x{110000}")
ptest("\\x{} with out-of-range Unicode value supported", r"\x{1FFFFF}")
ptest("\\0 as octal code 0", r"\0")
ptest("\\00 as octal code 0", r"\00")
mtest("\\0 matches \\x00", r"\0", "\x00")
mtest("\\1 matches \\x01", r"\1", "\x01")
mtest("\\11 matches \\x09", r"\11", "\x09")
mtest("\\00 matches \\x00", r"\00", "\x00")
mtest("\\01 matches \\x01", r"\01", "\x01")
mtest("\\011 matches \\x09", r"\011", "\x09")
mtest("\\0111 matches \\x49", r"\0111", "\x49")
mtest("\\111 matches \\x49", r"\111", "\x49")
mtest("\\377 matches \\xff", r"\377", "\xff")
mtest("\\777 matches \\u1ff", r"\777", "\u01ff")
ptest("Empty char class", "[]")
mtest(". matches \\n", ".", "\n")
ptest("Char class with ] literal", "[]]")
mtest("[]] matches ]", "[]]", "]")
ptest("Char class with [ literal", "[[]")
mtest("[[] matches [", "[[]", "[")
ptest("Named char class outside char class", "[:digit:]")
ptest("Char class with -] supported", "[a-]")
mtest("Char class with -] matches -", "[a-]", "-")
ptest("Char class with - as ending character of range", "[+--]")
mtest("Char class with - as ending character matches middles", "[+--]", "+")
mtest("Char class with - as ending character matches middles", "[+--]", ",")
ptest("Nested char class", "[[0-9]]")
ptest("Group with only anchor????", "(^)")
ptest("Star with only anchor", "(^)*")
ptest("\C in char class", r"[\C]")
mtest("\C in char class matches C", r"[\C]", "C")
mtest("\C in char class matches other than C", r"[\C]", "A")
mtest(". in char class matches other than .", r"[.]", "M")
ptest("Empty group", "()")
ptest("Group with only ?", "(?)")
mtest("Group with only ? matches ?", "(?)", "?")
mtest("Group with only ? matches other than ?", "(?)", "a")
mtest("Group with only ? is not zero-width", "^(?)$", "a")
