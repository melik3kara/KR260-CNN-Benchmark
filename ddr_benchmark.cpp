#include <stdint.h>

#define N 1024
#define RESULT64 2048
#define MARKER_BASE 0xA000000000000000ULL

void ddr_benchmark(volatile uint64_t *buf) {
#pragma HLS INTERFACE m_axi     port=buf offset=slave bundle=gmem depth=8192
#pragma HLS INTERFACE s_axilite port=buf    bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control

    for (uint32_t k = 0; k < N; k++) {
        uint64_t marker = MARKER_BASE | k;
        buf[RESULT64 + 2 * k + 1] = marker;
    }
}
