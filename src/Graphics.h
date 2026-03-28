#ifndef GRAPHICS_H
#define GRAPHICS_H

//==============================================================================
#include <globals.h>
#include <Arduino.h>
#include "StateManager.h"
#include "UIStateManager.h"
#include <esp_display_panel.hpp>
#include <lvgl.h>
#include "lvgl_v8_port.h"

// Font declaration
extern const lv_font_t LV_FONT_MONTSERRAT_120;

// Forward declaration for Board class from esp_panel namespace
namespace esp_panel::board {
    class Board;
}


//define action callback type
using ActionCallback = std::function<void(ACTION, int)>;

//------------------------------------------------------------------------------
class Graphics{
  public:
    Graphics();

    void createUI();

    void setActionCallback(ActionCallback cb) { _actionCb = cb; }

    void printVolume( uint8_t volume );
    void printChannel( DAC_INPUT channel_id );
    void printSettings( const DACState& state, int8_t index = -1 );
    void printLockStatus( const char* text );
    void printSampleRate( const char* text );
    void setDacAvailable( bool available );

    void showMainScreen();
    void showSettingsScreen();

    void applyUIState(bool darkMode, uint8_t colorIndex);
    void setUIStateManager(UIStateManager* mgr) { uiStateManager = mgr; }

  private:
    esp_panel::board::Board *board = nullptr;
    UIStateManager* uiStateManager = nullptr;

    lv_obj_t *main_screen;
    lv_obj_t *settings_screen;

    /* main screen widgets */
    lv_obj_t *buttons_column = nullptr;
    lv_obj_t *btn_usb = nullptr;
    lv_obj_t *btn_opt1 = nullptr;
    lv_obj_t *btn_opt2 = nullptr;
    lv_obj_t *btn_spdif = nullptr;
    lv_obj_t *vol_arc = nullptr;
    lv_obj_t *vol_label = nullptr;  
    lv_obj_t *lock_label_value = nullptr;
    lv_obj_t *sample_label_value = nullptr;
    lv_obj_t *settings_btn = nullptr;
    lv_obj_t *dac_status_label = nullptr;

    /* settings screen widgets */
    lv_obj_t *back_btn = nullptr;
    lv_obj_t *settings_btns[4] = {nullptr, nullptr, nullptr, nullptr};
    lv_obj_t *settings_vals[4] = {nullptr, nullptr, nullptr, nullptr};
    lv_obj_t *color_dropdown = nullptr;

    // styling
    lv_style_t button_style;
    lv_color_t button_color;
    int button_color_index = 2; // default to Asbest
    
    bool darkMode = false;
    lv_obj_t *theme_btn = nullptr;

    bool inSettings = false;

    ActionCallback _actionCb;

    void updateStyles();
    void createMainScreen();
    void createSettingsScreen();
    lv_obj_t *make_button(lv_obj_t *parent, const char *txt, lv_event_cb_t cb
      , const lv_font_t *font = &lv_font_montserrat_28
      , lv_align_t align = LV_ALIGN_CENTER);

    static void vol_arc_cb(lv_event_t *e);
    static void input_btn_cb(lv_event_t *e);
    static void settings_btn_cb(lv_event_t *e);
    static void settings_back_cb(lv_event_t *e);
    static void setting_item_cb(lv_event_t *e);  
    static void color_dropdown_cb(lv_event_t *e);
    static void settings_theme_cb(lv_event_t *e);

};


#endif // GRAPHICS_H