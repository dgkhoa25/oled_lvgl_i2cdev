/*
 * UI cho SSD1306 128x64, 1 bit/pixel, LVGL v8.3
 *
 * Bo cuc (don vi: pixel, font UNSCII 8x8 -> moi ky tu rong 8 px)
 *
 *   y  0.. 7   header:  "BBB-OLED"                "LVGL 8.3"
 *   y 10       duong ke ngang 1 px
 *   y 15..22   "Hello World"            (giua man hinh, theo chieu ngang)
 *   y 27..38   progress bar 104x12 (x = 12..115)
 *   y 42..49   "NN%"
 *   y 55..62   "Up MM:SS"               (bo dem, tang moi giay)
 */
#include "ui.h"

#define SCR_W   128
#define SCR_H   64

#define HDR_Y   0
#define SEP_Y   10
#define HELLO_Y 15
#define BAR_W   104
#define BAR_H   12
#define BAR_X   ((SCR_W - BAR_W) / 2)   /* 12 */
#define BAR_Y   27
#define PCT_Y   42
#define CNT_Y   55

#define PROGRESS_STEP       2           /* %/lan   */
#define PROGRESS_PERIOD_MS  100         /* ms/lan -> 0..100% mat 5 giay */

static struct {
    lv_obj_t *bar;
    lv_obj_t *lbl_pct;
    lv_obj_t *lbl_cnt;
    int       progress;
    uint32_t  seconds;
} ui;

/* ---------- Styles dung chung (chi khoi tao 1 lan) ---------- */
static lv_style_t st_text;
static lv_style_t st_bar_main;
static lv_style_t st_bar_ind;
static lv_style_t st_line;

static void styles_init(void)
{
    /* Chu: font bitmap 8x8, trang tren nen den */
    lv_style_init(&st_text);
    lv_style_set_text_font(&st_text, &lv_font_unscii_8);
    lv_style_set_text_color(&st_text, lv_color_white());
    lv_style_set_text_opa(&st_text, LV_OPA_COVER);

    /* Khung bar: vien trang 1 px, khong bo goc, chua khe voi phan ruot */
    lv_style_init(&st_bar_main);
    lv_style_set_bg_opa(&st_bar_main, LV_OPA_TRANSP);
    lv_style_set_border_width(&st_bar_main, 1);
    lv_style_set_border_color(&st_bar_main, lv_color_white());
    lv_style_set_border_opa(&st_bar_main, LV_OPA_COVER);
    lv_style_set_radius(&st_bar_main, 0);
    lv_style_set_pad_all(&st_bar_main, 2);

    /* Ruot bar: trang dac, khong bo goc */
    lv_style_init(&st_bar_ind);
    lv_style_set_bg_opa(&st_bar_ind, LV_OPA_COVER);
    lv_style_set_bg_color(&st_bar_ind, lv_color_white());
    lv_style_set_radius(&st_bar_ind, 0);

    /* Duong ke: hinh chu nhat cao 1 px (tranh khu rang cua)*/
    lv_style_init(&st_line);
    lv_style_set_bg_opa(&st_line, LV_OPA_COVER);
    lv_style_set_bg_color(&st_line, lv_color_white());
    lv_style_set_radius(&st_line, 0);
    lv_style_set_border_width(&st_line, 0);
}

/* ---------- Ham tao widget ---------- */
static lv_obj_t *label_create(lv_obj_t *parent, const char *txt,
                              lv_coord_t x, lv_coord_t y, lv_coord_t w,
                              lv_text_align_t align)
{
    lv_obj_t *l = lv_label_create(parent);
    lv_obj_add_style(l, &st_text, 0);
    lv_label_set_long_mode(l, LV_LABEL_LONG_CLIP);   /* khong tu xuong dong */
    lv_obj_set_width(l, w);                          /* chieu rong co dinh  */
    lv_obj_set_pos(l, x, y);
    lv_obj_set_style_text_align(l, align, 0);
    lv_label_set_text(l, txt);
    return l;
}

static lv_obj_t *hline_create(lv_obj_t *parent, lv_coord_t y)
{
    lv_obj_t *o = lv_obj_create(parent);
    lv_obj_remove_style_all(o);
    lv_obj_add_style(o, &st_line, 0);
    lv_obj_set_size(o, SCR_W, 1);
    lv_obj_set_pos(o, 0, y);
    return o;
}

/* ---------- Timer ---------- */
static void progress_timer_cb(lv_timer_t *t)
{
    (void)t;
    ui.progress += PROGRESS_STEP;
    if (ui.progress > 100) ui.progress = 0;

    lv_bar_set_value(ui.bar, ui.progress, LV_ANIM_OFF);
    lv_label_set_text_fmt(ui.lbl_pct, "%d%%", ui.progress);
}

static void counter_timer_cb(lv_timer_t *t)
{
    (void)t;
    ui.seconds++;
    lv_label_set_text_fmt(ui.lbl_cnt, "Up %02lu:%02lu",
                          (unsigned long)((ui.seconds / 60) % 100),
                          (unsigned long)(ui.seconds % 60));
}

/* ---------- Entry point ---------- */
void ui_init(void)
{
    styles_init();

    lv_obj_t *scr = lv_scr_act();
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    /* Header + duong ke */
    label_create(scr, "BBB-OLED", 0,  HDR_Y, 64, LV_TEXT_ALIGN_LEFT);
    label_create(scr, "LVGL 8.3", 64, HDR_Y, 64, LV_TEXT_ALIGN_RIGHT);
    hline_create(scr, SEP_Y);

    /* Chu o giua */
    label_create(scr, "Hello World", 0, HELLO_Y, SCR_W, LV_TEXT_ALIGN_CENTER);

    /* Progress bar */
    ui.bar = lv_bar_create(scr);
    lv_obj_remove_style_all(ui.bar);
    lv_obj_add_style(ui.bar, &st_bar_main, LV_PART_MAIN);
    lv_obj_add_style(ui.bar, &st_bar_ind,  LV_PART_INDICATOR);
    lv_obj_set_size(ui.bar, BAR_W, BAR_H);
    lv_obj_set_pos(ui.bar, BAR_X, BAR_Y);
    lv_bar_set_range(ui.bar, 0, 100);
    lv_bar_set_value(ui.bar, 0, LV_ANIM_OFF);

    /* Phan tram duoi bar */
    ui.lbl_pct = label_create(scr, "0%", 0, PCT_Y, SCR_W, LV_TEXT_ALIGN_CENTER);

    /* Bo dem thoi gian o day man hinh */
    ui.lbl_cnt = label_create(scr, "Up 00:00", 0, CNT_Y, SCR_W, LV_TEXT_ALIGN_CENTER);

    ui.progress = 0;
    ui.seconds  = 0;
    lv_timer_create(progress_timer_cb, PROGRESS_PERIOD_MS, NULL);
    lv_timer_create(counter_timer_cb, 1000, NULL);
}