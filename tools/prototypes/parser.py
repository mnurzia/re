STATES = {
    "GND": {
        "\\": [
            ["ast_stk_concat"],
            ["op_push", "ESCAPE"],
            ["state_set", "ESCAPE"]
        ],
        "(": [
            ["ast_stk_concat"],
            ["op_push", "PARENS"],
            ["state_set", "PARENS_INITIAL"]
        ],
        ")": [
            ["ast_stk_concat"],
            ["op_pop", "PARENS"],
            ["ast_apply", "group"]
        ],
        "|": [
            ["ast_stk_concat"],
            ["op_push", "ALT"]
        ],
        "*": [
            ["ast_apply", "star"],
            ["state_set", "MAYBE_QUESTION"]
        ],
        "+": [
            ["ast_apply", "plus"],
            ["state_set", "MAYBE_QUESTION"]
        ],
        "?": [
            ["ast_apply", "question"],
            ["state_set", "MAYBE_QUESTION"]
        ],
        "^": [
            ["ast_push", "text_start"]
        ],
        "$": [
            ["ast_push", "text_end"]
        ],
        ".": [
            ["ast_push", "any_char_maybe_newline"]
        ],
        "[": [
            ["state_set", "CHARCLASS_INITIAL"]
        ],
        "{": [
            ["radix_clear"],
            ["counting_clear"],
            ["state_set", "COUNTING_FIRST_NUM_INITIAL"]
        ],
        "last": [
            ["ast_stk_concat"]
        ],
        "else": [
            ["ast_push", "char"]
        ]
    },
    "MAYBE_QUESTION": {
        "?": [
            ["ast_apply", "greedy-ize"],
            ["state_set", "GND"]
        ],
        "last": [
            ["defer"],
            ["state_set", "GND"]
        ],
        "else": [
            ["defer"],
            ["state_set", "GND"]
        ]
    },
    "ESCAPE": {
        frozenset(['0', '1', '2', '3', '4', '5', '6', '7']): [
            # Octal escape
            ["radix_clear"],
            ["radix_consume_oct"],
            ["state_set", "OCTAL_SECOND_DIGIT"]
        ],
        "A": [
            # Not allowed in []
            ["assert_calling_state", "GND"],
            ["ast_push", "text_start_absolute"],
            ["op_pop", "ESCAPE"]
        ],
        "B": [
            # Not allowed in []
            ["assert_calling_state", "GND"],
            ["ast_push", "word_boundary_false"],
            ["op_pop", "ESCAPE"]
        ],
        "C": [
            # Single byte, even in UTF-8
            ["ast_or_charclass_push", "any_byte"],
            ["op_pop", "ESCAPE"]
        ],
        "D": [
            # Non digits
            ["ast_or_charclass_push", "charclass(not_decimal_digit)"],
            ["op_pop", "ESCAPE"]
        ],
        "E": [
            # End quote escape, handled in QUOTE
            ["error", "Can only use \E inside \Q"]
        ],
        frozenset(["F", "G", "H", "I", "J", "K", "L", "M"]): [
            # ambiguous across regex engines
            ["error", "Invalid escape"]
        ],
        "N": [
            # named unicode character, not now
            ["error", "Unimplemented"]
        ],
        "O": [
            # ambiguous across regex engines
            ["error", "Invalid escape"]
        ],
        "P": [
            # negated Unicode char class, not now
            ["unicode_charclass_invert"],
            ["state_set", "UNICODE_CHARCLASS_BEGIN"]
        ],
        "Q": [
            # Begin quoted text
            # Cannot occur in []
            ["assert_calling_state", "GND"],
            ["op_pop", "ESCAPE"],
            ["state_set", "QUOTE"]
        ],
        "R": [
            # newline, ambiguous across regex engines
            ["error", "Invalid escape"]
        ],
        "S": [
            # Not whitespace
            ["ast_or_charclass_push", "charclass(not_whitespace)"],
            ["op_pop", "ESCAPE"]
        ],
        "T": [
            # means nothing
            ["error", "Invalid escape"]
        ],
        "U": [
            # universal character or uppercase, ambigious across regex engines
            ["error", "Invalid escape"]
        ],
        "V": [
            # non vertical space, re2 doesn't support it
            ["error", "Invalid escape"]
        ],
        "W": [
            # non word characters
            ["ast_or_charclass_push", "charclass(not_word)"],
            ["op_pop", "ESCAPE"]
        ],
        "X": [
            # non hex digits (re2/python dont support it)
            ["error", "Invalid escape"]
        ],
        "Y": [
            # unassigned
            ["error", "Invalid escape"]
        ],
        "Z": [
            # at end of text or before newline at end of text, re2 no support
            ["error", "Invalid escape"]
        ],
        "a": [
            # Bell
            ["ast_or_charclass_push", "char(0x07)"],
            ["op_pop", "ESCAPE"]
        ],
        "b": [
            # Not allowed in []
            # Means backspace in python, might include later
            ["assert_calling_state", "GND"],
            ["ast_push", "word_boundary_true"],
            ["op_pop", "ESCAPE"]
        ],
        "c": [
            # ignore case or control character, unsupported by re2/python
            ["error", "Invalid escape"]
        ],
        "d": [
            # digits
            ["ast_or_charclass_push", "charclass(digit)"],
            ["op_pop", "ESCAPE"]
        ],
        "e": [
            # escape, unsupported by python or re2
            ["error", "Invalid escape"]
        ],
        "f": [
            # Form feed
            ["ast_or_charclass_push", "char(0x0C)"],
            ["op_pop", "ESCAPE"]
        ],
        "g": [
            # backreference. why even bother??
            ["error", "Invalid escape"]
        ],
        "h": [
            # horizontal space.
            ["error", "Invalid escape"]
        ],
        "i": [
            # no meaning
            ["error", "Invalid escape"]
        ],
        "j": [
            # no meaning
            ["error", "Invalid escape"]
        ],
        "k": [
            # backreference in pcre
            ["error", "Invalid escape"]
        ],
        "l": [
            # lowercase
            ["error", "Invalid escape"]
        ],
        "m": [
            # "magic" according to vim
            ["error", "Invalid escape"]
        ],
        "n": [
            # Newline
            # (Should be "logical")
            ["ast_or_charclass_push", "char(0x0A)"],
            ["op_pop", "ESCAPE"]
        ],
        "o": [
            # octal digits, vim
            ["error", "Invalid escape"]
        ],
        "p": [
            # Unicode character class, not now
            ["state_set", "UNICODE_CHARCLASS_BEGIN"]
        ],
        "q": [
            # no meaning
            ["error", "Not implemented"]
        ],
        "r": [
            # Carriage return
            ["ast_or_charclass_push", "char(0x0D)"],
            ["op_pop", "ESCAPE"]
        ],
        "s": [
            # Whitespace
            ["ast_or_charclass_push", "charclass(whitespace)"],
            ["op_pop", "ESCAPE"]
        ],
        "t": [
            # Horizontal tab
            ["ast_or_charclass_push", "char(0x09)"],
            ["op_pop", "ESCAPE"]
        ],
        "u": [
            # Uppercase or universal, not supported by re
            ["error", "Invalid escape"]
        ],
        "v": [
            # Vertical tab
            ["ast_or_charclass_push", "char(0x0B)"],
            ["op_pop", "ESCAPE"]
        ],
        "w": [
            # Word character
            ["ast_or_charclass_push", "charclass(word)"]
        ],
        "x": [
            # Two digit hex
            # Explicitly don't pop escape
            ["radix_clear"],
            ["state_set", "HEX_INITIAL"]
        ],
        "y": [
            # no meaning
            ["error", "Invalid escape"]
        ],
        "z": [
            # Not allowed in []
            ["assert_calling_state", "GND"],
            ["ast_push", "text_end_absolute"],
            ["op_pop", "ESCAPE"]
        ],
        "last": [
            ["error", "Unfinished escape"]
        ],
        "else": [
            # Almost every other char is accounted for (including \0!)
            ["ast_or_charclass_push", "char"],
            ["op_pop", "ESCAPE"]
        ]
    },
    "PARENS_INITIAL": {
        "?": [
            ["state_set", "PARENS_FLAG"]
        ],
        "last": [
            ["error", "Unclosed parentheses"]
        ],
        "else": [
            ["defer"],
            ["state_set", "GND"]
        ]
    },
    "PARENS_FLAG_INITIAL": {
        "P": [
            ["state_set", "PARENS_FLAG_NAMED"]
        ],
        ":": [
            ["group_flag_set", "NON_CAPTURE"],
            ["state_set", "GND"]
        ],
        "i": [
            ["group_flag_set", "CASE_INSENSITIVE"],
            ["state_set", "PARENS_FLAG"]
        ],
        "m": [
            ["group_flag_set", "MULTILINE"],
            ["state_set", "PARENS_FLAG"]
        ],
        "s": [
            ["group_flag_set", "DOT_NEWLINE"],
            ["state_set", "PARENS_FLAG"]
        ],
        "U": [
            ["group_flag_set", "UNGREEDY"],
            ["state_set", "PARENS_FLAG"]
        ],
        "last": [
            ["error", "Unset parenthetical group flag"]
        ],
        "else": [
            ["error", "Invalid flag character"]
        ]
    },
    "PARENS_FLAG": {
        ":": [
            ["group_flag_set", "NON_CAPTURE"],
            ["state_set", "GND"]
        ],
        "i": [
            ["group_flag_set", "CASE_INSENSITIVE"]
        ],
        "m": [
            ["group_flag_set", "MULTILINE"]
        ],
        "s": [
            ["group_flag_set", "DOT_NEWLINE"]
        ],
        "U": [
            ["group_flag_set", "UNGREEDY"]
        ],
        "last": [
            ["error", "Unclosed parentheses"]
        ],
        "else": [
            ["defer"],
            ["state_set", "GND"]
        ]
    },
    "PARENS_FLAG_NAMED_INITIAL": {
        "<": [
            ["state_set", "PARENS_FLAG_NAMED"]
        ],
        "last": [
            ["error", "Expected a < for named capturing group"]
        ],
        "else": [
            ["error", "Expected a < for named capturing group"]
        ]
    },
    "PARENS_FLAG_NAMED": {
        ">": [
            ["group_name_set"],
            ["state_set", "GND"]
        ],
        "last": [
            ["error", "Expected a > to close capturing group name"]
        ],
        "else": [
            ["group_name_add"]
        ]
    },
    "QUOTE": {
        # Initial 
        "\\": [
            ["state_set", "QUOTE_ESCAPE"]
        ],
        "last": [
            ["ast_push", "quote_string"],
            # Finish everything
            ["defer"],
            ["state_set", "GND"]
        ],
        "else": [
            ["quote_string_add"]
        ]
    },
    "QUOTE_ESCAPE": {
        "E": [
            # \E inside \Q
            ["ast_push", "quote_string"],
            ["state_set", "GND"]
        ],
        "\\": [
            # \\ inside \Q
            ["state_set", "QUOTE"],
        ],
        "last": [
            # \ ending string
            ["quote_string_add", "\\"],
            ["ast_push", "quote_string"],
            ["defer"],
            ["state_set", "GND"]
        ],
        "else": [
            # \*
            ["quote_string_add", "\\"],
            ["quote_string_add"],
            ["state_set", "QUOTE"]
        ]
    },
    "HEX_INITIAL": {
        # First hex digit or {
        frozenset(["0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F", "a", "b", "c", "d", "e", "f"]): [
            ["radix_consume_hex"],
            ["radix_check_hex"],
            ["state_set", "HEX_SECOND_DIGIT"]
        ],
        "{": [
            # Extended code
            ["state_set", "HEX_EXTENDED"]
        ],
        "last": [
            # \x ends string
            ["error", "Unfinished hex code"]
        ],
        "else": [
            ["error", "Invalid hexadecimal character, expected 0-9A-Fa-F or {"]
        ]
    },
    "HEX_SECOND_DIGIT": {
        # Second hex digit
        frozenset(["0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F", "a", "b", "c", "d", "e", "f"]): [
            ["radix_consume_hex"],
            ["radix_check_hex"],
            ["ast_or_charclass_push", "radix_char"],
            ["op_pop", "ESCAPE"]
        ],
        "last": [
            # \x* ends string
            ["error", "Unfinished two-digit hex code"]
        ],
        "else": [
            ["error", "Invalid hexadecimal character"]
        ]
    },
    "HEX_EXTENDED": {
        # Extended hex code
        frozenset(["0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F", "a", "b", "c", "d", "e", "f"]): [
            ["radix_consume_hex"],
            ["radix_check_hex"]
        ],
        "}": [
            ["radix_check_hex"],
            ["ast_or_charclass_push", "radix_char"],
            ["op_pop", "ESCAPE"]
        ],
        "last": [
            # unterminated {
            ["error", "Unfinished extended hex code"]
        ],
        "else": [
            ["error", "Invalid hexadecimal character"]
        ]
    },
    "OCTAL_SECOND_DIGIT": {
        # After first octal digit
        frozenset(["0", "1", "2", "3", "4", "5", "6", "7"]): [
            ["radix_consume_oct"],
            ["radix_check_oct"],
            ["state_set", "OCTAL_THIRD_DIGIT"]
        ],
        "last": [
            ["radix_check_oct"],
            ["ast_or_charclass_push", "radix_char"],
            ["op_pop", "ESCAPE"],
            ["defer"]
        ],
        "else": [
            ["radix_check_oct"],
            ["ast_or_charclass_push", "radix_char"],
            ["op_pop", "ESCAPE"],
            ["defer"]
        ]
    },
    "OCTAL_THIRD_DIGIT": {
        # After second octal digit
        frozenset(["0", "1", "2", "3", "4", "5", "6", "7"]): [
            ["radix_consume_oct"],
            ["radix_check_oct"],
            ["state_set", "GND"]
        ],
        "last": [
            ["radix_check_oct"],
            ["ast_or_charclass_push", "radix_char"],
            ["op_pop", "ESCAPE"],
            ["defer"]
        ],
        "else": [
            ["radix_check_oct"],
            ["ast_or_charclass_push", "radix_char"],
            ["op_pop", "ESCAPE"],
            ["defer"]
        ]
    },
    "CHARCLASS_INITIAL": {
        # After [
        "[": [
            ["state_set", "CHARCLASS_AFTER_CARET"],
            ["op_push", "CHARCLASS_NAMED"],
            ["state_set", "CHARCLASS_NAMED_INITIAL"]
        ],
        "]": [
            ["error", "Empty character class"]
        ],
        ":": [
            ["state_set", "GND"],
            ["op_push", "CHARCLASS_NAMED"],
            ["state_set", "CHARCLASS_NAMED_NAME_INITIAL"]
        ],
        "^": [
            ["charclass_invert"],
            ["state_set", "CHARCLASS_AFTER_CARET"]
        ],
        "\\": [
            ["op_push", "ESCAPE"],
            ["state_set", "ESCAPE"]
        ],
        "last": [
            ["error", "Unfinished char class"]
        ],
        # add other characters
        # hyphen at beginning: just push it
        "else": [
            ["charclass_range_add", "char"],
            ["state_set", "CHARCLASS_AFTER_RANGE_BEGIN"]
        ]
    },
    "CHARCLASS_AFTER_CARET": {
        # After [^
        "[": [
            ["state_set", "CHARCLASS_AFTER_CARET"],
            ["op_push", "CHARCLASS_NAMED"],
            ["state_set", "CHARCLASS_NAMED_INITIAL"]
        ],
        "]": [
            ["error", "Unfinished inverted char class"]
        ],
        "\\": [
            ["op_push", "ESCAPE"],
            ["state_set", "ESCAPE"]
        ],
        "last": [
            ["error", "Unfinished inverted char class"]
        ],
        # Add other characters
        # hyphen at beginning
        "else": [
            ["charclass_range_add", "char"],
            ["state_set", "CHARCLASS_AFTER_RANGE_BEGIN"]
        ]
    },
    "CHARCLASS_AFTER_RANGE_BEGIN": {
        # After [x or [^x
        "[": [
            ["state_set", "CHARCLASS_AFTER_CARET"],
            ["op_push", "CHARCLASS_NAMED"],
            ["state_set", "CHARCLASS_NAMED_INITIAL"]
        ],
        "]": [
            ["charclass_finish"],
            ["state_set", "GND"]
        ],
        "\\": [
            ["state_set", "CHARCLASS_AFTER_HYPHEN"],
            ["op_push", "ESCAPE"],
            ["state_set", "ESCAPE"]
        ],
        "-": [
            ["state_set", "CHARCLASS_AFTER_HYPHEN"]
        ],
        "last": [
            ["error", "Unterminated char class"]
        ],
        "else": [
            ["charclass_push"],
            ["charclass_range_add", "char"],
            # Go back to this first
            ["state_set", "CHARCLASS_AFTER_RANGE_BEGIN"]
        ]
    },
    "CHARCLASS_AFTER_HYPHEN": {
        # After [x-
        "[": [
            ["state_set", "CHARCLASS_AFTER_CARET"],
            ["op_push", "CHARCLASS_NAMED"],
            ["state_set", "CHARCLASS_NAMED_INITIAL"]
        ],
        "]": [
            ["charclass_range_add", "char(-)"],
            ["charclass_finish"],
            ["state_set", "GND"]
        ],
        "\\": [
            ["state_set", "CHARCLASS_AFTER_CARET"],
            ["op_push", "ESCAPE"],
            ["state_set", "ESCAPE"]
        ],
        "last": [
            ["error", "Unfinished char class"]
        ],
        "else": [
            ["charclass_range_add", "char"],
            ["charclass_finish"],
            ["state_set", "CHARCLASS_AFTER_CARET"]
        ]
    },
    "CHARCLASS_NAMED_INITIAL": {
        # After [[ or [^[
        ":": [
            ["state_set", "CHARCLASS_NAMED_NAME_INITIAL"]
        ],
        "last": [
            ["error", "Expected a : for named char class"]
        ],
        "else": [
            ["error", "Expected a : for named char class"]
        ]
    },
    "CHARCLASS_NAMED_NAME_INITIAL": {
        # After [[: or [^[: or just [:
        "^": [
            ["charclass_named_invert"],
            ["state_set", "CHARCLASS_NAMED_NAME"]
        ],
        "last": [
            ["error", "Expected either a ^ or char class name for named char class"]
        ],
        "else": [
            ["charclass_named_push"],
            ["state_set", "CHARCLASS_NAMED_NAME"]
        ]
    },
    "CHARCLASS_NAMED_NAME": {
        # After [: or [:^
        ":": [
            ["charclass_named_finish"],
            ["state_set", "CHARCLASS_NAMED_FINAL"]
        ],
        "last": [
            ["error", "Unfinished named char class"]
        ],
        "else": [
            ["charclass_named_push"]
        ]
    },
    "CHARCLASS_NAMED_FINAL": {
        # After [:name:
        "]": [
            ["charclass_class_add", "charclass_name"],
            ["op_pop", "CHARCLASS_NAMED"]
        ],
        "last": [
            ["error", "Expected ] to finish char class name"]
        ],
        "else": [
            ["error", "Expected ] to finish char class name"]
        ]
    },
    "UNICODE_CHARCLASS_BEGIN": {
        # \pN or \PN
        "{": [
            ["state_set", "UNICODE_CHARCLASS_NAMED_NAME"]
        ],
        "last": [
            ["error", "Expected a character or { for unicode char class"]
        ],
        "else": [
            ["unicode_charclass_name_add"],
            ["unicode_charclass_finish"],
            ["op_pop", "ESCAPE"]
        ]
    },
    "UNICODE_CHARCLASS_NAMED_NAME": {
        # \pN pr \P{NNN...}
        "}": [
            ["unicode_charclass_finish"],
            ["op_pop", "ESCAPE"]
        ],
        "last": [
            ["error", "Expected a } to end named Unicode char class"]
        ],
        "else": [
            ["unicode_charclass_name_add"]
        ]
    },
    "COUNTING_FIRST_NUM_INITIAL": {
        # {n,m} or {n}
        frozenset(["0", "1", "2", "3", "4", "5", "6", "7", "8", "9"]): [
            ["radix_consume_dec"],
            ["state_set", "COUNTING_FIRST_NUM"]
        ],
        "}": [
            ["error", "Expected a number"]
        ],
        "last": [
            ["error", "Unfinished counting form"]
        ],
        "else": [
            ["error", "Invalid decimal character"]
        ]
    },
    "COUNTING_FIRST_NUM": {
        # After {0 or {5 or etc.
        frozenset(["0", "1", "2", "3", "4", "5", "6", "7", "8", "9"]): [
            ["radix_consume_dec"],
            ["radix_check_dec"]
        ],
        "}": [
            ["radix_check_dec"],
            ["counting_set_upper"],
            ["counting_finish"],
            ["state_set", "MAYBE_QUESTION"]
        ],
        ",": [
            ["radix_check_dec"],
            ["counting_set_lower"],
            ["state_set", "COUNTING_SECOND_NUM"]
        ],
        "last": [
            ["error", "Unfinished counting form"]
        ],
        "else": [
            ["error", "Invalid decimal character or ,"]
        ]
    },
    "COUNTING_SECOND_NUM_INITIAL": {
        # After {0, or {5, or etc.
        frozenset(["0", "1", "2", "3", "4", "5", "6", "7", "8", "9"]): [
            ["radix_consume_dec"],
            ["radix_check_dec"],
            ["state_set", "COUNTING_SECOND_NUM"]
        ],
        "}": [
            ["error", "Expected number for upper counting bound"]
        ],
        "last": [
            ["error", "Unfinished counting form"]
        ],
        "else": [
            ["error", "Invalid decimal character"]
        ]
    },
    "COUNTING_SECOND_NUM": {
        # After {0,0 or {5,5 etc.
        frozenset(["0", "1", "2", "3", "4", "5", "6", "7", "8", "9"]): [
            ["radix_consume_dec"],
            ["radix_check_dec"]
        ],
        "}": [
            ["radix_check_dec"],
            ["counting_set_upper"],
            ["counting_finish"],
            ["state_set", "MAYBE_QUESTION"]
        ],
        "last": [
            ["error", "Unfinished counting form"]
        ],
        "else": [
            ["error", "Invalid decimal character"]
        ]
    }
}

