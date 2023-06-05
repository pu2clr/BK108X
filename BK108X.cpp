/**
 * @mainpage BK108X Arduino Library implementation 
 * @details BK108X Arduino Library implementation. This is an Arduino library for the BK108X, BROADCAST RECEIVER.  
 * @details This is an Arduino library for the BK1086 and BK1088 DSP BROADCAST RECEIVER.<br>  
 * @details It works with I2C protocol and can provide an easier interface for controlling the BK1086/88 devices.<br>
 * @details This library is based on the Document: BEKEN - BK1086/88 - BROADCAST AM/FM/SW/LW RADIO RECEIVER Rev 1.3. 
 * 
 * __THIS LIBRARY IS UNDER CONSTRUCTION.....___
 * 
 * This library can be freely distributed using the MIT Free Software model.
 * Copyright (c) 2020 Ricardo Lima Caratti.  
 * Contact: pu2clr@gmail.com 
 */

#include <BK108X.h>
#include <Wire.h> // This library is not used to communicate wwith BK108X family device. It is used to check I2C bus. 

// This library implements its own I2C code to communicate with BK108X. 



/** 
 * @defgroup GA02 BEKEN I2C BUS 
 * @section GA02 I2C
 *
 * @brief About I2C Address on BK1086/88 and Arduino platform
 * 
 * The BK1086/88 Datasheet says that the I2C buss address is 0x80. However, the Wire (I2C) Arduino library does not find 
 * the device on 0x80. Actually the Arduino finds the device on 0x40.
 * This must be due to the internal conversion of the address from 8 bits to 7 bits. (0x80 = 0b10000000; 0x40 = 0b01000000)
 * After a few unsuccessful attempts at using the Arduino I2C library, I decided to write the necessary I2C routines to deal 
 * with BK1086/88 device. 
 * 
 * @see setI2C, i2cInit, i2cStart, i2cEndTransaction(), i2cAck, i2cNack, i2cReceiveAck, i2cWriteByte, i2cReadByte, writeRegister, readRegister
 * 
 * IMPORTANT: 
 * For stable communication, the rising edge time of SCLK should be less than 200ns.
 */

/**
 * @ingroup GA02
 * @brief Sets I2C bus address 
 * @details Useful if some release of BEKEN device is different of 0x80. 
 * 
 * @param i2c_addr 
 */
void BK108X::setI2C(uint8_t i2c_addr) {
    this->deviceAddress = i2c_addr;
}

/**
 * @ingroup GA02
 * @brief Sets the MCU pins connected to the I2C bus 
 * @details Configures the I2C bus for BK108X
 * 
 * @param pin_sdio SDA/SDIO MCU/Arduino pin
 * @param pin_sclk CLK/SCLK MCU/Arduino pin 
 */
void BK108X::i2cInit(int pin_sdio, int pin_sclk){
    this->pin_sdio = pin_sdio;
    this->pin_sclk = pin_sclk;  
}

/**
 * @ingroup GA02
 * @brief Starts the I2C bus transaction  
 */
void BK108X::i2cBeginTransaction()
{
    pinMode(this->pin_sdio, OUTPUT);
    pinMode(this->pin_sclk, OUTPUT);
    digitalWrite(this->pin_sdio, HIGH);
    digitalWrite(this->pin_sclk, HIGH);
    delayMicroseconds(1);

    digitalWrite(this->pin_sdio, LOW);
    delayMicroseconds(1);
    digitalWrite(this->pin_sclk, LOW);
    delayMicroseconds(1);
    digitalWrite(this->pin_sdio, HIGH);
}

/**
 * @ingroup GA02
 * @brief Finish the I2C bus  transaction
 */
void BK108X::i2cEndTransaction()
{
    pinMode(pin_sdio, OUTPUT);
    digitalWrite(this->pin_sdio, LOW);
    delayMicroseconds(1);

    digitalWrite(this->pin_sclk, HIGH);
    delayMicroseconds(1);

    digitalWrite(this->pin_sdio, HIGH);
    delayMicroseconds(1);
}

/**
 * @ingroup GA02
 * @brief Sends Acknowledge (ACK)
 * @details Each byte of data (including the address byte) have to be followed by one ACK bit from the receiver.
 * @details The ACK bit allows the receiver to communicate to the transmitter.
 * @see https://www.ti.com/lit/an/slva704/slva704.pdf
 */
void BK108X::i2cAck()
{
    pinMode(pin_sdio, OUTPUT);
    digitalWrite(this->pin_sclk, LOW);
    digitalWrite(this->pin_sdio, LOW);
    delayMicroseconds(1);
    digitalWrite(this->pin_sclk, HIGH);
    delayMicroseconds(1);
    digitalWrite(this->pin_sclk, LOW);
}

/**
 * @ingroup GA02
 * @brief Sends Not Acknowledge (ACK)
 * @see https://www.ti.com/lit/an/slva704/slva704.pdf
 */
