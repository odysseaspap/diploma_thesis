# Harris Corner Detector Implementation in Myriad 2 by Intel - Movidius

Movidius website: https://www.movidius.com/myriad2

## Supported Platform
---
Myriad2 - This example works on Myriad2: ma2100, ma2150, ma2450 silicon and 
ma2100, ma2150, ma2450 simulator

## Overview
---
Starts RTEMS, Loads input image and runs the algorithm in parallel on 12 shaves.

## Software description
--- 
Harris Corner Detection Implementation in Myriad 2
The algorithm runs in parallel in 12 VLIW processors (SHAVEs)
We start by the Leon processor which handles the image size 
and allocates an image slice to each SHAVE. We take special care
in order to apply the correct padding to each image slice and not 
lose valuable features.

There are various modifications that we tested regarding the number
of SHAVEs used as well as hardware specific features. Each of these 
had a specific effect on the overall efiiciency and the final results
were presented in the Thesis.
        
## Hardware needed
--- 
Myriad2 -This software should run on MV182 board.

## Build
---
Please type "make help" if you want to learn available targets.

!!!Before cross-compiling make sure you do a "make clean"!!!

Myriad2 - To build the project please type:
     - "make clean"
     - "make all MV_SOC_REV={Myriad_version}"

Where {Myriad_version} may be ma2100, ma2150 or ma2450.
The default Myriad revision in this project is ma2150 so it is not necessary 
to specify the MV_SOC_REV in the terminal.

## Setup
---
Myriad2 simulator - To run the application:
    - open terminal and type "make start_simulator MV_SOC_REV={Myriad_version}"
    - open another terminal and type "make run MV_SOC_REV={Myriad_version}"
Myriad2 silicon - To run the application:
    - open terminal and type "make start_server"
    - open another terminal and type "make run MV_SOC_REV={Myriad_version}"

Where {Myriad_version} may be ma2100, ma2150 or ma2450.
The default Myriad revision in this project is ma2150 so it is not necessary 
to specify the MV_SOC_REV in the terminal.

## Expected output
--- 
The results  consist in printf seen using the debugger.



