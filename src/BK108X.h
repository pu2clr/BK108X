
/**
 * @mainpage PU2CLR BK108X Arduino Library 
 * @brief PU2CLR BK108X Arduino Library implementation. <br> 
 * @details This is an Arduino library for the BK1086 and BK1088 DSP BROADCAST RECEIVER.<br>  
 * @details It works with I2C protocol and can provide an easier interface for controlling the BK1086/88 devices.<br>
 * @details This library is based on BEKEN - BK1086/88 - BROADCAST AM/FM/SW/LW RADIO RECEIVER manual. 
 * 
 * @see https://github.com/pu2clr/BK108X
 * 
 * __THIS LIBRARY IS UNDER CONSTRUCTION.....___
 * 
 * 
 * This library can be freely distributed using the MIT Free Software model.
 * Copyright (c) 2020 Ricardo Lima Caratti.  
 * Contact: pu2clr@gmail.com
 */

#include <Arduino.h>

#define MAX_DELAY_AFTER_OSCILLATOR 500 // Max delay after the crystal oscillator becomes active

#define I2C_DEVICE_ADDR 0x80

#define MAX_SEEK_TIME 5000 // Maximum time have to be a seeking process (in ms).

#define OSCILLATOR_TYPE_CRYSTAL 1 // Crystal
#define OSCILLATOR_TYPE_REFCLK 0  // Reference clock

#define RDS_STANDARD 0 //!< RDS Mode.
#define RDS_VERBOSE 1  //!< RDS Mode.
#define BK_SEEK_DOWN 0 //!< Seek Down  Direction
#define BK_SEEK_UP 1   //!< Seek Up  Direction
#define BK_SEEK_WRAP 0 //
#define BK_SEEK_STOP 1

#define FM_FULL 0            //!< 64~108MHz
#define FM_BAND_JAPAN_WIDE 1 //!< 76–108 MHz (Japan wide band)
#define FM_BAND_JAPAN 2      //!< 76–90 MHz (Japan)
#define FM_BAND_USA_EU 3     //!< 87–108 MHz (US / Europe, Default)

#define AM_LW  0 //!< 153~279KHz
#define AM_MW1 1 //!< 520~1710Khz
#define AM_SW  2 //!< 2.3~21.85KHz
#define AM_M22 3 //!< 522~1710

#define BK_MODE_FM  0
#define BK_MODE_AM  1

#define DE_EMPHASIS_75 0
#define DE_EMPHASIS_50 1


#define REG00 0x00
#define REG01 0x01
#define REG02 0x02
#define REG03 0x03
#define REG04 0x04
#define REG05 0x05
#define REG06 0x06
#define REG07 0x07
#define REG08 0x08
#define REG09 0x09
#define REG0A 0x0A
#define REG0B 0x0B
#define REG0C 0x0C
#define REG0D 0x0D
#define REG0E 0x0E
#define REG0F 0x0F
#define REG10 0x10
#define REG11 0x11
#define REG12 0x12
#define REG13 0x13
#define REG14 0x14
#define REG15 0x15
#define REG16 0x16
#define REG17 0x17
#define REG18 0x18
#define REG19 0x19
#define REG1A 0x1A
#define REG1B 0x1B
#define REG1C 0x1C
#define REG1D 0x1D
#define REG1E 0x1E
#define REG1F 0x1F

/**
 * @defgroup GA01 Union, Structure and Defined Data Types  
 * @brief   BK108X Defined Data Types 
 * @details Defined Data Types is a way to represent the BK108X registers information
 * @details Some information appears to be inaccurate due to translation problems from Chinese to English.
 * @details The information shown here was extracted from Datasheet:
 * @details BK108X stereo FM digital tuning radio documentation.
 * @details Other information seems incomplete even in the original Chinese Datasheet. 
 * @details For example: Reg 10 (0x0A). There is no information about it. The Reg11 and 12 seem wrong  
 */

/**
 * @ingroup GA01
 * @brief Device ID
 * 
 */
typedef union
{
    struct
    {
        uint8_t lowByte;
        uint8_t highByte;
    } refined;
    uint16_t deviceId;
} bk_reg00;

/**
 * @ingroup GA01
 * @brief Chip ID
 * 
 */
typedef union
{
    struct
    {
        uint8_t lowByte;
        uint8_t highByte;
    } refined;
    uint16_t chipId;
} bk_reg01;

/**
 * @ingroup GA01
 * @brief Power Configuration
 * @see  BEKEN - BK1086/88 - BROADCAST AM/FM/SW/LW RADIO RECEIVER; pages 13 and 14 
 */
typedef union
{
    struct
    {
        uint8_t ENABLE : 1;  //!< Powerup Enable;
        uint8_t SNR_REF : 5; //!< Output SNR adjustment. Read SNR = SNR (calculated)
        uint8_t DISABLE : 1; //!< Powerup Disable;
        uint8_t SKAFCRL : 1; //!< Seek with AFC Rail; 0 = During seeking, the channel is valid no matter whether AFCRL is high or low; 1 = During seeking, the channel is invalid if AFCRL is high.
        uint8_t SEEK : 1;    //!< 0 = Disable (default); 1 = Enable;
        uint8_t SEEKUP : 1;  //!< Seek Direction; 0 = Seek down (default); 1 = Seek up.
        uint8_t SKMODE : 1;  //!< Seek Mode; 0 = Wrap at the upper or lower band limit and continue seeking (default); 1 = Stop seeking at the upper or lower band limit.
        uint8_t STEREO : 1;  //!< Stereo; 0 = Normal operation; 1 = Force stereo; MONO and STEREO cannot be set to 1 simultaneously.
        uint8_t MONO : 1;    //!< Mono; 0 = Normal operation; 1 = Force mono.
        uint8_t MUTER : 1;   //!< Mute R channel; 0 = R channel normal operation; 1 = R channel mute.
        uint8_t MUTEL : 1;   //!< Mute L channel; 0 = L channel normal operation; 1 = L channel mute.
        uint8_t DSMUTE : 1;  //!< Softmute Disable; 0 = Softmute enable (default); 1 = Softmute disable.
    } refined;
    uint16_t raw;
} bk_reg02;

