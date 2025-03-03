# HDCU Accelerator Integration

## Introduction

This repository provides the necessary files to integrate the Hyperdimensional Computing Unit (HDCU) accelerator into the pulpino‑klessydra project. The HDCU is a general-purpose accelerator designed for hyperdimensional computing tasks. It optimizes key operations—such as binding, bundling, permutation, clipping, similarity, and associative search—using a configurable hardware architecture. For further background on HDC and its advantages, please consult the accompanying article.

## Integration Tutorial

Follow the steps below to properly integrate the HDCU components into the pulpino‑klessydra environment:

1. **Integrate the IP Files:**
   - Locate the `T13x` folder in this repository.  
   - Copy the entire `T13x` folder into the `IPs` directory of the pulpino‑klessydra project.
   - This folder contains the project files for the Klessydra T13 core equipped with the HDCU accelerator.

2. **Install the HDC Software Library:**
   - Find the `hdc_libs` folder.  
   - Move `hdc_libs` to the path `pulpino‑klessydra/sw/libs/klessydra_lib`.
   - This directory includes the headers, source files, and helper functions to facilitate using the HDCU.

3. **Set Up the Test Applications:**
   - Identify the `klessydra_hdcu_tests` folder in the repository.  
   - Place the `klessydra_hdcu_tests` directory into `pulpino‑klessydra/sw/apps/klessydra_test`.
   - These test applications (written in C) are designed to verify the functionality of each HDCU unit.

4. **Follow the Main Project Tutorial:**
   - Before replacing or adding these files, make sure to follow the tutorial provided in the main pulpino‑klessydra repository.
   - The tutorial explains how to set up the environment, compile the project, and integrate new components.
   - Once the main tutorial is completed, incorporate the HDCU files as detailed above.

## About the HDCU

The HDCU accelerator is integrated into the Klessydra T13 core as a coprocessor, enhancing performance for hyperdimensional computing tasks. Its design allows configurable hardware parallelism and operational flexibility, making it suitable for a wide range of applications—from edge computing to performance-intensive tasks. The accelerator's instruction set extension (ISE) is seamlessly integrated into the GCC toolchain, providing an easy-to-use API for developers.
