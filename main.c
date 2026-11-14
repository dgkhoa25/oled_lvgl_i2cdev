/*
 * LVGL v8.3 tren SSD1306 128x64, giao tiep I2C truc tiep (khong dung /dev/fb0)
 *
 * LVGL ve vao buffer theo dinh dang page cua SSD1306 (moi byte = 8 pixel doc)
 * thong qua set_px_cb, roi flush_cb gui tung page qua I2C.
 */
#include "lvgl/lvgl.h"
#include "oled.h"
#include "ui.h"

#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

static volatile int running = 1;
static void on_sigint(int s) { (void)s; running = 0; }

/* Ghi 1 pixel vao buffer theo dinh dang page:
 * byte = x + (y / 8) * buf_w,  bit = y % 8   (bit 1 = diem sang) */
static void set_px_cb(lv_disp_drv_t *drv, uint8_t *buf, lv_coord_t buf_w,
                      lv_coord_t x, lv_coord_t y, lv_color_t color, lv_opa_t opa)
{
    (void)drv; (void)opa;
    uint8_t *p   = &buf[x + (y >> 3) * buf_w];
    uint8_t  bit = 1u << (y & 7);
    if (color.full) *p |=  bit;
    else            *p &= ~bit;
}

/* Vung ve luon la nguyen chieu ngang va tron theo page (boi so cua 8 hang) */
static void rounder_cb(lv_disp_drv_t *drv, lv_area_t *a)
{
    (void)drv;
    a->x1 = 0;
    a->x2 = OLED_W - 1;
    a->y1 = a->y1 & ~7;
    a->y2 = (a->y2 & ~7) + 7;
}

static void flush_cb(lv_disp_drv_t *drv, const lv_area_t *a, lv_color_t *c)
{
    const uint8_t *p = (const uint8_t *)c;
    int w = a->x2 - a->x1 + 1;

    for (int page = a->y1 >> 3; page <= (a->y2 >> 3); page++) {
        oled_write_page((uint8_t)page, (uint8_t)a->x1, p, w);
        p += w;
    }
    lv_disp_flush_ready(drv);
}

static uint32_t now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)(ts.tv_sec * 1000u + ts.tv_nsec / 1000000u);
}

int main(void)
{
    signal(SIGINT,  on_sigint);
    signal(SIGTERM, on_sigint);

    if (oled_init() < 0) return 1;

    lv_init();

    /* Chi dung 1024 byte dau (128 * 64 / 8); cap du de an toan */
    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t buf[OLED_W * OLED_H];
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, OLED_W * OLED_H);

    static lv_disp_drv_t drv;
    lv_disp_drv_init(&drv);
    drv.draw_buf     = &draw_buf;
    drv.flush_cb     = flush_cb;
    drv.rounder_cb   = rounder_cb;
    drv.set_px_cb    = set_px_cb;
    drv.hor_res      = OLED_W;
    drv.ver_res      = OLED_H;
    drv.antialiasing = 0;
    lv_disp_t *disp = lv_disp_drv_register(&drv);

    lv_theme_t *th = lv_theme_mono_init(disp, true, &lv_font_unscii_8);
    lv_disp_set_theme(disp, th);

    ui_init();

    uint32_t last = now_ms();
    while (running) {
        uint32_t t = now_ms();
        lv_tick_inc(t - last);
        last = t;

        lv_timer_handler();
        usleep(5000);
    }

    oled_close();
    return 0;
}