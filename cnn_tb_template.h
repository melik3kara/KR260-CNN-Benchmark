#ifndef CNN_TB_TEMPLATE_H
#define CNN_TB_TEMPLATE_H

#include <stdint.h>
#include <stdio.h>

#define IMG_H 32
#define IMG_W 32
#define IMG_C 3
#define OUT_CLASSES 10
#define INPUT_BYTES (IMG_H * IMG_W * IMG_C)
#define INPUT_WORDS ((INPUT_BYTES + 7) / 8)
#define SCORE64 2048
#define BUF_WORDS 4096

#ifndef CNN_TOP
#error "CNN_TOP must name the HLS top function"
#endif

void CNN_TOP(volatile uint64_t *buf);

static int8_t input_byte(int idx) {
    return (int8_t)(((idx * 17 + 3) % 255) - 127);
}

static void fill_input(uint64_t *buf) {
    for (int w = 0; w < INPUT_WORDS; w++) {
        int base = (w & ~1) * 8;
        uint64_t word = 0;
        for (int b = 0; b < 8; b++) {
            int idx = base + b;
            uint8_t v = idx < INPUT_BYTES ? (uint8_t)input_byte(idx) : 0;
            word |= ((uint64_t)v) << (8 * b);
        }
        buf[w] = word;
    }
}

int main() {
    static uint64_t buf[BUF_WORDS];

    for (int i = 0; i < BUF_WORDS; i++) {
        buf[i] = 0;
    }
    fill_input(buf);
    for (int pair = 0; pair < OUT_CLASSES / 2; pair++) {
        buf[SCORE64 + 2 * pair] = 0;
        buf[SCORE64 + 2 * pair + 1] = 0;
    }

    CNN_TOP(buf);

    int nonzero = 0;
    for (int pair = 0; pair < OUT_CLASSES / 2; pair++) {
        uint64_t packed = buf[SCORE64 + 2 * pair + 1];
        int32_t s0 = (int32_t)(uint32_t)(packed & 0xffffffffu);
        int32_t s1 = (int32_t)(uint32_t)(packed >> 32);
        printf("score[%d]=%d\n", 2 * pair, s0);
        printf("score[%d]=%d\n", 2 * pair + 1, s1);
        if (s0 != 0 || s1 != 0) {
            nonzero = 1;
        }
    }

    if (!nonzero) {
        printf("FAIL: all scores are zero\n");
        return 1;
    }

    printf("PASS\n");
    return 0;
}

#endif
