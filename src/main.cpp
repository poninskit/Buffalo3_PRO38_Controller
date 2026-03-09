//******************************************************************************
// ES 9038 Pro, Version see definition
// Poninski Tomasz, 2023-01-20
// https://github.com/twistedpearaudio/Buffalo-III-SE-Pro-On-Board-Firmware
// https://twistedpearaudio.github.io/blog/docs_b3sepro/manual.html
//******************************************************************************
//Arduino enviroment
#include <Arduino.h>
//Header for global declarations
#include "globals.h"
//header for all DAC classes
#include <dac.h>
#include <interfaces.h>
//#include <tftgraphics.h>

//------------------------------------------------------------------------------
#define READ_DAC_CYCLES   10000 //read the DAC Register every n-cycles, every 1s is enough
#define VERSION "v.1.1.3"
//------------------------------------------------------------------------------
// classes declarations

// DAC and EEPROM classes
DAC* dac;
//REMOTE CONTROLL
RemoteInterface* remoteInterface;
//TOUCH INTERFACE
TouchInterface* touchInterface;
//TFT SCREEN
//TFTGraphics* tftGraphics;


//------------------------------------------------------------------------------
//Variables
ACTION action = NONE;
ACTION lastAction = NONE;
int read_dac_counter = 0;
PAGE currentPage = MAIN_MENU;

//differentiate interfaces apply specific delay
INTERFACE interface = NONE_IFC;
//use delay if necessary to give user respond time
int delay_ms = 0;
//delay values
int delay_hold = 15;
int delay_switch = 400;
//dont draw sample rate if nothing changed
bool refreshSampleRate = true;
LOCK_STATUS last_lock = Unknown;
uint32_t last_fsr = 0;


//Initilize Settings Array
Settings settingsArr[] =  {
                          { FIR_FILTER_SHAPE   , "FIR Filter Shape"  , 0, "" },
                          { IIR_BANDWIDTH      , "IIR Bandwidth"     , 0, "" },
                          { DPLL_BANDWIDTH     , "DPLL Bandwidth"    , 0, "" },
                          { JITTER_ELIMINATOR  , "Jitter Eliminator" , 0, "" }
                          };

//******************************************************************************
// SETUP DAC
//******************************************************************************
void setup() {

  //Serial port for debugging
  if(DEBUG) Serial.begin(9600);
  LOG("Debug mode\n");

  //initilize classes
  dac = new DAC(); // Constructor powers up, sets the DAC I2C and default settings
  remoteInterface = new RemoteInterface();
  touchInterface =  new TouchInterface();
  //tftGraphics =     new TFTGraphics();

  settingsArr[0].value = dac->getFIRShape();
  settingsArr[1].value = dac->getIIRBandwidth();
  settingsArr[2].value = dac->getDpllSerial();
  settingsArr[3].value = dac->getJitterEl();

  settingsArr[0].value_string = dac->getFIRShapeString(settingsArr[0].value);
  settingsArr[1].value_string = dac->getIIRBandwidthString(settingsArr[1].value);
  settingsArr[2].value_string = dac->getDpllSerialString(settingsArr[2].value);
  settingsArr[3].value_string = dac->getJitterElString(settingsArr[3].value);


  // //display initial values
  // tftGraphics->printButtons( currentPage );
  // tftGraphics->printChannel( dac->getInput() );
  // tftGraphics->printVolume ( dac->getVolume() );

  // //Display Version for short time
  // tftGraphics->printInfoText( VERSION );
  // delay ( 500 );
  // tftGraphics->clearInfoText();

}


