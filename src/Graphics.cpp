#include <Graphics.h>


using namespace esp_panel::drivers;
using namespace esp_panel::board;

//******************************************************************************
// helper to make a labelled button (size and font may be scaled)

static lv_obj_t *make_button(lv_obj_t *parent, const char *txt, lv_event_cb_t cb, Graphics *self) 
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, self);
    // double size compared to original mockup
    lv_obj_set_size(btn, 180, 80);
    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, txt);
    // enlarge text for clarity
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_28, 0);
    lv_obj_center(lbl);
    return btn;
}

// callback for volume arc value change
static void vol_arc_cb(lv_event_t *e)
{
    lv_obj_t *arc = lv_event_get_target(e);
    lv_obj_t *label = (lv_obj_t *)lv_event_get_user_data(e);
    int val = lv_arc_get_value(arc);
    lv_label_set_text_fmt(label, "%d", val);
}


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

    // initialise LVGL port with display and touch interfaces
    lvgl_port_init(board->getLCD(), board->getTouch());
    // no rendering is performed here; use print() to draw content later
    
     if (board->getBacklight() != nullptr) {
         board->getBacklight()->setBrightness(50);
     }


    Serial.println("Creating UI");
    /* Lock the mutex due to the LVGL APIs are not thread-safe */
    lvgl_port_lock(-1);

    createMainScreen();

    lvgl_port_unlock();

}



void Graphics::printLockStatus( const char* text )
{
    lv_label_set_text(lock_label_value, text);
}



void Graphics::printSampleRate( const char* text )
{
    lv_label_set_text(sample_label_value, text);
}



void Graphics::printVolume( uint8_t volume )
{
    lv_arc_set_value(vol_arc, volume);
}



void Graphics::printChannel( DAC_INPUT channel_id )
{
    // simple toggle logic: reset all then set this active style
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
}



void Graphics::printSettings( Settings* settings, int8_t index )
{
    for (int i = 0 ; i < 4 ; i++ ){
        if ( index != -1 ){ // -1 = print all, otherwise print one selected
        //print only given index, skip otherwise
        if( i != index ) { 
            continue; //skip drawing this button
        }
        }
        lv_label_set_text(settings_vals[i], settings[i].value_string);
    }
}


