#ifndef DAC_H
#define DAC_H

#include <globals.h>
#include <Arduino.h>
#include <ES9028_38.h> //Buffalo config file
#include <stdint.h>


//------------------------------------------------------------------------------
#define DAC_ADDRESS 0x48    //0x90  device address for the Buffalo DAC chip
#define PE_ADDRESS 0x40     //Port expander for the switch states

//Work with scale 00-99 instead of dB (-255 to 0), calculate dB while setting dac volume
#define DEFAULT_VOL 50    //this is 50x2=100 or 0x64. Sabre32 is 0 to -127dB in .5dB steps
#define MAX_VOL 99       //this is 99x2=198 or 0xC6 -Can go higher but LCD shows 2 digits
#define MIN_VOL 0       //Minimum volume is -99 dB
#define MUTE_VOL 30      //-70 dB this is 70x2=142 or 78C. Set value to 30, 99-30 = -79dB


//DAC defines
#define PE_IOCONN           0x0A  // at least initially
#define PE_IPOLA            0x02
#define PE_IPOLB            0x03
#define PE_GPPUA            0x0C
#define PE_GPPUB            0x0D
#define PE_GPIOA            0x12
#define PE_GPIOB            0x13
// geared modes
#define GEARED_MODE_OFF         0
#define GEARED_MODE_ON          1
// input modes
#define INPUT_MODE_SPDIF        0
#define INPUT_MODE_SERIAL       1
// serial input formats
#define SERIAL_FORMAT_I2S       0
#define SERIAL_FORMAT_LJ        1
#define SERIAL_FORMAT_RJ        2
// serial data lengths
#define SERIAL_32_BIT           0
#define SERIAL_24_BIT           1
#define SERIAL_16_BIT           3
#define OUTPUT_MODE_STEREO      0
#define OUTPUT_MODE_MONO        1
// output normalization (auto calibration via ADC)
// when enabled output will be the same across modules at the cost of reduced dynamic range.
// for highest fidelity in stereo mode leave it off
#define OUTPUT_NORMALIZER_OFF   0
#define OUTPUT_NORMALIZER_ON    1

#define DPLL_VERY_LOW 1
#define DPLL_LOW 4
#define DPLL_MIDDLE 7
#define DPLL_HIGH 11
#define DPLL_VERY_HIGH 15

/**
 * MUX Truth Table:
 * A=B1 -> S0=LOW,  S1=LOW  (MUX_PIN_S0 = HIGH, MUX_PIN_S1 = HIGH)
 * A=B2 -> S0=HIGH, S1=LOW  (MUX_PIN_S0 = LOW,  MUX_PIN_S1 = HIGH)
 * A=B3 -> S0=LOW,  S1=HIGH (MUX_PIN_S0 = HIGH, MUX_PIN_S1 = LOW)
 * A=B4 -> S0=HIGH, S1=HIGH (MUX_PIN_S0 = LOW,  MUX_PIN_S1 = LOW)
 */

  enum LOCK_STATUS
  {
    No_Lock = 0,
    Locked_DSD,
    Locked_I2S,
    Locked_SPDIF,
    Locked_DOP,
    Locked_Unknown,
    Unknown
  };

  enum ERROR_CODE
  {
    No_Error = 0,
    Wire_Trans_Error,
    Volume_Out_Of_Scope
  };

//==============================================================================
//==============================================================================
class DAC{

  public:
    DAC();

    void startDAC();
    void setDefDacConfig();
    ERROR_CODE configureDAC();
    bool isAvailable() const { return _available; }
    bool checkAvailability();

    byte getVolume();
    byte increaseVolume();
    byte decreaseVolume();
    ERROR_CODE setVolume( uint8_t volume );
    byte muteVolume();

    DAC_INPUT getInput();
    DAC_INPUT increaseInput();
    DAC_INPUT decreaseInput();
    ERROR_CODE setInput( DAC_INPUT input );

    LOCK_STATUS getLockStatus();
    char* dacLockString(LOCK_STATUS);

    uint32_t getRawSampleRate();
    char* getSampleRateString(uint32_t fsr);

    char* dacErrorString(ERROR_CODE);

    uint8_t getFIRShape();
    uint8_t getCycleFIRShape();
    char*   getFIRShapeString(uint8_t value);

    uint8_t getIIRBandwidth();
    uint8_t getCycleIIRBandwidth();
    char*   getIIRBandwidthString(uint8_t value);

    uint8_t getDpllSerial();
    uint8_t getCycleDPLL();
    char*   getDpllSerialString(uint8_t value);

    uint8_t getJitterEl();
    uint8_t getToggleJitterEliminator();
    char*   getJitterElString(uint8_t value);



  private:

    typedef union {
        // the factory state for the switches is "on"
        // the switches close to GND so "on" is 0 (low) and "off" is 1 (high)
        struct {
            uint8_t input_mode    :1;
            uint8_t serial_format :2;
            uint8_t serial_length :2;
            // this maps directly to the DAC filter registers ON = 0 Off = 1
            uint8_t filter :3;
        };
        uint8_t byte;
    } SW1;

    typedef union {
        struct {
            uint8_t automute_enable :1;
            uint8_t osf_bypass      :1;
            uint8_t iir_bw          :2;
            uint8_t dpll            :4;
        };
        uint8_t byte;
    } SW2;


    // GPIO	BOARD_VIEWE_UEDX80480043E_WB_A label Status
    // 0	Boot, use with care

    // Remote conroller input
    // 17 RMOTE - 	    Not used	Free 

    //Inut slection on the dac
    // 10	MUX 0 INPUT -   SD-CS	Free if no SD card
    // 11	MUX 1 INPUT -   SD-DIN	Free if no SD card
    // 12	RELAY -  SD-CLK	Free if no SD card
    
    //Us for DAC control and switch states instead of SD card. 
    // 13 DAC RESET - 	  SD-DOUT	Free if no SD card



    const int MUX_PIN_S0 = 10;     //4:1 MUX PIN S0
    const int MUX_PIN_S1 = 11;     //4:1 MUX PIN S1
    const int RESET_PIN = 13;      //Reset pin
    //Because there is only one pin available for the relay, both will be driven b the same pin
    const int RELAY_PIN_1 = 12;      //8 the number of the Relay one
    //const int RELAY_PIN_2 = 12;      //9 the number of the Relay two

    SW1 sw1;
    SW2 sw2;

    DAC_INPUT input = OPT1;
    uint8_t dac_volume = DEFAULT_VOL;
    bool dimmed = false;
    bool _available = false;


    void powerDACup();
    void readSwitchStates();
    ERROR_CODE initializeDAC();
    ERROR_CODE writeRegister( int device, byte regAddr, byte dataVal );
    byte readRegister( int device, byte regAddr );
    
    R7_FILTER_BW_SYSTEM_MUTE cycleFIRShape();
    R7_FILTER_BW_SYSTEM_MUTE cycleIIRBandwidth();
    R12_JE_DPLL_BW           cycleDPLL();
    R13_JE_THD_COMP_CONFIG   toggleJitterEliminator();
    
};


#endif // DAC_H