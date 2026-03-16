#include <Graphics.h>


using namespace esp_panel::drivers;
using namespace esp_panel::board;

static const lv_color_t flatui_colors[7] = {
    lv_color_hex(0x3498db), // River
    lv_color_hex(0x34495e), // Asphalt
    lv_color_hex(0x7f8c8d), // Asbest
    lv_color_hex(0x1abc9c), // Turquise
    lv_color_hex(0xf5b95a)  // Carrot
};


static const char* color_names = "River\nAsphalt\nAsbest\nTurquise\nCarrot";


//******************************************************************************
//------------------------------------------------------------------------------
Graphics::Graphics()
{
    board = new Board();
    board->init();

    if (!board->begin()) {
        Serial.println("Warning: board->begin() failed, continuing without touch/backlight");
        // avoid assert crash; LVGL and backlight may not work
    }

    if (board->getBacklight() != nullptr) {
        board->getBacklight()->setBrightness(100);
    }

    // initialise LVGL port with display and touch interfaces
    lvgl_port_init(board->getLCD(), board->getTouch());
    // no rendering is performed here; use print() to draw content later
    

    Serial.println("Creating UI");
    /* Lock the mutex due to the LVGL APIs are not thread-safe */
    lvgl_port_lock(-1);

    // init styles
    lv_style_init(&button_style);
    button_color = flatui_colors[0]; // River default
    //lv_style_set_bg_color(&button_style, button_color);
    //lv_style_set_bg_opa(&button_style, LV_OPA_COVER);


    main_screen = lv_obj_create(NULL);
    settings_screen = lv_obj_create(NULL);
    createMainScreen();
    createSettingsScreen();

    lv_scr_load(main_screen);

    lvgl_port_unlock();

}





lv_obj_t *Graphics::make_button(lv_obj_t *parent, const char *txt, lv_event_cb_t cb) 
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, this);
    // double size compared to original mockup
    lv_obj_set_size(btn, 180, 80);
    lv_obj_add_style(btn, &button_style, 0);
    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, txt);
    // enlarge text for clarity
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_28, 0);
    lv_obj_center(lbl);
    return btn;
}


void Graphics::updateStyles()
{
    // Update button style color and apply to existing objects
    lv_style_set_bg_color(&button_style, button_color);

    // Update volume arc indicator to match button color
    if (vol_arc) {
        lv_obj_set_style_arc_color(vol_arc, button_color, LV_PART_INDICATOR);
        lv_obj_set_style_bg_color(vol_arc, button_color, LV_PART_KNOB);
        // keep neutral background for arc
        //lv_obj_set_style_arc_color(vol_arc, flatui_colors[2], LV_PART_MAIN); // Asbest grey
        //lv_obj_set_style_arc_opa(vol_arc, LV_OPA_COVER, LV_PART_INDICATOR);
        //lv_obj_set_style_arc_opa(vol_arc, LV_OPA_80, LV_PART_MAIN);
    }

    // Refresh existing buttons to pick up new style
    lv_obj_t *btns[] = {btn_usb, btn_opt1, btn_opt2, btn_spdif, settings_btn, back_btn};
    for (auto b:btns) {
        if (b) lv_obj_refresh_style(b, LV_PART_MAIN, (lv_style_prop_t)LV_STYLE_PROP_ALL);
    }

    for (int i = 0; i < 4; i++) {
        if (settings_btns[i]) lv_obj_refresh_style(settings_btns[i], LV_PART_MAIN, (lv_style_prop_t)LV_STYLE_PROP_ALL);
    }
}


