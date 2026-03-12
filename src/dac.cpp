#include <dac.h>
#include <Wire.h>  // For I2C


//==============================================================================
//==============================================================================
DAC::DAC()
{

  startDAC();

}

//------------------------------------------------------------------------------
void DAC::startDAC(){

    //Power DAC  UP
    powerDACup();
    //Initialize DAC Register setting
    initializeDAC();
    //config DAC with switches
    configureDAC();
    //Set default software configuration
    setDefDacConfig();
    //set initial volume
    setVolume( dac_volume );
    setInput( input );


}


//------------------------------------------------------------------------------
// Returns a text description of an ErrorCode
// keep at 28 characters
char* DAC::dacErrorString(ERROR_CODE code)
{
    switch (code)
    {
    case No_Error:            return (char*)("No error.              ");
    case Wire_Trans_Error:    return (char*)("Wire begin error.      ");
    case Volume_Out_Of_Scope: return (char*)("Volume reached max/min.");
    default:                  return (char*)("Unknown Error.         ");
    }
}

//------------------------------------------------------------------------------
void DAC::powerDACup(){

  //Set Relays for output
  digitalWrite(RELAY_PIN_2, HIGH);
  digitalWrite(RELAY_PIN_1, HIGH);
  pinMode(RELAY_PIN_1, OUTPUT);
  pinMode(RELAY_PIN_2, OUTPUT);

  pinMode(RESET_PIN, OUTPUT);
  pinMode(MUX_PIN_S0, OUTPUT);
  pinMode(MUX_PIN_S1, OUTPUT);

  // keep DAC in reset
  digitalWrite(RESET_PIN,  LOW);

  // Start DAC, put Relays on High
  delay(650);
  digitalWrite(RELAY_PIN_1, LOW);
  delay(50);
  digitalWrite(RELAY_PIN_2, LOW);
  //wait in reset for 1 sec
  delay(1000);
  // bring DAC out of reset
  digitalWrite(RESET_PIN, HIGH);


}
//------------------------------------------------------------------------------
ERROR_CODE DAC::initializeDAC(){

  //start I2C bus
  // Wire.begin();
  // delay(1);
  //  if(!Wire.available() ){
  //    LOG ( dacErrorString( Wire_Trans_Error ) );
  //  }

  Wire.begin(/*sda=*/8, /*scl=*/18, /*frequency=*/400000);
  Wire.setClock(400000);
  delay(1);
  // quick sanity check - try to read from the expander address
  Wire.beginTransmission(PE_ADDRESS);
  uint8_t err = Wire.endTransmission();
  if (err != 0) {
      //LOG("I2C probe failed: %u", err);
      Serial.println("I2C probe failed:");
      Serial.println(err);
  }

	// configure the port expander
	writeRegister(PE_ADDRESS, PE_IOCONN, 0b00100000);	// we will only be sending one byte at a time
	delay(1);
	writeRegister(PE_ADDRESS, PE_GPPUA, 0b11111111); // enable all weak pull-ups
	writeRegister(PE_ADDRESS, PE_GPPUB, 0b11111111); // enable all weak pull-ups
	delay(10);

return No_Error;
}



//------------------------------------------------------------------------------
void DAC::setDefDacConfig(){

    //set default JE
    R13_JE_THD_COMP_CONFIG r13; 
  	r13.byte = R13_DEFAULT;
  	writeRegister(DAC_ADDRESS, 13, r13.byte);

  	R12_JE_DPLL_BW r12;
    //r12.byte = R12_DEFAULT;
  	r12.dpll_bw_serial = DPLL_MIDDLE;
  	r12.dpll_bw_dsd = DPLL_MIDDLE;
  	writeRegister(DAC_ADDRESS, 12, r12.byte);

   	R7_FILTER_BW_SYSTEM_MUTE r7;
  	r7.byte = R7_DEFAULT;
  	writeRegister(DAC_ADDRESS, 7, r7.byte);

}

//------------------------------------------------------------------------------
// read the port expander switch states
void DAC::readSwitchStates() {
	uint8_t s1 = readRegister(PE_ADDRESS, PE_GPIOA);
	uint8_t s2 = readRegister(PE_ADDRESS, PE_GPIOB);
	if (s1 != sw1.byte || s2 != sw2.byte) {
		sw1.byte = s1;
		sw2.byte = s2;
	}
}


