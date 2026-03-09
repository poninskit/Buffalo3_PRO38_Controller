#ifndef GLOBALS_H
#define GLOBALS_H


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

//Action
enum ACTION{
  NONE = 0,
  CHANNEL_LEFT,
  CHANNEL_RIGHT,
  VOLUME_UP,
  VOLUME_DOWN,
  SET_FIR_FILTER,
  SET_IIR_BANDWIDTH,
  SET_DPLL,
  TOGGLE_JE,
  ENTER,
  MENU,
  PLAY_PAUSE,
  POWER_ON,
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
  char* value_string;
};


//------------------------------------------------------------------------------
#endif // GLOBALS_H
