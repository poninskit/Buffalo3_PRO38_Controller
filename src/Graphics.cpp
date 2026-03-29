#include <Graphics.h>


using namespace esp_panel::drivers;
using namespace esp_panel::board;

static const lv_color_t flatui_colors[7] = {
    lv_color_hex(0x3498db), //0 River
    lv_color_hex(0x34495e), //1 Asphalt
    lv_color_hex(0x7f8c8d), //2 Asbest
    lv_color_hex(0x1abc9c), //3 Turquise
    lv_color_hex(0x2980b9), //4 Belize Hole
    lv_color_hex(0xe67e22), //5 Carrot
    lv_color_hex(0x16a085)  //6 Green Sea
};


static const lv_color_t flatui_colors_sys[7] = {
    lv_color_hex(0x000000), //0 Black
    lv_color_hex(0xffffff), //1 White
    lv_color_hex(0xecf0f1), //2 Clouds
    lv_color_hex(0x2c3e50), //3 Midnight
    lv_color_hex(0xbdc3c7), //4 Silver
    lv_color_hex(0x95a5a6), //5 Concrete
    lv_color_hex(0xff0000)  //6 Red 
};


static const char* color_names = "River\nAsphalt\nAsbest\nTurquise\nBelize Hole\nCarrot\nGreen Sea";


//******************************************************************************
//------------------------------------------------------------------------------
Graphics::Graphics()
{
    board = new Board();
    board->init();

    if (!board->begin()) {
        LOG("Warning: board->begin() failed, continuing without touch/backlight\n");
    }

    
    if (board->getBacklight() != nullptr) {
        board->getBacklight()->setBrightness(70);
    }


    auto lcd = board->getLCD();
    auto touch = board->getTouch();

    delay(100); // give some time for devices to initialize, especially important for touch to avoid I2C errors

    if (!lcd) {
        LOG("LCD is NULL \n");
    } else {
        LOG("LCD OK \n");
    }

    if (!touch) {
        LOG("Touch is NULL");
    } else {
        LOG("Touch OK \n");
    }

    // initialise LVGL port with display and touch interfaces
    if (lcd && touch) {
        lvgl_port_init(lcd, touch);
        LOG("LVGL init OK\n");
        delay(100); // give some time for LVGL to initialize before creating objects
    } else {
        LOG("LVGL init skipped due to missing devices\n");
    }
    



}

void Graphics::createUI(){
    if (!board) {
        LOG("Error: Board not initialized, cannot create UI\n");
        return;
    }

    LOG("Creating UI...\n");
    /* Lock the mutex due to the LVGL APIs are not thread-safe */
    if (!lvgl_port_lock(100)) {
        Serial.println("LVGL lock timeout!");
        return;
    }   

    // init styles
    lv_style_init(&button_style);
    button_color = flatui_colors[2]; // Asbest default
    //lv_style_set_bg_color(&button_style, button_color);
    //lv_style_set_bg_opa(&button_style, LV_OPA_COVER);


    main_screen = lv_obj_create(NULL);
    settings_screen = lv_obj_create(NULL);
    createMainScreen();
    createSettingsScreen();

    updateStyles();

    lv_scr_load(main_screen);

    lvgl_port_unlock();
}



lv_obj_t *Graphics::make_button(lv_obj_t *parent, const char *txt
    , lv_event_cb_t cb, const lv_font_t *font
    , lv_align_t align ) 
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, this);
    lv_obj_set_size(btn, 180, 80);
    lv_obj_add_style(btn, &button_style, 0);
    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, txt);
    lv_obj_set_style_text_font(lbl, font, 0);
    lv_obj_align(lbl, align, 0, 0);
    return btn;
}