/**
 * @ingroup GA01
 * @brief Channel
 * @details The tuned Frequency = Band + CHAN * SPACE
 * @see BEKEN - BK1086/88 - BROADCAST AM/FM/SW/LW RADIO RECEIVER; page 14 
 */
typedef union
{
    struct
    {
        uint16_t CHAN : 15; //!< Channel Select;  The tuned Frequency = Band + CHAN * SPACE.
        uint16_t TUNE : 1;  //!< Tune. 0 = Disable (default); 1 = Enable.
    } refined;
    uint16_t raw;
} bk_reg03;

/**
 * @ingroup GA01
 * @brief  Register 04h. System Configuration1 (0x1180)
 * @details When register GPIO2[1:0]=2’b01 and seek or tune finish, a 5ms low pulse will appear at GPIO2 .Both RDSIEN and STCIEN can be high;
 * @details When register GPIO2[1:0]=2’b01 and new RDS come, a 5ms low pulse will appear at GPIO2.
 * 
 */
typedef union
{
    struct
    {
        uint16_t DUMMY : 2;   //!< Not used / RESERVED General Purpose I/O 1; 00 = High impedance (default); 01 = CLK38MHz; 10 = Low; 11 = High.
        uint16_t GPIO2 : 2;   //!< General Purpose I/O 2. 00 = High impedance (default); 01 = STC/RDS interrupt; 10 = Low; 11 = High.
        uint16_t GPIO3 : 2;   //!< General Purpose I/O 2. 00 = High impedance (default); 01 = Mono/Stereo indicator (ST); 10 = Low; 11 = High.
        uint16_t PILOTS : 3;   //!< Stereo/Mono Blend Level Adjustment. Sets the RSSI range for stereo/mono blend. See table above.
        uint16_t TCPILOT : 2; //!< The Time Used to Cal The Strength of Pilot
        uint16_t DE : 1;      //!< De-emphasis; 0 = 75 μs. Used in USA (default); 1 = 50 μs. Used in Europe, Australia, Japan.
        uint16_t RDSEN : 1;   //!< RDS Enable; 0 = Disable (default); 1 = Enable.
        uint16_t AFCINV : 1;  //!< AFC Invert; 0 = Normal AFC into mixer; 1 = Reverse AFC into mixer.
        uint16_t STCIEN : 1;  //!< Seek/Tune Complete Interrupt Enable; 0 = Disable Interrupt (default); 1 = Enable Interrupt. See details above.
        uint16_t RDSIEN : 1;  //!< RDS Interrupt Enable; 0 = Disable Interrupt (default); 1 = Enable Interrupt. See details above.
    } refined;
    uint16_t raw;
} bk_reg04;

/**
 * @ingroup GA01
 * @brief Register 05h. System Configuration2 (0x3ddf)
 * @details LW and SW Band are only defined at BK1088
 * 
 *  AM and Fm Band table ()
 * | BAND value | AM band KHz     | FM band MHz       | 
 * | ---------- | ----------------| ----------------- | 
 * |    0       | 00: LW 153~279  | FULL 64~108       |
 * |    1       | 10: MW 520~1710 | East Europe 64~76 | 
 * |    2       | 10: SW 2.3~21.85| Japan 76~91       | 
 * |    3       | 11: MW 522~1710 | Europe 87~108     | 
 * 
 * 
 * AM and FM Channel Space 
 * | BAND value | AM Space KHz  |  FM Space KHz | 
 * | ---------- | ------------- | ------------- |
 * |    0       |       1       |       10      |
 * |    1       |       5       |       50      |
 * |    2       |       9       |      100      |
 * |    3       |      10       |      200      |
 * 
 * @see bk_reg07
 */
typedef union
{
    struct
    {
        uint16_t VOLUME : 5; //!< 0x00 is the lowest and 0x1F is highest (0dBFS). 2dB each
        uint16_t SPACE : 2;  //!< Channel Spacing; See AM and FM Channel Space table above.
        uint16_t BAND : 2;   //!< Band Select. See AM and Fm Band table above.
        uint16_t SEEKTH : 7; //!< RSSI Seek Threshold. 0x00 = min RSSI (default); 0x7F = max RSSI.
    } refined;
    uint16_t raw;
} bk_reg05;

/**
 * @ingroup GA01
 * @brief  Register 06h. System Configuration3 (0x01ef)
 * @details SKCNT - Seek Impulse Detection Threshold Allowable number of impulse for a valid seek channel while setting all zeros means not use Impulse number to judge the channel’s validity.
 * 
 * 
 * SMUTEA table 
 * 
 * | Softmute Attenuation   | Description     | 
 * | ---------------------- | --------------- |
 * |            0           | 16 dB (default) |
 * |            1           | 14 dB           | 
 * |            2           | 12 dB           |
 * |            3           | 10 dB           |
 * 
 * SMUTER table 
 * 
 * | Softmute Attack/Recover Rate   | Description       | 
 * | ------------------------------ | ----------------- |
 * |            0                   | fastest (default) |
 * |            1                   | fast              | 
 * |            2                   | slow              |
 * |            3                   | slowest           |
 * 
 */
typedef union
{
    struct
    {
        uint16_t SKCNT : 4;  //!< See details above.
        uint16_t SKSNR : 7;  //!< Seek SNR Threshold. Required channel SNR for a valid seek channel
        uint16_t CLKSEL : 1; //!< Clock Select. 0 = External clock input; 1= Internal oscillator input.
        uint16_t SMUTEA : 2; //!< Softmute Attenuation; See table above.
        uint16_t SMUTER : 2; //!< Softmute Attack/Recover Rate; See table above
    } refined;
    uint16_t raw;
} bk_reg06;

/**
 * @ingroup GA01
 * @brief Register 07h. Test1 (0x0900)
 */