def gather_states():
    return list(STATES.keys())

def gather_unique_inst_types():
    out = set()
    for v in STATES.values():
        for v2 in v.values():
            for i in v2:
                out.add(i[0])
    return out

#out.append(gather_states(), len(gather_states()))
#for i, ty in enumerate(sorted(gather_unique_inst_types())):
#    out.append(i, ty)

def visualize():
    import graphviz
    dot = graphviz.Digraph(comment="Parser")

    nodes = gather_states()
    edges = []
    for state, state_info in STATES.items():
        for char, insts in state_info.items():
            last_state_set = None
            for inst in insts:
                if inst[0] == "state_set":
                    last_state_set = inst[1]
            if last_state_set is None:
                edges.append((state, state, char, insts))
            else:
                edges.append((state, last_state_set, char, insts))
    
    for node in nodes:
        dot.node(str(node), shape="box")
    
    dot.node("ERROR")
    
    for fr, to, chars, insts in edges:
        assert fr in STATES.keys()
        assert to in STATES.keys()
        if chars == "\\":
            chars = "<backslash>"
        if "error" in [i[0] for i in insts]:
            dot.edge(fr, "ERROR", label=chars)
        else:
            dot.edge(fr, to, label=chars)

    dot.render(filename="parser", format="png", cleanup=True)

