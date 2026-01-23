#include <lvgl.h>
#include <Arduino_GFX_Library.h>
#include <Wire.h>
#include <TAMC_GT911.h>

//https://github.com/znyisztor77/ESP32_5.0inc_JC8048W550

/* --- Kijelző beállítások --- */
#define GFX_BL 2
Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
    40 /* DE */, 41 /* VSYNC */, 39 /* HSYNC */, 42 /* PCLK */,
    45 /* R0 */, 48 /* R1 */, 47 /* R2 */, 21 /* R3 */, 14 /* R4 */,
    5 /* G0 */, 6 /* G1 */, 7 /* G2 */, 15 /* G3 */, 16 /* G4 */, 4 /* G5 */,
    8 /* B0 */, 3 /* B1 */, 46 /* B2 */, 9 /* B3 */, 1 /* B4 */,
    0 /* hsync_polarity */, 8 /* hsync_front_porch */, 4 /* hsync_pulse_width */, 8 /* hsync_back_porch */,
    0 /* vsync_polarity */, 8 /* vsync_front_porch */, 4 /* vsync_pulse_width */, 8 /* vsync_back_porch */,
    1 /* pclk_active_neg */, 16000000 /* prefer_speed */);

Arduino_RGB_Display *gfx = new Arduino_RGB_Display(
    800 /* width */, 480 /* height */, rgbpanel, 0 /* rotation */, true /* auto_flush */);

/* --- Érintőképernyő beállítások --- */
#define TOUCH_SDA 19
#define TOUCH_SCL 20
#define TOUCH_INT 18
#define TOUCH_RST 38
TAMC_GT911 ts = TAMC_GT911(TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST, 800, 480);

/* --- LVGL Puffer --- */
#define DRAW_BUF_SIZE (800 * 480 / 10)
uint16_t *draw_buf;


lv_obj_t * btn_exit;
lv_obj_t * lbl_header;

/* --- LVGL Interfész Függvények --- */

// Kijelző frissítése (Display Flush)
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);
    
    // Az LVGL által rajzolt területet átadjuk a kijelzőnek
    gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)px_map, w, h);
    
    lv_display_flush_ready(disp);
}

