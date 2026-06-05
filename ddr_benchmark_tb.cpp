#include <stdint.h>
#include <stdio.h>

#define N 4096
#define KERNEL_N 1024
#define RESULT64 2048
#define MARKER_BASE 0xA000000000000000ULL

void ddr_benchmark(volatile uint64_t *buf);

int main() {
    static uint64_t buf[N];
    for (uint32_t i = 0; i < 2 * KERNEL_N; i++) {
        buf[RESULT64 + i] = 0;
    }

    ddr_benchmark(buf);

    for (uint32_t k = 0; k < KERNEL_N; k++) {
        uint64_t expected = MARKER_BASE | k;
        uint32_t hls_slot = RESULT64 + 2 * k + 1;
        if (buf[hls_slot] != expected) {
            printf("k=%u hls_slot=%u got=%llx expected=%llx\n",
                   k,
                   hls_slot,
                   (unsigned long long)buf[hls_slot],
                   (unsigned long long)expected);
            return 1;
        }
    }
    printf("PASS\n");
    return 0;
}