//------------------------------------------------------------------------------
ERROR_CODE DAC::configureDAC(){

  //Read Switch states
  readSwitchStates();

  // configurable stuff:
  // comment block to configure based on software and not hardware sw

    //Input from software! uncomment to get input from switch!
  	// R1_INPUT_SELECTION r1;
  	// r1.byte = R1_DEFAULT;
  	// if (sw1.input_mode == INPUT_MODE_SERIAL) {
  	// 	r1.auto_select = R1_AUTO_SELECT_DSD_SERIAL;
  	// 	r1.input_select = R1_INPUT_SELECT_SERIAL;
  	// } else {
  	// 	r1.auto_select = R1_AUTO_SELECT_DISABLE;
  	// 	r1.input_select = R1_INPUT_SELECT_SPDIF;
  	// }
  	// writeRegister(DAC_ADDRESS, 1, r1.byte);



    R2_SERIAL_DATA_AUTOMUTE_CONFIG r2;
  	r2.byte = R2_DEFAULT;
  	if (sw2.automute_enable) r2.automute_config = R2_AUTOMUTE_CONFIG_MUTE;
    switch(sw1.serial_format) {
  		case 0 :
  			r2.serial_mode = R2_SERIAL_MODE_I2S;
  			break;
  		case 1 :
  			r2.serial_mode = R2_SERIAL_MODE_LJ;
  			break;
  		case 2 :
  			r2.serial_mode = R2_SERIAL_MODE_I2S;
  			break;
  		default :
  			r2.serial_mode = R2_SERIAL_MODE_RJ;
  	}

  	switch(sw1.serial_length) {
  		case 0:
  			r2.serial_length = R2_SERIAL_LENGTH_32;
  			r2.serial_bits = R2_SERIAL_BITS_32;
  			break;
  		case 1:
  			r2.serial_length = R2_SERIAL_LENGTH_24;
  			r2.serial_bits = R2_SERIAL_BITS_24;
  			break;
  		case 2:
  			r2.serial_length = R2_SERIAL_LENGTH_16;
  			r2.serial_bits = R2_SERIAL_BITS_16;
  			break;
  		default:
  			r2.serial_length = R2_SERIAL_LENGTH_32;
  			r2.serial_bits = R2_SERIAL_BITS_32;
  	}
  	writeRegister(DAC_ADDRESS, 2, r2.byte);

  	R4_AUTOMUTE_TIME r4;
  	r4.byte = R4_DEFAULT;
  	if (sw2.automute_enable) r4.automute_time = 4;
  	writeRegister(DAC_ADDRESS, 4, r4.byte);

  	// R7_FILTER_BW_SYSTEM_MUTE r7;
  	// r7.byte = R7_DEFAULT;
  	// r7.filter_shape = sw1.filter;
  	// r7.iir_bw = sw2.iir_bw;
  	// writeRegister(DAC_ADDRESS, 7, r7.byte);

  	R8_GPIO_1_2_CONFIG r8;
  	r8.byte = R8_DEFAULT;
  	r8.gpio1_cfg = GPIO_OUT_AUTOMUTE_STATUS;
  	r8.gpio2_cfg = GPIO_IN_STANDARD_INPUT;
  	writeRegister(DAC_ADDRESS, 8, r8.byte);

  	R9_GPIO_3_4_CONFIG r9;
  	r9.byte = R9_DEFAULT;
  	r9.gpio3_cfg = GPIO_IN_STANDARD_INPUT;
  	r9.gpio4_cfg = GPIO_OUT_LOCK_STATUS;
  	writeRegister(DAC_ADDRESS, 9, r9.byte);

  	R11_SPDIF_MUX_GPIO_INVERT r11;
  	r11.byte = R11_DEFAULT;
  	r11.spdif_sel = 3;
  	writeRegister(DAC_ADDRESS, 11, r11.byte);

  	// R12_JE_DPLL_BW r12;
  	// uint8_t dpll = sw2.dpll;
  	// // we don't ever want the DPLL to be off
  	// if (dpll == 0) dpll = 5;
  	// r12.byte = R12_DEFAULT;
  	// r12.dpll_bw_serial = dpll;
  	// r12.dpll_bw_dsd = dpll;
  	// writeRegister(DAC_ADDRESS, 12, r12.byte);

    //Input dependent
  	// R13_JE_THD_COMP_CONFIG r13; 
  	// r13.byte = R13_DEFAULT;
  	// writeRegister(DAC_ADDRESS, 13, r13.byte);

  	R15_GPIO_INPUT_SEL_VOLUME_CONFIG r15;
  	r15.byte = R15_DEFAULT;
  	r15.stereo_mode = R15_STEREO_MODE_ENABLED;
  	r15.use_only_ch1_vol = R15_USE_ONLY_CH1_VOLUME_ENABLED;
  	r15.latch_vol = R15_LATCH_VOLUME_ENABLED;
  	writeRegister(DAC_ADDRESS, 15, r15.byte);

  	R37_PROGRAMMABLE_FIR_CONFIG r37;
  	r37.byte = R37_DEFAULT;
  	if (sw2.osf_bypass) {
  		r37.bypass_osf = R37_OSF_DISABLED;
  	} else {
  		r37.bypass_osf = R37_OSF_ENABLED;
  	}
  	writeRegister(DAC_ADDRESS, 37, r37.byte);


return No_Error;
}