void BK108X::i2cNack()
{
    pinMode(pin_sdio, OUTPUT);

    digitalWrite(this->pin_sclk, LOW);
    digitalWrite(this->pin_sdio, HIGH);
    delayMicroseconds(1);
    digitalWrite(this->pin_sclk, HIGH);
    delayMicroseconds(1);
    digitalWrite(this->pin_sclk, LOW);
}

/**
 * @ingroup GA02
 * @brief Gets Acknowledge (ACK)
 * @see https://www.ti.com/lit/an/slva704/slva704.pdf
 * @return ack value
 */
uint8_t BK108X::i2cReceiveAck()
{
    uint8_t ack;
    pinMode(pin_sdio, INPUT);
    delayMicroseconds(1);

    digitalWrite(this->pin_sclk, HIGH);
    delayMicroseconds(1);

    ack = digitalRead(this->pin_sdio);

    digitalWrite(this->pin_sclk, LOW);
    delayMicroseconds(1);

    return ack;
}

/**
 * @ingroup GA02
 * @brief Sends a Byte to the slave device
 * @param data to be sent to the slave device
 */
void BK108X::i2cWriteByte( uint8_t data)
{
    pinMode(pin_sdio, OUTPUT);
    delayMicroseconds(1);

    for (int i = 0; i < 8; i++) {

        digitalWrite(this->pin_sdio, (bool)(data & this->deviceAddress) );

        delayMicroseconds(1);
        digitalWrite(this->pin_sclk, HIGH);
        delayMicroseconds(1);
        digitalWrite(this->pin_sclk, LOW);
        data = data << 1;
    }
}

/**
 * @ingroup GA02
 * @brief Gets a Byte from the slave device
 * @return value read from the device
 */
uint8_t BK108X::i2cReadByte()
{
    uint8_t value = 0;

    pinMode(pin_sdio, INPUT);
    delayMicroseconds(1);

    for (int i = 0; i < 8; i++)
    {
        digitalWrite(this->pin_sclk, HIGH);
        value = value << 1;
        delayMicroseconds(1);
        if ( digitalRead(this->pin_sdio) ) 
            value = value | 1;
        digitalWrite(this->pin_sclk, LOW);
        delayMicroseconds(1);
    }

    return value;
}

/**
 * @ingroup GA02
 * @brief Sends an array of values to a BK108X given register
 * @param reg register to be written
 * @param value content to be stored into the register
 */
void BK108X::writeRegister(uint8_t reg, uint16_t value) {

    word16_to_bytes data;
    data.raw = value;
    
    this->i2cBeginTransaction();
    this->i2cWriteByte(this->deviceAddress);
    this->i2cReceiveAck();

    reg = reg << 1; // Converts address and sets to write operation

    this->i2cWriteByte(reg);
    this->i2cReceiveAck();

    this->i2cWriteByte(data.refined.highByte);
    this->i2cReceiveAck();
    this->i2cWriteByte(data.refined.lowByte);
    this->i2cReceiveAck();

    this->i2cEndTransaction();
}

/**
 * @ingroup GA02
 * @brief Gets an array of values from a BK108X given register
 * @param reg register to be read
 * @return register content
 */
uint16_t BK108X::readRegister(uint8_t reg) {

    word16_to_bytes data;

    this->i2cBeginTransaction();
    this->i2cWriteByte(this->deviceAddress);
    this->i2cReceiveAck();

    reg = (reg << 1) | 1;  // Converts address and sets to read operation

    this->i2cWriteByte(reg);
    this->i2cReceiveAck();

    data.refined.highByte = this->i2cReadByte();
    this->i2cAck();
    data.refined.lowByte = this->i2cReadByte();
    this->i2cNack();

    this->i2cEndTransaction();

    return data.raw;
}


/** 
 * @defgroup GA03 Basic Functions
 * @section GA03 Basic
 */

/**
 * @ingroup GA03
 * @brief Gets a givens current register content of the device
 * @see shadowRegisters;  
 * @param device register address
 * @return the register content (the shadowRegisters array has this content. So, you do not need to use it most of the cases)
 */
uint16_t BK108X::getRegister(uint8_t reg)
{
    uint16_t reg_content;
    reg_content = this->readRegister(reg);
    shadowRegisters[reg] = reg_content; // Syncs with the shadowRegisters
    return reg_content;                 // Optional
}

/**
 * @ingroup GA03
 * @brief   Sets a given value to the device registers 
 * @details For write operations, the device acknowledge is followed by an eight bit data word latched internally on rising edges of SCLK. The device acknowledges each byte of data written by driving SDIO low after the next falling SCLK edge, for 1 cycle.
 * @details An internal address counter automatically increments to allow continuous data byte writes, starting with the upper byte of register 02h, followed by the lower byte of register 02h, and onward until the lower byte of the last register is reached. The internal address counter then automatically wraps around to the upper byte of register 00h and proceeds from there until continuous writes end.
 * @details The registers from 0x2 to 0x07 are used to setup the device. This method writes the array  shadowRegisters, elements 8 to 14 (corresponding the registers 0x2 to 0x7 respectively)  into the device. See Device registers map  in BK108X.h file.
 * @details To implement this, a register maping was created to deal with each register structure. For each type of register, there is a reference to the array element. 
 *  
 * @see shadowRegisters; 
 * 
 * @param device register address
 */