void Graphics::updateStyles()
{
    lv_color_t bg;
    lv_color_t text;

    if (darkMode) {
        bg   = flatui_colors_sys[0]; // dark
        text = flatui_colors_sys[4]; // light
    } else {
        bg   = flatui_colors_sys[2]; // light
        text = flatui_colors_sys[3]; // dark
    }

    // Apply background to both screens
    lv_obj_set_style_bg_color(main_screen, bg, 0);
    lv_obj_set_style_bg_color(settings_screen, bg, 0);


    // Update button style color and apply to existing objects
    lv_style_set_bg_color(&button_style, button_color);


    if (vol_arc) {
        // indicator (filled part)
        lv_obj_set_style_arc_color(vol_arc, button_color, LV_PART_INDICATOR);
        // knob (the handle)
        lv_obj_set_style_bg_color(vol_arc, button_color, LV_PART_KNOB);

        if (darkMode) {
            // dark mode → subtle grey track
            lv_obj_set_style_arc_color(vol_arc, flatui_colors_sys[3], LV_PART_MAIN);
        } else {
            // light mode → darker track for contrast
            lv_obj_set_style_arc_color(vol_arc, flatui_colors_sys[4], LV_PART_MAIN);
        }
    }


    if (buttons_column) {
        if (darkMode) {
            lv_obj_set_style_bg_color(buttons_column, flatui_colors_sys[3], LV_PART_MAIN);
            lv_obj_set_style_border_color(buttons_column, flatui_colors[2], 0);
        } else {
            lv_obj_set_style_bg_color(buttons_column, flatui_colors_sys[1], LV_PART_MAIN);
            lv_obj_set_style_border_color(buttons_column, flatui_colors_sys[4], 0);
        }
    }



    // labels on main screen
    if (sample_label_value) lv_obj_set_style_text_color(sample_label_value, text, 0);
    if (lock_label_value) lv_obj_set_style_text_color(lock_label_value, text, 0);
    if (vol_label) lv_obj_set_style_text_color(vol_label, text, 0);



    // Refresh existing buttons to pick up new style
    lv_obj_t *btns[] = {btn_usb, btn_opt1, btn_opt2, btn_spdif, settings_btn, back_btn};
    for (auto b:btns) {
        if (b){ 
            lv_obj_refresh_style(b, LV_PART_MAIN, (lv_style_prop_t)LV_STYLE_PROP_ALL);
        }
    }

    for (int i = 0; i < 4; i++) {
        if (settings_btns[i]) lv_obj_refresh_style(settings_btns[i], LV_PART_MAIN, (lv_style_prop_t)LV_STYLE_PROP_ALL);
    }

    // // settings labels
    // for (int i = 0; i < 4; i++) {
    //     if (settings_vals[i])
    //         lv_obj_set_style_text_color(settings_vals[i], text, 0);
    // }
}


void Graphics::applyUIState(bool dark, uint8_t colorIdx)
{
    darkMode = dark;
    button_color_index = colorIdx;
    button_color = flatui_colors[colorIdx];

    updateStyles();

    // update dropdown
    if (color_dropdown)
        lv_dropdown_set_selected(color_dropdown, colorIdx);

    // update theme button text
    if (theme_btn) {
        lv_obj_t *label = lv_obj_get_child(theme_btn, 0);
        lv_label_set_text(label, darkMode ? "Light" : "Dark");
    }
}


void Graphics::printChannel( DAC_INPUT channel_id )
{
    if (!lvgl_port_lock(100)) {
        Serial.println("LVGL lock timeout!");
        return;
    }

    lv_obj_t *btns[] = {btn_usb, btn_opt1, btn_opt2, btn_spdif};
    for (auto b:btns) {
        lv_obj_clear_state(b, LV_STATE_CHECKED);
    }

    switch ( channel_id ){
        case USB:
            lv_obj_add_state(btn_usb, LV_STATE_CHECKED);
            break;
        case SPDIF:
            lv_obj_add_state(btn_spdif, LV_STATE_CHECKED);
            break;
        case OPT1:
            lv_obj_add_state(btn_opt1, LV_STATE_CHECKED);
            break;
        case OPT2:
            lv_obj_add_state(btn_opt2, LV_STATE_CHECKED);
            break;
        default:
            break;
    }

    lvgl_port_unlock();
}



void Graphics::printSettings( const DACState& state, int8_t index ) {
    const char* values[4] = {
        state.firShapeStr,
        state.iirBandwidthStr,
        state.dpllBandwidthStr,
        state.jitterElStr
    };

    if (index == -1) {
        for (int i = 0; i < 4; i++)
            lv_label_set_text_fmt(settings_vals[i], "%s\n%s", settingsArr[i].sett_name, values[i]);
    } else {
        lv_label_set_text_fmt(settings_vals[index], "%s\n%s", settingsArr[index].sett_name, values[index]);
    }
}