typedef union
{
    struct
    {
        uint16_t FMGAIN : 3;     //!< The gain of Frequency demodulated; 000 = 0dB ... 011= +18dB; 100= 0dB ... 111= -18dB
        uint16_t RESERVED :3;
        uint16_t STHYS_SEL : 1;  //!< ST/MONO Transition Hysterisis Select.  0 = 6dB; 1=2dB
        uint16_t DACCK_SEL : 1;  //!< DAC Clock Select
        uint16_t IMPTH : 2;      //!< Threshold of Impulse Detect. 00 = strictest; 11 = loosest
        uint16_t BPDE : 1;       //!< De-emphasis Bypass; 0 = Normal operation; 1 = Bypass de-emphasis.
        uint16_t IMPEN : 1;      //!< Impulse Remove Enable; 0 = Disable; 1 = Enable.
        uint16_t SIQ : 1;        //!< IF I/Q Signal switch; 0 = Normal operation; 1 = Reversed I/Q signal.
        uint16_t MODE : 1;       //!< 0 = FM receiver; 1 = AM receiver
        uint16_t LINEIN_EN : 1;  //!< Audio Line in Enable; 0 = Disable (Receiver Mode) 
        uint16_t LINEIN_SEL : 1; //!< Audio Line in Channel Select; 0 = Channel 1; 1=Channel 2 ((QFN24 only support one line in channel))
    } refined;
    uint16_t raw;
} bk_reg07;

/**
 * @ingroup GA01
 * @brief Test 2
 * @details RSSI Threshold for Instant AFC updating; AFC Average Range; Variation Threshold for average AFC calculation;
 * @details AFC Average; AFCRL Threshold; AFC/RSSI/SNR Calculate Rate; AFC Enable
 * 
 */
typedef union
{
    struct
    {
        uint16_t AFCRSSIT : 7; //!< RSSI Threshold for Instant AFC updating
        uint16_t RANGE : 2;    //!< AFC Average Range; 00 = the strictest; 11 = the loosest
        uint16_t VAR : 2;      //!< Variation Threshold for average AFC calculation; 00 = Disable; 01 = the strictest; 11 = the loosest
        uint16_t AVE : 1;      //!< AFC Average
        uint16_t SEL25K : 1;   //!< AFCRL Threshold; 0 = Channel space/2; 1 = 25kHz
        uint16_t TCSEL : 2;    //!< AFC/RSSI/SNR Calculate Rate; 00 = fastest; 11 = slowest. 4X times each
        uint16_t AFCEN : 1;    //!< AFC Enable; 0 = Disable; 1 = Enable.
    } refined;
    uint16_t raw;
} bk_reg08;

/**
 * @ingroup GA01
 * @brief Register 09h. Status1 (0x0000)
 */
typedef union
{
    struct
    {
        uint16_t SNR:7; //!< The AFC value.
        uint16_t AFC:9; //!< unit AM 0.15k Hz, FM 0.6k Hz
    } refined;
    uint16_t raw;
} bk_reg09;

/**
 * @ingroup GA01
 * @brief Register 0Ah. Status2 (0x0000)
 */
typedef union
{
    struct
    {
        uint16_t RSSI : 7;   //!< RSSI (Received Signal Strength Indicator).
        uint16_t ST : 1;     //!< Stereo Indicator; 0 = Mono; 1 = Stereo.
        uint16_t STEN : 1;   //!< Impulse Number
        uint16_t DUMMY: 3; 
        uint16_t AFCRL : 1;  //!< AFC Rail; 0 = AFC not railed; 1 = AFC railed.
        uint16_t SF_BL : 1;  //!< Seek Fail/Band Limit; 0 = Seek successful; 1 = Seek failure/Band limit reached.
        uint16_t STC : 1;    //!< Seek/Tune Complete; 0 = Not complete (default); 1 = Complete.
        uint16_t RDSR : 1;   //!< RDS Ready; 0 = No RDS group ready (default); 1 = New RDS group ready. Keep high for 40ms after new RDS is received
    } refined;
    uint16_t raw;
} bk_reg0a;

/**
 * @ingroup GA01
 * @brief Register 0Bh. Read Channel (0x0000)
 */
typedef union
{
    struct
    {
        uint16_t READCHAN : 14; //!< Read Channel. Provides the current working channel
        uint16_t RESERVED : 2;
    } refined;
    uint16_t raw;
} bk_reg0b;

/**
 * @ingroup GA01
 * @brief Register 0Ch. RDS1 (0x0000)
 * @details RDS Block A - The First Register of RDS Received
 */
typedef union
{
    struct
    {
        uint16_t BLERB : 2;  //!< Block Errors Level of RDS_DATA_1
        uint16_t BLERA : 2;  //!< Block Errors Level of RDS_DATA_0
        uint16_t ABCD_E : 1; //!< 1 = the block id of register 0cH,0dH,0eH,0fH is E;  0 = the block id of register 0cH, 0dH, 0eH,0fH is A, B, C, D
        uint16_t DUMMY : 2;
        uint16_t FM_READY : 1; //!< 1=ready; 0=not ready.
        uint16_t FM_TRUE : 1;  //!< 1 = the current channel is a station; 0 = the current channel is not a station.
        uint16_t RSSI : 7;     //!< RSSI; 000000 = min; 111111 = max; RSSI scale is logarithmic.
    } rds_status;    
    struct
    {
        uint8_t lowByte;
        uint8_t highByte;
    } refined;
    uint16_t raw;
} bk_reg0c;

/**
 * @ingroup GA01
 * @brief Register 0Dh. RDS2 (0x0000)
 * @details RDS Block B - The second register of RDS received
 */
typedef union
{
    struct
    {
        uint8_t lowByte;
        uint8_t highByte;
    } refined;
    uint16_t raw;
} bk_reg0d;

/**
 * @ingroup GA01
 * @brief Register 0Eh. RDS2 (0x0000)
 * @details RDS Block C - The third register of RDS received
 */