void BK108X::setRegister(uint8_t reg, uint16_t value)
{
    this->writeRegister(reg, value);
    shadowRegisters[reg] = value;  // Syncs with the shadowRegisters
    delayMicroseconds(250);
}

/**
 * @brief Returns the Device Indentifiction
 * @return device id 
 */
uint16_t BK108X::getDeviceId()
{
    return getRegister(REG00);
}

/**
 * @brief Returns the Chip Indentifiction
 * 
 * @return IC id
 */
uint16_t BK108X::getChipId()
{
    return getRegister(REG01);
}

/**
 * @ingroup GA03
 * @brief  Gets the current status (register 0x0A) content
 * @details You can use this function when you need to get more than one status attribute at once.
 * @details See example code below.
 * @code 
 * bk_reg0a status = getStatus();
 * 
 * Serial.println(status.refined.ST);   // Stereo Signal Received Indicator 
 * Serial.println(status.refined.RSSI); // Current RSSI value 
 * Serial.println(status.refined.RDSR); // RDS Ready Indicator
 * 
 * @endcode
 * 
 * @see getRSSI
 * @see BK1086/88E - BROADCAST AM/FM/SW/LW RADIO RECEIVER Rev 1.3; page 17. 
 * 
 * @return  bk_reg0a data type status register (Register 0Ah. Status2)
 */
bk_reg0a BK108X::getStatus()
{
    bk_reg0a tmp;
    tmp.raw = getRegister(REG0A); // update the reg0a shadow register
    return tmp;
}


/**
 * @ingroup GA03
 * @brief   Wait STC (Seek/Tune Complete) status becomes 0
 * @details Should be used before processing Tune or Seek.
 * @details The STC bit being cleared indicates that the TUNE or SEEK bits may be set again to start another tune or seek operation. Do not set the TUNE or SEEK bits until the BK108X clears the STC bit. 
 */
void BK108X::waitAndFinishTune()
{
    while ( reg0a->refined.STC == 0) {
        delay(10);
        getRegister(REG0A);
    }

    reg03->refined.TUNE = 0;
    setRegister(REG03, reg03->raw);
    // delay(40);
}

/**
 * @ingroup GA03
 * @brief Resets the device
 */
void BK108X::reset()
{
    reg02->refined.DISABLE = 1;
    reg02->refined.ENABLE = 0;
    setRegister(REG02, reg02->raw);
    reg02->refined.DISABLE = 0;
    reg02->refined.ENABLE = 1;
    setRegister(REG02, reg02->raw);
}

/**
 * @ingroup GA03
 * @brief Powers the receiver on 
 * @details Starts the receiver and set default configurations suggested by the BELEN
 * @see BEKEN - BK1086/88 - BROADCAST AM/FM/SW/LW RADIO RECEIVER Rev 1.3; pages 12-21
 * @see setup
 */
void BK108X::powerUp()
{
    // Starts the mains register with default values suggested by BEKEN.

    reg02->raw = 0x0280;            // Sets to 0 all attributes of the register 0x02 (Power Configuration)
    reg02->refined.DISABLE = 0;     // Force stereo
    reg02->refined.ENABLE = 1;      // Power the receiver UP (DISABLE has to be 0)
    setRegister(REG02,reg02->raw);  // Stores the register 0x02

    setRegister(REG03, 0x0000);     // Sets to 0 all attributes of the register 0x03 (Channel)

    setRegister(REG04, 0x60D4);     // 0b0110000011010100
    setRegister(REG05, 0x37CF);     // 0b0011011111001111

    
    reg06->raw = 0x086F;            // Sets to the default value - 0b0000100001101111 -> CLKSEL = 1
    reg06->refined.CLKSEL = this->oscillatorType;  // Sets to the clock type selected by the user
    setRegister(REG06, reg06->raw);

    setRegister(REG07, 0x0101); // 0b0000000100000001
    setRegister(REG08, 0xAC90); // 0b1010110010010000

    setRegister(REG10, 0x7B11); // 0b0111101100010001
    setRegister(REG11, 0x004A); // 0b0000000001001010
    setRegister(REG12, 0x4000); // 0b0100000000000000
    setRegister(REG13, 0x3E00); // 0b0011111000000000
    setRegister(REG14, 0xC29A); // 0b1100001010011010
    setRegister(REG15, 0x79F8); // 0b0111100111111000
    setRegister(REG16, 0x4012); // 0b0100000000010010

    // setRegister(REG17, 0x0040); // 0b0000000001000000
    setRegister(REG17, 0x0800); // 0b0000100000000000
    setRegister(REG18, 0x341C); // 0b0011010000011100
    setRegister(REG19, 0x0080); // 0b0000000010000000
    setRegister(REG1A, 0x0000); // 0
    setRegister(REG1B, 0x4CA2); // 0b0100110010100010

    // setRegister(REG1C, 0x8820); // 0b1000100000100000
    setRegister(REG1C, 0); // 0b1000100000100000
    setRegister(REG1D, 0x0200); // 0b0000001000000000  ->  512

    delay(250);
}