// Érintőpanel olvasása (Input Read)
void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
    ts.read();
    if (ts.isTouched) {
        data->state = LV_INDEV_STATE_PRESSED;
        data->point.x = ts.points[0].x;
        data->point.y = ts.points[0].y;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

// LVGL belső órájának biztosítása (Tick)
uint32_t my_tick_cb() {
    return millis();
}

/* --- UI Létrehozása --- */
void setup_ui() {
    // Egy stílusos gomb létrehozása a képernyő közepén
    lv_obj_t * btn = lv_button_create(lv_screen_active());
    lv_obj_set_size(btn, 200, 80);
    lv_obj_center(btn);

    lv_obj_t * label = lv_label_create(btn);
    lv_label_set_text(label, "LVGL 9.2 MUKODIK!");
    lv_obj_center(label);
}

void setup() {
    Serial.begin(115200);

    // 1. Hardverek indítása
    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, HIGH);
    
    if (!gfx->begin()) {
        Serial.println("GFX hiba!");
    }
    
    Wire.begin(TOUCH_SDA, TOUCH_SCL);
    ts.begin();
    ts.setRotation(ROTATION_INVERTED);

    // 2. LVGL inicializálása
    lv_init();
    lv_tick_set_cb(my_tick_cb); // Időzítés beállítása

    // Puffer lefoglalása a belső memóriában (vagy PSRAM-ban)
    draw_buf = (uint16_t *)heap_caps_malloc(DRAW_BUF_SIZE * sizeof(uint16_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);

    // 3. LVGL Kijelző regisztráció
    lv_display_t *disp = lv_display_create(800, 480);
    lv_display_set_flush_cb(disp, my_disp_flush);
    lv_display_set_buffers(disp, draw_buf, NULL, DRAW_BUF_SIZE * sizeof(uint16_t), LV_DISPLAY_RENDER_MODE_PARTIAL);

    // 4. LVGL Touch regisztráció
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, my_touchpad_read);

    // 5. Felület megrajzolása
    //setup_ui();
    // bottom layer opaque and white
  lv_obj_set_style_bg_opa(lv_layer_bottom(), LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_bg_color(lv_layer_bottom(), lv_color_white(), LV_PART_MAIN);

  // btn_exit is not really an LVGL button, just an obj made clickable.
  // It holds the little 'x' to exit back to main screen, but is bigger
  // because the little 'x' would be near impossible to hit.
  btn_exit = lv_obj_create(lv_layer_top());
  lv_obj_clear_flag(btn_exit, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(btn_exit, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_bg_opa(btn_exit, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_border_width(btn_exit, 0, LV_PART_MAIN);
  lv_obj_set_size(btn_exit, 40, 40);
  lv_obj_align(btn_exit, LV_ALIGN_TOP_RIGHT, 0, 0);
  lv_obj_add_event_cb(btn_exit, [](lv_event_t * e) -> void {
    go_main();
  }, LV_EVENT_CLICKED, NULL);

  // holds actual little 'x', lives inside btn_exit
  lv_obj_t * lbl_exit_symbol = lv_label_create(btn_exit);
  lv_obj_set_style_text_font(lbl_exit_symbol, &lv_font_montserrat_18, LV_PART_MAIN);
  lv_obj_set_style_text_align(lbl_exit_symbol, LV_TEXT_ALIGN_RIGHT, 0);
  lv_label_set_text(lbl_exit_symbol, LV_SYMBOL_CLOSE);
  lv_obj_align(lbl_exit_symbol, LV_ALIGN_TOP_RIGHT, 5, -10);  

  // page header
  lbl_header = lv_label_create(lv_layer_top());
  lv_obj_set_style_text_font(lbl_header, &lv_font_montserrat_18, LV_PART_MAIN);
  lv_obj_align(lbl_header, LV_ALIGN_TOP_LEFT, 5, 3);

    go_touch();			// Start with touchpad test, nice when evaluating new boards

    Serial.println("LVGL rendszer kesz.");
}

void loop() {
    // LVGL időzítő kezelő hívása (frissíti a képet és kezeli a touch-ot)
    lv_timer_handler();
    delay(5);
}

// new_screen creates a new obj to use as a base screen, and sets some properties
lv_obj_t * new_screen() {

  lv_obj_t * obj = lv_obj_create(NULL);
  // transparent background (we use the bottom screen layer for that)
  lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, LV_PART_MAIN);
  // no scrollable
  lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
  // no border
  lv_obj_set_style_border_width(obj, 0, 0);
  // flex layout (self-arranging its children)
  lv_obj_set_layout(obj, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  // 20 pixels padding at top and bottom of screen
  lv_obj_set_style_pad_top(obj, 20, LV_PART_MAIN);
  // 10 pixels padding between items
  lv_obj_set_style_pad_row(obj, 10, LV_PART_MAIN);

  return obj;
}

//
// main screen
//

lv_obj_t * scr_main = nullptr;

void go_main() {
  if (!scr_main) {
    // if screen doesn't exist yet, create it and its children
    scr_main = new_screen();

    lv_obj_t * btn_touch = lv_button_create(scr_main);
    lv_obj_t * lbl_touch = lv_label_create(btn_touch);
    lv_label_set_text(lbl_touch, "Touch indicator");
    lv_obj_add_event_cb(btn_touch, [](lv_event_t * e) -> void {
      go_touch();
    }, LV_EVENT_CLICKED, NULL);

  }

  lv_label_set_text(lbl_header, "LVGL_ST7262_touch_demo");   // screen header
  lv_obj_add_flag(btn_exit, LV_OBJ_FLAG_HIDDEN);    // disable exit, already at main screen
  lv_screen_load(scr_main);                         // Tell LVGL to load as active screen

}

//
// Touch demonstration screen
//

lv_obj_t * scr_touch;

// global because used in callback.
lv_obj_t * horizontal;
lv_obj_t * vertical;

void go_touch() {

  if (!scr_touch) {

    scr_touch = new_screen();
    // cancel flex from new_screen: need to place children (the crosshairs) manually
    lv_obj_set_layout(scr_touch, LV_LAYOUT_NONE);
    // no padding at top
    lv_obj_set_style_pad_top(scr_touch, 0, LV_PART_MAIN);
    // clickable
    lv_obj_add_flag(scr_touch, LV_OBJ_FLAG_CLICKABLE);

    // horizontal crosshair
    horizontal = lv_obj_create(scr_touch);
    lv_obj_set_style_border_width(horizontal, 0, 0);
    lv_obj_set_style_radius(horizontal, 0, LV_PART_MAIN);
    lv_obj_set_size(horizontal, lv_pct(100), 3);
    lv_obj_set_style_bg_color(horizontal, lv_color_black(), LV_PART_MAIN);
    lv_obj_clear_flag(horizontal, LV_OBJ_FLAG_CLICKABLE);

    // vertical crosshair
    vertical = lv_obj_create(scr_touch);
    lv_obj_set_style_border_width(vertical, 0, 0);
    lv_obj_set_style_radius(vertical, 0, LV_PART_MAIN);
    lv_obj_set_size(vertical, 3, lv_pct(100));
    lv_obj_set_style_bg_color(vertical, lv_color_black(), LV_PART_MAIN);
    lv_obj_clear_flag(vertical, LV_OBJ_FLAG_CLICKABLE);
    
    // callback
    lv_obj_add_event_cb(scr_touch, [](lv_event_t * e) -> void {
      if (lv_event_get_code(e) == LV_EVENT_PRESSED || lv_event_get_code(e) == LV_EVENT_PRESSING) {
        // when pressed get the current touch point
        lv_point_t point;
        lv_indev_get_point(lv_indev_get_act(), &point);
        // show crosshairs in proper position
        lv_obj_set_y(horizontal, point.y);
        lv_obj_set_x(vertical, point.x);
        lv_obj_clear_flag(horizontal, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(vertical, LV_OBJ_FLAG_HIDDEN);
        // remove exit button
        lv_obj_add_flag(btn_exit, LV_OBJ_FLAG_HIDDEN);
      } else if (lv_event_get_code(e) == LV_EVENT_RELEASED) {
        // when released hide crosshairs
        lv_obj_add_flag(horizontal, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(vertical, LV_OBJ_FLAG_HIDDEN);
        // put back exit button
        lv_obj_clear_flag(btn_exit, LV_OBJ_FLAG_HIDDEN);
      }
    }, LV_EVENT_ALL, NULL);
  }

  lv_obj_add_flag(horizontal, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(vertical, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(btn_exit, LV_OBJ_FLAG_HIDDEN);
  lv_label_set_text(lbl_header, "Touch indicator");
  lv_screen_load(scr_touch);

}