typedef union
{
    struct
    {
        uint8_t lowByte;
        uint8_t highByte;
    } refined;
    uint16_t raw;
} bk_reg0e;

/**
 * @ingroup GA01
 * @brief Register 0Fh. RDS4 (0x0000)
 * @details RDS Block D - The fourth register of RDS received when read
 */
typedef union
{
    struct
    {
        uint8_t lowByte;
        uint8_t highByte;
    } refined;
    uint16_t raw;
} bk_reg0f;

/**
 * @ingroup GA01
 * @brief Register 10h. Boot Configuration1 (0x7b11)
 */
typedef union
{
    struct
    {
        uint8_t lowByte;
        uint8_t highByte;
    } refined;
    uint16_t raw;
} bk_reg10;

/**
 * @ingroup GA01
 * @brief Register 11h. Boot Configuration2 (0x0080)
 */
typedef union
{
    struct
    {
        uint8_t lowByte;
        uint8_t highByte;
    } refined;
    uint16_t raw;
} bk_reg11;

/**
 * @ingroup GA01
 * @brief Register 12h. Boot Configuration3 (0x4000)
 */
typedef union
{
    struct
    {
        uint8_t lowByte;
        uint8_t highByte;
    } refined;
    uint16_t raw;
} bk_reg12;

/**
 * @ingroup GA01
 * @brief Register 13h. Boot Configuration4 (0x3e00)
 */
typedef union
{
    struct
    {
        uint8_t lowByte;
        uint8_t highByte;
    } refined;
    uint16_t raw;
} bk_reg13;

/**
 * @ingroup GA01
 * @brief Register 14h. Boot Configuration5 (0x0000)
 */
typedef union
{
    struct
    {
        uint16_t RSSIMTH : 7; //!< The Mute Threshold Based on RSSI
        uint16_t SNRMTH : 7;  //!< The Mute Threshold Based on SNR
        uint16_t AFCMUTE : 1; //!< 0: disable soft mute when AFCRL is high; 1: enable soft mute when AFCRL is high
        uint16_t SKMUTE : 1;  //!< 0: disable soft mute when seeking; 1: enable soft mute when seeking
    } refined;
    uint16_t raw;
} bk_reg14;

/**
 * @ingroup GA01
 * @brief 15h. Boot Configuration6 (0x0000)
 */
typedef union
{
    struct
    {
        uint8_t lowByte;
        uint8_t highByte;
    } refined;
    uint16_t raw;
} bk_reg15;

/**
 * @ingroup GA01
 * @brief Register 16h. Boot Configuration7 (0x0400)
 */
typedef union
{
    struct
    {
        uint8_t lowByte;
        uint8_t highByte;
    } refined;
    uint16_t raw;
} bk_reg16;

/**
 * @ingroup GA01
 * @brief Register 17h. Boot Configuration8 (0x0001)
 */
typedef union
{
    struct
    {
        uint8_t lowByte;
        uint8_t highByte;
    } refined;
    uint16_t raw;
} bk_reg17;

/**
 * @ingroup GA01
 * @brief Register 18h. Boot Configuration9 (0x143c)
 */
typedef union
{
    struct
    {
        uint8_t lowByte;
        uint8_t highByte;
    } refined;
    uint16_t raw;
} bk_reg18;

/**
 * @ingroup GA01
 * @brief Register 19h. Boot Configuration10 (0x4351)
 */
typedef union
{
    struct
    {
        uint8_t lowByte;
        uint8_t highByte;
    } refined;
    uint16_t raw;
} bk_reg19;

/**
 * @ingroup GA01
 * @brief Register 1Ah. Boot Configuration11 (0x0000)
 */
typedef union
{
    struct
    {
        uint16_t RESERVED1 : 3;
        uint16_t ANT_SEL : 5;   //!< Antenna varactor tune
        uint16_t RESERVED2 : 4;
    } refined;
    uint16_t raw;
} bk_reg1A;

/**
 * @ingroup GA01
 * @brief Register 1Bh. Analog Configuration1 (0x0000)
 */
typedef union
{
    struct {
        uint16_t RESERVED;
    } refined; 
    uint16_t raw;
} bk_reg1B;

/**
 * @ingroup GA01
 * @brief Register 1Ch. Analog Configuration2 (0x0000)
 */
typedef union
{
    struct
    {    
        uint16_t RESERVED : 14;
        uint16_t FREQ_SEL : 2; //!<  Reference clock divider control ,Refer to Reg1D Default 0 for 32.768kHz reference input.
    } refined;
    uint16_t raw;
} bk_reg1C;

/**
 * @ingroup GA01
 * @brief Register 1Dh. Analog Configuration2 (0x0000)
 */
typedef union
{   
    struct {
     uint16_t FREQ_SEL;
    }  refined;
    uint16_t raw; //!< //!< Reference clock divider control , FREQ_SEL[17:0] = HEX | Ref Frequency/512+0.5 | Default 16 for 32.768kHz reference.
} bk_reg1D;

/**
 * @brief The user does not have access to registsres 0x1E, 0x1F and 0x20.
 * @details They are Internal test registers and can be provided separately by BEKEN.
 * @details The author of this library did not have access to these register.
 */
typedef uint16_t bk_reg1E; // Internal register
typedef uint16_t bk_reg1F; // Internal Register
typedef uint16_t bk_reg20; // Internal Register


/**
 * @ingroup GA01
 * @brief RDS Block B data type
 * 
 * @details For GCC on System-V ABI on 386-compatible (32-bit processors), the following stands:
 * 
 * 1) Bit-fields are allocated from right to left (least to most significant).
 * 2) A bit-field must entirely reside in a storage unit appropriate for its declared type.
 *    Thus a bit-field never crosses its unit boundary.
 * 3) Bit-fields may share a storage unit with other struct/union members, including members that are not bit-fields.
 *    Of course, struct members occupy different parts of the storage unit.
 * 4) Unnamed bit-fields' types do not affect the alignment of a structure or union, although individual 
 *    bit-fields' member offsets obey the alignment constraints.   
 * 
 * @see also https://en.wikipedia.org/wiki/Radio_Data_System
 */
