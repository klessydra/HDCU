<h1 align="center" style="margin-bottom: 0;">HDCU -
Configurable Hardware Acceleration for Hyperdimensional Computing Extension on RISC-V</h1>

<p align="center" style="font-size: 18px;">
    <a href="#-model-overview"  style="font-weight: bold;">🗺️ Model Overview</a> &nbsp;·&nbsp; 
    <a href="#-project-structure">📂 Project Structure</a> &nbsp;·&nbsp; 
    <a href="#-getting-started" style="font-weight: bold;">⚙️ Getting Started</a> &nbsp;·&nbsp;
    <a href="#-documentation">🚀 Next Updates </a> &nbsp;·&nbsp;
    <a href="#-publications">📜 Publications</a> &nbsp;·&nbsp;
    <a href="#-license">⚖️ License</a> &nbsp;·&nbsp;
    <a href="#-acknowledgements">🌟 Acknowledgements</a> &nbsp;·&nbsp;
    <a href="#-contact">📞 Contact</a>
</p>

## 🗺️ Model Overview
HDCU is an open-source and general purpose reconfigurable hardware accelerator powered by a novel RISC-V instruction set extension for Hyperdimensional Computing (HDC).
HDCU has been designed at Sapienza University of Rome, accelerates core arithmetic operations on hypervectors, and can be configured at synthesis time to balance execution speed and resource usage, adapting to diverse applications. A custom RISC-V Instruction Set Extension is designed to efficiently control the accelerator, with instructions fully integrated into the GCC compiler toolchain and exposed to the programmer as intrinsic function calls, constituting a simple yet effective Application Programming Interface (API) to implement HDC tasks.  Dedicated Control Status Registers allow users to specify the characteristics of the high-dimensional space and the target learning tasks at runtime, controlling the hardware loops of the accelerator and enabling the same hardware architecture to be used for various tasks. The dual flexibility coming from hardware configuration and software programmability sets this work apart from application-specific solutions in the literature, offering a unique, versatile accelerator adaptable to a wide range of applications and learning tasks. 

### Context: 
Since HDC is well-suited for hardware acceleration on FPGAs and ASICs due to the highly parallel nature of the operations on HVs, various studies have proposed very optimized hardware solutions to accelerate HDC tasks. However, these architectures are often application-specific. They rely on fixed design parameters aligned with particular learning tasks, datasets, hypervector sizes, and hardware/energy constraints. 
In contrast, our work introduces a flexible, open-source architecture that empowers the community to accelerate virtually any HDC-based learning task and to assess the impact of hardware acceleration on their own algorithms. Rather than optimizing a single, predefined sequence of operations, HDCU accelerates the fundamental arithmetic operations at the core of HDC. A dedicated set of custom RISC-V instructions—exposed as high-level C intrinsic function calls—provides intuitive control over the accelerator, enabling both simplicity of integration and broad applicability across diverse workloads.

### Description:
The designed HDCU is integrated into the execution stage of the Klessydra-T03 core, as depicted in the Figure below, and may operate in parallel with the Load-Store Unit (LSU) and the Arithmetic Logic Unit (ALU). It may be synthesized to be replicated for each thread running in the core, or shared among the threads
The HDCU operates as a coprocessor in charge of executing HDC operations. In the decode phase of the instruction processing pipeline of the Klessydra T03 core, if the fetched instruction belongs to the HDC ISE and the corresponding acceleration unit is available, the HDCU intervention is requested to handle the operation. The instructions executed by the HDCU operate on HVs contained in local Scratchpad Memories (SPMs), which are designed to be synthesized in Block-RAMs on FPGA and allow adequate bandwidth for the parallel operations on HVs. The size of each SPM can be configured at synthesis time. HVs in the Data Memory are loaded into and from the SPMs through the Scratchpad Memory Interface (SPMI) using two dedicated instructions (hvmemld and hvmemstr).

<p align="center">
  <img src="https://github.com/user-attachments/assets/816f5858-6740-45a7-a343-9360c8d3a74a" width="400">
</p>
The Figure below shows a detailed view of the microarchitecture of the HDCU. The design includes highly optimized functional units for each of the basic HDC arithmetic operations. The hardware parallelism inside each functional unit can be configured at synthesis time using a parameter called SIMD degree (Single Instruction Multiple Data). For instance, setting SIMD equal to 256 replicates the hardware to process 256 HV elements in one clock cycle. This flexibility enables trading off execution time with energy consumption and hardware requirements, allowing the accelerator to adapt to the specific needs of various application scenarios, ranging from embedded systems to high-performance computing. Additionally, each functional unit can be optionally disabled to reduce hardware requirements when necessary, leaving the software execution of the corresponding HDC operation on the core. 
<p align="center">
  <img src="https://github.com/user-attachments/assets/143314fe-17c7-42d6-a235-f8366b3a4126" width="400">
