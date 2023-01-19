# re

Fast single-header regular expression library implemented in C89. 

Features
- Linear-time execution guarantee (non-backtracking)
- Utilizes an NFA for group-search, and a lazy-loaded DFA for bounds-search
- Fully multithread compatible
- Extensively tested (10,000+ lines of tests)

Currently, the API is not stable. I am working to develop a version of the API that is flexible enough to offer easy usage in both the single-threaded and the multi-threaded use case.

See `re_api.h` for details on the API.