typedef union
{
    struct
    {
        uint16_t address : 2;            // Depends on Group Type and Version codes. If 0A or 0B it is the Text Segment Address.
        uint16_t DI : 1;                 // Decoder Controll bit
        uint16_t MS : 1;                 // Music/Speech
        uint16_t TA : 1;                 // Traffic Announcement
        uint16_t programType : 5;        // PTY (Program Type) code
        uint16_t trafficProgramCode : 1; // (TP) => 0 = No Traffic Alerts; 1 = Station gives Traffic Alerts
        uint16_t versionCode : 1;        // (B0) => 0=A; 1=B
        uint16_t groupType : 4;          // Group Type code.
    } group0;
    struct
    {
        uint16_t address : 4;            // Depends on Group Type and Version codes. If 2A or 2B it is the Text Segment Address.
        uint16_t textABFlag : 1;         // Do something if it chanhes from binary "0" to binary "1" or vice-versa
        uint16_t programType : 5;        // PTY (Program Type) code
        uint16_t trafficProgramCode : 1; // (TP) => 0 = No Traffic Alerts; 1 = Station gives Traffic Alerts
        uint16_t versionCode : 1;        // (B0) => 0=A; 1=B
        uint16_t groupType : 4;          // Group Type code.
    } group2;
    struct
    {
        uint16_t content : 4;            // Depends on Group Type and Version codes.
        uint16_t textABFlag : 1;         // Do something if it chanhes from binary "0" to binary "1" or vice-versa
        uint16_t programType : 5;        // PTY (Program Type) code
        uint16_t trafficProgramCode : 1; // (TP) => 0 = No Traffic Alerts; 1 = Station gives Traffic Alerts
        uint16_t versionCode : 1;        // (B0) => 0=A; 1=B
        uint16_t groupType : 4;          // Group Type code.
    } refined;
    uint16_t blockB;
} bk_rds_blockb;

/**
 * @ingroup GA01
 * Group RDS type 4A ( RDS Date and Time)
 * When group type 4A is used by the station, it shall be transmitted every minute according to EN 50067.
 * This Structure uses blocks 2,3 and 5 (B,C,D)
 * 
 * ATTENTION: 
 * To make it compatible with 8, 16 and 32 bits platforms and avoid Crosses boundary, it was necessary to
 * split minute and hour representation. 
 */
typedef union
{
    struct
    {
        uint32_t offset : 5;       // Local Time Offset
        uint32_t offset_sense : 1; // Local Offset Sign ( 0 = + , 1 = - )
        uint32_t minute : 6;      // UTC Minutes - 2 bits less significant (void “Crosses boundary”).
        uint32_t hour  : 5;        // UTC Hours - 4 bits less significant (void “Crosses boundary”)
        uint32_t mjd : 17;        // Modified Julian Day Code
    } refined;
    uint8_t raw[6];
} bk_rds_date_time;

/**
 * @ingroup GA01
 * @brief Converts 16 bits word to two bytes 
 */
typedef union
{
    struct
    {
        uint8_t lowByte;
        uint8_t highByte;
    } refined;
    uint16_t raw;
} word16_to_bytes;



/**
 * @ingroup GA01  
 * @brief KT0915 Class 
 * @details This class implements all functions that will help you to control the KT0915 devices. 
 * 
 * @author PU2CLR - Ricardo Lima Caratti 
 */
class BK108X
{

private:

    // uint8_t  i2cBuffer[32];

    uint16_t shadowRegisters[32]; //!< shadow registers 0x00  to 0x1F (0 - 31)

    // Device registers map - References to the shadow registers
    bk_reg00 *reg00 = (bk_reg00 *)&shadowRegisters[REG00]; //  0
    bk_reg01 *reg01 = (bk_reg01 *)&shadowRegisters[REG01]; //  1
    bk_reg02 *reg02 = (bk_reg02 *)&shadowRegisters[REG02]; //  2
    bk_reg03 *reg03 = (bk_reg03 *)&shadowRegisters[REG03]; //  3
    bk_reg04 *reg04 = (bk_reg04 *)&shadowRegisters[REG04]; //  4
    bk_reg05 *reg05 = (bk_reg05 *)&shadowRegisters[REG05]; //  5
    bk_reg06 *reg06 = (bk_reg06 *)&shadowRegisters[REG06]; //  6
    bk_reg07 *reg07 = (bk_reg07 *)&shadowRegisters[REG07]; //  7
    bk_reg08 *reg08 = (bk_reg08 *)&shadowRegisters[REG08]; //  8
    bk_reg09 *reg09 = (bk_reg09 *)&shadowRegisters[REG09]; //  9
    bk_reg0a *reg0a = (bk_reg0a *)&shadowRegisters[REG0A]; // 10
    bk_reg0b *reg0b = (bk_reg0b *)&shadowRegisters[REG0B]; // 11
    bk_reg0c *reg0c = (bk_reg0c *)&shadowRegisters[REG0C]; // 12
    bk_reg0d *reg0d = (bk_reg0d *)&shadowRegisters[REG0D]; // 13
    bk_reg0e *reg0e = (bk_reg0e *)&shadowRegisters[REG0E]; // 14
    bk_reg0f *reg0f = (bk_reg0f *)&shadowRegisters[REG0F]; // 15
    bk_reg10 *reg10 = (bk_reg10 *)&shadowRegisters[REG10]; // 16
    bk_reg11 *reg11 = (bk_reg11 *)&shadowRegisters[REG11]; // 17
    bk_reg12 *reg12 = (bk_reg12 *)&shadowRegisters[REG12]; // 18
    bk_reg13 *reg13 = (bk_reg13 *)&shadowRegisters[REG13]; // 19
    bk_reg14 *reg14 = (bk_reg14 *)&shadowRegisters[REG14]; // 20
    bk_reg15 *reg15 = (bk_reg15 *)&shadowRegisters[REG15]; // 21
    bk_reg16 *reg16 = (bk_reg16 *)&shadowRegisters[REG16]; // 22
    bk_reg17 *reg17 = (bk_reg17 *)&shadowRegisters[REG17]; // 23
    bk_reg18 *reg18 = (bk_reg18 *)&shadowRegisters[REG18]; // 24
    bk_reg19 *reg19 = (bk_reg19 *)&shadowRegisters[REG19]; // 25
    bk_reg1A *reg1A = (bk_reg1A *)&shadowRegisters[REG1A]; // 26
    bk_reg1B *reg1b = (bk_reg1B *)&shadowRegisters[REG1B]; // 27
    bk_reg1C *reg1c = (bk_reg1C *)&shadowRegisters[REG1C]; // 28
    bk_reg1D *reg1d = (bk_reg1D *)&shadowRegisters[REG1D]; // 29
    bk_reg1E *reg1e = (bk_reg1E *)&shadowRegisters[REG1E]; // 30
    bk_reg1F *reg1f = (bk_reg1F *)&shadowRegisters[REG1F]; // 31

