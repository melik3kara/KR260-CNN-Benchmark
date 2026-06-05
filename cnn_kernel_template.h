#ifndef CNN_KERNEL_TEMPLATE_H
#define CNN_KERNEL_TEMPLATE_H

#include <stdint.h>

#ifndef CNN_TOP
#error "CNN_TOP must name the HLS top function"
#endif

#ifndef CNN_CONV_LAYERS
#error "CNN_CONV_LAYERS must be defined"
#endif

#ifndef CNN_PIPELINE
#define CNN_PIPELINE 0
#endif

#ifndef CNN_UNROLL
#define CNN_UNROLL 1
#endif

#define IMG_H 32
#define IMG_W 32
#define IMG_C 3
#define CNN_C 8
#define OUT_CLASSES 10
#define INPUT_BYTES (IMG_H * IMG_W * IMG_C)
#define SCORE64 2048

static int8_t weight_at(int layer, int oc, int ic, int ky, int kx) {
    int v = (layer * 13 + oc * 7 + ic * 5 + ky * 3 + kx * 11) % 5;
    return (int8_t)(v - 2);
}

static int8_t score_weight(int cls, int ch) {
    int v = (cls * 3 + ch * 5) % 7;
    return (int8_t)(v - 3);
}

static int16_t relu_clamp(int32_t v) {
    if (v <= 0) return 0;
    v >>= 3;
    if (v > 127) return 127;
    return (int16_t)v;
}

static int8_t read_i8(volatile uint64_t *buf, int byte_idx) {
    uint64_t word = buf[byte_idx >> 3];
    uint32_t shift = (uint32_t)(byte_idx & 7) * 8u;
    return (int8_t)((word >> shift) & 0xffu);
}

static void conv_relu_layer(
    int layer,
    int in_ch,
    int16_t src[CNN_C][IMG_H][IMG_W],
    int16_t dst[CNN_C][IMG_H][IMG_W]) {
    for (int oc = 0; oc < CNN_C; oc++) {
        for (int y = 0; y < IMG_H; y++) {
            for (int x = 0; x < IMG_W; x++) {
#if CNN_PIPELINE
#pragma HLS PIPELINE II=1
#endif
                int32_t acc = 0;
                for (int ic = 0; ic < CNN_C; ic++) {
#if CNN_UNROLL == 2
#pragma HLS UNROLL factor=2
#elif CNN_UNROLL == 4
#pragma HLS UNROLL factor=4
#endif
                    if (ic < in_ch) {
                        for (int ky = 0; ky < 3; ky++) {
                            for (int kx = 0; kx < 3; kx++) {
                                int yy = y + ky - 1;
                                int xx = x + kx - 1;
                                int16_t pix = 0;
                                if (yy >= 0 && yy < IMG_H && xx >= 0 && xx < IMG_W) {
                                    pix = src[ic][yy][xx];
                                }
                                acc += (int32_t)pix * (int32_t)weight_at(layer, oc, ic, ky, kx);
                            }
                        }
                    }
                }
                dst[oc][y][x] = relu_clamp(acc);
            }
        }
    }
}

static void write_scores(volatile uint64_t *buf, int32_t scores[OUT_CLASSES]) {
    for (int pair = 0; pair < OUT_CLASSES / 2; pair++) {
        uint64_t lo = (uint32_t)scores[2 * pair];
        uint64_t hi = (uint32_t)scores[2 * pair + 1];
        buf[SCORE64 + 2 * pair + 1] = lo | (hi << 32);
    }
}

void CNN_TOP(volatile uint64_t *buf) {
#pragma HLS INTERFACE m_axi     port=buf offset=slave bundle=gmem depth=4096
#pragma HLS INTERFACE s_axilite port=buf    bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control

    int16_t fmap_a[CNN_C][IMG_H][IMG_W];
    int16_t fmap_b[CNN_C][IMG_H][IMG_W];
    int32_t scores[OUT_CLASSES];
#pragma HLS BIND_STORAGE variable=fmap_a type=ram_2p impl=bram
#pragma HLS BIND_STORAGE variable=fmap_b type=ram_2p impl=bram
#pragma HLS ARRAY_PARTITION variable=scores complete

    for (int c = 0; c < CNN_C; c++) {
        for (int y = 0; y < IMG_H; y++) {
            for (int x = 0; x < IMG_W; x++) {
#if CNN_PIPELINE
#pragma HLS PIPELINE II=1
#endif
                if (c < IMG_C) {
                    int idx = (y * IMG_W + x) * IMG_C + c;
                    fmap_a[c][y][x] = (int16_t)read_i8(buf, idx);
                } else {
                    fmap_a[c][y][x] = 0;
                }
                fmap_b[c][y][x] = 0;
            }
        }
    }

    for (int layer = 0; layer < CNN_CONV_LAYERS; layer++) {
        int in_ch = layer == 0 ? IMG_C : CNN_C;
        if ((layer & 1) == 0) {
            conv_relu_layer(layer, in_ch, fmap_a, fmap_b);
        } else {
            conv_relu_layer(layer, in_ch, fmap_b, fmap_a);
        }
    }

    int16_t (*final_map)[IMG_H][IMG_W] = (CNN_CONV_LAYERS & 1) ? fmap_b : fmap_a;
    for (int cls = 0; cls < OUT_CLASSES; cls++) {
        int32_t acc = 0;
        for (int c = 0; c < CNN_C; c++) {
            int8_t w = score_weight(cls, c);
            for (int y = 0; y < IMG_H; y++) {
                for (int x = 0; x < IMG_W; x++) {
#if CNN_PIPELINE
#pragma HLS PIPELINE II=1
#endif
                    acc += (int32_t)final_map[c][y][x] * (int32_t)w;
                }
            }
        }
        scores[cls] = acc;
    }

    write_scores(buf, scores);
}

#endif
