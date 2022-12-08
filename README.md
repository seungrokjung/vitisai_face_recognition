# Building Face Recognition system from AMD's Vitis-AI
This guide explains how to build a face recognition application from Vitis-AI library, which is AMD’s neural network SW stack for data-center AI Accelerators. This application was built on AMD's VCK5000 AI Accelerator but compatible with other AMD's Alveo AI Accelerators, U50LV and U55C. 
More information about the cards can be found in this link.  
https://www.xilinx.com/products/boards-and-kits/alveo.html 
If you don’t have a card, you can also try VMAccel cloud instance.  
https://www.vmaccel.com/

**Face Recognition on AMD's AI Accelerator: Press play button**

![Alt Text](./images/fr_tomcruise_sample.gif)

## Table of Contents
- [Building Face Recognition system from Vitis-AI](#overview)
  - [Table Of Contents](#table-of-contents)
  - [Introduction](#introduction)
    - [1. Model overview](#model-overview)
    - [2. Data set overview](#data-set-overview)
  - [Implementation steps](#implementation-steps)
    - [Step 1](#step-1)
    - [Step 2](#step-2)
    - [Step 3](#step-3)
    - [Step 4](#step-4)
    - [Step 5](#step-5)
    - [Step 6](#step-6)
  - [Additional information about AMD's VCK5000 card setup in the host machine](#Additional information)
  - [References](#references)


## Introduction

### 1. Model overview
Face recognition can be divided into two steps. 
Face detection where face candidates are selected from the input image. Outputs of face detection are the probabilities and bounding boxes of face candidates. In this example, densbox model is used as it is provided in the Vitis-AI model zoo. The model is already quantized into INT8 data type and compiled for AMD’s AI Accelerator cards. 
https://github.com/Xilinx/Vitis-AI/blob/master/model_zoo/model-list/cf_densebox_wider_360_640_1.11G_2.5/model.yaml
Face feature extraction 512 face features from the cropped face image from the face detector. Output of the face feature extraction is the probability of the class. For example, when the model classifier (from 512 features to #classes) is trained to differentiate among 10 persons, the feature extraction of sample input image gives probability of similarities among 10 persons. InceptionResnetV1 model is used. This model is implemented in pytorch and it needs quantization and compilation to be run on AMD’s AI Accelerator cards. 
https://github.com/timesler/facenet-pytorch

### 2. Data set overview
Public face data set is used for this implementation. But you can also use your own face data. 17 classes of celebrity face data in the following link are used and each class has 100 faces from different angles, orientation and lighting. 
https://www.kaggle.com/datasets/vishesh1412/celebrity-face-image-dataset

## Implementation steps
### Step 1
Fine-tune Face feature extraction model (InceptionResnetV1) to the 17 classes celebrity face data set. Please refer to the finetune script in the facenet-pytorch git ( https://github.com/timesler/facenet-pytorch/blob/master/examples/finetune.ipynb).
After fine-tuning the InceptionResnetV1 model to celebrity face data set, make sure to save the model. 
```bash
torch.save(resnet.state_dict(), "./InceptionResnetV1_classify_state_ITR1.pth")
```
### Step 2
Download Vitis-AI and setup AI Accelerator cards (VCK5000 is used in this example)
Download Vitis-AI
If this step fails, please refer to this section 
  - [Additional information about AMD's VCK5000 card setup in the host machine](#Additional information)

```bash
git clone --recurse-submodules https://github.com/Xilinx/Vitis-AI  
cd Vitis-AI/setup/vck5000/
source ./install.sh
```
### Step 3
Launch docker (xilinx/vitis-ai-cpu:latest). If you have GPUs in your machine, you can also select (xilinx/vitis-ai-gpu:latest). The GPUs can be used to quantize models. 
```bash
cd Vitis-AI
./docker_run.sh xilinx/vitis-ai-cpu:latest
```
### Step 4
Invoke pytorch conda environment to quantize and compile the face feature extraction model  (InceptionResnetV1).
 
Before quantization, fp32 InceptionResnetV1 has following accuracies:
loss: 0.0100238
top-1 / top-5 accuracy: 90.6563 / 99.1101
After quantization, INT8 InceptionResnetV1 recovers accuracies as follows:
loss: 0.0101847
top-1 / top-5 accuracy: 90.6563 / 99.2214
```bash
conda activate vitis-ai-pytorch
cd /workspace/src/Vitis-AI-Quantizer/vai_q_pytorch/example
mkdir ../data_celeb_cropped
mkdir ../data_celeb_cropped/val
cp -r {data_celeb_cropped folder generated from step 1.} ../data_celeb_cropped/val/.
pip install facenet_pytorch
cp {./quantization/InceptionResnetV1_classify_quant.py in this git} ./.
python InceptionResnetV1_classify_quant.py --quant_mode float
python InceptionResnetV1_classify_quant.py --quant_mode calib --subset_len 200
python InceptionResnetV1_classify_quant.py --quant_mode test
python InceptionResnetV1_classify_quant.py --quant_mode test  --subset_len 1 --batch_size 1 --deploy
vai_c_xir -x quantize_result/InceptionResnetV1_int.xmodel -o quantize_result -n InceptionResnetV1 -a /opt/vitis_ai/compiler/arch/DPUCVDX8H/VCK50006PEDWC/arch.json
```
After Step 4., following files are generated:
```bash
./quantize_result/InceptionResnetV1.py
./quantize_result/InceptionResnetV1.xmodel
./quantize_result/InceptionResnetV1_int.onnx
./quantize_result/InceptionResnetV1_int.xmodel
./quantize_result/bias_corr.pth
./quantize_result/md5sum.txt
./quantize_result/meta.json
./quantize_result/quant_info.json                
```
### Step 5
Select a DPU kernel (DPUCVDX8H_6pe_dwc) to run the application. You can also select other types of kernels. Please refer to the following description for further details.
https://docs.xilinx.com/r/en-US/ug1354-xilinx-ai-sdk/VCK5000-Versal-Development-Card-for-AI-Inference
```bash
source /workspace/setup/vck5000/setup.sh DPUCVDX8H_6pe_dwc
```
### Step 6
Prepare face recognition application and run image/mp4 face recognition demo.
Face detection (densebox_640_320) and face feature extraction (InceptionResnetV1) models need to be setup. 
Use InceptionResnetV1.prototxt, build.bash, fr.cpp and fr_mp4.cpp code in this git. 
 
```bash
cd /workspace/examples/Vitis-AI-Library/samples/facedetect/
cp -r /workspace/src/Vitis-AI-Quantizer/vai_q_pytorch/example/quantize_result InceptionResnetV1
cp {./example/InceptionResnetV1.prototxt in this git} ./InceptionResnetV1/.
sudo mkdir /usr/share/vitis_ai_library/models
sudo cp -rf InceptionResnetV1 /usr/share/vitis_ai_library/models
wget https://www.xilinx.com/bin/public/openDownload?filename=densebox_640_360-vck5000-DPUCVDX8H-6pe-aieDWC-r2.5.0.tar.gz
tar xvf openDownload\?filename\=densebox_640_360-vck5000-DPUCVDX8H-6pe-aieDWC-r2.5.0.tar.gz
sudo cp -rf densebox_640_360 /usr/share/vitis_ai_library/models/
cp {./example/build.bash in this git} ./.
cp {./example/fr.cpp in this git} ./.
cp {./example/fr_mp4.cpp in this git} ./.
bash -x build.sh
cp {images/multi_face.jpg in this git} ./. 
./fr images/multi_face.jpg 2.5
./fr_mp4 {any *.mp4 file} 2.5
```
Given the classification confidence of 2.5, the result will look like this. Note that one of the faces, Gal Godot, which is not in the training data set is classified as UNIDENTIFIED. 
```bash
class: AngelinaJolie, score: 6.375
class: TomCruise, score: 6.25
class: RobertDowneyJr, score: 9
class: UNIDENTIFIED, score: -1
class: BradPitt, score: 3.125
class: JohnnyDepp, score: 2.75
```

<div align=center><img width=800 src ="multi_face.png"/></div>
<div align=center> Fig 1. sample input image </div>
<br/><br/>

<div align=center><img width=800 src ="result_multi_face.png"/></div>
<div align=center> Fig 2. sample feature recognition result </div>
<br/><br/>

## Additional information about AMD's VCK5000 card setup in the host machine
Make sure you can pass these commands after you setup card. 

AI Accelerator status check
Check management and user BDFs(Bus:Device:Function). In this example, they are 0000:0b:00.0 and 0000:0b:00.1, respectively.  Make sure Kernel driver in use are xclmgmt and xocl.  
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

## References

Vitis AI github
https://github.com/Xilinx/Vitis-AI

Vitis AI User Guide (UG1414)
https://docs.xilinx.com/r/en-US/ug1414-vitis-ai

Vitis AI Library User Guide (UG1354)
https://docs.xilinx.com/r/en-US/ug1354-xilinx-ai-sdk

(Advanced) VCK5000 Data Center Acceleration Development Kit Hardware Installation Guide (UG1531)
https://docs.xilinx.com/r/en-US/ug1531-vck5000-install
