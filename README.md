# PU2CLR BK108X Arduino Library

The BK1086 and BK1088 are DSP receivers from BAKEN. The BK1088 is a BROADCAST FM and AM (LW, MW and ) RECEIVER and BK1086 is a subset of the BK1088 (it does not have LW and SW acording to the Datasheet).  

This project is about an Arduino Library to control the BK1086 and BK1088 devices. It was based on "BK1086/88E BROADCAST AM/FM/SW/LW RADIO RECEIVER; Rev.1.3" documentation from BEKEN Corporation.

__This library is under construction...__

![Under construction](extras/images/under_construction.png)


This library can be freely distributed using the MIT Free Software model. 

[Copyright (c) 2019 Ricardo Lima Caratti](https://pu2clr.github.io/bk108X/#mit-license). 

Contact: __pu2clr@gmail.com__.


## Contents

1. [Preface](https://pu2clr.github.io/BK108X#preface)
2. [BK1086/88E features](https://pu2clr.github.io/BK108X#bk108688-features)
3. [Library Installation](https://pu2clr.github.io/BK108X#library-installation)
4. [API Documentation](https://pu2clr.github.io/BK108X/extras/docs/html/index.html)
5. [Functional Block Diagram](https://pu2clr.github.io/BK108X#functional-block-diagram)
6. [BK1086/88E TSSOP20 pinout](https://pu2clr.github.io/BK108X#bk108688e-tssop20--pinout)
7. [Schematic](https://pu2clr.github.io/BK108X#schematic)
8. [Sketch examples](https://github.com/pu2clr/BK108X/tree/master/examples)
9. [References](https://pu2clr.github.io/BK108X#references)



## Preface 

At the end of 2019 I started some Arduino library development projects to control DSP receivers. The first implementation was the Arduino library for the SI4844 and then for the ICx Si473X family, all manufactured by Silicon Labs. Given the acceptance and enthusiasm of various radio listeners, experimenters and hobbyists, around the use of the implementations of these libraries, I decided to expand the number of Arduino libraries for the Arduino IDE platform and also developed the libraries for the AKC695X, KT0915, SI470X and RDA5807. When I started reading about the BK1086/88 Datasheet I notice it is also very apropriated for radio listener and hobbyists. Unlike the other devices previously mentioned, I found very few documentation about it at first. So, it encouraged me to build as much information as possible about this BEKEN family of devices. I requested some information about the BK1086/88E from BEKEN Corporation and I was promptly answered. I would like to thank the BEKEN Corporation fot its attention. 

This library is based on the BEKEN manual “BK1086/88 - BROADCAST AM/FM/SW/LW RADIO RECEIVER REV1.3” provided by BEKEN Corporation and experiments made by me during the development process. 

__This library is under construction...__


### See also

1. [PU2CLR Si4735 Library for Arduino](https://pu2clr.github.io/SI4735/). This library was built based on “Si47XX PROGRAMMING GUIDE; AN332” and it has support to FM, AM and SSB modes (LW, MW and SW). It also can be used on all members of the SI47XX family respecting, of course, the features available for each IC version; 
2. [PU2CLR SI4844 Arduino Library](https://github.com/pu2clr/SI4844). This is an Arduino library for the SI4844, BROADCAST ANALOG TUNING DIGITAL DISPLAY AM/FM/SW RADIO RECEIVER,  IC from Silicon Labs.  It is available on Arduino IDE. This library is intended to provide an easier interface for controlling the SI4844.
3. [PU2CLR AKC695X Arduino Library](https://pu2clr.github.io/AKC695X/). The AKC695X is a family of IC DSP receiver from AKC technology. The AKC6955 and AKC6959sx support AM and FM modes. On AM mode the AKC6955 and AKC6959sx work on LW, MW and SW. On FM mode they work from 64MHz to 222MHz.
4. [PU2CLR KT0915 Arduino Library](https://pu2clr.github.io/KT0915/). The KT0915 is a full band AM (LW, MW and SW) and FM DSP receiver that can provide you a easy way to build a high quality radio with low cost. 
5. [PU2CLR RDA5807 Arduino Library](https://pu2clr.github.io/RDA5807/). The RDA5807 is a FM DSP integrated circuit receiver (50 to 115MHz) with low noise amplifier support. This device requires very few external components if compared with other similar devices. It also supports RDS/RBDS functionalities, direct auto gain control (AGC) and real time adaptive noise cancellation function.
6. [PU2CLR SI470X Arduino Library](https://pu2clr.github.io/SI470X/). It is a Silicon Labs device family that integrates the complete functionalities for FM receivers, including RDS (Si4703).



## MIT License 

Copyright (c) 2019 Ricardo Lima Caratti

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE ARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

<BR>


## Thanks

Mr. Peng Roy, from BEKEN Corporation, for providing me with the technical information on the BK1086/88.


## Your support is important

If you have suggestions to improve this project, please let me know. 



## BK1086/88 features


### Main features

1. Worldwide 64~108 MHz FM band support;
2. Worldwide 520~1710kHz AM band support;
3. LW band support(153-279kHz,BK1088E only)
4. SW band support(2.3-21.85MHz, BK1088E only);
5. Automatic gain control(AGC);
6. Automatic frequency control(AFC);
7. Digital FM stereo decoder;
8. Automatic FM stereo/mono blend;
9. Automatic noise suppression;
10. 50us/75us de-emphasis;
11. RDS/RBDS decoder;
12. 2.4 ~ 5.5 V supply voltage;
13. Wide range reference clock support;
14. 32.768KHz crystal oscillator.


### Useful informarion

* BK1086/88 can be controlled by an MCU through 2-wire I2C mode. 
* BK1086/88 has an internal automatic tuning technology, which can automatically adjust the internal variable capacitance value to make the resonant circuit in the best resonance state at the current operating frequency, thus greatly simplifying the front-end design;
* BK1086 only supports FM and medium wave, BK1088 supports short wave and long wave in addition to FM and medium wave;
* If the control mode is I2C mode, SEN pin can be connected high or left floating. MODE pin can Ground directly.
* There is already a 40K pull-up resistor inside the chip for the SCLK and SDIO. So, in general, no external pull-up resistor is needed. 

 

## Library Installation

__Under construction.....__



## Functional Block Diagram


![BK1086/88E Functional Description](https://github.com/pu2clr/BK108X/blob/master/extras/images/bk1088_functional_block_diagram.png)

__Source: "BK1086/88E BROADCAST AM/FM/SW/LW RADIO RECEIVER; Rev.1.3; page 3"__



## BK1086/88E TSSOP20  pinout


![BK1086/88E PINOUT](extras/images/BK1088_pinout.png)

__Source: "BK1086/88E BROADCAST AM/FM/SW/LW RADIO RECEIVER; Rev.1.3; page 23"__


| Pin Number | Name     | Description | 
| ---------- | -------- | ----------- | 
| 1          | GND      | Ground. Connect to ground plane on PCB |
| 2          | GND      | Ground. Connect to ground plane on PCB |
| 3          | FMI      | FM RF input | 
| 4          | RFGND    | RF ground | 
| 5          | AMI      | MW/SW/LW RF input |
| 6          | GND      | Ground. Connect to ground plane on PCB |
| 7          | SCLK     | Clock for Serial communications |
| 8          | SDIO     | Serial data input/output | 
| 9          | RCLK     | 32.768kHz - 38.4MHz external reference clock input/32.768KHz Oscillator input |
| 10         | VDD      | Power supply for digital and I/O | 
| 11         | ROUT     | Right audio output | 
| 12         | LOUT     | Left audio output |
| 13         | VA       | Power supply for analog |
| 14         | GPIO3    | General purpose output 3 | 
| 15         | GPIO2    | General purpose output 2 | 
| 16         | EXTLNA   | External LNA control | 
| 17         | RIN2     | Line in input (Right channel2) | 
| 18         | LIN2     | Line in input (Left channel2) | 
| 19         | RIN1     | Line in input (Right channel1) | 
| 20         | LIN1     | Line in input (Left channel1) |



## Schematic



### BEKEN Typical Application Schematic

The image below was extracted from "BK1086/88E BROADCAST AM/FM/SW/LW RADIO RECEIVER; Rev.1.3; page 25". It is a basic circuit suggested by BEKEN.

![BEKEN Typical Application Schematic](extras/images/basic_circuit_from_beken.png)


The schematic below is based on the BEKEN typical application schematic added to the Arduino controller. The front-end circuit has been modified to use regular antenna instead headphone antenna setup.  

The main porpuse of this circuit is to test the  BK108X Arduino Library. However, you may be surprised at the performance of this simple circuit due to the high features of the BK1086/88 device. Also, it is importante to say that this receiver does not intend to be a real radio for exigent listener. However, it is possible to start with it and then, if you wish, you can include some devices to the circuit to improve, for example,  its sensibility, output sound quality beyond other desired features.

__Schematic under construction....__

![Basic Schematic](extras/images/basic_schematic.png)



## References

* BK1086/88E - BROADCAST AM/FM/SW/LW RADIO RECEIVER; Rev 1.3 (Copyright©2012 by Beken Corporation);
* BK1086/88 Application Manual (In Chinese - Copyright©2012 by Beken Corporation);
* [BK1088ES](http://www.bekencorp.com/index/goods/detail/cid/36.html)
* [1201682 Radio Alarm Clock with Bluetooth Speaker / Bluetooth Alarm Clock Radio Schematics Circuit Diagram Shenzhen Great Power Enterprise](https://fccid.io/ZY9-1201682/Schematics/Circuit-Diagram-2803394);
* [AudioComm RAD-F630Z](http://nice.kaze.com/rad-f630z.html);
  














