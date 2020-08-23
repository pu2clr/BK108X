/**
 * @mainpage BK108X Arduino Library implementation 
 * @details BK108X Arduino Library implementation. This is an Arduino library for the BK108X, BROADCAST RECEIVER.  
 * @details This is an Arduino library for the BK1086 and BK1088 DSP BROADCAST RECEIVER.<br>  
 * @details It works with I2C protocol and can provide an easier interface for controlling the BK1086/88 devices.<br>
 * @details This library is based on BEKEN - BK1086/88 - BROADCAST AM/FM/SW/LW RADIO RECEIVER manual. 
 * 
 * __THIS LIBRARY IS UNDER CONSTRUCTION.....___
 * 
 * This library can be freely distributed using the MIT Free Software model.
 * Copyright (c) 2020 Ricardo Lima Caratti.  
 * Contact: pu2clr@gmail.com 
 */

#include <BK108X.h>

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
    word16_to_bytes result;
    result.raw = this->readRegister(reg);
    shadowRegisters[reg] = result.raw; // Syncs with the shadowRegisters
    return result.raw;  // Optional 
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
 * @return  bk_reg0a data type status register (Register 0Ah. Status2)
 */
bk_reg0a BK108X::getStatus()
{
    getRegister(REG0A); // update the reg0a shadow register
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
    delay(40);
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
 * @details Starts the receiver with some default configurations 
 */
void BK108X::powerUp()
{

    reg02->raw = 0;                 // Sets to 0 all attributes of the register 0x02 (Power Configuration)
    reg02->refined.DSMUTE = 1;      // Soft Mute disabled
    reg02->refined.STEREO = 1;      // Force stereo
    reg02->refined.ENABLE = 1;      // Power the receiver UP (DISABLE has to be 0)
    setRegister(REG02,reg02->raw);  // Stores the register 0x02

    setRegister(REG03, 0x00);       // Sets to 0 all attributes of the register 0x03 (Channel)
    setRegister(REG04, 0x00);       // Sets to 0 all attributes of the register 0x04 (System Configuration1)


    this->currentVolume = reg05->refined.VOLUME = 15;
    setRegister(REG05, reg05->raw); //  System Configuration2

    reg06->raw = 0;
    reg06->refined.CLKSEL = 1;
    reg06->refined.SKSNR = 0x06;
    reg06->refined.SKCNT = 0x0F;
    setRegister(REG06, reg06->raw);

    delay(100);
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
 * @param step  increment and decrement frequency step
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
    delay(50);
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
 * @details The increment uses the band space as step. See array: uint16_t fmSpace[4] = {20, 10, 5, 1};
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
 * @ingroup GA03
 * @brief Seek function
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
void BK108X::seek(uint8_t seek_mode, uint8_t direction, void (*showFunc)())
{
    long max_time = millis();
    do
    {
        reg02->refined.SKMODE = seek_mode;
        reg02->refined.SEEKUP = direction;
        reg02->refined.SEEK = 1;
        reg03->refined.TUNE = 1;
        setRegister(REG02, reg02->raw);
        setRegister(REG03, reg03->raw);

        delay(40);
        if (showFunc != NULL)
        {
            this->currentFrequency = getRealFrequency();
            showFunc();                                    
        }
        getRegister(REG0A);
    } while ( reg0a->refined.STC && (millis() - max_time) < MAX_SEEK_TIME);

    reg02->refined.SEEK = 0;
    reg03->refined.TUNE = 0;
    setRegister(REG02, reg02->raw);
    setRegister(REG03, reg03->raw);

    this->currentFrequency = getRealFrequency();
}

/**
 * @ingroup GA03
 * @brief Sets RSSI Seek Threshold
 * @param  value between 0 and 127
 */
void BK108X::setSeekThreshold(uint8_t value)
{


}

/**
 * @ingroup GA03
 * @brief Sets the FM Band  

 * 
 * @param band  
 */
void BK108X::setBand(uint8_t band)
{

}

/**
 * @ingroup GA03
 * @brief Sets the FM Space  
 */
void BK108X::setSpace(uint8_t space)
{

}

/**
 * @ingroup GA03
 * @brief Gets the Rssi
 * 
 * @return int 
 */
int BK108X::getRssi()
{

}

/**
 * @ingroup GA03
 * @brief Sets the Softmute true or false
 * @param value  TRUE or FALSE
 */
void BK108X::setSoftmute(bool value)
{

}

/**
 * @ingroup GA03
 * @brief Sets Softmute Attack/Recover Rate.
 * 
 * @param value 
 */
void BK108X::setSoftmuteAttack(uint8_t value)
{

}

/**
 * @ingroup GA03
 * @brief Sets  Softmute Attenuation..
 * 
 * @param value 
 */
void BK108X::setSoftmuteAttenuation(uint8_t value)
{

}

/**
 * @ingroup GA03
 * @brief Sets the AGC enable or disable
 * @param value true = enable; fale = disable
 */
void BK108X::setAgc(bool value)
{

}

/**
 * @ingroup GA03
 * @brief Sets the Mute true or false
 * 
 * @param value TRUE or FALSE
 */
void BK108X::setMute(bool value)
{

}

/**
 * @ingroup GA03
 * @brief Sets the Mono true or false (stereo)
 * 
 * @param value TRUE or FALSE
 */
void BK108X::setMono(bool value)
{

}

/**
 * @ingroup GA03
 * @brief Checks stereo / mono status
 * 
 * @param value TRUE if stereo
 */
bool BK108X::isStereo()
{

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
 * @ingroup GA03
 * @brief Sets Extended Volume Range.
 * @param value false = disabled (default); true = enabled.
 */
void BK108X::setExtendedVolumeRange(bool value)
{

}



/**
 * @ingroup GA03
 * @brief Sets De-emphasis.
 * @details 75 μs. Used in USA (default); 50 μs. Used in Europe, Australia, Japan.
 * 
 * @param de  0 = 75 μs; 1 = 50 μs
 */
void BK108X::setFmDeemphasis(uint8_t de)
{

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

}

/**
 * @ingroup GA04
 * @brief Return the group type 
 * 
 * @return uint16_t 
 */
uint16_t BK108X::getRdsGroupType()
{

}

/**
 * @ingroup GA04
 * 
 * @brief Gets the version code (extracted from the Block B)
 * @returns  0=A or 1=B
 */
uint8_t BK108X::getRdsVersionCode(void)
{

}

/**  
 * @ingroup GA04  
 * @brief Returns the Program Type (extracted from the Block B)
 * @see https://en.wikipedia.org/wiki/Radio_Data_System
 * @return program type (an integer betwenn 0 and 31)
 */
uint8_t BK108X::getRdsProgramType(void)
{

}

/**
 * @ingroup GA04
 * 
 * @brief Process data received from group 2B
 * @param c  char array reference to the "group 2B" text 
 */
void BK108X::getNext2Block(char *c)
{

}

/**
 * @ingroup GA04
 * 
 * @brief Process data received from group 2A
 * 
 * @param c  char array reference to the "group  2A" text 
 */
void BK108X::getNext4Block(char *c)
{

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