/**
 * @ingroup GA03
 * @brief Powers the receiver off
 */
void BK108X::powerDown()
{
    reg02->refined.DISABLE = 1;
    reg02->refined.ENABLE = 0;
    setRegister(REG02, reg02->raw);
    delay(100);
}

/**
 * @ingroup GA03
 * @brief Starts the device 
 * @details sets the reset pin, interrupt pins and oscillator type you are using in your project.
 * @details You have to inform at least two parameters: RESET pin and I2C SDA pin of your MCU
 * @param rdsInterruptPin  // optional. Sets the Interrupt Arduino pin used to RDS function control.
 * @param seekInterruptPin // optional. Sets the Arduino pin used to Seek function control. 
 * @param oscillator_type  // optional. Sets the Oscillator type used Crystal (default) or Ref. Clock. 
 */
void BK108X::setup(int sda_pin, int sclk_pin, int rdsInterruptPin, int seekInterruptPin, uint8_t oscillator_type)
{
    // Configures BEKEN I2C bus 
    this->i2cInit(sda_pin, sclk_pin);

    if (rdsInterruptPin >= 0)
        this->rdsInterruptPin = rdsInterruptPin;
    if (seekInterruptPin >= 0)
        this->seekInterruptPin = seekInterruptPin;

    this->oscillatorType = oscillator_type;

    powerUp();

}

/**
 * @ingroup GA03
 * @brief Sets the receiver to FM mode
 * @details Configures the receiver on FM mode; Also sets the band limits, defaul frequency and step.
 * 
 * @param minimum_frequency  minimum frequency for the band
 * @param maximum_frequency  maximum frequency for the band
 * @param default_frequency  default freuency
 * @param step  increment and decrement frequency step in KHz (default 10 * 10KHz)
 */
void BK108X::setFM(uint16_t minimum_frequency, uint16_t maximum_frequency, uint16_t default_frequency, uint16_t step)
{
    this->currentStep = step;
    this->currentFrequency = default_frequency;
    this->minimumFrequency = minimum_frequency;
    this->maximumFrequency = maximum_frequency;
    this->currentMode = MODE_FM;

    reg07->refined.MODE = MODE_FM;
    setRegister(REG07, reg07->raw);
    delay(50);
    // Sets BAND, SPACE and other parameters
    this->currentFMBand =  reg05->refined.BAND = 0;
    this->currentFMSpace = reg05->refined.SPACE = 2;
    setRegister(REG05, reg05->raw);
    setFrequency(default_frequency);
};

/**
 * @ingroup GA03
 * @brief Sets the receiver to AM mode
 * @details Configures the receiver on AM mode; Also sets the band limits, defaul frequency and step.
 * 
 * @param minimum_frequency  minimum frequency for the band
 * @param maximum_frequency  maximum frequency for the band
 * @param default_frequency  default freuency
 * @param step  increment and decrement frequency step
 */
void BK108X::setAM(uint16_t minimum_frequency, uint16_t maximum_frequency, uint16_t default_frequency, uint16_t step, uint16_t am_space)
{
    this->currentStep = step;
    this->currentFrequency = default_frequency;
    this->minimumFrequency = minimum_frequency;
    this->maximumFrequency = maximum_frequency;

    this->currentMode =  reg07->refined.MODE = MODE_AM;
    setRegister(REG07, reg07->raw);
    delay(50);
    // Sets BAND, SPACE and other parameters

    if (minimum_frequency < 520 )
        this->currentAMBand = reg05->refined.BAND = 0;  // LW
    else if (minimum_frequency < 1800)
        this->currentAMBand = reg05->refined.BAND = 1;  // MW
    else
        this->currentAMBand = reg05->refined.BAND = 2;  // SW

    this->currentAMSpace = reg05->refined.SPACE = am_space;    // Space default value 0 (0=1KHz; 1 = 5KHz; 2=9KHz; 3 = 10KHz)

    setRegister(REG05, reg05->raw);
    this->setFrequency(default_frequency);
}


/**
 * @ingroup GA03
 * @brief Sets the channel 
 * @param channel 
 */
void BK108X::setChannel(uint16_t channel)
{
    reg02->refined.SEEK = 0;
    setRegister(REG02,reg02->raw);

    reg03->refined.TUNE = 1;
    reg03->refined.CHAN = channel;

    setRegister(REG03,reg03->raw);
    // delay(50);
    waitAndFinishTune();

    this->currentChannel = channel;
}

/**
 * @ingroup GA03
 * @brief Sets the FM frequency 
 * @details ....
 * @param frequency  
 */