void Graphics::printChannel( DAC_INPUT channel_id )
{
    lvgl_port_lock(-1);

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
    lv_obj_t *col = lv_obj_create(scr);
    lv_coord_t screen_height = lv_disp_get_ver_res(NULL);
    lv_obj_set_size(col, 210, screen_height - 20);
    lv_obj_align(col, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_clear_flag(col, LV_OBJ_FLAG_SCROLLABLE); // make buttons fixed, not movable with slider
    btn_usb  = make_button(col, "USB", input_btn_cb);
    lv_obj_align(btn_usb, LV_ALIGN_TOP_MID, 0, 0);
    btn_opt1 = make_button(col, "OPT1", input_btn_cb);
    lv_obj_align_to(btn_opt1, btn_usb, LV_ALIGN_OUT_BOTTOM_MID, 0, 32);
    btn_opt2 = make_button(col, "OPT2", input_btn_cb);
    lv_obj_align_to(btn_opt2, btn_opt1, LV_ALIGN_OUT_BOTTOM_MID, 0, 32);
    btn_spdif= make_button(col, "SPDIF", input_btn_cb);
    lv_obj_align_to(btn_spdif, btn_opt2, LV_ALIGN_OUT_BOTTOM_MID, 0, 32);

    // volume arc bottom right (50% bigger)
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
    
    //lv_obj_add_event_cb(vol_arc, vol_arc_cb, LV_EVENT_VALUE_CHANGED, vol_label);
    lv_obj_add_event_cb(vol_arc, vol_arc_cb, LV_EVENT_RELEASED, this); // trigger on release to avoid too many updates while dragging

    // sample rate label bottom left and bigger font (double size)
    sample_label = lv_label_create(scr);
    lv_label_set_text(sample_label, "Sample rate: ");
    lv_obj_set_style_text_font(sample_label, &lv_font_montserrat_26, 0);
    lv_obj_align(sample_label, LV_ALIGN_BOTTOM_LEFT, 250, -70);
    lv_obj_set_style_text_color(sample_label, flatui_colors[1], 0);

    sample_label_value = lv_label_create(scr);
    lv_label_set_text(sample_label_value, "PCM 44.1 kHz");
    lv_obj_set_style_text_font(sample_label_value, &lv_font_montserrat_26, 0);
    lv_obj_align(sample_label_value, LV_ALIGN_BOTTOM_LEFT, 250, -30);
    lv_obj_set_style_text_color(sample_label_value, flatui_colors[2], 0);

    // lock status labels
    lock_label = lv_label_create(scr);
    lv_label_set_text(lock_label, "Lock status: ");
    lv_obj_set_style_text_font(lock_label, &lv_font_montserrat_26, 0);
    lv_obj_align(lock_label, LV_ALIGN_TOP_LEFT, 250, 30);
    lv_obj_set_style_text_color(lock_label, flatui_colors[1], 0);

    lock_label_value = lv_label_create(scr);
    lv_label_set_text(lock_label_value, "Locked_SPDIF");
    lv_obj_set_style_text_font(lock_label_value, &lv_font_montserrat_26, 0);
    lv_obj_align(lock_label_value, LV_ALIGN_TOP_LEFT, 250, 70);
    lv_obj_set_style_text_color(lock_label_value, flatui_colors[2], 0);

    // settings button top right
    settings_btn = make_button(scr, "Settings", settings_btn_cb);
    lv_obj_align(settings_btn, LV_ALIGN_TOP_RIGHT, -20, 20);

    dac_status_label = lv_label_create(scr);
    lv_label_set_text(dac_status_label, "");
    lv_obj_set_style_text_font(dac_status_label, &lv_font_montserrat_26, 0);
    lv_obj_set_style_text_color(dac_status_label, lv_color_make(255, 0, 0) , 0); // flat red lv_color_hex(0xe74c3c)
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

    //  // button color selection
    //  lv_obj_t *color_label = lv_label_create(scr);
    //  lv_label_set_text(color_label, "Buttons");
    //  lv_obj_set_style_text_font(color_label, &lv_font_montserrat_26, 0);
    //  lv_obj_align(color_label, LV_ALIGN_RIGHT_MID, -200, 0);

    color_dropdown = lv_dropdown_create(scr);
    lv_dropdown_set_options(color_dropdown, color_names);
    lv_obj_set_size(color_dropdown, 180, 40);
     lv_obj_align(color_dropdown, LV_ALIGN_BOTTOM_RIGHT, -20, -80);
    lv_obj_add_event_cb(color_dropdown, color_dropdown_cb, LV_EVENT_VALUE_CHANGED, this);
    lv_dropdown_set_selected(color_dropdown, button_color_index);

    // version label
    lv_obj_t *version_label = lv_label_create(scr);
    lv_label_set_text_fmt(version_label, "Version: %s", VERSION);
    lv_obj_set_style_text_font(version_label, &lv_font_montserrat_26, 0);
    lv_obj_align(version_label, LV_ALIGN_BOTTOM_RIGHT, -26, -26);
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
    self->updateStyles();
}


void Graphics::showMainScreen()
{
    lvgl_port_lock(-1);
    lv_scr_load(main_screen);
    inSettings = false;
    lvgl_port_unlock();
}

void Graphics::showSettingsScreen()
{
    lvgl_port_lock(-1);
    lv_scr_load(settings_screen);
    inSettings = true;
    lvgl_port_unlock();
}

void Graphics::printLockStatus( const char* text )
{
    lvgl_port_lock(-1);
    lv_label_set_text(lock_label_value, text);
    lvgl_port_unlock();

}


void Graphics::printSampleRate( const char* text )
{
    lvgl_port_lock(-1);
    lv_label_set_text(sample_label_value, text);
    lvgl_port_unlock();
}


void Graphics::printVolume( uint8_t volume )
{
    lvgl_port_lock(-1);
    lv_arc_set_value(vol_arc, volume);
    lvgl_port_unlock();
}

void Graphics::setDacAvailable( bool available )
{
    lvgl_port_lock(-1);
    lv_label_set_text(dac_status_label, available ? "" : "DAC unavailable");
    lvgl_port_unlock();
}
