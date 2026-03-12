#include <Graphics.h>


using namespace esp_panel::drivers;
using namespace esp_panel::board;
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
    
    // if (board->getBacklight() != nullptr) {
    //     board->getBacklight()->setBrightness(50);
    // }


    Serial.println("Creating UI");
    /* Lock the mutex due to the LVGL APIs are not thread-safe */
    lvgl_port_lock(-1);

    /**
     * Create the simple labels
     */
    lv_obj_t *label_1 = lv_label_create(lv_scr_act());
    lv_label_set_text(label_1, "Hello World!");
    lv_obj_set_style_text_font(label_1, &lv_font_montserrat_30, 0);
    lv_obj_align(label_1, LV_ALIGN_CENTER, 0, -20);
    lv_obj_t *label_2 = lv_label_create(lv_scr_act());
    lv_label_set_text_fmt(
        label_2, "ESP32_Display_Panel(%d.%d.%d)",
        ESP_PANEL_VERSION_MAJOR, ESP_PANEL_VERSION_MINOR, ESP_PANEL_VERSION_PATCH
    );
    lv_obj_set_style_text_font(label_2, &lv_font_montserrat_16, 0);
    lv_obj_align_to(label_2, label_1, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_t *label_3 = lv_label_create(lv_scr_act());
    lv_label_set_text_fmt(label_3, "LVGL(%d.%d.%d)", LVGL_VERSION_MAJOR, LVGL_VERSION_MINOR, LVGL_VERSION_PATCH);
    lv_obj_set_style_text_font(label_3, &lv_font_montserrat_16, 0);
    lv_obj_align_to(label_3, label_2, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    lvgl_port_unlock();

}


void Graphics::print()
{

}