void BK108X::setFrequency(uint16_t frequency)
{
    uint16_t channel;

    if (this->currentMode == MODE_FM) {
        channel = (frequency - this->fmStartBand[this->currentFMBand]) /  this->fmSpace[this->currentFMSpace];
    }
    else {
        channel = (frequency - this->amStartBand[this->currentAMBand]) / this->amSpace[this->currentAMSpace];
    }

    this->setChannel(channel);
}

/**
 * @ingroup GA03
 * @brief Increments the current frequency
 * @details The increment uses the band space as step. 
 */
void BK108X::setFrequencyUp()
{
    this->currentFrequency += this->currentStep;

    if (this->currentFrequency > this->maximumFrequency ) 
        this->currentFrequency = this->minimumFrequency;

    setFrequency(this->currentFrequency);
}

/**
 * @ingroup GA03
 * @brief Decrements the current frequency
 * @details The drecrement uses the band space as step.
 */
void BK108X::setFrequencyDown()
{
    this->currentFrequency -= this->currentStep;

    if (this->currentFrequency < this->minimumFrequency)
        this->currentFrequency = this->maximumFrequency;

    setFrequency(this->currentFrequency);
}

/**
 * @ingroup GA03
 * @brief Gets the current frequency. 
 * @return uint16_t 
 */
uint16_t BK108X::getFrequency()
{
    return this->currentFrequency;
}

/**
 * @ingroup GA03
 * @brief Gets the current channel. 
 * @return uint16_t 
 */
uint16_t BK108X::getChannel()
{
    return this->currentChannel;
}

/**
 * @ingroup GA03
 * @brief Gets the current channel stored in register 0x0B
 * @details This method is useful to query the current channel during the seek operations. 
 * @return uint16_t 
 */
uint16_t BK108X::getRealChannel()
{
    getRegister(REG0B);
    return reg0b->refined.READCHAN;
}

/**
 * @ingroup GA03
 * @brief Gets the frequency based on READCHAN register (0x0B)
 * @details Unlike getFrequency method, this method queries the device. 
 * 
 * @return uint16_t 
 */
uint16_t BK108X::getRealFrequency()
{
    if (currentMode == MODE_AM) {
        return getRealChannel() * this->amSpace[this->currentAMSpace] + this->amStartBand[this->currentAMBand];
    } else {
        return getRealChannel() * this->fmSpace[this->currentFMSpace] + this->fmStartBand[this->currentFMBand];
    }  
}



/**
 * @todo it is not working properly
 * @ingroup GA03
 * @brief Seeks a station via Software 
 * @details Seeks a station up or down.
 * @details Seek up or down a station and call a function defined by the user to show the frequency during the seek process. 
 * @details Seek begins at the current channel, and goes in the direction specified with the SEEKUP bit. Seek operation stops when a channel is qualified as valid according to the seek parameters, the entire band has been searched (SKMODE = 0), or the upper or lower band limit has been reached (SKMODE = 1).
 * @details The STC bit is set high when the seek operation completes and/or the SF/BL bit is set high if the seek operation was unable to find a channel qualified as valid according to the seek parameters. The STC and SF/BL bits must be set low by setting the SEEK bit low before the next seek or tune may begin.
 * @details Seek performance for 50 kHz channel spacing varies according to RCLK tolerance. Silicon Laboratories recommends ±50 ppm RCLK crystal tolerance for 50 kHz seek performance.
 * @details A seek operation may be aborted by setting SEEK = 0.
 * @details It is important to say you have to implement a show frequency function. This function have to get the frequency via getFrequency function.  
 * @details Example:
 * @code
 * 
 * BK108X rx;
 * 
 * void showFrequency() {
 *    uint16_t freq = rx.getFrequency();
 *    Serial.print(freq); 
 *    Serial.println("MHz ");
 * }
 * 
 * void loop() {
 *  .
 *  .
 *      rx.seek(BK_SEEK_WRAP, BK_SEEK_UP, showFrequency); // Seek Up
 *  .
 *  .
 * }
 * @endcode
 * @param seek_mode  Seek Mode; 0 = Wrap at the upper or lower band limit and continue seeking (default); 1 = Stop seeking at the upper or lower band limit.
 * @param direction  Seek Direction; 0 = Seek down (default); 1 = Seek up.
 * @param showFunc  function that you have to implement to show the frequency during the seeking process. Set NULL if you do not want to show the progress. 
 */
void BK108X::seekSoftware(uint8_t seek_mode, uint8_t direction, void (*showFunc)())
{
    long max_time = millis();

    // reg06->refined.SKSNR = 9;
    // setRegister(REG06,reg06->raw);

    reg03->refined.TUNE = 0;
    setRegister(REG03, reg03->raw);
    delay(50);

    do
    {
        reg02->refined.SKMODE = seek_mode;
        reg02->refined.SEEKUP = direction;
        reg02->refined.SEEK = 1;
        setRegister(REG02, reg02->raw);
        delay(50);
        if (showFunc != NULL)
        {
            this->currentFrequency = getRealFrequency();
            showFunc();                                    
        }
        getRegister(REG0A);
    } while ( (!reg0a->refined.STC && reg0a->refined.SF_BL)  && (millis() - max_time) < MAX_SEEK_TIME);

    reg02->refined.SEEK = 0;
    reg03->refined.TUNE = 0;
    setRegister(REG02, reg02->raw);
    setRegister(REG03, reg03->raw);

    this->currentFrequency = getRealFrequency();
}

