# EPSILOD: Efficient Parallel Skeleton for Iterative Stencil Computations in Distributed Heterogeneous Systems

EPSILOD is an parallel skeleton for generic iterative stencil computations in distributed heterogeneous systems. It can distribute the execution of iterative stencil computations across devices of different computation capabilities, with policies to balance the workload, and coordinate the execution and communication across different device technologies.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

## Overview

EPSILOD is presented as a reentrant, higher-order function written in C11, compatible with any modern C/C++ compiler. The function solves a fixed number of iterations of a geometric stencil computation, applying the stencil operator to each element of an n-dimensional array. The programmer provides the stencil operator as skeleton parameters.

EPSILOD is built upon [the Controller model](https://gitlab.com/trasgo-group-valladolid/controllers) and [the Hitmap library](https://gitlab.com/trasgo-group-valladolid/controllers/-/tree/master/extern/hitmap). The Controller model is the portability layer that enables EPSILOD's heterogeneous computing capabilities. The Hitmap library enables the distributed computing capabilities.

Currently, the device types supported by EPSILOD are:
* Multi-core CPUs (through OpenMP)
* NVIDIA GPUs (through CUDA)
* AMD GPUs (through HIP)
* GPUs (through OpenCL)
* Intel FPGAs (through the Intel FPGA SDK for OpenCL)

## Citation

If you use EPSILOD in your research in any meaningful way, we would appreciate that you cite it as follows:
```BibTeX
@article{deCastro2023:EPSILOD,
	author = {de Castro, Manuel and Santamar{\'i}a Valenzuela, Inmaculada and Torres, Yuri and Gonzalez-Escribano, Arturo and Llanos, Diego},
	year = {2023},
	month = {01},
	pages = {9409–9442},
	title = {EPSILOD: efficient parallel skeleton for generic iterative stencil computations in distributed GPUs},
	volume = {79},
	journal = {The Journal of Supercomputing},
	doi = {10.1007/s11227-022-05040-y}
}
```
