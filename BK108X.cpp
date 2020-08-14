/**
 * @mainpage BK108X Arduino Library implementation 
 * @details BK108X Arduino Library implementation. This is an Arduino library for the BK108X, BROADCAST RECEIVER.  
 * @details This is an Arduino library for the BK1086 and BK1088 DSP BROADCAST RECEIVER.<br>  
 * @details It works with I2C protocol and can provide an easier interface for controlling the BK1086/88 devices.<br>
 * @details This library is based on BEKEN - BK1086/88 - BROADCAST AM/FM/SW/LW RADIO RECEIVER manual. 
 * 
 * @copyright Copyright (c) 2020 Ricardo Lima Caratti. 
 * @author Ricardo LIma Caratti (pu2clr@gmail.com)
 * 
 */

#include <BK108X.h>

/** 
 * @defgroup GA03 Basic Functions
 * @section GA03 Basic
 */

/**
 * @ingroup GA03
 * @brief Gets a givens current register content of the device
 * @see shadowRegisters;  
 * @param device register address
 */
uint16_t BK108X::getRegister(uint8_t reg)
{
    word16_to_bytes result;

    Wire.beginTransmission(this->deviceAddress);
    Wire.write(reg);
    Wire.endTransmission(false);
    delayMicroseconds(6000);
    Wire.requestFrom(this->deviceAddress, 2);
    result.refined.highByte = Wire.read();
    result.refined.lowByte = Wire.read();
    Wire.endTransmission(true);
    delayMicroseconds(6000);

    shadowRegisters[reg] = result.raw; // Syncs with the shadowRegisters

    return result.raw;
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
    word16_to_bytes param;

    param.raw = value;

    Wire.beginTransmission(this->deviceAddress);
    Wire.write(reg);

    Wire.write(param.refined.highByte);
    Wire.write(param.refined.lowByte);
    Wire.endTransmission();

    shadowRegisters[reg] = value;  // Syncs with the shadowRegisters

    delayMicroseconds(6000);
}

/**
 * @ingroup GA03
 * @return 
 */
void BK108X::getStatus()
{

}

/**
 * @ingroup GA03
 * @brief   Wait STC (Seek/Tune Complete) status becomes 0
 * @details Should be used before processing Tune or Seek.
 * @details The STC bit being cleared indicates that the TUNE or SEEK bits may be set again to start another tune or seek operation. Do not set the TUNE or SEEK bits until the BK108X clears the STC bit. 
 */
void BK108X::waitAndFinishTune()
{

}

/**
 * @ingroup GA03
 * @brief Resets the device
 */
void BK108X::reset()
{

}

/**
 * @ingroup GA03
 * @brief Powers the receiver on 
 * @details Starts the receiver with some default configurations 
 */
void BK108X::powerUp()
{
    reg02->raw = 0;    
    reg02->refined.DSMUTE = 1;
    reg02->refined.STEREO = 1; 
    reg02->refined.ENABLE = 1;
    setRegister(REG02,reg02->raw);
}

/**
 * @ingroup GA03
 * @brief Powers the receiver off
 */
void BK108X::powerDown()
{

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
void BK108X::setup(int rdsInterruptPin, int seekInterruptPin, uint8_t oscillator_type)
{
    if (rdsInterruptPin >= 0)
        this->rdsInterruptPin = rdsInterruptPin;
    if (seekInterruptPin >= 0)
        this->seekInterruptPin = seekInterruptPin;

    this->oscillatorType = oscillator_type;

    reset();
    Wire.begin();
    delay(1);
    powerUp();
}


/**
 * @ingroup GA03
 * @brief Sets the channel 
 * @param channel 
 */
void BK108X::setChannel(uint16_t channel)
{

}

/**
 * @ingroup GA03
 * @brief Sets the FM frequency 
 * @details ....
 * @param frequency  
 */
void BK108X::setFrequency(uint16_t frequency)
{

}

/**
 * @ingroup GA03
 * @brief Increments the current frequency
 * @details The increment uses the band space as step. See array: uint16_t fmSpace[4] = {20, 10, 5, 1};
 */
void BK108X::setFrequencyUp()
{
    if (this->currentFrequency < this->endBand[this->currentFMBand])
        this->currentFrequency += this->fmSpace[currentFMSpace];
    else
        this->currentFrequency = this->startBand[this->currentFMBand];

    setFrequency(this->currentFrequency);
}

/**
 * @ingroup GA03
 * @brief Decrements the current frequency
 * @details The drecrement uses the band space as step. See array: uint16_t fmSpace[4] = {20, 10, 5, 1};
 */
void BK108X::setFrequencyDown()
{
    if (this->currentFrequency > this->startBand[this->currentFMBand])
        this->currentFrequency -= this->fmSpace[currentFMSpace];
    else
        this->currentFrequency = this->endBand[this->currentFMBand];

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
    return getRealChannel() * this->fmSpace[this->currentFMSpace] + this->startBand[this->currentFMBand];
}

/**
 * @ingroup GA03
 * @brief Seek function
 * @details Seeks a station up or down.
 * @details Seek begins at the current channel, and goes in the direction specified with the SEEKUP bit. Seek operation stops when a channel is qualified as valid according to the seek parameters, the entire band has been searched (SKMODE = 0), or the upper or lower band limit has been reached (SKMODE = 1).
 * @details The STC bit is set high when the seek operation completes and/or the SF/BL bit is set high if the seek operation was unable to find a channel qualified as valid according to the seek parameters. The STC and SF/BL bits must be set low by setting the SEEK bit low before the next seek or tune may begin.
 * @details Seek performance for 50 kHz channel spacing varies according to RCLK tolerance. Silicon Laboratories recommends ±50 ppm RCLK crystal tolerance for 50 kHz seek performance.
 * @details A seek operation may be aborted by setting SEEK = 0.
 * 
 * @param seek_mode  Seek Mode; 0 = Wrap at the upper or lower band limit and continue seeking (default); 1 = Stop seeking at the upper or lower band limit.
 * @param direction  Seek Direction; 0 = Seek down (default); 1 = Seek up.
 */
void BK108X::seek(uint8_t seek_mode, uint8_t direction)
{

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
 *      rx.seek(bk_SEEK_WRAP, bk_SEEK_UP, showFrequency); // Seek Up
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
 * @param value  0 to 15 (if 0, mutes the audio)
 */
void BK108X::setVolume(uint8_t value)
{

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
    if (this->currentVolume < 15)
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
 * @brief Gets the Device identification
 * @return number 
 */
uint8_t BK108X::getDeviceId()
{

}

/**
 * @ingroup GA03
 * @brief Gets the Chip Version
 * @return number 
 */
uint8_t BK108X::getChipVersion()
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