void Graphics::createMainScreen()
{
    //lv_obj_t *scr = lv_scr_act();
    lv_obj_t *scr = main_screen;

    // left column (wider now to hold bigger buttons)
    buttons_column = lv_obj_create(scr);
    lv_coord_t screen_height = lv_disp_get_ver_res(NULL);
    lv_obj_set_size(buttons_column, 230, screen_height - 20);
    lv_obj_align(buttons_column, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_clear_flag(buttons_column, LV_OBJ_FLAG_SCROLLABLE); // make buttons fixed, not movable with slider
    
    btn_usb  = make_button(buttons_column, "USB", input_btn_cb);
    lv_obj_align(btn_usb, LV_ALIGN_TOP_MID, 0, 0);
    btn_opt1 = make_button(buttons_column, "OPT1", input_btn_cb);
    lv_obj_align_to(btn_opt1, btn_usb, LV_ALIGN_OUT_BOTTOM_MID, 0, 32);
    btn_opt2 = make_button(buttons_column, "OPT2", input_btn_cb);
    lv_obj_align_to(btn_opt2, btn_opt1, LV_ALIGN_OUT_BOTTOM_MID, 0, 32);
    btn_spdif= make_button(buttons_column, "SPDIF", input_btn_cb);
    lv_obj_align_to(btn_spdif, btn_opt2, LV_ALIGN_OUT_BOTTOM_MID, 0, 32);

    // volume arc bottom right
    vol_arc = lv_arc_create(scr);
    lv_obj_set_size(vol_arc, 300, 300);
    lv_obj_align(vol_arc, LV_ALIGN_BOTTOM_RIGHT, -30, -30);
    lv_arc_set_range(vol_arc, 0, 99);
    lv_arc_set_value(vol_arc, 50);
    lv_obj_set_style_arc_width(vol_arc, 24, 0); // double the thickness for background
    lv_obj_set_style_arc_width(vol_arc, 24, LV_PART_INDICATOR); // match indicator thickness


    vol_label = lv_label_create(vol_arc);
    lv_label_set_text(vol_label, "50");
    lv_obj_set_style_text_font(vol_label, &LV_FONT_MONTSERRAT_120, 0);
    lv_obj_center(vol_label);
    
    
    lv_obj_add_event_cb(vol_arc, vol_arc_cb, LV_EVENT_VALUE_CHANGED, this);
    //lv_obj_add_event_cb(vol_arc, vol_arc_cb, LV_EVENT_RELEASED, this); // trigger on release to avoid too many updates while dragging


    lock_label_value = lv_label_create(scr);
    lv_label_set_text(lock_label_value, "No Lock");
    lv_obj_set_style_text_font(lock_label_value, &lv_font_montserrat_32, 0);
    lv_obj_align(lock_label_value, LV_ALIGN_TOP_LEFT, 280, 34);
    lv_obj_set_style_text_color(lock_label_value, flatui_colors[2], 0);

    sample_label_value = lv_label_create(scr);
    lv_label_set_text(sample_label_value, "Unknown SR");
    lv_obj_set_style_text_font(sample_label_value, &lv_font_montserrat_32, 0);
    lv_obj_align(sample_label_value, LV_ALIGN_TOP_LEFT, 280, 74);
    lv_obj_set_style_text_color(sample_label_value, flatui_colors[2], 0);


    // settings button top right
    settings_btn = make_button(scr, "Settings", settings_btn_cb);
    lv_obj_align(settings_btn, LV_ALIGN_TOP_RIGHT, -20, 20);

    dac_status_label = lv_label_create(scr);
    lv_label_set_text(dac_status_label, "");
    lv_obj_set_style_text_font(dac_status_label, &lv_font_montserrat_26, 0);
    lv_obj_set_style_text_color(dac_status_label, flatui_colors_sys[6] , 0); // flat red lv_color_hex(0xe74c3c)
    lv_obj_align(dac_status_label, LV_ALIGN_BOTTOM_RIGHT, -70, -30);

    updateStyles();
}


void Graphics::createSettingsScreen()
{
    //lv_obj_t *scr = lv_scr_act();
    lv_obj_t *scr = settings_screen;

    for (int i=0;i<4;i++) {
        settings_btns[i] = lv_btn_create(scr);
        // (height 80), (width 360)
        lv_obj_set_size(settings_btns[i], 360, 90);
        lv_obj_add_style(settings_btns[i], &button_style, 0);
        // spread equally on vertical axis over screen height (assuming 480px, button 80px)
        lv_obj_align(settings_btns[i], LV_ALIGN_TOP_LEFT, 20, 20 + i * 115);

        //each utton gets callback with its index as value, so we know which setting to update in main
        lv_obj_add_event_cb(settings_btns[i], setting_item_cb, LV_EVENT_CLICKED, this);

        lv_obj_t *lbl = lv_label_create(settings_btns[i]);
        lv_label_set_text_fmt(lbl, "%s\nunknown", settingsArr[i].sett_name);
        //lv_obj_set_style_text_color(lbl, flatui_colors[2], 0);
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_28, 0);
        lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 10, 0);
        settings_vals[i] = lbl; // reuse label for value
    }

    
    back_btn = make_button(scr, "Back", settings_back_cb);
    lv_obj_align(back_btn, LV_ALIGN_TOP_RIGHT, -20, 20);


    theme_btn = make_button(scr, "Light", settings_theme_cb, &lv_font_montserrat_20, LV_ALIGN_LEFT_MID);
    lv_obj_align(theme_btn, LV_ALIGN_BOTTOM_RIGHT, -20, -140);
    lv_obj_set_size(theme_btn, 180, 40);
    lv_obj_set_style_bg_color(theme_btn, flatui_colors_sys[4], 0);
    lv_obj_set_style_text_color(theme_btn, flatui_colors_sys[0], 0);
    lv_obj_set_style_border_color(theme_btn, flatui_colors_sys[5], 0);
    

    color_dropdown = lv_dropdown_create(scr);
    lv_dropdown_set_options(color_dropdown, color_names);
    lv_obj_set_style_text_font(color_dropdown, &lv_font_montserrat_20, 0);
    lv_obj_set_size(color_dropdown, 180, 40);
    lv_obj_align(color_dropdown, LV_ALIGN_BOTTOM_RIGHT, -20, -80);
    lv_obj_add_event_cb(color_dropdown, color_dropdown_cb, LV_EVENT_VALUE_CHANGED, this);
    lv_dropdown_set_selected(color_dropdown, button_color_index);
    lv_obj_set_style_bg_color(color_dropdown, flatui_colors_sys[4], 0);
    lv_obj_set_style_border_color(color_dropdown, flatui_colors_sys[5], 0);

    // version label
    lv_obj_t *version_label = lv_label_create(scr);
    lv_label_set_text_fmt(version_label, "Version: %s", VERSION);
    lv_obj_set_style_text_font(version_label, &lv_font_montserrat_26, 0);
    lv_obj_align(version_label, LV_ALIGN_BOTTOM_RIGHT, -30, -26);
    lv_obj_set_style_text_color(version_label, flatui_colors[2], 0);

    updateStyles();
}