//------------------------------------------------------------------------------
byte DAC::getVolume(){
  return dac_volume;
}
//------------------------------------------------------------------------------
DAC_INPUT DAC::getInput(){
  return input;
}

//------------------------------------------------------------------------------
LOCK_STATUS DAC::getLockStatus(){

  R64_CHIP_ID_STATUS r64;
  r64.byte = readRegister(DAC_ADDRESS, 64);
  if ( r64.lock_status == 1 ){

    //Read signal type
    R100_INPUT_STATUS r100;
    r100.byte = readRegister(DAC_ADDRESS, 100);
    if( r100.dsd_is_valid == 1 ){
      return LOCK_STATUS::Locked_DSD;
    }else if ( r100.i2s_is_valid == 1 ) {
      return LOCK_STATUS::Locked_I2S;
    }else if ( r100.spdif_is_valid == 1 ) {
      return LOCK_STATUS::Locked_SPDIF;
    }else if ( r100.dop_is_valid == 1 ) {
      return LOCK_STATUS::Locked_DOP;
    }else{
      return LOCK_STATUS::Locked_Unknown;
    }
    
  }else{
    return LOCK_STATUS::No_Lock;
  }

}

//------------------------------------------------------------------------------
// Returns a text description of an ErrorCode
// keep at 28 characters
char* DAC::dacLockString(LOCK_STATUS lock)
{
    //keep the strings equal so no need to cler screen
    switch (lock)
    {
    case Locked_Unknown:  return (char*)("Unknown ");
    case Locked_DSD:      return (char*)("DSD     ");
    case Locked_I2S:      return (char*)("I2S     ");
    case Locked_SPDIF:    return (char*)("SPDIF   ");
    case Locked_DOP:      return (char*)("DOP     ");
    case No_Lock:         return (char*)("No Lock ");
    default:              return (char*)("        ");
    }
  
}


//------------------------------------------------------------------------------
uint32_t DAC::getRawSampleRate(){

  // Registers 66-69 are read only and contain the 32bit DPLL number
  // FSR = (dpll_number * fmck) / 4294967296
  
  volatile unsigned long DPLLNum = 0; // Variable to hold DPLL value
  uint8_t MCLK = 10; //MHz Clock Hardware Fixed for Buffallo 3 ES9038pro  // Value of Clock used (in 10s of MHz). 10 = 100MHz.

  // Register 69-66 : DPLL Number
  // Reading the 4 registers of DPLL one byte at a time and stuffing into a single 32-bit number
  DPLLNum |= readRegister(DAC_ADDRESS, 69); //0x45
  DPLLNum <<= 8;
  DPLLNum |= readRegister(DAC_ADDRESS, 68); //0x44
  DPLLNum <<= 8;
  DPLLNum |= readRegister(DAC_ADDRESS, 67); //0x43
  DPLLNum <<= 8;
  DPLLNum |= readRegister(DAC_ADDRESS, 66); //0x42

  uint32_t fsr = ( DPLLNum * MCLK ) / 42940; //2^32 = 4 294 967 296 / Mega = 4 295 

 return fsr;
}

