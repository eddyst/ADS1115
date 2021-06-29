/*
ADS1115 library 
Adapted from 
  adafruit ADS1015/ADS1115 library: https://learn.adafruit.com/adafruit-4-channel-adc-breakouts
  https://github.com/terryjmyers/ADS1115-Lite
  https://wolles-elektronikkiste.de/ads1115
*/

#pragma once
#include <Arduino.h>

#include <Wire.h>

/*=========================================================================
    POINTER REGISTERS
    -----------------------------------------------------------------------*/
    #define ADS1115_REG_POINTER_CONVERT     (0x00)
    #define ADS1115_REG_POINTER_CONFIG      (0x01) 
/*=========================================================================*/

/*=========================================================================
    CONFIG REGISTER
    -----------------------------------------------------------------------*/
    #define ADS1115_REG_CONFIG_OS_SINGLE    (0x8000)  // Write: Set to start a single-conversion

	//voltageRange parameter.  Used as input to setvoltageRange()
  typedef enum ADS1115_RANGE{
    ADS1115_voltageRange_6_144V = 0x0000,  // +/-6.144V range = Gain 2/3
    ADS1115_voltageRange_4_096V = 0x0200,  // +/-4.096V range = Gain 1
    ADS1115_voltageRange_2_048V = 0x0400,  // +/-2.048V range = Gain 2 (default)
    ADS1115_voltageRange_1_024V = 0x0600,  // +/-1.024V range = Gain 4
    ADS1115_voltageRange_0_512V = 0x0800,  // +/-0.512V range = Gain 8
    ADS1115_voltageRange_0_256V = 0x0A00,  // +/-0.256V range = Gain 16
  } range;

  typedef enum ADS1115_MUX{
    ADS1115_COMP_0_1   = 0x0000,//      0000 0000 0000 0000
    ADS1115_COMP_0_3   = 0x1000,//      0001 0000 0000 0000
    ADS1115_COMP_1_3   = 0x2000,//      0010 0000 0000 0000
    ADS1115_COMP_2_3   = 0x3000,//      0011 0000 0000 0000
    ADS1115_COMP_0_GND = 0x4000,//16384 0100 0000 0000 0000
    ADS1115_COMP_1_GND = 0x5000,//20480 0101 0000 0000 0000
    ADS1115_COMP_2_GND = 0x6000,//24576 0110 0000 0000 0000
    ADS1115_COMP_3_GND = 0x7000 //28672 0111 0000 0000 0000
  } mux;
	//Single Shot or Continuous Mode.  Cont mode is not used in this program but is preserved here for posterity
    //#define ADS1115_REG_CONFIG_MODE_CONTIN  (0x0000)  // Continuous conversion mode
    #define ADS1115_REG_CONFIG_MODE_SINGLE  (0x0100)  // Power-down single-shot mode (default)

	//Sample Rate or Data Rate.  Used as input to setSampleRate().  Note
	//Note that there is some overhead time of the I2C bus so the actual throughput is slightly less
  typedef enum ADS1115_CONV_RATE{ 
    ADS1115_8_SPS	  = 0x0000,  //   8 SPS, or a sample every 125ms
    ADS1115_16_SPS	= 0x0020,  //  16 SPS, or every 62.5ms
    ADS1115_32_SPS	= 0x0040,  //  32 SPS, or every 31.3ms
    ADS1115_64_SPS	= 0x0050,  //  64 SPS, or every 15.6ms
    ADS1115_128_SPS	= 0x0080,  // 128 SPS, or every  7.8ms  (default)
    ADS1115_250_SPS	= 0x00A0,  // 250 SPS, or every  4  ms, note that noise free resolution is reduced to ~14.75-16bits, see table 2 in datasheet
    ADS1115_475_SPS	= 0x00C0,  // 475 SPS, or every  2.1ms, note that noise free resolution is reduced to ~14.3-15.5bits, see table 2 in datasheet
    ADS1115_860_SPS	= 0x00E0   // 860 SPS, or every  1.16ms, note that noise free resolution is reduced to ~13.8-15bits, see table 2 in datasheet
  } convRate;

  //Sample Rate or Data Rate.  Used as input to setSampleRate().  Note
	//Note that there is some overhead time of the I2C bus so the actual throughput is slightly less
  typedef enum ADS1115_CONV_STATE{ 
    ADS1115_State_NothingToDo	= 0, 
    ADS1115_State_Timeout     = 1,
    ADS1115_State_Wait        = 2,
    ADS1115_State_Ready     	= 3,
  } convState;
/*=========================================================================*/

class ADS1115 {
	protected:
		// Instance-specific properties
		uint8_t		         _i2cAddress;
    uint16_t           _mux;
		ADS1115_RANGE      _voltageRange;
		ADS1115_CONV_RATE  _rate;
    ADS1115_CONV_STATE _ConversionState;
	public:
		ADS1115();
    /*Sets the Configuration Register to default values. 
    *Returns Binary 1 if the ADS1115 is connected or 0 if it is not
    *  1111 = 15 = all connected
    *   101 = 10 = 0x48 + 0x4A is connected
    *     0 =  0 = nothing connected
    */
    uint8_t testConnection(); //returns 1 if ADS1115 is properly connected, 0 if not connected.  There is no point in calling any other function is this doesn't return a 1
		
    /*Gets the protected property for gain and input voltage range.  
     */
    ADS1115_RANGE getVoltageRange(){return _voltageRange;};
    /*Sets the protected property for gain and input voltage range.  
     *a too high gain will destroy the board. Max 0.3V over VCC is allowed
     */
    void setVoltageRange(ADS1115_RANGE voltageRange); 
    
    ADS1115_CONV_RATE getSampleRate(){return _rate;};
		void setSampleRate(ADS1115_CONV_RATE rate);

    /*Gets the I2cAddress for the current/last conversation.   
     */
    uint8_t getI2cAddress(){return _i2cAddress;};
    /*Gets the channel for the current/last conversation.  
     */
    uint16_t getMux(){return _mux;};

    /*Triggers a single conversion with currently configured settings in protected variables.  Immediately returns
		//Port  0- 3 = I2CAddress 0x48 (1001000) ADR -> GND  
    //Port  4- 7 = I2CAddress 0x49 (1001001) ADR -> VDD
    //Port  8-11 = I2CAddress 0x4A (1001010) ADR -> SDA
    //Port 12-15 = I2CAddress 0x4B (1001011) ADR -> SCL
     */
		void	triggerConversion(uint8_t Port); 

    //Polls ADS1115 for the conversion done register.  Returns 1 if conversion is finished, 0 if in the middle of conversion.
		ADS1115_CONV_STATE ConversionState(); 

		//Polls ADS1115 for the conversion done register.  Waits until it is finished and returns value.
    int16_t	getConversion(); 
};

