#ifndef GLOBALS_H
#define GLOBALS_H

#define VERSION "2.0.2"

//------------------------------------------------------------------------------
//DEBUGGIG
#define DEBUG 1

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

//Initilize Settings Array globally for now as its used in main and graphics
static Settings settingsArr[] =  {
                          { FIR_FILTER_SHAPE   , "FIR Filter Shape"  , 0, "" },
                          { IIR_BANDWIDTH      , "IIR Bandwidth"     , 0, "" },
                          { DPLL_BANDWIDTH     , "DPLL Bandwidth"    , 0, "" },
                          { JITTER_ELIMINATOR  , "Jitter Eliminator" , 0, "" }
                          };


//------------------------------------------------------------------------------
#endif // GLOBALS_H