void Graphics::createMainScreen()
{
    lv_obj_t *scr = lv_scr_act();
    // left column (wider now to hold bigger buttons)
    lv_obj_t *col = lv_obj_create(scr);
    lv_coord_t screen_height = lv_disp_get_ver_res(NULL);
    lv_obj_set_size(col, 210, screen_height - 20);
    lv_obj_align(col, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_clear_flag(col, LV_OBJ_FLAG_SCROLLABLE); // make buttons fixed, not movable with slider
    btn_usb  = make_button(col, "USB", input_btn_cb, this);
    lv_obj_align(btn_usb, LV_ALIGN_TOP_MID, 0, 0);
    btn_opt1 = make_button(col, "OPT1", input_btn_cb, this);
    lv_obj_align_to(btn_opt1, btn_usb, LV_ALIGN_OUT_BOTTOM_MID, 0, 32);
    btn_opt2 = make_button(col, "OPT2", input_btn_cb, this);
    lv_obj_align_to(btn_opt2, btn_opt1, LV_ALIGN_OUT_BOTTOM_MID, 0, 32);
    btn_spdif= make_button(col, "SPDIF", input_btn_cb, this);
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
    lv_obj_add_event_cb(vol_arc, vol_arc_cb, LV_EVENT_VALUE_CHANGED, vol_label);

    // sample rate label bottom left and bigger font (double size)
    sample_label = lv_label_create(scr);
    lv_label_set_text(sample_label, "Sample rate: ");
    lv_obj_set_style_text_font(sample_label, &lv_font_montserrat_26, 0);
    lv_obj_align(sample_label, LV_ALIGN_BOTTOM_LEFT, 250, -70);

    sample_label_value = lv_label_create(scr);
    lv_label_set_text(sample_label_value, "PCM 44.1 kHz");
    lv_obj_set_style_text_font(sample_label_value, &lv_font_montserrat_26, 0);
    lv_obj_align(sample_label_value, LV_ALIGN_BOTTOM_LEFT, 250, -30);

    // lock status labels
    lock_label = lv_label_create(scr);
    lv_label_set_text(lock_label, "Lock status: ");
    lv_obj_set_style_text_font(lock_label, &lv_font_montserrat_26, 0);
    lv_obj_align(lock_label, LV_ALIGN_TOP_LEFT, 250, 30);

    lock_label_value = lv_label_create(scr);
    lv_label_set_text(lock_label_value, "Locked_SPDIF");
    lv_obj_set_style_text_font(lock_label_value, &lv_font_montserrat_26, 0);
    lv_obj_align(lock_label_value, LV_ALIGN_TOP_LEFT, 250, 70);

    // settings button top right
    settings_btn = make_button(scr, "Settings", settings_btn_cb, this);
    lv_obj_align(settings_btn, LV_ALIGN_TOP_RIGHT, -20, 20);
}


void Graphics::createSettingsScreen()
{
    lv_obj_t *scr = lv_scr_act();
    static const char *titles[4] = {"FIRShape","IIRBW","DPLLS","Jitter"};
    for (int i=0;i<4;i++) {
        settings_btns[i] = lv_btn_create(scr);
        // (height 80), (width 360)
        lv_obj_set_size(settings_btns[i], 360, 90);
        // spread equally on vertical axis over screen height (assuming 480px, button 80px)
        lv_obj_align(settings_btns[i], LV_ALIGN_TOP_LEFT, 20, 20 + i * 115);
        lv_obj_add_event_cb(settings_btns[i], settings_btn_cb, LV_EVENT_CLICKED, this);
        lv_obj_t *lbl = lv_label_create(settings_btns[i]);
        lv_label_set_text_fmt(lbl, "%s\nunknown", titles[i]);
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_28, 0);
        lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 10, 0);
        settings_vals[i] = lbl; // reuse label for value
    }
    back_btn = make_button(scr, "Back", settings_back_cb, this);
    // back button is larger now (160x80) by make_button default
    lv_obj_align(back_btn, LV_ALIGN_TOP_RIGHT, -20, 20);

    // version label
    lv_obj_t *version_label = lv_label_create(scr);
    lv_label_set_text_fmt(version_label, "Version: %s", VERSION);
    lv_obj_set_style_text_font(version_label, &lv_font_montserrat_26, 0);
    lv_obj_align(version_label, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
}


void Graphics::input_btn_cb(lv_event_t *e)
{
    Graphics *self = (Graphics*)lv_event_get_user_data(e);
    lv_obj_t *btn = lv_event_get_target(e);
    // simple toggle logic: reset all then set this active style
    lv_obj_t *btns[] = {self->btn_usb,self->btn_opt1,self->btn_opt2,self->btn_spdif};
    for (auto b:btns) {
        lv_obj_clear_state(b, LV_STATE_CHECKED);
    }
    lv_obj_add_state(btn, LV_STATE_CHECKED);
}


void Graphics::settings_btn_cb(lv_event_t *e)
{
    Graphics *self = (Graphics*)lv_event_get_user_data(e);
    if (!self->inSettings) {
        self->showSettingsScreen();
    } else {
        // nothing for individual setting buttons yet
    }
}


void Graphics::settings_back_cb(lv_event_t *e)
{
    Graphics *self = (Graphics*)lv_event_get_user_data(e);
    self->showMainScreen();
}


void Graphics::showMainScreen()
{
    inSettings = false;
    lv_obj_clean(lv_scr_act());
    createMainScreen();
}


void Graphics::showSettingsScreen()
{
    inSettings = true;
    lv_obj_clean(lv_scr_act());
    createSettingsScreen();
}
