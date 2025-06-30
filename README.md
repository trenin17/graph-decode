# Graph Decode

## Overview
This project is designed for high-performance experimentation with thread-per-stage and task-per-stage (fiber-based) concurrency models, particularly in multimedia-like data processing workflows.

## Features

- Reproduction of DAG-based decoding pipelines
- Four different implementation approaches:
  - **Thread-per-stage** using `std::thread` in the `threads` branch
  - **Thread-per-stage** using `Boost.Thread` in the `boost-threads` branch
  - **Task-per-stage** using `Boost.Fiber` in the `main` branch
  - **Task-per-stage** using `OneTBB Flow Graph` in the `tbb` branch
- Support for custom decoder stages and packet processing
- Batching and memory pooling optimizations
- Performance profiling using Visual Studio tools
- Easily extensible for additional topologies

---

## Project Goals

This project was created to:

- Validate the performance of different parallelization strategies
- Simulate multimedia-style pipelines
- Serve as a reusable platform for benchmarking and tuning concurrency models