//------------------------------------------------------------------------------
char* DAC::getSampleRateString(uint32_t fsr)
{

  R100_INPUT_STATUS r100;
  r100.byte = readRegister(DAC_ADDRESS, 100);

  //keep the strings equal so no need to cler screen
  if( r100.dop_is_valid == 1 || r100.dsd_is_valid == 1)
  {
           if(fsr > 461520) return (char*)"Invalid DSD ";
      else if(fsr > 451000) return (char*)"44.8 MHz DSD";
      else if(fsr > 225700) return (char*)"22.4 MHz DSD";              
      else if(fsr > 112000) return (char*)"11.2 MHz DSD";
      else if(fsr > 56000)  return (char*)"5.6 MHz DSD ";
      else if(fsr > 28000)  return (char*)"2.8 MHz DSD ";
      else if(fsr > 1700)   return (char*)"2.8 MHz DSD ";
      else                  return (char*)"Invalid DSD "; 
  }
  else if (r100.spdif_is_valid == 1 || r100.i2s_is_valid == 1)   
  {
           if(fsr > 7690) return (char*)"Invalid SR  ";
      else if(fsr > 7675) return (char*)"768K PCM    ";
      else if(fsr > 7050) return (char*)"705.6K PCM  ";
      else if(fsr > 3835) return (char*)"384K PCM    ";
      else if(fsr > 3510) return (char*)"352.8K PCM  ";
      else if(fsr > 1910) return (char*)"192K PCM    ";
      else if(fsr > 1756) return (char*)"176.4K PCM  ";
      else if(fsr > 954)  return (char*)"96K PCM     ";
      else if(fsr > 878)  return (char*)"88.2K PCM   ";
      else if(fsr > 475)  return (char*)"48K PCM     ";
      else if(fsr > 430)  return (char*)"44.1K PCM   ";
      else if(fsr > 310)  return (char*)"32K PCM     ";
      else                return (char*)"Invalid SR  ";     
  }

  return (char*)"            ";
}

//------------------------------------------------------------------------------
DAC_INPUT DAC::increaseInput(){
  switch( input ){
    case USB:   input = OPT1;  break;
    case OPT1:  input = OPT2;  break;
    case OPT2:  input = SPDIF; break;
    case SPDIF: input = USB;   break;
  }
  setInput( input );

return input;
}

//------------------------------------------------------------------------------
DAC_INPUT DAC::decreaseInput(){

  switch( input ){
    case USB:   input = SPDIF; break;
    case OPT1:  input = USB;   break;
    case OPT2:  input = OPT1;  break;
    case SPDIF: input = OPT2;  break;
  }
  setInput( input );

  return input;
}
//------------------------------------------------------------------------------
ERROR_CODE DAC::setInput( DAC_INPUT input ){
  /*
  //Settings for input switch
  * A=B1 -> S0=LOW,  S1=LOW  (MUX_PIN_S0 = HIGH, MUX_PIN_S1 = HIGH), TOS1
  * A=B2 -> S0=HIGH, S1=LOW  (MUX_PIN_S0 = LOW,  MUX_PIN_S1 = HIGH), TOS2
  * A=B3 -> S0=LOW,  S1=HIGH (MUX_PIN_S0 = HIGH, MUX_PIN_S1 = LOW),  SPDIF
  * A=B4 -> S0=HIGH, S1=HIGH (MUX_PIN_S0 = LOW,  MUX_PIN_S1 = LOW),
  */

  R1_INPUT_SELECTION r1;
  r1.byte = R1_DEFAULT;

  switch( input ){
    case USB:
        //MUX S0=HIGH, S1=HIGH -> B4, NONE
        digitalWrite(MUX_PIN_S0, LOW);
        digitalWrite(MUX_PIN_S1, LOW);
        //DAC input settings
        r1.auto_select = R1_AUTO_SELECT_DSD_SERIAL;
        r1.input_select = R1_INPUT_SELECT_SERIAL;
      break;
    case OPT1:
        //MUX S0=LOW, S1=LOW -> B1, TOS1
        digitalWrite(MUX_PIN_S0, HIGH);
        digitalWrite(MUX_PIN_S1, HIGH);
        //DAC input settings
        r1.auto_select = R1_AUTO_SELECT_DISABLE;
        r1.input_select = R1_INPUT_SELECT_SPDIF;
      break;
    case OPT2:
        //MUX S0=HIGH, S1=LOW -> B2, TOS2
        digitalWrite(MUX_PIN_S0, LOW);
        digitalWrite(MUX_PIN_S1, HIGH);
        //DAC input settings
        r1.auto_select = R1_AUTO_SELECT_DISABLE;
        r1.input_select = R1_INPUT_SELECT_SPDIF;
      break;
    case SPDIF:
        //MUX S0=LOW, S1=HIGH -> B3, SPDIF
        digitalWrite(MUX_PIN_S0, HIGH);
        digitalWrite(MUX_PIN_S1, LOW);
        //DAC input settings
        r1.auto_select = R1_AUTO_SELECT_DISABLE;
        r1.input_select = R1_INPUT_SELECT_SPDIF;
      break;
  }


  writeRegister(DAC_ADDRESS, 1, r1.byte);


return No_Error;
}

