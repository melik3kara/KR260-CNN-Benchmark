# KR260 FPGA Accelerator Benchmark Report

## Executive Summary

This report presents measured FPGA accelerator benchmarks on the AMD Kria KR260
Robotics Starter Kit. The evaluated designs use Linux userspace control through
AXI-Lite, physically contiguous DDR buffers through `u-dma-buf`, and HLS
`m_axi` ports for accelerator memory access. No DMA engine is used in the
reported data path.

The benchmark suite contains one DDR memory-path benchmark, one reference CNN20
design, and one pipeline-oriented CNN benchmark family. All results in this
report are measured on real KR260 hardware and have `PASS` status.

The DDR benchmark verifies the end-to-end userspace-to-DDR-to-FPGA-to-DDR path.
It achieves `56.970 us` latency per 8192-byte iteration, corresponding to
`17553.098` iterations/s and `143.795 MB/s`.

The CNN results show acceleration over the measured CPU baseline. The
`cnn20_reference` design reaches `132.334 FPS` and `1.485191 GOPS`, with an
`11.843x` speedup. The separate `cnn20_pipeline` benchmark reaches `548.604 FPS`
and `6.157006 GOPS`, with a `49.094x` speedup.

The name `cnn20_reference` replaces the earlier name `cnn20_original`. This
renaming is intentional: the reference design and the pipeline benchmark family
use different architectural organizations and should not be described as simple
optimization levels of the same implementation.

## Platform

| Field | Value |
|---|---|
| Board | AMD Kria KR260 Robotics Starter Kit |
| FPGA | K26 SOM |
| Clock | 177.778 MHz |
| HLS tool | Vitis HLS 2025.1 |
| Implementation tool | Vivado 2025.1 |
| Host OS | Ubuntu Linux |
| Control interface | AXI-Lite |
| Data interface | HLS `m_axi` DDR memory access |
| Shared DDR buffer | `u-dma-buf` |

## Architecture Description

The measured accelerators use a simple userspace-controlled architecture:

```text
Linux userspace
  -> u-dma-buf shared DDR buffer
  -> HLS accelerator through m_axi
  -> u-dma-buf shared DDR buffer
  -> Linux userspace
```

Control and status are handled through AXI-Lite registers. Bulk data is stored
in DDR and accessed directly by the HLS accelerator through an `m_axi` interface.
The benchmark design avoids DMA and uses a shared DDR buffer for input and
output regions.

The DDR benchmark isolates the memory path by measuring accelerator access to a
DDR buffer. The CNN benchmarks use the same software-controlled execution model,
but their measured runtime is dominated by convolution compute rather than DDR
traffic.

## Architectural Evolution

### Stage 1: Reference / Legacy Design

`cnn20_reference` is the initial validated CNN20 accelerator design, formerly
called `cnn20_original`. Its purpose is early validation:

- proof-of-concept accelerator
- proof that the KR260 userspace-to-DDR-to-HLS flow works
- stable reference implementation for later reporting

Measured `cnn20_reference` resources:

| LUT | FF | BRAM | DSP | Clock (MHz) |
|---:|---:|---:|---:|---:|
| 6472 | 5962 | 19 | 261 | 177.778 |

### Stage 2: Pipeline Benchmark Family

The pipeline benchmark family consists of:

- `cnn10_pipeline`
- `cnn20_pipeline`
- `cnn20_pipeline_unroll2`

These designs are a separate benchmark family built using a later
pipeline-oriented architecture. They are appropriate for pipeline-family scaling
and unroll studies, but they should not be presented as direct resource-level
variants of `cnn20_reference`.

Measured `cnn20_pipeline` resources:

| LUT | FF | BRAM | DSP | Clock (MHz) |
|---:|---:|---:|---:|---:|
| 13983 | 12067 | 145 | 234 | 177.778 |

The reference CNN20 design and the pipeline benchmark family use different
architectural organizations and therefore should not be directly compared using
resource counts alone. In particular, BRAM increases from `19` to `145`, LUT
count roughly doubles, and DSP count changes from `261` to `234`.

## Benchmark Methodology

For each FPGA benchmark, the host program prepares the shared DDR buffer,
launches the accelerator through AXI-Lite, waits for completion, verifies the
output, and records timing. Only runs with correct output verification are
reported as `PASS`.

The DDR benchmark reports iterations, bytes per iteration, latency, iterations
per second, effective bandwidth, and implementation resources.

The CNN benchmarks report layers, input shape, output shape, latency, frames per
second, estimated operation count, estimated GOPS, CPU baseline latency, FPGA
speedup over CPU, and implementation resources.

The CNN operation counts are algorithmic estimates used for performance
normalization. They are not hardware event-counter measurements.

## DDR Benchmark

The DDR benchmark verifies the shared-buffer memory path:

```text
PS userspace -> DDR buffer -> FPGA accelerator m_axi -> DDR buffer -> PS userspace
```

| Metric | Value |
|---|---:|
| Benchmark | `ddr_benchmark` |
| Iterations | 1000 |
| Bytes per iteration | 8192 |
| Latency | 56.970 us |
| Throughput | 17553.098 iter/s |
| Bandwidth | 143.795 MB/s |
| Status | PASS |