void Graphics::vol_arc_cb(lv_event_t *e)
{
    Graphics *self = (Graphics*)lv_event_get_user_data(e);
    lv_obj_t *arc  = lv_event_get_target(e);
    int val = lv_arc_get_value(arc);
    lv_label_set_text_fmt(self->vol_label, "%d", val);

    if (self->_actionCb) self->_actionCb(VOLUME_SET, val);
}


void Graphics::input_btn_cb(lv_event_t *e)
{
    Graphics *self = (Graphics*)lv_event_get_user_data(e);
    lv_obj_t *btn  = lv_event_get_target(e);

    // visual toggle
    lv_obj_t *btns[] = {self->btn_usb, self->btn_opt1, self->btn_opt2, self->btn_spdif};
    for (auto b : btns) lv_obj_clear_state(b, LV_STATE_CHECKED);
    lv_obj_add_state(btn, LV_STATE_CHECKED);

    // notify main
    if      (btn == self->btn_usb)   { if (self->_actionCb) self->_actionCb(CHANNEL_USB,   0); }
    else if (btn == self->btn_opt1)  { if (self->_actionCb) self->_actionCb(CHANNEL_OPT1,  0); }
    else if (btn == self->btn_opt2)  { if (self->_actionCb) self->_actionCb(CHANNEL_OPT2,  0); }
    else if (btn == self->btn_spdif) { if (self->_actionCb) self->_actionCb(CHANNEL_SPDIF, 0); }
}