/**
 * @todo make it work. 
 * @ingroup GA03
 * @brief Seeks a station via hardware functionality 
 * 
 * @param seek_mode  Seek Mode; 0 = Wrap at the upper or lower band limit and continue seeking (default); 1 = Stop seeking at the upper or lower band limit.
 * @param direction  Seek Direction; 0 = Seek down (default); 1 = Seek up.
 */
void BK108X::seekHardware(uint8_t seek_mode, uint8_t direction) {

    reg03->refined.TUNE = 0;
    setRegister(REG03, reg03->raw);

    reg02->refined.SKMODE = seek_mode;
    reg02->refined.SEEKUP = direction;
    reg02->refined.SKAFCRL = 1;


    do {
        reg02->refined.SEEK = 1;
        setRegister(REG02, reg02->raw);
        delay(40);
        while (reg0a->refined.STC == 0 )
        {
            delay(10);
            getRegister(REG0A);
        }
    } while ( reg0a->refined.SF_BL != 0 );

    reg02->refined.SEEK = 0;
    setRegister(REG02, reg02->raw);
    delay(50);

    this->setChannel(this->getRealChannel());
    this->currentFrequency = getRealFrequency();
}

/**
 * @todo make it work. 
 * @ingroup GA03
 * @brief Sets RSSI and SNR Seek Threshold
 * @param  rssiValue between 0 and 127
 * @param  snrValue between 0 and 127
 */
void BK108X::setSeekThreshold(uint8_t rssiValue, uint8_t snrValue)
{
    reg05->refined.SEEKTH = rssiValue;
    setRegister(REG05,reg05->raw);

    reg06->refined.SKSNR = snrValue;
    setRegister(REG06,reg06->raw);
}

/**
 * @ingroup GA03
 * @brief Sets the current band for AM or FM  
 * @details Configures the band by setting the Register 05h. System Configuration2
 * 
 * | Band value |  AM / KHz      |  FM / MHz           | 
 * | ---------- | -------------- | ------------------- | 
 * |    0       | LW - 153~279   | FULL - 64~108       |
 * |    1       | MW - 520~1710  | East Europe 64~76   | 
 * |    2       | SW - 2.3~21.85 | Japan 76~91         | 
 * |    3       | MW - 522~1710  | Europe 87~108       | 
 * 
 * @see BK1086/88E - BROADCAST AM/FM/SW/LW RADIO RECEIVER Rev 1.3; page 15
 * @param band  the valid values are 0, 1, 2 and 3. See table above.
 */ 

void BK108X::setBand(uint8_t band)
{
    if (this->currentMode == MODE_AM ) 
        this->currentAMBand = band; 
    else
        this->currentFMBand = band;

    reg05->refined.BAND = band;
    setRegister(REG05,reg05->raw);
}

/**
 * @ingroup GA03
 * @brief Sets the Space channel  for AM or FM
 * 
 * | Band value |  AM      | FM        | 
 * | ---------- | ---------| --------- | 
 * |    0       |  1 KHz  |   10 KHz   |
 * |    1       |  5 KKz  |   50 KHz   | 
 * |    2       |  9 KHz  |  100 KHz   | 
 * |    3       | 10 KHz  |  200 KHz   | 
 * 
 * @see BK1086/88E - BROADCAST AM/FM/SW/LW RADIO RECEIVER Rev 1.3; page 15
 * @param space valid values 0,1,2 and 3. See table above.
 */
void BK108X::setSpace(uint8_t space)
{
    if (this->currentMode == MODE_AM ) 
        this->currentAMSpace = space;
    else 
        this->currentFMSpace = space;

    reg05->refined.SPACE = space;
    setRegister(REG05, reg05->raw);
}

/**
 * @ingroup GA03
 * @brief Gets the current Rssi
 * 
 * @return int 
 */
int BK108X::getRssi()
{
    getRegister(REG0A);
    return reg0a->refined.RSSI;
}

/**
 * @ingroup GA03
 * @brief Gets the current SNR
 * 
 * @return int  The SNR Value.( in dB)
 */
int BK108X::getSnr()
{
    getRegister(REG09);
    return reg09->refined.SNR;
}

/**
 * @ingroup GA03
 * @brief Sets the Softmute true or false
 * @details Enable or Disable Soft Mute resource.
 * @param value  TRUE or FALSE
 */
void BK108X::setSoftMute(bool value)
{
    reg02->refined.DSMUTE = !value;  // Soft mute TRUE/ENABLE means DSMUTE = 0. 
    setRegister(REG02,reg02->raw);
}

/**
 * @ingroup GA03
 * @brief Sets Softmute Attack/Recover Rate.
 * 
 * Soft mute Attack/Recover 
 * 
 * | Value | Description | 
 * | ----- | ----------- | 
 * | 0     | fastest |
 * | 1     | fast    | 
 * | 2     | slow    | 
 * | 3     | slowest | 
 * 
 * @param value  See table above. 
 */