</p>

Differently from existing solutions in the literature, the HV size in HDCU is not fixed but can be set at runtime by writing into a dedicated Control and Status Register (CSR), denoted as HVSIZE and visible to the software program through the CSR HVSIZE instruction. The value of HVSIZE controls the hardware loops in the functional units. The hardware loops keep the required functional unit busy until HVSIZE elements are processed, avoiding the need for repetitive software loops
in which the same instructions are repeatedly fetched and decoded.

Table I lists the instructions expressed via the intrinsic function syntax.
<p align="center">
  <img src="https://github.com/user-attachments/assets/1cdf5f7e-531e-48dd-a928-da533dc36e27" width="400">
</p>

Table II summarizes the parameters that can be tuned at synthesis time and at runtime.

<p align="center">
  <img src="https://github.com/user-attachments/assets/bc281b09-9fd3-4043-bad3-74e6f996caf1" width="400">
</p>

## ⚙️ Getting Started

Follow these steps to download and deploy the accelerator in the Klessydra environment:

### 1. Install the Toolchain
- Clone the [RISC-V GNU Toolchain with Klessydra Instruction Extensions](https://github.com/klessydra/riscv-gnu-toolchain).
- Replace the original `riscv-opc.c` and `riscv-opc.h` files in the cloned repository with those provided in this repository to enable the accelerator-specific instructions.
- Build the modified toolchain.

### 2. Acquire the Klessydra Core
- Clone the [Klessydra core](https://github.com/klessydra/pulpino-klessydra).
- Follow the provided tutorial to set up the `pulpino-klessydra` environment.

### 3. Apply the Patch
- Replace the `pulpino-klessydra/ips/T13x` directory with the patched version available in this repository.
- Copy the `hdc_libs` folder into the `pulpino-klessydra/sw/libs` directory.
- Copy the `klessydra_t13h_hdcu_tests` folder into the `pulpino-klessydra/sw/apps` directory.
- Replace the `dsp_functions.h` file in `pulpino-klessydra/sw/libs/klessydra_libs/dsp_libs/inc` with the version provided in this repository.

_Stay tuned! A fully automated installation procedure and a more detailed guide will be available soon. For assistance, feel free to contact us._



## 📂 Project Structure
```
.
├── T13x                      # Hardware design and implementation files for the HDCU
├── hdc_libs                  # Hyperdimensional Computing software libraries
├── klessydra_t13h_hdcu_tests # Test programs and examples for verifying HDCU functionality
├── dsp_functions.h           # Inline assembly macros and function prototypes for HDC
├── riscv-opc.c               # Definitions of RISC-V opcode mappings for custom HDCU instructions.
├── riscv-opc.h               # Header file containing opcode definitions and macros for custom HDCU instructions in RISC-V.
├── LICENSE                   # License information
└── README.md                 # Main project documentation
```

## 🚀 Next Updates
-  Working on a fully automated procedure for deployment on FPGA;
-  New HVs type will be supported soon
  
## 📜 Publications

<details id="citation">
  <summary><strong>Citation</strong></summary>

  **Highlight and manually copy the citation format of your choice.**

  - **BibTeX**: 
    ```
    @article{HDCU,
    title={Configurable Hardware Acceleration for Hyperdimensional Computing Extension on RISC-V},
    url={http://dx.doi.org/10.36227/techrxiv.173337827.72919533/v1},
    DOI={10.36227/techrxiv.173337827.72919533/v1},
    publisher={Institute of Electrical and Electronics Engineers (IEEE)},
    journal={TechRxiv},
    author={Martino, Rocco and Angioli, Marco and Rosato, Antonello and Barbirotta, Marcello and Cheikh, Abdallah and Olivieri, Mauro},
    year={2024},
    month=dec }
    ```
</details>

 🔗 [**Download Paper**](https://www.techrxiv.org/users/744590/articles/1245752-configurable-hardware-acceleration-for-hyperdimensional-computing-extension-on-risc-v)


## ⚖️ License

This project is fortified with the Apache license.

## 📞 Contact

For any collaboration, discussion, or support request, get in touch with:

- **Marco Angioli** - 📧 [Email](mailto:marco.angioli@uniroma1.it)
- **Rocco Martino** - 📧 [Email](mailto:rocco.martino@uniroma1.it)
- **Antonello Rosato** - 📧 [Email](mailto:antonello.rosato@uniroma1.it)
