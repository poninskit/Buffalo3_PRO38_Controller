//==============================================================================
#include <globals.h>
#include <Arduino.h>
#include <esp_display_panel.hpp>
#include <lvgl.h>
#include "lvgl_v8_port.h"

// Forward declaration for Board class from esp_panel namespace
namespace esp_panel::board {
    class Board;
}

//------------------------------------------------------------------------------
class Graphics{
  public:
    Graphics();
    void print();

  private:
    esp_panel::board::Board *board = nullptr;

};
