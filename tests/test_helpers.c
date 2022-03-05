#include "test_helpers.h"

re_rune re_rune_rand() {
    int cls = (int)RAND_PARAM(4);
    re_rune ch = 0;
    if (cls == 0) {
        int sub = (int)RAND_PARAM(3);
        if (sub == 0) {
            ch = (int)RAND_PARAM(10) + '0';
        } else if (sub == 1) {
            ch = (int)RAND_PARAM(26) + 'a';
        } else if (sub == 2) {
            ch = (int)RAND_PARAM(26) + 'A';
        }
    } else if (cls == 1) {
        ch = (int)RAND_PARAM(0x800 - 0x80) + 0x80;
    } else if (cls == 2) {
        int sub = (int)RAND_PARAM(2);
        if (sub == 0) {
            ch = (int)RAND_PARAM(0xD800 - 0x800) + 0x800;
        } else if (sub == 1) {
            ch = (int)RAND_PARAM(0x10000 - 0xE000) + 0xE000;
        }
    } else if (cls == 3) {
        ch = (int)RAND_PARAM(0x10FFFF - 0x10000) + 0x10000;
    }
    return ch;
}
