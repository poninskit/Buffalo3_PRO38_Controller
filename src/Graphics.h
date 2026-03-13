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

    /* main screen widgets */
    lv_obj_t *btn_usb;
    lv_obj_t *btn_opt1;
    lv_obj_t *btn_opt2;
    lv_obj_t *btn_spdif;
    lv_obj_t *vol_arc;
    lv_obj_t *vol_label;
    lv_obj_t *sample_label;
    lv_obj_t *sample_label_value;
    lv_obj_t *settings_btn;

    /* settings screen widgets */
    lv_obj_t *back_btn;
    lv_obj_t *settings_btns[4];
    lv_obj_t *settings_vals[4];

    bool inSettings = false;

    void createMainScreen();
    void createSettingsScreen();
    static void input_btn_cb(lv_event_t *e);
    static void settings_btn_cb(lv_event_t *e);
    static void settings_back_cb(lv_event_t *e);
    void showMainScreen();
    void showSettingsScreen();
};