//------------------------------------------------------------------------------
uint8_t DAC::increaseVolume(){

  uint8_t vol = dac_volume;

  if (vol < MAX_VOL)              // Check if not already at maximum attenuation
  {
    if(dimmed) {
    setVolume(vol);
    dimmed = false;
    }
  vol += 1;                 // Increase 1 dB attenuation (decrease dac_volume 1 db)
  setVolume( vol );         // Write value into registers
  }


return dac_volume;
}

//------------------------------------------------------------------------------
uint8_t DAC::decreaseVolume(){

  uint8_t vol = dac_volume;
  if ( vol > MIN_VOL )                // Check if not already at minimum attenuation
  {
      if( dimmed )
      {
      setVolume( vol );
      dimmed = false;
      }

    vol -= 1;     // Decrease attenuation 1 dB
    setVolume( vol );  // try to set dac_volume, if set correctly dac::dac_volume will be changed
  }

return dac_volume;
}


//------------------------------------------------------------------------------
byte DAC::muteVolume(){

uint8_t vol = dac_volume;

    if( dimmed ){                // Back to previous dac_volume level 1 db at a time
      setVolume( vol );          // Write value into registers
      dimmed = false;
    } else {
      if( vol > MUTE_VOL ) {            // only DIM if current attenuation is lower
        setVolume( MUTE_VOL );          // Write value into registers
        dimmed = true;
        byte retVol = dac_volume;       //save actual mute volume
        dac_volume = vol;               //set dac volume to last value, for restoring
        return retVol;                  //return mute volume
      }
    }

return dac_volume;
}

//------------------------------------------------------------------------------
ERROR_CODE DAC::setVolume( uint8_t vol ){

if( vol < MIN_VOL || vol > MAX_VOL ){
  LOG ( dacErrorString( Volume_Out_Of_Scope ) );
  return Volume_Out_Of_Scope;
}

  uint8_t vol_dB = 99 - vol; //possible 0 to -127dB , vol (0-99) 0 MAX, 255 min
  vol_dB = vol_dB * 2;      //increase scale from 0 - 198, step x2, 0,5dB x 2 = 1dB
  //write volume byte to register 16
  writeRegister( DAC_ADDRESS, 16, vol_dB );
  dac_volume = vol;

return No_Error; //readRegister(16);
}



//------------------------------------------------------------------------------
uint8_t DAC::getFIRShape(){

  R7_FILTER_BW_SYSTEM_MUTE r7;
  r7.byte = readRegister(DAC_ADDRESS, 7);

return  r7.filter_shape;
}

//------------------------------------------------------------------------------
uint8_t DAC::getCycleFIRShape(){
  R7_FILTER_BW_SYSTEM_MUTE r7 = cycleFIRShape();
return r7.filter_shape;  
}