#visualize()

import string

def printable(ch):
    if ch == '\\':
        return "'\\\\'"
    elif ch in string.printable:
        return "'%s'" % ch
    else:
        return "0x%02X" % ord(ch)

def cescape(s):
    out = s.replace('"', '\\"')
    return '"' + out + '"'

def chclsort(chcls):
    l = chcls.pop("last")
    return {"last": l, **chcls}

def generate_switch():
    out = []
    for i, (state_old, classes) in enumerate(STATES.items()):
        state = "RE__PARSE_STATE_" + state_old
        if i == 0:
            out.append("if (parser.state == %s) {" % state)
        else:
            out.append("} else if (parser.state == %s) {" % state)
        for j, (chcl, funcs) in enumerate(chclsort(classes).items()):
            if chcl == "last":
                pred = "last"
            elif chcl == "else":
                pred = ""
            elif isinstance(chcl, frozenset):
                pred = " || ".join(["ch == %s" % printable(c) for c in sorted(list(chcl))])
            else:
                pred = "ch == %s" % printable(chcl)
            if j == 0:
                out.append("    if (%s) {" % pred)
            else:
                if chcl == "else":
                    out.append("    } else {")
                else:
                    out.append("    } else if (%s) {" % pred)
            for func in funcs:
                func_name = func[0]
                if func_name == "state_set":
                    out.append("        parser.state = RE__PARSE_STATE_%s;" % func[1])
                else:
                    args = ["parser"]
                    if func_name == "error":
                        args.append(cescape(func[1]))
                    out.append("        re__parser_%s(%s);" % (func_name, ", ".join(args)))
        out.append("    }")
    out.append("} else {")
    out.append("    RE__ASSERT_UNREACHABLE();")
    out.append("}")
    return list(["        "+l for l in out])

def generate_enum():
    out = []
    out.append("typedef enum re__parse_state {")
    for i, name in enumerate(STATES):
        out.append("    RE__PARSE_STATE_%s" % name, end="")
        if i != len(STATES) - 1:
            out.append(",")
        else:
            out.append()
    out.append("} re__parse_state;")
    return out

import sys

def printlines(ls):
    for l in ls:
        print(l)

if sys.argv[1] == "switch":
    printlines(generate_switch())
elif sys.argv[1] == "enum":
    printlines(generate_enum())
