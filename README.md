# KR260 FPGA Accelerator Benchmarks

This repository contains measured FPGA accelerator benchmarks for the AMD Kria
KR260 Robotics Starter Kit using Vitis HLS and Vivado. Linux userspace controls
the accelerators through AXI-Lite, shares physically contiguous DDR buffers
through `u-dma-buf`, and accelerator kernels access DDR through HLS `m_axi`
interfaces.

All results listed below are measured on real KR260 hardware and have `PASS`
status.

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
| Data interface | HLS `m_axi` DDR access |
| Shared buffer mechanism | `u-dma-buf` |

## Benchmark Groups

### DDR Benchmark

- `ddr_benchmark`: DDR-to-accelerator-to-DDR memory path benchmark.

### Reference / Legacy Designs

- `cnn20_reference`: formerly `cnn20_original`; an early validated CNN20
  proof-of-concept accelerator and reference implementation.

### Pipeline Benchmark Family

- `cnn10_pipeline`: 10-layer pipeline-oriented CNN benchmark.
- `cnn20_pipeline`: 20-layer pipeline-oriented CNN benchmark.
- `cnn20_pipeline_unroll2`: 20-layer pipeline-oriented CNN benchmark with
  explicit unroll factor 2.

The reference CNN20 design and the pipeline benchmark family use different
architectural organizations and therefore should not be directly compared using
resource counts alone.

## Measured Results

| Benchmark | Family | Workload | Iterations | Latency (us) | Throughput | GOPS | Bandwidth (MB/s) | Status |
|---|---|---|---:|---:|---:|---:|---:|---|
| `ddr_benchmark` | DDR | 8192 bytes/iteration | 1000 | 56.970 | 17553.098 iter/s | N/A | 143.795 | PASS |
| `cnn20_reference` | Reference / Legacy | 10 Conv + 10 ReLU | 100 | 7556.630 | 132.334 FPS | 1.485191 | 0.412 | PASS |
| `cnn10_pipeline` | Pipeline | 5 Conv + 5 ReLU | 100 | 1411.737 | 708.347 FPS | 3.771807 | N/A | PASS |
| `cnn20_pipeline` | Pipeline | 10 Conv + 10 ReLU | 100 | 1822.808 | 548.604 FPS | 6.157006 | N/A | PASS |
| `cnn20_pipeline_unroll2` | Pipeline | 10 Conv + 10 ReLU | 100 | 1822.849 | 548.592 FPS | 6.156868 | N/A | PASS |

## Resource Summary

| Benchmark | Family | LUT | FF | BRAM | DSP | Clock (MHz) |
|---|---|---:|---:|---:|---:|---:|
| `ddr_benchmark` | DDR | 2016 | 2338 | 0 | 0 | 177.778 |
| `cnn20_reference` | Reference / Legacy | 6472 | 5962 | 19 | 261 | 177.778 |
| `cnn10_pipeline` | Pipeline | 13712 | 11914 | 145 | 234 | 177.778 |
| `cnn20_pipeline` | Pipeline | 13983 | 12067 | 145 | 234 | 177.778 |
| `cnn20_pipeline_unroll2` | Pipeline | 13983 | 12067 | 145 | 234 | 177.778 |

## Architectural Evolution

Stage 1 produced `cnn20_reference`, the initial validated CNN20 design. Its role
is early validation: it proved the end-to-end accelerator flow and provides a
known working reference point.

Stage 2 produced the pipeline benchmark family: `cnn10_pipeline`,
`cnn20_pipeline`, and `cnn20_pipeline_unroll2`. These designs use a later
pipeline-oriented architectural organization intended for benchmark scaling and
optimization studies.

Because Stage 1 and Stage 2 differ architecturally, the lower latency of the
pipeline family should not be described as the result of merely adding pipeline
pragmas to `cnn20_reference`.

## Key Findings

The DDR benchmark demonstrates a working userspace-to-DDR-to-FPGA-to-DDR path
without DMA, using AXI-Lite control and HLS `m_axi` memory access. It is the
appropriate benchmark for memory-path bandwidth in this project.

The CNN benchmarks are compute-oriented and operate on a small `32x32x3` int8
input image. Their reported `bandwidth_MB_s` values should not be interpreted as
peak DDR bandwidth. For memory bandwidth, use `ddr_benchmark`.

Within the pipeline benchmark family, `cnn20_pipeline` and
`cnn20_pipeline_unroll2` produce virtually identical latency and identical
resource utilization. The measured difference is below practical measurement
significance, and the result strongly suggests that explicit unroll2 did not
change the generated hardware beyond the pipeline design.

## Documentation

- [benchmark_summary.md](benchmark_summary.md): compact result summary.
- [benchmark_report.md](benchmark_report.md): publication-style analysis,
  methodology, tables, discussion, lessons learned, and future work.
- [benchmark_analysis.md](benchmark_analysis.md): technical analysis of the
  benchmark families and interpretation risks.

## Scope

The CNN designs are benchmark scaffolds with deterministic fixed weights. They
are intended for performance and resource measurement, not trained-model
accuracy evaluation.