void BK108X::setSoftMuteAttack(uint8_t value)
{
    reg06->refined.SMUTER = value;
    setRegister(REG06,reg06->raw);
}

/**
 * @ingroup GA03
 * @brief Sets  Softmute Attenuation.
 * 
 * Soft mute Attenuation.
 * | Value | Description | 
 * | ----- | ----------- | 
 * | 0     | fastest |
 * | 1     | fast    | 
 * | 2     | slow    | 
 * | 3     | slowest | 
 * @param value See table above
 */
void BK108X::setSoftMuteAttenuation(uint8_t value)
{
    reg06->refined.SMUTEA = value;
    setRegister(REG06, reg06->raw);
}

/**
 * @ingroup GA03
 * @brief Set the Mute Threshold based on RSSI and SNR
 * 
 * @see BK1086/88 - BROADCAST AM/FM/SW/LW RADIO RECEIVER Rev 1.3; page 19; Register 0x14 (Boot Configuration5)
 * @param rssi  The Mute Threshold Based on RSSI (default 26)
 * @param snr   The Mute Threshold Based on SNR  (default 5)
 */
void BK108X::setMuteThreshold(uint8_t rssi, uint8_t snr)
{
    reg14->refined.RSSIMTH = rssi;
    reg14->refined.SNRMTH = snr;
    setRegister(REG14,reg14->raw);
}

/**
 * @ingroup GA03
 * @brief Disable or Enable soft mute when seeking
 * 
 * @param value If true, enable mute during the seek;
 */
void BK108X::setSeekMute(bool value){
    reg14->refined.SKMUTE = value;
    setRegister(REG14, reg14->raw);
}

/**
 * @ingroup GA03
 * @brief Disable or Enable soft mute when AFCRL is high
 * 
 * @param value  If true, enable soft mute when AFCRL is high
 */
void BK108X::setAfcMute(bool value) {
    reg14->refined.AFCMUTE = value;
    setRegister(REG14, reg14->raw);
}



/**
 * @ingroup GA03
 * @brief Sets the Mute true or false
 * 
 * @param left left channel (TRUE = MUTE/ FALSE = UNMUTE)
 * @param left right channel (TRUE = MUTE / FALSE = UMUTE)
 */
void BK108X::setAudioMute(bool left, bool right)
{
    reg02->refined.MUTEL = left;
    reg02->refined.MUTER = right;
    setRegister(REG02, reg02->raw);    
}

/**
 * @ingroup GA03
 * @brief Sets the Mute true or false
 * 
 * @param value left and right channels (TRUE = MUTE/ FALSE = UNMUTE)
 */
void BK108X::setAudioMute(bool value)
{
    this->setAudioMute(value,value);
}

/**
 * @ingroup GA03
 * @brief Sets the Mono true or false (stereo)
 * @details if TRUE, force mono; else force stereo
 * @param value TRUE or FALSE
 */
void BK108X::setMono(bool value)
{
  reg02->refined.MONO = value;
  reg02->refined.STEREO = !value;
  setRegister(REG02,reg02->raw);  
}

/**
 * @ingroup GA03
 * @brief Checks stereo / mono status
 * 
 * @see getStatus
 * @param value TRUE if stereo
 */
bool BK108X::isStereo()
{
    getRegister(REG0A);
    return reg0a->refined.STEN;
}

/**
 * @ingroup GA03
 * @brief Sets the audio volume level
 * 
 * @param value  0 to 31 (if 0, mutes the audio)
 */
void BK108X::setVolume(uint8_t value)
{
    if ( value > 31) return;
    this->currentVolume = value;
    // reg05 is a shadow register and has the last value read or written from/to the internal device register
    reg05->refined.VOLUME = value; 

    setRegister(REG05,reg05->raw);
}

/**
 * @ingroup GA03
 * @brief Gets the current audio volume level
 * 
 * @return uint8_t  0 to 15
 */
uint8_t BK108X::getVolume()
{
    return this->currentVolume;
}

/**
 * @ingroup GA03
 * @brief Increments the audio volume
 * 
 */
void BK108X::setVolumeUp()
{
    if (this->currentVolume < 31)
    {
        this->currentVolume++;
        setVolume(this->currentVolume);
    }
}

/**
 * @ingroup GA03
 * @brief Decrements the audio volume
 * 
 */
void BK108X::setVolumeDown()
{
    if (this->currentVolume > 0)
    {
        this->currentVolume--;
        setVolume(this->currentVolume);
    }
}



/** 
 * @defgroup GA04 RDS Functions
 * @section GA04 RDS/RBDS
 */

/**
 * @ingroup GA04
 * @brief Gets the RDS registers information
 * @details Gets the value of the registers from 0x0A to 0x0F
 * @details This function also updates the value of shadowRegisters[0];
 * @return bk_reg0a 
 */
void BK108X::getRdsStatus()
{

}