    uint16_t fmStartBand[4] = {6400, 7400, 7600, 8700};  //!< Start FM band limit
    uint16_t fmEndBand[4] = {10800, 7600, 9100, 10800};  //!< End FM band limit
    uint16_t fmSpace[4] = {1, 5, 10, 20};             //!< FM channel space

    uint16_t amStartBand[4] = {153, 520, 2300, 522}; //!< Start FM band limit
    uint16_t amEndBand[4] = {279, 1710, 21850, 1710}; //!< End FM band limit
    uint16_t amSpace[4] = {1, 5, 9, 10}; //!< AM channel space

    char strFrequency[9];

    int pin_sdio, pin_sclk; 

protected:
    char rds_buffer2A[65]; //!<  RDS Radio Text buffer - Program Information
    char rds_buffer2B[33]; //!<  RDS Radio Text buffer - Station Informaation
    char rds_buffer0A[9];  //!<  RDS Basic tuning and switching information (Type 0 groups)
    char rds_time[20];     //!<  RDS date time received information

    uint8_t rds_mode; 

    int deviceAddress = I2C_DEVICE_ADDR;

    uint32_t currentFrequency;
    uint32_t minimumFrequency; 
    uint32_t maximumFrequency; 

    uint16_t currentChannel = 0;
    uint16_t currentStep = 1;
    uint8_t currentFMBand = 0;
    uint8_t currentAMBand = 1;
    uint8_t currentFMSpace = 0;
    uint8_t currentAMSpace = 0;
    uint8_t currentMode = BK_MODE_FM;

    uint8_t currentVolume = 0;
    int rdsInterruptPin = -1;
    int seekInterruptPin = -1;
    int oscillatorType = OSCILLATOR_TYPE_CRYSTAL;
    uint32_t oscillatorFrequency = 32768;
    
    uint16_t maxDelayAfterCrystalOn = MAX_DELAY_AFTER_OSCILLATOR;


public:
    void setI2C(uint8_t i2c_addr = I2C_DEVICE_ADDR);
    void i2cInit(int pin_sdio, int pin_sclk);
    void i2cBeginTransaction();
    void i2cEndTransaction();
    void i2cAck();
    void i2cNack();
    uint8_t i2cReceiveAck();
    void i2cWriteByte(uint8_t data);
    uint8_t i2cReadByte();
    void writeRegister(uint8_t reg,uint16_t vakue);
    uint16_t readRegister(uint8_t reg);

    void reset();
    void powerUp();
    void powerDown();
    void waitAndFinishTune();



    /**
     * @ingroup GA03
     * @brief Sets the I2C bus address 
     * @details This function must to be called before setup function if your device are not using 0x10 (default)
     * @param bus_addr I2C buss address
     */
    inline void setI2CAddress(int bus_addr) { this->deviceAddress = bus_addr; };

    /**
     * @ingroup GA03
     * @brief Set the Delay After Crystal On (default 500ms)
     * 
     * @param ms_value  Value in milliseconds 
     */
    inline void setDelayAfterCrystalOn(uint8_t ms_value) { maxDelayAfterCrystalOn = ms_value; };

    uint16_t getRegister(uint8_t reg);
    void setRegister(uint8_t reg, uint16_t value);
    bk_reg0a getStatus();

    /**
     * @ingroup GA03
     * @brief Get the Shadown Register object
     * @details if you want to get the current value of the device register, call getAllRegisters() before calling this function. 
     * @details if you are dealing with the status register (0x0A), you can call getStatus() instead getAllRegisters(). 
     * @see setAllRegisters, getAllRegisters, getShadownRegister, getStatus
     * @param register_number 
     * @return  16 bits word with the Shadown registert 
     */
    inline uint16_t getShadownRegister(uint8_t register_number) { return shadowRegisters[register_number]; };

    /**
     * @ingroup GA03
     * @brief Sets a given value to the Shadown Register
     * @details You have to call setAllRegisters() after setting the Shadow Registers to store the value into the device.
     * @see setAllRegisters, getAllRegisters, getShadownRegister, getStatus
     * @param register_number  register index (from 0x00 to 0x0F)
     * @param value            16 bits word with the content of the register 
     */
    void setShadownRegister(uint8_t register_number, uint16_t value)
    {
        if (register_number > 0x0F)
            return;
        shadowRegisters[register_number] = value;
    };

    uint16_t getDeviceId();
    uint16_t getChipId();

    void setup(int sda_pin, int sclk_pin, uint8_t oscillator_type = OSCILLATOR_TYPE_CRYSTAL, uint32_t oscillator_frequency = 32768);
    // void setup(int sda_pin, int sclk_pin, int rdsInterruptPin = -1, int seekInterruptPin = -1, uint8_t oscillator_type = OSCILLATOR_TYPE_CRYSTAL, uint16_t maxDelayAfterCrystalOn = MAX_DELAY_AFTER_OSCILLATOR);
    
