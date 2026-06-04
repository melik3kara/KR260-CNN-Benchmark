# Benchmark Summary

This summary reports measured FPGA benchmark results on the AMD Kria KR260
Robotics Starter Kit. All rows shown here are measured on real hardware and have
`PASS` status.

## Platform

| Field | Value |
|---|---|
| Board | AMD Kria KR260 Robotics Starter Kit |
| FPGA | K26 SOM |
| Clock | 177.778 MHz |
| Toolchain | Vitis HLS 2025.1, Vivado 2025.1 |
| Host | Ubuntu Linux |
| Control | AXI-Lite |
| Data path | HLS `m_axi` DDR access |
| Shared DDR buffer | `u-dma-buf` |

## Benchmark Organization

| Group | Designs | Purpose |
|---|---|---|
| DDR benchmark | `ddr_benchmark` | Memory-path validation and bandwidth measurement |
| Reference / Legacy designs | `cnn20_reference` | Early validated CNN20 proof-of-concept and reference implementation |
| Pipeline benchmark family | `cnn10_pipeline`, `cnn20_pipeline`, `cnn20_pipeline_unroll2` | Later pipeline-oriented CNN benchmark family |

`cnn20_reference` was formerly named `cnn20_original`. The new name is used to
avoid implying that the pipeline family is merely a pragma-optimized version of
the same implementation.

The reference CNN20 design and the pipeline benchmark family use different
architectural organizations and therefore should not be directly compared using
resource counts alone.

## Performance Results

| Benchmark | Group | Design | Iterations | Latency (us) | Throughput | Estimated Ops | GOPS | Bandwidth (MB/s) | Status |
|---|---|---|---:|---:|---:|---:|---:|---:|---|
| `ddr_benchmark` | DDR | DDR buffer read/write path | 1000 | 56.970 | 17553.098 iter/s | N/A | N/A | 143.795 | PASS |
| `cnn20_reference` | Reference / Legacy | 10 Conv + 10 ReLU | 100 | 7556.630 | 132.334 FPS | 11223040 | 1.485191 | 0.412 | PASS |
| `cnn10_pipeline` | Pipeline | 5 Conv + 5 ReLU | 100 | 1411.737 | 708.347 FPS | 5324800 | 3.771807 | N/A | PASS |
| `cnn20_pipeline` | Pipeline | 10 Conv + 10 ReLU | 100 | 1822.808 | 548.604 FPS | 11223040 | 6.157006 | N/A | PASS |
| `cnn20_pipeline_unroll2` | Pipeline | 10 Conv + 10 ReLU | 100 | 1822.849 | 548.592 FPS | 11223040 | 6.156868 | N/A | PASS |

## Resource Results

| Benchmark | Group | LUT | FF | BRAM | DSP | Clock (MHz) |
|---|---|---:|---:|---:|---:|---:|
| `ddr_benchmark` | DDR | 2016 | 2338 | 0 | 0 | 177.778 |
| `cnn20_reference` | Reference / Legacy | 6472 | 5962 | 19 | 261 | 177.778 |
| `cnn10_pipeline` | Pipeline | 13712 | 11914 | 145 | 234 | 177.778 |
| `cnn20_pipeline` | Pipeline | 13983 | 12067 | 145 | 234 | 177.778 |
| `cnn20_pipeline_unroll2` | Pipeline | 13983 | 12067 | 145 | 234 | 177.778 |

## CPU Comparison

The measured CPU baseline latency used for the CNN comparisons is
`89489.679 us`.

| Benchmark | Group | FPGA Latency (us) | CPU Latency (us) | Speedup |
|---|---|---:|---:|---:|
| `cnn20_reference` | Reference / Legacy | 7556.630 | 89489.679 | 11.843x |
| `cnn10_pipeline` | Pipeline | 1411.737 | 89489.679 | 63.390x |
| `cnn20_pipeline` | Pipeline | 1822.808 | 89489.679 | 49.094x |
| `cnn20_pipeline_unroll2` | Pipeline | 1822.849 | 89489.679 | 49.093x |

## Architectural Evolution

Stage 1: `cnn20_reference` is the initial validated CNN20 accelerator. Its role
is proof of correct accelerator bring-up and a stable reference point.

Stage 2: `cnn10_pipeline`, `cnn20_pipeline`, and `cnn20_pipeline_unroll2` form a
separate pipeline benchmark family built with a later pipeline-oriented
architecture.

The resource differences between `cnn20_reference` and `cnn20_pipeline` are too
large to treat them as the same architecture with only a pipeline directive
changed. BRAM increases from `19` to `145`, LUT count roughly doubles, and DSP
count changes from `261` to `234`.

## Main Observations

The DDR benchmark is the memory-path benchmark and reports `143.795 MB/s` for
8192 bytes per iteration with a latency of `56.970 us`.

The CNN benchmarks are compute-oriented. Their input and output transfers are
small, so CNN `bandwidth_MB_s` should not be interpreted as peak DDR bandwidth.

Within the pipeline benchmark family, `cnn20_pipeline` and
`cnn20_pipeline_unroll2` are effectively identical in both runtime and
resources. The latency difference is `0.041 us`, and both designs use the same
LUT, FF, BRAM, DSP, and clock values. Explicit unroll2 did not improve this
measured pipeline-family implementation.

The CNN designs are fixed-weight benchmark scaffolds. They are useful for
performance and resource comparison, but not for trained-model accuracy claims.