//******************************************************************************
// LOOP THE DAC COMMANDS
//******************************************************************************
void loop() {


  //get action from remote controller
  action = remoteInterface->getAction( currentPage );
  //if no action from remote controller read the touch interface
  if( action != NONE ){
    interface = REMOTE; 
  }else{
    action = touchInterface->getAction( currentPage );
    if (action != NONE) interface = TOUCH;
  }


  //No Action taken, update DAC lock status after given amount of cycles
  if( action == NONE && read_dac_counter >= READ_DAC_CYCLES){
    read_dac_counter = 0;
    
    //Read the switches and config DAC
    //dac->configureDAC();

    //Print lock Status
    if( currentPage == MAIN_MENU ){
      //get current lock status
      LOCK_STATUS lock_st = dac->getLockStatus();
      uint32_t fsr = dac->getRawSampleRate();
      //see if any changes
      if ( last_lock != lock_st ) refreshSampleRate = true;
      if ( last_fsr != fsr )      refreshSampleRate = true;
      //display changes
      if( lock_st != No_Lock && refreshSampleRate)
      {
          //display lock status if changed
          //tftGraphics->printInfoText( dac->dacLockString( lock_st ), TFTGraphics::FIRST_LINE );
          last_lock = lock_st;
          //Display sample rate if changed
          //tftGraphics->printInfoText( dac->getSampleRateString ( fsr ), TFTGraphics::SECOND_LINE );
          last_fsr = fsr;
          refreshSampleRate = false;
      } 
      else if (refreshSampleRate) 
      {
        //tftGraphics->clearInfoText( TFTGraphics::FIRST_LINE );
        //tftGraphics->clearInfoText( TFTGraphics::SECOND_LINE );
        refreshSampleRate = false;
      }
    }
  }


  DAC_INPUT input; //buffer
  uint8_t vol; //buffer
  //TAKE ACTION
  switch ( action ){
    //No Action
    case NONE:
      break;
    //MAIN PAGE
    case CHANNEL_LEFT:
      input = dac->decreaseInput();
      if(currentPage == MAIN_MENU) //tftGraphics->printChannel( input );
      if(interface == TOUCH) delay_ms = delay_switch;
      refreshSampleRate = true;
      break;
    case CHANNEL_RIGHT:
      input = dac->increaseInput();
      if(currentPage == MAIN_MENU) //tftGraphics->printChannel( input );
      if(interface == TOUCH) delay_ms = delay_switch;
      refreshSampleRate = true;
      break;
    case VOLUME_UP:
      vol = dac->increaseVolume();
      if(currentPage == MAIN_MENU) //tftGraphics->printVolume( vol );
      if(interface == TOUCH) delay_ms = delay_hold;
      break;
    case VOLUME_DOWN:
      vol = dac->decreaseVolume();
      if(currentPage == MAIN_MENU)//tftGraphics->printVolume( vol );
      if(interface == TOUCH) delay_ms = delay_hold;
      break;
    case ENTER:
    case PLAY_PAUSE:
      vol = dac->muteVolume();
      if(currentPage == MAIN_MENU) //tftGraphics->printVolume( vol );
      if(interface == TOUCH) delay_ms = delay_switch;      
      break;
    // case POWER_ON:
    // if( touchInterface->detectHold( action, lastAction ) ){ LOG("\nPower off\n"); }
    // break;

    //SWITCH PAGE
    case MENU:
      currentPage = (currentPage == MAIN_MENU)? SETTINGS_MENU : MAIN_MENU;
      //tftGraphics->clrScr();
      //tftGraphics->printButtons ( currentPage );
      if( currentPage == MAIN_MENU){
        //tftGraphics->printChannel( dac->getInput() );
        //tftGraphics->printVolume( dac->getVolume() );
        refreshSampleRate = true;
      }else{
        //tftGraphics->printSettings( settingsArr );
      }
      if(interface == TOUCH) delay_ms = delay_switch;
      break;
    
    //SETTINGS PAGE
    //ONLY touch
    case SET_FIR_FILTER:
      settingsArr[0].value = dac->getCycleFIRShape();
      settingsArr[0].value_string = dac->getFIRShapeString(settingsArr[0].value);
      //tftGraphics->printSettings( settingsArr, 0 );
      if(interface == TOUCH) delay_ms = delay_switch;      
      break;
    case SET_IIR_BANDWIDTH:
      settingsArr[1].value = dac->getCycleIIRBandwidth();
      settingsArr[1].value_string = dac->getIIRBandwidthString(settingsArr[1].value);
      //tftGraphics->printSettings( settingsArr, 1 );
      if(interface == TOUCH) delay_ms = delay_switch;      
      break; 
    case SET_DPLL:
      settingsArr[2].value = dac->getCycleDPLL();
      settingsArr[2].value_string = dac->getDpllSerialString(settingsArr[2].value);
      //tftGraphics->printSettings( settingsArr, 2 );
      if(interface == TOUCH) delay_ms = delay_switch;      
      break;
    case TOGGLE_JE:
      settingsArr[3].value = dac->getToggleJitterEliminator();
      settingsArr[3].value_string = dac->getJitterElString(settingsArr[3].value);
      //tftGraphics->printSettings( settingsArr, 3 );
      if(interface == TOUCH) delay_ms = delay_switch;      
      break;    
    default:
      break;
  }

    
  //response delay if set
  if ( delay_ms ) delay ( delay_ms );


  //remember last action
  lastAction = action;
  //After taking action reset the flag
  action = NONE;
  //reset interface
  interface = NONE_IFC;
  //increment var to update DAC connection status
  read_dac_counter++;
  //reset delay
  delay_ms = 0;
  
}
//******************************************************************************