    void setFM(uint16_t minimum_frequency, uint16_t maximum_frequency, uint16_t default_frequency, uint16_t step);
    void setAM(uint16_t minimum_frequency, uint16_t maximum_frequency, uint16_t default_frequency, uint16_t step, uint16_t am_space = 0);

    /**
     * @ingroup GA03
     * @brief   set the FM gain.
     * @param if 0, +18db. If 7, - 18db
     * 
     * @param value 
     */
    void inline setFMGain(uint8_t value = 0) {
        reg07->refined.FMGAIN = value;
        setRegister(REG07,reg07->raw); 
    }

    /**
     * @ingroup GA03
     * @brief Get the Current Mode (AM or FM)
     * 
     * @return uint8_t  0 = FM; 1 = AM
     */
    inline uint8_t getCurrentMode() { return this->currentMode; };

    /**
     * @ingroup GA03
     * @brief Sets the Stereo Threshold of Pilotto Strength 
     * 
     * @param value  0 ~ 7
     */
    inline void setStereoThresholdPilotStrength(uint8_t value) {
        reg04->refined.PILOTS = value;
        setRegister(REG04,reg04->raw);
    }

    /**
     * @ingroup GA03
     * @brief Sets De-emphasis.
     * @details 75 μs. Used in USA (default); 50 μs. Used in Europe, Australia, Japan.
     * 
     * @param de  0 = 75 μs; 1 = 50 μs
     */
    inline void setFmDeemphasis(uint8_t de)
    {
        reg04->refined.DE = de;
        setRegister(REG04, reg04->raw);
    }

    /**
     * @ingroup GA03
     * @brief Sets the time used to call the strength of pilot
     * @details he Strength Pilot refers to a specific technique used in digital broadcasting systems to provide an indication of the signal strength or quality. 
     * @details It is primarily used in FM (Frequency Modulation) broadcasting.
     * @param value  0 ~ 3
     */
    inline void setTimeCallStrengthPilot(uint8_t value)
    {
        reg04->refined.TCPILOT = value;
        setRegister(REG04, reg04->raw);
    }

    /**
     * @ingroup GA03
     * @brief Sets the Gpio2 
     * @details the General Purpose I/O 2. You can use this pin according to the table below.
     * 
     * | value | description | 
     * | ----- | ----------- | 
     * |  0    | Low         |
     * |  1    | STC/RDS interrupt |
     * |  2    | Low         | 
     * |  3    | High        |
     * 
     * @param value See table above
     */
    inline void setGpio2( uint8_t value) {
        reg04->refined.GPIO2 = value;
        setRegister(REG04,reg04->raw);
    }

    /**
     * @ingroup GA03
     * @brief Sets the Gpio3 
     * @details the General Purpose I/O 3. You can use this pin according to the table below.
     * 
     * | value | description | 
     * | ----- | ----------- | 
     * |  0    | Low         |
     * |  1    | Mono/Stereo Indicator |
     * |  2    | Low         | 
     * |  3    | High        |
     * 
     * @param value See table above
     */
    inline void setGpio3(uint8_t value)
    {
        reg04->refined.GPIO3 = value;
        setRegister(REG04, reg04->raw);
    }

    /**
     * @ingroup GA03
     * @brief Enables or Disables AFC
     * @details In DSP receivers, AFC stands for Automatic Frequency Control. 
     * @details It is a mechanism used to automatically adjust the frequency of the receiver's local oscillator to accurately track and lock onto the desired signal.
     * @param value if True, it enables AFC
     */
    inline void setAfc(bool value)
    {
        reg08->refined.AFCEN = value;
        setRegister(REG08, reg08->raw);
    }

    /**
     * @ingroup GA03
     * @brief Sets AFC Rail 
     * @details In the context of a DSP receiver, AFC (Automatic Frequency Control) rail refers to a control signal or voltage that is used to adjust the frequency of the receiver's local oscillator.
     * @details AFC rail helps maintain the receiver's frequency locked onto the desired signal by compensating for any variations or drifts in the received signal's frequency. 
     * @details It is part of the automatic frequency control mechanism that ensures the receiver stays tuned accurately to the desired station or frequency. 
     * @param value True or false. If true, it enables AFC Rail 
     */
    inline void AfcRail(bool value) {
        reg0a->refined.AFCRL = value;
        setRegister(REG0A,reg0a->raw);
    }

    /**
     * @ingroup GA03
     * @brief Sets AFC/RSSI/SNR Calculate Rate
     * @details 00 = fastest; 11 = slowest. 4X times each
     * @param value 0˜3
     */
    inline void setAfcRssiSnrCalculateRate(uint8_t value)
    {
        reg08->refined.TCSEL =  value;
        setRegister(REG08, reg08->raw);
    }

    /**
     * @ingroup GA03
     * @brief Sets AFCRL Threshold
     * @details 0 = Channel space/2; 1 = 25kHz
     * @param value 0=Channel space/2; 1 = 25kHz
     */
    inline void setAfcThreshold(uint8_t value)
    {
        reg08->refined.SEL25K =  value;
        setRegister(REG08, reg08->raw);
    }

    /**
     * @ingroup GA03
     * @brief Sets AFC Average
     * @details 0 = Use the instant AFC value; 1 = Use the average AFC value
     * @details AFC Average refers to the averaging of the Automatic Frequency Control (AFC) signal over a certain period of time. 
     * @details AFC Average is used to mitigate short-term fluctuations or noise in the received signal's frequency. 
     * @param value 0 or 1
     */
    inline void setAfcAve(uint8_t value)
    {
        reg08->refined.AVE =  value;
        setRegister(REG08, reg08->raw);
    }

