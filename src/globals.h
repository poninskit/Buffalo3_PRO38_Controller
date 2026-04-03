#ifndef GLOBALS_H
#define GLOBALS_H

#define VERSION "2.1.4"

//------------------------------------------------------------------------------
//DEBUGGIG
#define DEBUG 0

#if DEBUG == 1
  #define LOG(s) Serial.print(s) 
#else
  #define LOG(s)
#endif

//------------------------------------------------------------------------------
//ENUMERATORS

enum DAC_INPUT{
  USB = 0,
  OPT1,
  OPT2,
  SPDIF
};


enum ACTION {
    NONE = 0,
    CHANNEL_LEFT, 
    CHANNEL_RIGHT,        
    CHANNEL_USB, 
    CHANNEL_OPT1,          
    CHANNEL_OPT2, 
    CHANNEL_SPDIF,
    VOLUME_UP, 
    VOLUME_DOWN, 
    VOLUME_SET,        
    ENTER, 
    PLAY_PAUSE,
    MENU,
    SET_FIR_FILTER, 
    SET_IIR_BANDWIDTH,
    SET_DPLL, 
    TOGGLE_JE,
    RESET
};


//MENU PAGES
enum PAGE{
  MAIN_MENU,
  SETTINGS_MENU
};

//INTERFACES
enum INTERFACE{
  NONE_IFC,
  TOUCH,
  REMOTE
};


struct Point{
  int x;
  int y;
};

struct Rect{
  Point p1;
  Point p2;
};

//Settings Map
enum SETTINGS_REG
{
  FIR_FILTER_SHAPE = 0,
  IIR_BANDWIDTH,
  DPLL_BANDWIDTH,
  JITTER_ELIMINATOR
};



struct Settings
{
  SETTINGS_REG sett;
  char* sett_name;
  int value;
  char* value_string; // this is only bur
};

// Initilize Settings Array globally for now as its used in main and graphics
// Disable Jitter elimanator to improve Lock, lower quality
// Hgher DPLL ore stable Lock, lower quality
static Settings settingsArr[] =  {
                          { FIR_FILTER_SHAPE   , "FIR Filter Shape"  , 0, "" }, //default R7_F_SHAPE_FAST_RO_MINIMUM_PHASE
                          { IIR_BANDWIDTH      , "IIR Bandwidth"     , 0, "" }, //default R7_IIR_BW_47K
                          { DPLL_BANDWIDTH     , "DPLL Bandwidth"    , 0, "" }, //default DPLL_LOW
                          { JITTER_ELIMINATOR  , "Jitter Eliminator" , 0, "" }  //default R13_JITTER_ELIMINATOR_ENABLED
                          };


//------------------------------------------------------------------------------
#endif // GLOBALS_H
