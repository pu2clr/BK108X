# PU2CLR BK108X Arduino Library

The BK1086 and BK1088 are DSP receivers from BAKEN. The BK1088 is a BROADCAST FM and AM (LW, MW and ) RECEIVER and BK1086 is a subset of the BK1088 (it does not have LW and SW acording to the Datasheet).  

This project is about an Arduino Library to control the BK1086 and BK1088 devices. 

__This library is under construction...__

![Under construction](extras/images/under_construction.png)


This library can be freely distributed using the MIT Free Software model. 

[Copyright (c) 2019 Ricardo Lima Caratti](https://pu2clr.github.io/bk108X/#mit-license). 

Contact: __pu2clr@gmail.com__.


## Contents

1. [Preface](https://pu2clr.github.io/TEF668X#preface)
2. [Library Installation]()
3. [API Documentation]()
4. [Schematic]()
5. [Sketch examples]()



## Preface 

At the end of 2019 I started some Arduino library development projects to control DSP receivers. The first implementation was the Arduino library for the SI4844 and then for the ICx Si473X family, all manufactured by Silicon Labs. Given the acceptance and enthusiasm of various radio listeners, experimenters and hobbyists, around the use of the implementations of these libraries, I decided to expand the number of libraries for the platform and also developed the libraries for the AKC695X, KT0915, SI470X and RDA5807. When I started reading about the BK1086/88 Datasheet I notice it is also very apropriated for radio listener and hobbyists. Unlik the other devices previously mentioned, I found very few documentation about it. So, it encouraged me to build as much information as possible about this BEKEN family of devices.

This library is based on the BEKEN manual “BK1086/88 - BROADCAST AM/FM/SW/LW RADIO RECEIVER” and experiments made by me during the developemnt process. 

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



## Library Installation





## API Documentation





## Schematic







## Sketch examples





## Main features

1. Worldwide 64~108 MHz FM band support;
2. Worldwide 520~1710kHz AM band support;
3. SW band support(2.3-21.85MHz, BK1088 only);
4. Automatic gain control(AGC);
5. Automatic frequency control(AFC);
6. Digital FM stereo decoder;
7. Automatic FM stereo/mono blend;
8. Automatic noise suppression;
9. 50us/75us de-emphasis;
10. RDS/RBDS decoder;
11. 2.4 ~ 5.5 V supply voltage;
12. Wide range reference clock support;
13. 32.768KHz crystal oscillator.










