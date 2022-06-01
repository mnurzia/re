#if !defined(MN__RE_CONFIG_H)
#define MN__RE_CONFIG_H

#include "../tests/test_config.h"
/* bits/hooks/assert */
/* desc */
/* cppreference */
#if !defined(MN_ASSERT)
#include <assert.h>
#define MN_ASSERT assert
#endif

/* bits/hooks/malloc */
/* Set to 1 in order to define malloc(), free(), and realloc() replacements. */
#if !defined(MN_USE_CUSTOM_ALLOCATOR)
#include <stdlib.h>
#define MN_MALLOC malloc
#define MN_REALLOC realloc
#define MN_FREE free
#endif

/* bits/types/size */
/* desc */
/* cppreference */
#if !defined(MN_SIZE_TYPE)
#include <stdlib.h>
#define MN_SIZE_TYPE size_t
#endif

/* bits/util/debug */
/* Set to 1 in order to override the setting of the NDEBUG variable. */
#if !defined(MN_DEBUG)
#define MN_DEBUG 0
#endif

/* bits/util/exports */
/* Set to 1 in order to define all symbols with static linkage (local to the
 * including source file) as opposed to external linkage. */
#if !defined(MN_STATIC)
#define MN_STATIC 0
#endif

/* bits/types/char */
/* desc */
/* cppreference */
#if !defined(MN_CHAR_TYPE)
#define MN_CHAR_TYPE char
#endif

/* bits/types/fixed/int32 */
/* desc */
/* cppreference */
#if !defined(MN_INT32_TYPE)
#endif

/* bits/types/fixed/uint32 */
/* desc */
/* cppreference */
#if !defined(MN_UINT32_TYPE)
#endif

/* bits/types/fixed/uint8 */
/* desc */
/* cppreference */
#if !defined(MN_UINT8_TYPE)
#endif

#endif /* MN__RE_CONFIG_H */