//------------------------------------------------------------------------------
R7_FILTER_BW_SYSTEM_MUTE DAC::cycleFIRShape(){

  R7_FILTER_BW_SYSTEM_MUTE r7;
  r7.byte = readRegister(DAC_ADDRESS, 7);
  uint8_t fir_shape = R7_F_SHAPE_FAST_RO_MINIMUM_PHASE; //default
  switch ( r7.filter_shape )
  {
    case R7_F_SHAPE_BRICKWALL:
      fir_shape = R7_F_SHAPE_HYBRID_FAST_RO_MIN_PHASE; 
    break;
    case R7_F_SHAPE_HYBRID_FAST_RO_MIN_PHASE:
      fir_shape = R7_F_SHAPE_APODIZING_FAST_RO_LINEAR_PHASE;
    break;
    case R7_F_SHAPE_APODIZING_FAST_RO_LINEAR_PHASE:
      fir_shape = R7_F_SHAPE_SLOW_RO_MINIMUM_PHASE;
    break;
    case R7_F_SHAPE_SLOW_RO_MINIMUM_PHASE:
      fir_shape = R7_F_SHAPE_FAST_RO_MINIMUM_PHASE;
    break;
    case R7_F_SHAPE_FAST_RO_MINIMUM_PHASE:
      fir_shape = R7_F_SHAPE_SLOW_RO_LINEAR_PHASE;
    break;
    case R7_F_SHAPE_SLOW_RO_LINEAR_PHASE:
      fir_shape = R7_F_SHAPE_FAST_RO_LINEAR_PHASE;
    break;
    case R7_F_SHAPE_FAST_RO_LINEAR_PHASE:
      fir_shape = R7_F_SHAPE_BRICKWALL;
    break;
    default:
      fir_shape = R7_F_SHAPE_FAST_RO_MINIMUM_PHASE; //default
    break;
  }

  r7.filter_shape = fir_shape; 

  writeRegister(DAC_ADDRESS, 7, r7.byte);

  //read the actual values back from register 
  r7.byte = readRegister(DAC_ADDRESS, 7);

return r7;

}
//------------------------------------------------------------------------------
char* DAC::getFIRShapeString(uint8_t value){

    switch (value)
    {
    case R7_F_SHAPE_BRICKWALL:                        return (char*)("Brickwall          ");
    case R7_F_SHAPE_HYBRID_FAST_RO_MIN_PHASE:         return (char*)("Hybr fast ro min ph");
    case R7_F_SHAPE_APODIZING_FAST_RO_LINEAR_PHASE:   return (char*)("Apod fast ro lin ph");
    case R7_F_SHAPE_SLOW_RO_MINIMUM_PHASE:            return (char*)("Slow roll min phase");
    case R7_F_SHAPE_FAST_RO_MINIMUM_PHASE:            return (char*)("Fast roll min phase");
    case R7_F_SHAPE_SLOW_RO_LINEAR_PHASE:             return (char*)("Slow roll lin phase");
    case R7_F_SHAPE_FAST_RO_LINEAR_PHASE:             return (char*)("Fast roll lin phase");
    default:                                          return (char*)("");
    }


}

//------------------------------------------------------------------------------
uint8_t DAC::getIIRBandwidth(){

  R7_FILTER_BW_SYSTEM_MUTE r7;
  r7.byte = readRegister(DAC_ADDRESS, 7);

return  r7.iir_bw;
}

//------------------------------------------------------------------------------
uint8_t DAC::getCycleIIRBandwidth(){
  R7_FILTER_BW_SYSTEM_MUTE r7 = cycleIIRBandwidth();
return r7.iir_bw;  
}
//------------------------------------------------------------------------------
R7_FILTER_BW_SYSTEM_MUTE DAC::cycleIIRBandwidth(){

  R7_FILTER_BW_SYSTEM_MUTE r7;
  r7.byte = readRegister(DAC_ADDRESS, 7);
  uint8_t iir_width = R7_IIR_BW_47K; //default
  switch ( r7.iir_bw )
  {
    case R7_IIR_BW_47K:
      iir_width = R7_IIR_BW_50K; 
    break;
    case R7_IIR_BW_50K:
      iir_width = R7_IIR_BW_60K;
    break;
    case R7_IIR_BW_60K:
      iir_width = R7_IIR_BW_70K;
    break;
    case R7_IIR_BW_70K:
      iir_width = R7_IIR_BW_47K;
    break;
    default:
      iir_width = R7_IIR_BW_47K; //default
    break;
  }

  r7.iir_bw = iir_width; 

  writeRegister(DAC_ADDRESS, 7, r7.byte);

  //read the actual values back from register 
  r7.byte = readRegister(DAC_ADDRESS, 7);

return r7;
}

//------------------------------------------------------------------------------
char* DAC::getIIRBandwidthString(uint8_t value){

    switch (value)
    {
    case R7_IIR_BW_47K:  return (char*)("47K (PCM)");
    case R7_IIR_BW_50K:  return (char*)("50K (DSD)");
    case R7_IIR_BW_60K:  return (char*)("60K (DSD)");
    case R7_IIR_BW_70K:  return (char*)("70K (DSD)");
    default:             return (char*)("");
    }

}


//------------------------------------------------------------------------------
uint8_t DAC::getDpllSerial()
{
  R12_JE_DPLL_BW r12;
  r12.byte = readRegister(DAC_ADDRESS, 12);

return  r12.dpll_bw_serial;
}

//------------------------------------------------------------------------------
uint8_t DAC::getCycleDPLL(){
  R12_JE_DPLL_BW r12 = cycleDPLL();
return r12.dpll_bw_serial;  
}

