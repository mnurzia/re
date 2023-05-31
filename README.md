# re

Fast single-header regular expression library implemented in C89. 

```c
re regex;
const char *text;
if (re_init(&regex, "[a-fA-F0-9]*")) {
  /* error */
}
text = "ABC123";
if (re_is_match(&regex, text, strlen(text), RE_ANCHOR_START) == RE_MATCH) {
  printf("Matched!\n");
}
re_destroy(&regex);
```

Features
- Linear-time execution guarantee (non-backtracking)
- Utilizes an NFA for group-search, and a lazy-loaded DFA for bounds-search
- Fully multithread compatible
- Extensively tested (10,000+ lines of tests)
- Unicode compatible (no support for set-based character class notation, otherwise feature-parity with google re2)

Currently, the API is not stable. I am working to develop a version of the API that is flexible enough to offer easy usage in both the single-threaded and the multi-threaded use case.

See `re_api.h` for details on the API.

The design is mostly textbook, but notably contains an implementation for compiling charclasses that is very efficient. I developed an algorithm to compute the state-minimal NFA (in bytecode instruction format) of any set of UTF-8 ranges in linear time. See `re_compile_charclass.c` for more details. To my knowledge this has not been done anywhere else before.

The design is heavily inspired by Google's [re2](https://www.github.com/google/re2) and the implementation was motivated by Russ Cox's series of [posts](https://swtch.com/~rsc/regexp/) on implementing regular expressions.