/**
 * @ingroup GA04
 * @brief Sets the Rds Mode Standard or Verbose
 * 
 * @param rds_mode  0 = Standard (default); 1 = Verbose
 */
void BK108X::setRdsMode(uint8_t rds_mode)
{
    this->rds_mode = rds_mode;
}

/**
 * @ingroup GA04
 * @brief Sets the RDS operation 
 * @details Enable or Disable the RDS
 * 
 * @param true = turns the RDS ON; false  = turns the RDS OFF
 */
void BK108X::setRds(bool value)
{
    reg04->refined.RDSEN = value;
    setRegister(REG04,reg04->raw);

}


/**
 * @ingroup GA04
 * @brief Returns true if RDS Ready
 * @details Read address 0Ah and check the bit RDSR.
 * @details If in verbose mode, the BLERA bits indicate how many errors were corrected in block A. If BLERA indicates 6 or more errors, the data in RDSA should be discarded.
 * @details When using the polling method, it is best not to poll continuously. The data will appear in intervals of ~88 ms and the RDSR indicator will be available for at least 40 ms, so a polling rate of 40 ms or less should be sufficient.
 * @return true 
 * @return false 
 */
bool BK108X::getRdsReady()
{
    getRegister(REG0A);
    return reg0a->refined.RDSR;
};

/**
 * @ingroup GA04
 * 
 * @brief Returns the current Text Flag A/B  
 * 
 * @return uint8_t current Text Flag A/B  
 */
uint8_t BK108X::getRdsFlagAB(void)
{
    return 0;
}

/**
 * @ingroup GA04
 * @brief Return the group type 
 * 
 * @return uint16_t 
 */
uint16_t BK108X::getRdsGroupType()
{
    return 0;
}

/**
 * @ingroup GA04
 * 
 * @brief Gets the version code (extracted from the Block B)
 * @returns  0=A or 1=B
 */
uint8_t BK108X::getRdsVersionCode(void)
{
   return 0; 
}

/**  
 * @ingroup GA04  
 * @brief Returns the Program Type (extracted from the Block B)
 * @see https://en.wikipedia.org/wiki/Radio_Data_System
 * @return program type (an integer betwenn 0 and 31)
 */
uint8_t BK108X::getRdsProgramType(void)
{
    return 0;
}

/**
 * @todo to be implemented 
 * @ingroup GA04
 * 
 * @brief Process data received from group 2B
 * @param c  char array reference to the "group 2B" text 
 */
void BK108X::getNext2Block(char *c)
{
    this->rds_buffer2B[0] = *c;
    c = NULL;
}

/**
 * @todo to be implemented 
 * @ingroup GA04
 * 
 * @brief Process data received from group 2A
 * 
 * @param c  char array reference to the "group  2A" text 
 */
void BK108X::getNext4Block(char *c)
{
    this->rds_buffer2A[0] = *c;
    c = NULL;
}

/**
 * @ingroup GA04
 * 
 * @brief Gets the RDS Text when the message is of the Group Type 2 version A
 * @return char*  The string (char array) with the content (Text) received from group 2A 
 */
char *BK108X::getRdsText(void)
{
    return NULL;
}

/**
 * @ingroup GA04
 * @todo RDS Dynamic PS or Scrolling PS support
 * @brief Gets the station name and other messages. 
 * 
 * @return char* should return a string with the station name. 
 *         However, some stations send other kind of messages
 */
char *BK108X::getRdsText0A(void)
{
    return NULL;
}

/**
 * @ingroup @ingroup GA04
 * 
 * @brief Gets the Text processed for the 2A group
 * 
 * @return char* string with the Text of the group A2  
 */
char *BK108X::getRdsText2A(void)
{
    return NULL;
}

/**
 * @ingroup GA04
 * @brief Gets the Text processed for the 2B group
 * @return char* string with the Text of the group AB  
 */
char *BK108X::getRdsText2B(void)
{
    return NULL;
}

/**
 * @ingroup GA04 
 * @brief Gets the RDS time and date when the Group type is 4 
 * @return char* a string with hh:mm +/- offset
 */
char *BK108X::getRdsTime()
{
    return NULL;
}


/**
 * @ingroup GA04 
 * @brief Get the Rds Sync 
 * @details Returns true if RDS currently synchronized.
 * @return true or false
 */
bool BK108X::getRdsSync()
{
    return NULL;
}


/**
 * @ingroup G05 Check the I2C buss address
 * @brief Check the I2C bus address
 *
 * @param uint8_t address Array - this array will be populated with the I2C bus addresses found (minimum three elements)
 * @return 0 if no i2c device is found; -1 if error is found or n > 0, where n is the number of I2C bus address found
 */
int BK108X::checkI2C(uint8_t *addressArray)
{
    Wire.begin();
    int error, address;
    int idx = 0;
    for (address = 1; address < 127; address++)
    {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        if (error == 0)
        {
            addressArray[idx] = address;
            idx++;
        }
        else if (error == 4)
            return -1;
    }
    return idx;
}