//------------------------------------------------------------------------------
R12_JE_DPLL_BW DAC::cycleDPLL()
{

  R12_JE_DPLL_BW r12;
  r12.byte = readRegister(DAC_ADDRESS, 12);
  // we don't ever want the DPLL to be off
  uint8_t dpll = DPLL_LOW;
  switch ( r12.dpll_bw_serial )
  {
    case DPLL_VERY_LOW:
      dpll = DPLL_LOW;
    break;
    case DPLL_LOW:
      dpll = DPLL_MIDDLE;
    break;
    case DPLL_MIDDLE:
      dpll = DPLL_HIGH;
    break;
    case DPLL_HIGH:
      dpll = DPLL_VERY_HIGH;
    break;
    case DPLL_VERY_HIGH:
      dpll = DPLL_VERY_LOW;
    break;
    default:
      dpll = DPLL_LOW;
    break;
  }


  r12.dpll_bw_serial = dpll;
  r12.dpll_bw_dsd = dpll;

  writeRegister(DAC_ADDRESS, 12, r12.byte);

  //read the actual values from register 
  r12.byte = readRegister(DAC_ADDRESS, 12);

return r12;
}
//------------------------------------------------------------------------------
char* DAC::getDpllSerialString(uint8_t value){

    //keep the strings equal so no need to cler screen
    switch (value)
    {
    case DPLL_VERY_LOW:   return (char*)("DPLL Very Low ");
    case DPLL_LOW:        return (char*)("DPLL Low      ");
    case DPLL_MIDDLE:     return (char*)("DPLL Middle   ");
    case DPLL_HIGH:       return (char*)("DPLL High     ");
    case DPLL_VERY_HIGH:  return (char*)("DPLL Very High");
    default:              return (char*)("              ");
    }

}

//------------------------------------------------------------------------------
uint8_t DAC::getJitterEl()
{
   	R13_JE_THD_COMP_CONFIG r13;
    r13.byte = readRegister(DAC_ADDRESS, 13);

return r13.jitter_eliminator_enable;
}
//------------------------------------------------------------------------------
uint8_t DAC::getToggleJitterEliminator()
{
  R13_JE_THD_COMP_CONFIG r13 = toggleJitterEliminator();
return r13.jitter_eliminator_enable;
}

//------------------------------------------------------------------------------
R13_JE_THD_COMP_CONFIG DAC::toggleJitterEliminator()
{

 	R13_JE_THD_COMP_CONFIG r13;
  r13.byte = readRegister(DAC_ADDRESS, 13);
  if( r13.jitter_eliminator_enable == R13_JITTER_ELIMINATOR_ENABLED ){
    r13.jitter_eliminator_enable = R13_JITTER_ELIMINATOR_DISABLED; //Turn OFF jitter eliminator to improve lockout, Lower Quality
  }	else {
    r13.jitter_eliminator_enable = R13_JITTER_ELIMINATOR_ENABLED;
  }
  writeRegister(DAC_ADDRESS, 13, r13.byte); //Jitter Eliminator ON / OFF 

  //read the actual values from register 
  r13.byte = readRegister(DAC_ADDRESS, 13);

return r13;
};

//------------------------------------------------------------------------------
char* DAC::getJitterElString(uint8_t value){

    //keep the strings equal so no need to cler screen
    switch (value)
    {
    case 0:   return (char*)("Disabled");
    case 1:   return (char*)("Enabled ");
    default:  return (char*)("        ");
    }
}

//------------------------------------------------------------------------------
ERROR_CODE DAC::writeRegister( int device, byte regAddr, byte dataVal){
  //for device address get DAC adress
  Wire.beginTransmission( device ); // device
  Wire.write(regAddr); // register
  Wire.write(dataVal); // data
  Wire.endTransmission();

return No_Error;
}

//------------------------------------------------------------------------------
byte DAC::readRegister( int device, byte regAddr ) {

  Wire.beginTransmission( device ); // Hard coded the Sabre/Buffalo device  address
  Wire.write(regAddr);          // Queues the address of the register
  Wire.endTransmission();       // Sends the address of the register
  Wire.requestFrom( device, 1);     // Hard coded to Buffalo, request one byte from address
                                // specified with Wire.write()/wire.endTransmission()
  //while(!Wire.available()) {  // Wait for byte to be available on the bus
  if( !Wire.available() ){          // Wire.available indicates if data is available
    return 0;
  }


return  Wire.read();                   //  return byte
}