    /**
     * @ingroup GA03
     * @brief Sets Variation Threshold for average AFC calculation
     * @details 0 = Disable; 1 = the most strict; 2= ?; 3 = the loosest
     * @param value 0, 1, 2 or 3
     */
    inline void setAfcVar(uint8_t value)
    {
        reg08->refined.VAR =  value;
        setRegister(REG08, reg08->raw);
    }

    /**
     * @brief Sets AFC range
     * @details 0 = the most strict; 3 = the loosest
     * @param value 0,1,2 or 3
     */
    inline void setAfcRange(uint8_t value)
    {
        reg08->refined.RANGE =  value;
        setRegister(REG08, reg08->raw);
    }

    /**
     * @ingroup GA03
     * @brief Sets RSSI Threshold for Instant AFC updating
     * @details default value is 16 (0x10)
     * @param value 
     */
    inline void setAfcRssiThreshold(uint8_t value)
    {
        reg08->refined.AFCRSSIT =  value;
        setRegister(REG08, reg08->raw);
    }



    void setFrequency(uint16_t frequency);
    void setFrequencyUp();
    void setFrequencyDown();
    uint16_t getFrequency();
    uint16_t getChannel();
    uint16_t getRealFrequency();
    uint16_t getRealChannel();
    void setChannel(uint16_t channel);
    void seekHardware(uint8_t seek_mode, uint8_t direction, void (*showFunc)() = NULL);
    void seekSoftware(uint8_t seek_mode, uint8_t direction, void (*showFunc)() = NULL);
    // Alias to seekSoftware.
    inline void seek(uint8_t seek_mode, uint8_t direction, void (*showFunc)() = NULL) 
    {
         this->seekHardware(seek_mode, direction, showFunc );
    };
    void setSeekThreshold(uint8_t rssiValue, uint8_t snrValue);

    void setBand(uint8_t band = 1);
    void setSpace(uint8_t space = 0);
    int getRssi();
    int getSnr();

    void setSoftMute(bool value);
    void setSoftMuteAttack(uint8_t value);
    void setSoftMuteAttenuation(uint8_t value);
    void setMuteThreshold(uint8_t rssi, uint8_t snr);
    void setSeekMute(bool value); 
    void setAfcMute(bool value); 
    
    void setMono(bool value);

    bool isStereo();

    void setAudioMute(bool left, bool right);
    void setAudioMute(bool value);
    void setVolume(uint8_t value);
    uint8_t getVolume();
    void setVolumeUp();
    void setVolumeDown();

    void setRdsMode(uint8_t rds_mode = 0);
    void setRds(bool value, bool interrupt_enable = false);
    inline void setRDS(bool value, bool interrupt_enable = false ) { setRds(value, interrupt_enable); };
    bool getRdsReady();

    uint8_t getRdsFlagAB(void);
    uint8_t getRdsVersionCode(void);
    uint16_t getRdsGroupType();
    uint8_t getRdsProgramType(void);
    void getNext2Block(char *c);
    void getNext4Block(char *c);
    char *getRdsText(void);
    char *getRdsText0A(void);
    char *getRdsText2A(void);
    char *getRdsText2B(void);
    bool getRdsAllData(char **stationName, char **stationInformation, char **programInformation, char **utcTime);
    char *getRdsTime();
    char *getRdsLocalTime();
    bool getRdsSync();

    /**
     * @ingroup @ingroup GA04
     * @brief Gets the Program Information (RT - Radio Text)
     * @details Process the program information data. Same getRdsText2A(). It is a alias for getRdsText2A.
     * @details ATTENTION: You must call getRdsReady before calling this function. 
     * @return char array with the program information (63 bytes)
     * @see getRdsText2A
     */
    inline char *getRdsProgramInformation(void) { return getRdsText2A(); };

     /**
     * @ingroup GA04
     * @brief Gets the Station Information.
     * @details ATTENTION: You must call getRdsReady before calling this function. 
     * @return char array with the Text of Station Information (33 bytes)
     * @see getRdsReady
     */
    inline char *getRdsStationInformation(void) { return getRdsText2B(); };   

    /**
     * @ingroup GA04
     * @brief Gets the Station Name
     * @details Alias for getRdsText0A
     * @details ATTENTION: You must call getRdsReady before calling this function. 
     * @return char* should return a string with the station name. However, some stations send other kind of messages
     * @see getRdsText0A
     */
    inline char *getRdsStationName(void) { return getRdsText0A(); };


    void clearRdsBuffer();

    int checkI2C(uint8_t *addressArray);
    void convertToChar(uint16_t value, char *strValue, uint8_t len, uint8_t dot, uint8_t separator, bool remove_leading_zeros = true);
    uint16_t *getRegisterValues();

    /**
     * @ingroup GA05 Format the Frequency
     * @brief Convert a numeric frequency to a formated string (char *) frequency
     *
     * @param uint16_t value  - A given frequency to be formated
     * @param char *strValue - Formated frequency (Exe: 103,90) - Array of char ( minimal 7 elements )
     * @param char decimalSeparator - the symbol that separates the decimal part (Exe: . or ,)
     * @return point of strValue
     */
    inline char *formatFrequency(uint16_t value, char *strValue, char decimalSeparator = ',', uint8_t decimalPosition = 3)
    {
        this->convertToChar(value, strValue, 5, decimalPosition, decimalSeparator, true);
        return strValue;
    };
    /**
     * @ingroup GA05 Format the Frequency
     * @brief Convert the current frequency to a formated string (char *) frequency
     * @details The current frequency is the latest setted frequency by setFrequency, seek, setFrequencyUp and setFrequencyDown.
     * @param char decimalSeparator - the symbol that separates the decimal part (Exe: . or ,)
     * @return point char string strFrequency (member variable)
     * @see setFrequency, seek, setFrequencyUp and setFrequencyDown
     */
    inline char *formatCurrentFrequency(char decimalSeparator = ',', uint8_t decimalPosition = 3)
    {
        this->convertToChar(this->currentFrequency, this->strFrequency, 5, decimalPosition, decimalSeparator, true);
        return this->strFrequency;
    };

};