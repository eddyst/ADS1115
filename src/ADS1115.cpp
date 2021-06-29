/**************************************************************************/
/*!
Adapted from adafruit ADS1015/ADS1115 library
    v1.0 - First release
*/
/**************************************************************************/

#include "ADS1115.h"

/**************************************************************************/
/*!
    @brief  Instantiates a new ADS1115 class w/appropriate properties
*/
/**************************************************************************/
ADS1115::ADS1115() {
	Wire.begin();
	_voltageRange = ADS1115_voltageRange_6_144V; /* +/- 6.144V range (limited to VDD +0.3V max!) */
	_rate = ADS1115_128_SPS; /* to default */
  _ConversionState = ADS1115_State_NothingToDo;
}

uint8_t ADS1115::testConnection() {
	uint8_t r=0;
	for (uint16_t a = 0x48; a <= 0x4B; a++) {
    Wire.beginTransmission(a);
    Wire.write(ADS1115_REG_POINTER_CONVERT);
    Wire.endTransmission();
    Wire.requestFrom(a, (uint8_t)2);
    if (Wire.available()) {
      r=(r>>1)+1;
    } else {
      r=(r>>1)+0;
    }
	}
	return r;
}

void ADS1115::setVoltageRange(ADS1115_RANGE voltageRange) {
  _voltageRange = voltageRange;
}

void ADS1115::setSampleRate(ADS1115_CONV_RATE rate) {
	_rate = rate;
}

void ADS1115::triggerConversion(uint8_t Port) {
	_i2cAddress = 0x48 + (Port / 4);
  _mux = (4 + (Port % 4)) << 12;
	//Bits 0 through 4 deal with the comparator function.  The combined default value for these bits is 0x0003
	uint16_t config = 0x0003; // Disable the comparator (default val)
	// OR in the Data rate or Sample Per Seconds bits 5 through 7
	config |= _rate;
	// OR in the mode bit 8
	config |= ADS1115_REG_CONFIG_MODE_SINGLE; // Single-shot mode (default) 
	// OR in the PGA/voltage range bits 9 through 11
	config |= _voltageRange;
	// OR in the mux channel, bits 12 through 14
	config |= _mux;
	// OR in the start conversion bit bit 15
	config |= ADS1115_REG_CONFIG_OS_SINGLE;

	// Write config register to the ADC
	Wire.beginTransmission(_i2cAddress);
	Wire.write(ADS1115_REG_POINTER_CONFIG);
	Wire.write((uint8_t)(config>>8));
	Wire.write((uint8_t)(config & 0xFF));
	Wire.endTransmission();
  _ConversionState = ADS1115_State_Wait;
}

ADS1115_CONV_STATE ADS1115::ConversionState() {
  switch (_ConversionState) {
  case ADS1115_State_Wait:
    Wire.beginTransmission(_i2cAddress); //Sets the Address of the ADS1115.
    Wire.write(ADS1115_REG_POINTER_CONFIG); //queue the data to be sent, in this case modify the pointer register so that the following RequestFrom reads the config register
    Wire.endTransmission(); //Set the stop bit
    Wire.requestFrom(_i2cAddress, (uint8_t)2); //Request 2 byte config register
    if (((Wire.read() << 8) | Wire.read())>>15) {//Read 2 bytes.  Return the most signifagant bit
      _ConversionState=ADS1115_State_Ready;
    } else if (1==2) {
      _ConversionState=ADS1115_State_Timeout;
    }
  default:
    break;
  }
  return _ConversionState;
}

int16_t ADS1115::getConversion() {  // Wait for the conversion to complete
  do {} while(ConversionState()==ADS1115_State_Wait);
  if (_ConversionState==ADS1115_State_Ready) {
    _ConversionState=ADS1115_State_NothingToDo;
    // Read the conversion results
    Wire.beginTransmission(_i2cAddress); //Sets the Address of the ADS1115.
    Wire.write(ADS1115_REG_POINTER_CONVERT); //queue the data to be sent, in this case modify the pointer register so that the following RequestFrom reads the conversion register
    Wire.endTransmission(); //Send the data
    Wire.requestFrom(_i2cAddress, (uint8_t)2); //Request the 2 byte conversion register
    return ((Wire.read() << 8) | Wire.read()); //Read each byte.  Shift the first byte read 8 bits to the left and OR it with the second byte.
  } else {
    return -2;    
  }
}