void Graphics::settings_btn_cb(lv_event_t *e)
{
    // this is ONLY for the "Settings" button on the main screen
    Graphics *self = (Graphics*)lv_event_get_user_data(e);
    self->showSettingsScreen();
    if (self->_actionCb) self->_actionCb(MENU, 0);
}

void Graphics::setting_item_cb(lv_event_t *e)
{
    // this is for the 4 individual setting buttons on the settings screen
    Graphics *self = (Graphics*)lv_event_get_user_data(e);
    lv_obj_t *btn  = lv_event_get_target(e);

    if      (btn == self->settings_btns[0]) { if (self->_actionCb) self->_actionCb(SET_FIR_FILTER,    0); }
    else if (btn == self->settings_btns[1]) { if (self->_actionCb) self->_actionCb(SET_IIR_BANDWIDTH, 0); }
    else if (btn == self->settings_btns[2]) { if (self->_actionCb) self->_actionCb(SET_DPLL,          0); }
    else if (btn == self->settings_btns[3]) { if (self->_actionCb) self->_actionCb(TOGGLE_JE,         0); }
}



void Graphics::settings_back_cb(lv_event_t *e)
{
    Graphics *self = (Graphics*)lv_event_get_user_data(e);
    self->showMainScreen();
    if (self->_actionCb) self->_actionCb(MENU, 0);
}



void Graphics::color_dropdown_cb(lv_event_t *e)
{
    Graphics *self = (Graphics*)lv_event_get_user_data(e);
    lv_obj_t *dropdown = lv_event_get_target(e);

    self->button_color_index = lv_dropdown_get_selected(dropdown);
    self->button_color = flatui_colors[self->button_color_index];

    if (self->uiStateManager) self->uiStateManager->setColorIndex(self->button_color_index);

    self->updateStyles();
}


void Graphics::settings_theme_cb(lv_event_t *e)
{
    Graphics *self = (Graphics*)lv_event_get_user_data(e);

    self->darkMode = !self->darkMode;

    if (self->uiStateManager) self->uiStateManager->setDarkMode(self->darkMode);

    // update button text
    lv_obj_t *label = lv_obj_get_child(self->theme_btn, 0);
    lv_label_set_text(label, self->darkMode ? "Light" : "Dark");

    self->updateStyles();
}


void Graphics::showMainScreen()
{
    if (!lvgl_port_lock(100)) {
        Serial.println("LVGL lock timeout!");
        return;
    }
    lv_scr_load(main_screen);
    inSettings = false;
    lvgl_port_unlock();
}



void Graphics::showSettingsScreen()
{
    if (!lvgl_port_lock(100)) {
    Serial.println("LVGL lock timeout!");
    return;
}
    lv_scr_load(settings_screen);
    inSettings = true;
    lvgl_port_unlock();
}


void Graphics::printLockStatus( const char* text )
{
    if (!lvgl_port_lock(100)) {
        Serial.println("LVGL lock timeout!");
        return;
    }
    lv_label_set_text(lock_label_value, text);
    lvgl_port_unlock();

}



void Graphics::printSampleRate( const char* text )
{
    if (!lvgl_port_lock(100)) {
        Serial.println("LVGL lock timeout!");
        return;
    }
    lv_label_set_text(sample_label_value, text);
    lvgl_port_unlock();
}


void Graphics::printVolume( uint8_t volume )
{
    if (!lvgl_port_lock(100)) {
        Serial.println("LVGL lock timeout!");
        return;
    }

    lv_arc_set_value(vol_arc, volume);
    lv_label_set_text_fmt(vol_label, "%d", volume);
    lvgl_port_unlock();
}

void Graphics::setDacAvailable( bool available )
{
    if (!lvgl_port_lock(100)) {
        Serial.println("LVGL lock timeout!");
        return;
    }
    
    lv_label_set_text(dac_status_label, available ? "" : "DAC unavailable");
    lvgl_port_unlock();
}
