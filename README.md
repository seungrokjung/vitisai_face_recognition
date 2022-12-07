# VAI Face Recognition 
This guide explains implementation detail of face recognition application.
The application was built on top of AMD's Vitis-AI SW stack.
AMD's VCK5000 AI Accelerator was used to run the application but it's compatible with other AMD's Alveo AI Accelerators.

**Face Recognition on AMD's AI Accelerator**

![Alt Text](./images/fr_tomcruise_sample.gif)

## Table of Contents
- [VAI Face Recognition](#VAI-Face-Recognition])
  - [Table Of Contents](#table-of-contents)
  - [Implementation Steps](#implementation-steps)
  - [Backgrounds](#backgrounds)
  - [Model overview](#model-overview)
  - [Preparation](#preparation)


## Model overview
In Face Recognition, 

## Preparation 
In Face Recognition, 

## Implementation Steps
AMD's VCK5000 card setup in the host machine.
Clone Vitis-AI git (https://github.com/Xilinx/Vitis-AI) and setup the card by running this command

```bash
cd Vitis-AI/setup/vck5000/
source ./install.sh
```

AI Accelerator status check
Check management and user BDFs(Bus:Device:Function). In this example, they are 0000:0b:00.0 and 0000:0b:00.1, respectively.
Make sure Kernel driver in use are xclmgmt and xocl.

1. $sudo lspci -vd 10ee:

Examine the the card by using managment BDF.
Make sure that you can find Platform and SC Version Platform UUID, Interface UUID, and Mac Address.

2. sudo xbmgmt examine -d 0000:0b:00.0

Validate the card by user BDF.
The card should pass Test 1~6.

3. sudo xbutil validate -d 0000:0b:00.1

```bash
$sudo lspci -vd 10ee:
0b:00.0 Memory controller: Xilinx Corporation Device 5048
        Subsystem: Xilinx Corporation Device 000e
        Physical Slot: 0-10
        Flags: bus master, fast devsel, latency 0
        Memory at e0000000 (64-bit, prefetchable) [size=128M]
        Memory at e8000000 (64-bit, prefetchable) [size=128K]
        Capabilities: [40] Power Management version 3
        Capabilities: [48] MSI: Enable- Count=1/1 Maskable- 64bit+
        Capabilities: [60] MSI-X: Enable- Count=32 Masked-
        Capabilities: [70] Express Endpoint, MSI 00
        Capabilities: [100] Advanced Error Reporting
        Capabilities: [600] Vendor Specific Information: ID=0020 Rev=0 Len=010 <?>
        Kernel driver in use: xclmgmt
        Kernel modules: xclmgmt

0b:00.1 Memory controller: Xilinx Corporation Device 5049
        Subsystem: Xilinx Corporation Device 000e
        Physical Slot: 0-10
        Flags: bus master, fast devsel, latency 0
        Memory at e8020000 (64-bit, prefetchable) [size=64K]
        Memory at d0000000 (64-bit, prefetchable) [size=256M]
        Capabilities: [40] Power Management version 3
        Capabilities: [48] MSI: Enable- Count=1/1 Maskable- 64bit+
        Capabilities: [60] MSI-X: Enable+ Count=32 Masked-
        Capabilities: [70] Express Endpoint, MSI 00
        Capabilities: [100] Advanced Error Reporting
        Capabilities: [600] Vendor Specific Information: ID=0020 Rev=0 Len=010 <?>
        Kernel driver in use: xocl
        Kernel modules: xocl

$ sudo xbmgmt examine -d 0000:0b:00.0
--------------------------------------------------------
1/1 [0000:0b:00.0] : xilinx_vck5000_gen3x16_xdma_base_1
--------------------------------------------------------
Flash properties
  Type                 : ospi_versal
  Serial Number        : XFL1RLSNAW4Q

Device properties
  Type                 : vck5000
  Name                 : VCK5000-P
  Config Mode          : 8
  Max Power            : 300W

Flashable partitions running on FPGA
  Platform             : xilinx_vck5000_gen3x16_xdma_base_1
  SC Version           : 4.4.12
  Platform UUID        : A96ADB1F-78C8-BEAC-9173-81FEF01B08CD
  Interface UUID       : E221A0FF-8695-D5EB-8725-FC65147F90C3

Flashable partitions installed in system
  Platform             : xilinx_vck5000_gen3x16_xdma_base_1
  SC Version           : 4.4.12
  Platform UUID        : A96ADB1F-78C8-BEAC-9173-81FEF01B08CD


  Mac Address          : 00:0A:35:0D:DB:BE
                       : 00:0A:35:0D:DB:BF

$ sudo xbutil validate -d 0000:0b:00.1
Starting validation for 1 devices

Validate Device           : [0000:0b:00.1]
    Platform              : xilinx_vck5000_gen3x16_xdma_base_1
    SC Version            : 4.4.12
    Platform ID           : A96ADB1F-78C8-BEAC-9173-81FEF01B08CD
-------------------------------------------------------------------------------
Test 1 [0000:0b:00.1]     : PCIE link
    Test Status           : [PASSED]
-------------------------------------------------------------------------------
Test 2 [0000:0b:00.1]     : SC version
    Test Status           : [PASSED]
-------------------------------------------------------------------------------
Test 3 [0000:0b:00.1]     : Verify kernel
    Test Status           : [PASSED]
-------------------------------------------------------------------------------
Test 4 [0000:0b:00.1]     : DMA
    Details               : Host -> PCIe -> FPGA write bandwidth = 11160.3 MB/s
                            Host <- PCIe <- FPGA read bandwidth = 11779.2 MB/s
    Test Status           : [PASSED]
-------------------------------------------------------------------------------
Test 5 [0000:0b:00.1]     : iops
    Details               : IOPS: 130861 (hello)
    Test Status           : [PASSED]
-------------------------------------------------------------------------------
Test 6 [0000:0b:00.1]     : Bandwidth kernel
    Details               : Maximum throughput: 50177 MB/s
    Test Status           : [PASSED]
-----------------------------------
```

If any of the previous steps do not properly run, reset the card by the following command or reboot the machine.

```bash
$sudo xbutil reset -d 0000:0b:00.1
```

If you passed the above checkups, you can select DPU kernels by running this command. In this example, I used 6PE 350Hz with DWC kernel for my application.

```bash
source /workspace/setup/vck5000/setup.sh DPUCVDX8H_6pe_dwc
```

## Backgrounds 

Vitis AI github
https://github.com/Xilinx/Vitis-AI

Vitis AI User Guide (UG1414)
https://docs.xilinx.com/r/en-US/ug1414-vitis-ai

Vitis AI Library User Guide (UG1354)
https://docs.xilinx.com/r/en-US/ug1354-xilinx-ai-sdk

(Advanced) VCK5000 Data Center Acceleration Development Kit Hardware Installation Guide (UG1531)
https://docs.xilinx.com/r/en-US/ug1531-vck5000-install
