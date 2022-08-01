#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <sys/mman.h>
#include <zlib.h>

extern uint32_t crc32_ref(const void *data, size_t len);
extern uint32_t crc32_c(const void *data, size_t len);
extern uint32_t crc32_asm(const void *data, size_t len);

static uint32_t crc32_zlib(const void *data, size_t len)
{
    uint32_t val = 0;
    while (len > UINT_MAX) {
        val = crc32(val, data, UINT_MAX);
        data += UINT_MAX;
        len -= UINT_MAX;
    }
    return crc32(val, data, len);
}

static void test_crc32(const char *desc, const void *data, size_t len)
{
    printf("[data=%p] [len=0x%zx] [%s]\n", data, len, desc);

    uint32_t correct = crc32_zlib(data, len);

#define TEST(func) do { \
    printf("%-10s ", #func); \
    clock_t start = clock(); \
    uint32_t current = func(data, len); \
    clock_t end = clock(); \
    printf("%08x ", current); \
    printf("%.3fs\n", (double)(end - start) / CLOCKS_PER_SEC); \
    assert(current == correct); \
} while (0)

    TEST(crc32_zlib);
    TEST(crc32_ref);
    TEST(crc32_c);
    TEST(crc32_asm);

    printf("\n");
}

static void fill_random(void *data, unsigned seed, size_t len)
{
    uint8_t *ptr = data;
    srandom(seed);
    for (size_t idx = 0; idx < len; idx++) {
        ptr[idx] = random();
    }
}

int main()
{
    setbuf(stdout, NULL);

    unsigned seed = time(NULL);
    printf("[seed=0x%08x]\n", seed);

#define GUARD (1024 * 1048576UL)
#define MEMSZ (6144 * 1048576UL)
    void *region = mmap(NULL, GUARD + MEMSZ + GUARD, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    assert(region != MAP_FAILED);
    printf("[region=%p]\n", region);
    void *mem = region + GUARD;
    printf("[mem=%p]\n", mem);
#define SETRW(rw) do { \
    int ret = mprotect(mem, MEMSZ, (rw ? PROT_READ | PROT_WRITE : PROT_READ)); \
    assert(ret == 0); \
} while (0)

    printf("\n");

#define N 200000
    for (size_t i = 0; i <= N; i++) {
        SETRW(1); memset(mem, 0, N); SETRW(0);
        test_crc32("small, zero, low", mem, i);
        SETRW(1); memset(mem + MEMSZ - N, 0, N); SETRW(0);
        test_crc32("small, zero, high", mem + MEMSZ - i, i);

        SETRW(1); memset(mem, 0xff, N); SETRW(0);
        test_crc32("small, one, low", mem, i);
        SETRW(1); memset(mem + MEMSZ - N, 0xff, N); SETRW(0);
        test_crc32("small, one, high", mem + MEMSZ - i, i);

        SETRW(1); fill_random(mem, seed + i, i); SETRW(0);
        test_crc32("small, random, low", mem, i);
        SETRW(1); fill_random(mem + MEMSZ - i, seed + i, i); SETRW(0);
        test_crc32("small, random, high", mem + MEMSZ - i, i);
    }

#define M 4
    for (size_t i = 0; i <= M; i++) {
        SETRW(1); memset(mem, 0, MEMSZ); SETRW(0);
        test_crc32("big, zero, low", mem, MEMSZ - i);
        SETRW(1); memset(mem, 0, MEMSZ); SETRW(0);
        test_crc32("big, zero, high", mem + i, MEMSZ - i);

        SETRW(1); memset(mem, 0xff, MEMSZ); SETRW(0);
        test_crc32("big, one, low", mem, MEMSZ - i);
        SETRW(1); memset(mem, 0xff, MEMSZ); SETRW(0);
        test_crc32("big, one, high", mem + i, MEMSZ - i);

        SETRW(1); fill_random(mem, seed + i, MEMSZ - i); SETRW(0);
        test_crc32("big, random, low", mem, MEMSZ - i);
        SETRW(1); fill_random(mem + i, seed + i, MEMSZ - i); SETRW(0);
        test_crc32("big, random, high", mem + i, MEMSZ - i);
    }

    printf("[done]\n");

    return 0;
}
