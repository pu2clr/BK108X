# Examples

It is an AM (LW, MW and SW) and FM receiver based on the DSP BK1088E device. This sketch uses an Arduino Pro Mini, 3.3V (8MZ); a SPI TFT ST7735 1.8; an encoder and pushbuttons. 

## Wire up on Arduino UNO, Pro mini


  | Device name       | Device Pin / Description  |  Arduino Pin  |
  | ----------------  | --------------------      | ------------  |
  | Display TFT       |                           |               |
  |                   | RST (RESET)               |      8        |
  |                   | RS  or DC                 |      9        |
  |                   | CS  or SS                 |     10        |
  |                   | SDI                       |     11        |
  |                   | CLK                       |     13        |
  |     BK1088        |                           |               |
  |                   | SDIO/SDA (pin 6)          |     A4        |
  |                   | SCLK/CLK (pin 7)          |     A5        |
  |     Buttons       |                           |               |
  |                   | Band Up                   |      4        |
  |                   | Band Down                 |      5        |
  |                   | Volume Up                 |      6        |
  |                   | Volume Down               |      7        |
  |    Encoder        |                           |               |
  |                   | A                         |      2        |
  |                   | B                         |      3        |




