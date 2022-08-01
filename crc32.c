#include <stdint.h>
#include <stddef.h>

// reference implementation
uint32_t crc32_ref(const void *data, size_t len)
{
    const uint8_t *ptr = data;
    uint32_t rem = 0xFFFFFFFF;
    while (len--) {
        rem ^= *ptr++;
        for (int cnt = 0; cnt < 8; cnt++) {
            if ((rem & 1)) {
                rem = (rem >> 1) ^ 0xEDB88320;
            } else {
                rem >>= 1;
            }
        }
    }
    return ~rem;
}

// c implementation
uint32_t crc32_c(const void *data, size_t len)
{
    const uint8_t *ptr = data;
    uint32_t rem = 0;
    while (len--) {
        rem ^= *ptr++;
        for (int cnt = 0; cnt < 8; cnt++) {
            int bit = rem & 1;
            rem = (rem >> 1) | 0x80000000;
            if (!bit) rem ^= 0xEDB88320;
        }
    }
    return rem;
}

// asm implementation
uint32_t crc32_asm(const void *data, size_t len)
{
    const uint8_t (*ptr)[len] = data;
    uint32_t rem = 0;
    uint32_t cnt;
    __asm__ (
    "1:"
#if 1
        "sub $1, %[len];"
        "jc 4f;"
#else
        // restrict len <= 2GB, can save 2 more bytes
        "dec %k[len];"
        "js 4f;"
#endif
        "xorb (%[ptr]), %b[rem];"
        "inc %[ptr];"
#if 1
        "xor %[cnt], %[cnt];"
        "mov $8, %b[cnt];"
#else
        // use stack to set %ecx to 8, can save 1 more byte
        "push $8;"
        "pop %q[cnt];"
#endif
    "2:"
        "stc;"
        "rcr $1, %[rem];"
        "jc 3f;"
        "xor $0xEDB88320, %[rem];"
    "3:"
        "loop 2b;"

        "jmp 1b;"
    "4:"
        : [ptr] "+r" (ptr),
          [len] "+r" (len),
          [rem] "+r" (rem),
          [cnt] "=&c" (cnt)
        : "m" (*ptr)
        : "cc"
    );
    return rem;
}