Resource usage:

| LUT | FF | BRAM | DSP | Clock (MHz) |
|---:|---:|---:|---:|---:|
| 2016 | 2338 | 0 | 0 | 177.778 |

This benchmark should be treated as the memory-path benchmark in the project.
It directly measures DDR buffer access through the accelerator data path.

## CNN Benchmark Designs

The CNN accelerators are deterministic benchmark scaffolds. They use fixed dummy
weights and are not trained accuracy models.

| Group | Design | Layers | Input | Output |
|---|---|---:|---|---|
| Reference / Legacy | `cnn20_reference` | 10 Conv + 10 ReLU | 32x32x3 int8 | 10 int32 scores |
| Pipeline | `cnn10_pipeline` | 5 Conv + 5 ReLU | 32x32x3 int8 | 10 int32 scores |
| Pipeline | `cnn20_pipeline` | 10 Conv + 10 ReLU | 32x32x3 int8 | 10 int32 scores |
| Pipeline | `cnn20_pipeline_unroll2` | 10 Conv + 10 ReLU | 32x32x3 int8 | 10 int32 scores |

## Performance Results

| Benchmark | Group | Iterations | Latency (us) | FPS / Throughput | Estimated Ops | GOPS | Bandwidth (MB/s) | Status |
|---|---|---:|---:|---:|---:|---:|---:|---|
| `ddr_benchmark` | DDR | 1000 | 56.970 | 17553.098 iter/s | N/A | N/A | 143.795 | PASS |
| `cnn20_reference` | Reference / Legacy | 100 | 7556.630 | 132.334 FPS | 11223040 | 1.485191 | 0.412 | PASS |
| `cnn10_pipeline` | Pipeline | 100 | 1411.737 | 708.347 FPS | 5324800 | 3.771807 | N/A | PASS |
| `cnn20_pipeline` | Pipeline | 100 | 1822.808 | 548.604 FPS | 11223040 | 6.157006 | N/A | PASS |
| `cnn20_pipeline_unroll2` | Pipeline | 100 | 1822.849 | 548.592 FPS | 11223040 | 6.156868 | N/A | PASS |

The reported CNN FPS values are consistent with latency:

| Benchmark | Latency (us) | Reported FPS |
|---|---:|---:|
| `cnn20_reference` | 7556.630 | 132.334 |
| `cnn10_pipeline` | 1411.737 | 708.347 |
| `cnn20_pipeline` | 1822.808 | 548.604 |
| `cnn20_pipeline_unroll2` | 1822.849 | 548.592 |

## Resource Utilization

| Benchmark | Group | LUT | FF | BRAM | DSP | Clock (MHz) |
|---|---|---:|---:|---:|---:|---:|
| `ddr_benchmark` | DDR | 2016 | 2338 | 0 | 0 | 177.778 |
| `cnn20_reference` | Reference / Legacy | 6472 | 5962 | 19 | 261 | 177.778 |
| `cnn10_pipeline` | Pipeline | 13712 | 11914 | 145 | 234 | 177.778 |
| `cnn20_pipeline` | Pipeline | 13983 | 12067 | 145 | 234 | 177.778 |
| `cnn20_pipeline_unroll2` | Pipeline | 13983 | 12067 | 145 | 234 | 177.778 |

The DDR benchmark is small and uses no BRAM or DSP. This is expected because it
is primarily a memory-path validation and bandwidth benchmark.

The pipeline benchmark family uses much more BRAM and logic than
`cnn20_reference`. This should be interpreted as an architectural difference,
not as the isolated cost of enabling pipeline pragmas on the reference design.

Within the pipeline benchmark family, `cnn20_pipeline` and
`cnn20_pipeline_unroll2` have identical resource counts:

| Design | LUT | FF | BRAM | DSP | Clock (MHz) |
|---|---:|---:|---:|---:|---:|
| `cnn20_pipeline` | 13983 | 12067 | 145 | 234 | 177.778 |
| `cnn20_pipeline_unroll2` | 13983 | 12067 | 145 | 234 | 177.778 |

This equality is important evidence that the explicit unroll2 directive did not
materially change the generated hardware within the pipeline family.

## FPGA vs CPU Comparison

The measured CPU baseline latency used for the CNN comparisons is
`89489.679 us`.

| Benchmark | Group | FPGA Latency (us) | CPU Latency (us) | Speedup |
|---|---|---:|---:|---:|
| `cnn20_reference` | Reference / Legacy | 7556.630 | 89489.679 | 11.843x |
| `cnn10_pipeline` | Pipeline | 1411.737 | 89489.679 | 63.390x |
| `cnn20_pipeline` | Pipeline | 1822.808 | 89489.679 | 49.094x |
| `cnn20_pipeline_unroll2` | Pipeline | 1822.849 | 89489.679 | 49.093x |

The `cnn20_reference` accelerator demonstrates a clear end-to-end speedup over
the CPU baseline. The pipeline family also demonstrates strong speedups, but
those results should be interpreted as results from a separate architectural
family rather than as direct optimization deltas from `cnn20_reference`.


